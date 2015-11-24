#!/usr/bin/env python3

#
# The TinyVM Assembler
# Usage: tasm -o image.bin sourcecode.tasm
#

import sys, argparse, re, array

# # #
# Tokenization
# # #

TOK_DISCARD = 1
TOK_NO_TEXT = 2

TOK_RULES = [(n, re.compile(r), f) for n, r, f in [
    ("newline", r"\r\n|\n", (TOK_NO_TEXT,)),
    ("whitespace", r"\s+", (TOK_DISCARD,)),
    ("comment", r";[^\n]*", (TOK_DISCARD,)),
    ("label", r"\w+:", ()),
    ("label_ref", r":\w+", ()),
    ("specifier", r"\.\w+", ()),
    ("literal", r"#\d+", ()),
    ("brk_open", r"\[", ()),
    ("brk_close", r"\]", ()),
    ("number", r"\d+", ()), # TODO: Support more number types
    ("register", r"r([0-9]+|IP|IC|SP|SBP|RMD)", ()),
    ("identifier", r"\w+", ()),
]]

def tokenize(string):
    "Break up an input string into a bunch of (token, text) tuples"
    def process_flags(tok, text, flags):
        if TOK_NO_TEXT in flags:
            text = ""
        if TOK_DISCARD in flags:
            tok = None
            text = ""
        return tok, text
    pos = 0
    last_end = 0
    while True:
        if pos >= len(string):
            break
        for tok, rule, flags in (tok_rule for tok_rule in TOK_RULES):
            match = rule.match(string, pos)
            if match is not None:
                start, end = match.span()
                if start > last_end:
                    yield "unknown", string[last_end:start]
                ptok, ptext = process_flags(tok, match.group(), flags)
                if ptok is not None:
                    yield ptok, ptext
                last_end = pos = match.end()
                break
        else:
            pos += 1
    if last_end < len(string):
        yield "unknown", string[last_end:]

# # #
# Parsing
# # #

def parse_single_operand(tokens):
    "Parse a single operand from a bunch of tokens"
    def fixup_tok(tok, text):
        "Removes the leading # from literals and such things"
        # TODO: do this somewhere more appropriate
        if tok == "literal" or tok == "label_ref":
            text = text[1:]
        if tok == "number":
            tok = "memory"
        return tok, text
    token, mode = (tokens[0], "direct") if len(tokens) == 1 else (tokens[1], "indirect")
    tok, text = fixup_tok(*token)
    return tok, text, mode

def separate_operands(tokens):
    "Separate a bunch of tokens into one or more groups, each corresponding to one operand"
    operand_tokens = []
    indirect = False
    for tok, txt in tokens:
        operand_tokens.append((tok, txt))
        if tok == "brk_open":
            indirect = True
        if not indirect or (indirect and tok == "brk_close"):
            yield operand_tokens
            operand_tokens = []
            indirect = False

def parse_operands(tokens):
    "Parse a list of tokens into a list of instruction operands"
    groups = separate_operands(tokens)
    return [parse_single_operand(token_group) for token_group in groups]

def parse_line(line):
    """Parse a line of tokens into a label, a specifier, or a instruction
    Returns tuples of (type, name, [operands])"""
    (tok, text), *rest = line
    if tok == "label":
        return "label", text[:-1]
    elif tok == "specifier":
        return "specifier", text[1:], list(rest)
    elif tok == "identifier":
        return "instruction", text, parse_operands(rest) if len(rest) > 0 else []

def parse(tokens):
    "Parse a bunch of tokens into instructions, labels, and specifiers"
    line = []
    for tok, text in tokens:
        if tok != "newline":
            line.append((tok, text))
            continue
        if len(line) > 0:
            yield parse_line(line)
            line = []
    else:
        if len(line) > 0:
            # Make sure to parse the last line even if there is no final newline
            yield parse_line(line)

# # #
# Assembling
# # #

# VM memory size in 64 bit words
VM_IMAGE_SIZE = 0x10000

# Dict mapping instruction mnemonics to tuples with (opcode, operand count)
INSTRUCTION_INFO = {
    "nop": (0, 0),
    "halt": (1, 0),
    "push": (2, 1),
    "pop": (3, 1),
    "add": (4, 3),
    "sub": (5, 3),
    "mul": (6, 3),
    "div": (7, 3),
    "shl": (8, 3),
    "shr": (9, 3),
    "mod": (10, 3),
    "inc": (11, 1),
    "dec": (12, 1),
    "not": (13, 1),
    "cmp": (14, 3),
    "mov": (15, 2),
    "call": (16, 1),
    "ret": (17, 0),
    "jmp": (18, 1),
    "jeq": (19, 3),
    "jne": (20, 3),
    "jnz": (21, 2),
    "rdrand": (22, 3),
}

