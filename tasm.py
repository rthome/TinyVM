#!/usr/bin/env python

import argparse, re

# #
# Tokenization
# #

class Token(object):
    text_processors = {
        "number": lambda t: int(t),
        "label_ref": lambda t: t[1:],
        "label": lambda t: t[:-1],
        "specifier": lambda t: t[1:],
        "literal": lambda t: int(t[1:]),
    }
    
    @staticmethod
    def process_value(token_type, token_value):
        """Process the value of a token, using registered processors.
            If no processor exists for the given token type, the original value is returned.
        """
        try:
            proc = Token.text_processors[token_type]
            return proc(token_value)
        except KeyError:
            return token_value

    def __init__(self, token_type, token_value, span = None):
        """Create a token.
            token_type: type of the new token
            token_value: value (captured text) of the new token
            span: tuple of (start_position, end_position) of the token value. Can be omitted if no such information is available.
        """
        self.type = token_type
        self.original_value = token_value
        self.span = span if span is not None else (0, 0, 0)
        self.value = Token.process_value(token_type, token_value)
        
    def __repr__(self):
        return "({}, {})".format(self.type, str(self.value))

TOK_DISCARD = 1
TOK_NO_TEXT = 2
class TokenStream(object):
    "Helper class that splits a stream of tokens into lines. Iterate over TokenStream to access lines."
    type_flags = {
        "comment": (TOK_DISCARD,),
        "whitespace": (TOK_DISCARD,),
        "newline": (TOK_NO_TEXT,),
    }
    
    def __init__(self, token_source):
        """Create a token stream
            token_source: Iterable source for tokens
        """
        self.source = token_source
        self.lines = self.split_lines(self.source)
    
    def process_token(self, token):
        try:
            flags = TokenStream.type_flags[token.type]
            if TOK_DISCARD in flags:
                return None
            if TOK_NO_TEXT in flags:
                token.value = ""
            if token.type == "unknown":
                raise Exception("Found unknown token at {}: {}".format(token.span, token))
            return token
        except KeyError:
            return token
    
    def split_lines(self, tokens):
        lines = []
        line_tokens = []
        for token in tokens:
            processed_token = self.process_token(token)
            if processed_token is None:
                continue
            elif processed_token.type == "newline":
                if len(line_tokens) > 0:
                    lines.append(line_tokens)
                    line_tokens = []
            else:
                line_tokens.append(processed_token)
        else:
            if len(line_tokens) > 0:
                lines.append(line_tokens)
        return lines
    
    def __iter__(self):
        return iter(self.lines)
            
class TokenRule(object):
    def __init__(self, expression, token_type):
        self.original_expression = expression
        self.regex = re.compile(expression)
        self.type = token_type
    
    def match(self, string, pos):
        return self.regex.match(string, pos)

TOKEN_RULES = [TokenRule(e, t) for t, e in [
    ("newline", r"\r?\n"),
    ("whitespace", r"\s+"),
    ("comment", r";[^\n]*"),
    ("label", r"\w(\w|\d)*:"),
    ("label_ref", r":\w(\w|\d)*"),
    ("specifier", r"\.(\w|\d)+"),
    ("literal", r"#\d+"),
    ("number", r"\d+"),
    ("register", r"r([0-9]+|IP|IC|SP|SBP|RMD)"),
    ("identifier", r"\w(\w|\d)*"),
    ("brk_open", r"\["),
    ("brk_close", r"\]"),
]]

def tokenize(string, rules):
    pos = 0
    last_end = 0
    while True:
        if pos >= len(string):
            break
        for rule in TOKEN_RULES:
            match = rule.match(string, pos)
            if match is not None:
                start, end = match.span()
                if start > last_end:
                    yield Token("unknown", string[last_end:start], match.span())
                yield Token(rule.type, match.group(), match.span())
                last_end = pos = match.end()
                break
        else:
            pos += 1
    if last_end < len(string):
        yield Token("unknown", string[last_end:], (last_end, len(string)))

# #
# Parsing
# #

def parse_single_operand(tokens):
    "Parse a single operand from a bunch of tokens"
    token, mode = (tokens[0], "direct") if len(tokens) == 1 else (tokens[1], "indirect")
    return token.type, token.value, mode

def separate_operands(tokens):
    "Separate a bunch of tokens into one or more groups, each corresponding to one operand"
    operand_tokens = []
    indirect = False
    for tok in tokens:
        operand_tokens.append(tok)
        if tok.type == "brk_open":
            indirect = True
        if not indirect or (indirect and tok.type == "brk_close"):
            yield operand_tokens
            operand_tokens = []
            indirect = False

def parse_operands(tokens):
    "Parse a list of tokens into a list of instruction operands"
    groups = separate_operands(tokens)
    return [parse_single_operand(tg) for tg in groups]

def parse_line(tokens):
    "Parse a list of tokens that form a line"
    token, *rest = tokens
    if token.type == "label":
        return token
    elif token.type == "specifier":
        return token, rest
    elif token.type == "identifier":
        return token, parse_operands(rest) if len(rest) > 0 else []
    else:
        raise Exception("Unexpected token {} at start of line at {}".format(token, token.span))

# #
# Assembly
# #

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

# #
# Glue
# #

def parse_arguments():
    parser = argparse.ArgumentParser(description="The TinyVM Assembler")
    parser.add_argument("file", metavar="FILE", help="source file to assemble")
    parser.add_argument("-o", metavar="OUTFILE", default="tvmimage.bin", help="name of the output memory image (default: tvmimage.bin)")
    return parser.parse_args()
    
def main():
    args = parse_arguments()
    with open(args.file, "r") as source_file:
        source_string = source_file.read()
        token_iterator = tokenize(source_string, TOKEN_RULES)
        token_stream = TokenStream(token_iterator)
        token_lines = token_stream.lines
        
        print("token lines")
        for t in token_lines:
            print(t)
        
        print("parsed lines")
        parsed_lines = [parse_line(token_line) for token_line in token_stream]
        for t in parsed_lines:
            print(t)

if __name__ == "__main__":
    main()
