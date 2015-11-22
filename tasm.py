#!/usr/bin/env python3

#
# The TinyVM Assembler
# Usage: tasm -o image.bin sourcecode.tasm
#

import sys, argparse, re

# # #
# Tokenization
# # #

class TokRule(object):
    DISCARD_TXT = 1
    DISCARD_TOK = 2

    def __init__(self, name, rule, flags=()):
        self.name = name
        self.rule = re.compile(rule)
        self.flags = flags
    
    @property
    def tuple(self):
        return self.name, self.rule, self.flags

TOK_RULES = [
    TokRule("newline", r"\r\n|\n", flags=(TokRule.DISCARD_TXT,)),
    TokRule("whitespace", r"\s+", flags=(TokRule.DISCARD_TOK,)),
    TokRule("comment", r";[^\n]*", flags=(TokRule.DISCARD_TOK,)),
    TokRule("label", r"\w+:"),
    TokRule("label_ref", r":\w+"),
    TokRule("specifier", r"\.\w+"),
    TokRule("literal", r"#\d+"),
    TokRule("brk_open", r"\["),
    TokRule("brk_close", r"\]"),
    TokRule("number", r"\d+"), # TODO: Support more number types
    TokRule("register", r"r([0-9]+|IP|IC|SP|SBP|RMD)"),
    TokRule("identifier", r"\w+"),
]

def tokenize(string):
    "Break up an input string into a bunch of (token, text) tuples"
    def process_flags(tok, text, flags):
        if TokRule.DISCARD_TXT in flags:
            text = ""
        if TokRule.DISCARD_TOK in flags:
            tok = None
            text = ""
        return tok, text
    pos = 0
    last_end = 0
    while True:
        if pos >= len(string):
            break
        for tok, rule, flags in (tok_rule.tuple for tok_rule in TOK_RULES):
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

def extract_texts(tokens):
    return [txt for (tok, txt) in tokens]

def parse_operands(op_tokens):
    "Parse a list of tokens into a list of instruction operands"
    return extract_texts(op_tokens)

def parse_line(line):
    """Parse a line of tokens into a label, a specifier, or a instruction
    Returns tuples of (type, name, [operands])"""
    (tok, text), *rest = line
    if tok == "label":
        return "label", text[:-1]
    elif tok == "specifier":
        return "specifier", text[1:], extract_texts(rest)
    elif tok == "identifier":
        return "instruction", text, parse_operands(rest)

def parse(tokens):
    "Parse a bunch of tokens into instructions, labels, and specifiers"
    line = []
    for tok, text in tokens:
        if tok != "newline":
            line.append((tok, text))
            continue
        if len(line) > 0:
            yield parse_line(line)
            line.clear()

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
    with open(args.file) as f:
        tokens = list(tokenize(f.read()))
        for tok, text in tokens:
            print(tok + ": " + text)
        for x in parse(tokens):
            print(x)

if __name__ == "__main__":
    main()