# Dict mapping register names to register numbers
REGISTER_INFO = {
    "r0": 0,
    "r1": 1,
    "r2": 2,
    "r3": 3,
    "r4": 4,
    "r5": 5,
    "r6": 6,
    "r7": 7,
    "r8": 8,
    "r9": 9,
    "r10": 10,
    "r11": 11,
    "r12": 12,
    "r13": 13,
    "r14": 14,
    "r15": 15,
    "rIP": 16,
    "rIC": 17,
    "rSP": 18,
    "rSBP": 19,
    "rRMD": 20,
}

# Dict mapping specifier names to required arguments
SPECIFIER_INFO = {
    "base": ["number"], # Sets the address in memory where the next instruction will be mapped at. Argument must be multiple of 4.
}

# Opcode/Instruction flags
OF_NORMAL = 0

# Addressing mode flags
AM_INDIRECT = 1
AM_LITERAL = 2
AM_MEMORY = 4
AM_REGISTER = 8

def encode_instruction(opcode, flags, operands):
    "Encode an instruction into 4 vm words"
    def encode_controlword():
        e_opcode = opcode << 32
        e_flags = flags << 24
        e_am0 = operands[0][0] << 16
        e_am1 = operands[1][0] << 8
        e_am2 = operands[2][0]
        return e_opcode | e_flags | e_am0 | e_am1 | e_am2
    words = [encode_controlword()]
    words.extend([o[1] for o in operands])
    return words

def convert_instruction(instr_tuple, label_dict):
    """"Takes an instruction tuple from the parser and convert it into something that can be assembled
    Returns a tuple of (opcode, flags, operands), where operands is a list of 3 (AM, value) tuples"""
    def conv_op(op_tuple):
        optype, val, mode = op_tuple
        am = AM_INDIRECT if mode == "indirect" else 0
        if optype == "register":
            val = REGISTER_INFO[val]
            am |= AM_REGISTER
        elif optype == "literal":
            val = int(val)
            am |= AM_LITERAL
        elif optype == "memory":
            val = int(val)
            am |= AM_MEMORY
        elif optype == "label_ref":
            val = label_dict[val]
            am |= AM_LITERAL
        else:
            raise Exception("Invalid addressing mode identifier: {}".format(optype))
        return am, val
    t, instr, op_tuples = instr_tuple
    opcode, op_count = INSTRUCTION_INFO[instr]
    if (len(op_tuples) != op_count):
        raise Exception("Instruction {} expected {} operands, got  {}".format(instr, op_count, len(op_tuples)))
    operands = [conv_op(t) for t in op_tuples]
    while len(operands) < 3:
        # Extend operands list to 3 operands with zeroes
        operands.append((0, 0))
    return opcode, OF_NORMAL, operands

def make_label_index(lines):
    addr = 0
    label_dict = {}
    for line in lines:
        if line[0] == "specifier":
            t, spec, args = line
            if spec == "base":
                addr = int(args[0][1])
        elif line[0] == "instruction":
            addr += 4
        elif line[0] == "label":
            t, label = line
            label_dict[label] = addr
    return label_dict

def encode_image(value_list):
    "Encode an array of integers into a vm image"
    return array.array("Q", value_list)

def assemble(line_tuples, outfile):
    addr = 0
    val_list = [0] * VM_IMAGE_SIZE # List of words in the vm image
    label_dict = make_label_index(line_tuples)
    for line in line_tuples:
        if line[0] == "instruction":
            converted_instr = convert_instruction(line, label_dict)
            encoded_instr = encode_instruction(*converted_instr)
            for i in range(4):
                val_list[addr + i] = encoded_instr[i]
            addr += 4
        elif line[0] == "specifier":
            t, spec, args = line
            if spec == "base":
                addr = int(args[0][1])
    image = encode_image(val_list)
    image.tofile(outfile)

# # #
# Glue
# # #

def parse_arguments():
    parser = argparse.ArgumentParser(description="The TinyVM Assembler")
    parser.add_argument("file", metavar="FILE", help="source file to assemble")
    parser.add_argument("-o", metavar="OUTFILE", default="tvmimage.bin", help="name of the output memory image (default: tvmimage.bin)")
    return parser.parse_args()

def main():
    args = parse_arguments()
    with open(args.file, "r") as f:
        tokens = list(tokenize(f.read()))
        lines = list(parse(tokens))
        for x in lines:
            print(x)
        label_dict = make_label_index(lines)
        for x in lines:
            if x[0] == "instruction":
                converted = convert_instruction(x, label_dict)
                encoded = encode_instruction(*converted)
                print("converted: ", converted)
                print("encoded: ", encoded)
        with open(args.o, "wb") as outfile:
            assemble(lines, outfile)

if __name__ == "__main__":
    main()
