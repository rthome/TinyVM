#!/usr/bin/env python3

#
# The TinyVM Assembler
# Usage: tasm -o image.bin sourcecode.tasm
#

import sys, argparse, re

class Token(object):
    def __init__(self, name, rule, flags=()):
        self.name = name
        self.rule = re.compile(rule)
        self.flags = flags
    
    @property
    def tuple(self):
        return self.name, self.rule, self.flags

TOK_DISCARD_TXT = 1
TOK_DISCARD_TOK = 2
TOK_RULES = [
    Token("newline", r"\r\n|\n", flags=(TOK_DISCARD_TXT,)),
    Token("whitespace", r"\s+", flags=(TOK_DISCARD_TOK,)),
    Token("comment", r";[^\n]*", flags=(TOK_DISCARD_TOK,)),
    Token("label", r"\w+:"),
    Token("label_ref", r":\w+"),
    Token("specifier", r"\.\w+"),
    Token("literal", r"#\d+"),
    Token("brk_open", r"\["),
    Token("brk_close", r"\]"),
    Token("number", r"\d+"), # TODO: Support more number types
    Token("register", r"r([0-9]+|IP|IC|SP|SBP|RMD)"),
    Token("identifier", r"\w+"),
]

def tokenize(string):
    def process(tok, text, flags):
        if TOK_DISCARD_TXT in flags:
            text = ""
        if TOK_DISCARD_TOK in flags:
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
                ptok, ptext = process(tok, match.group(), flags)
                if ptok is not None:
                    yield ptok, ptext
                last_end = pos = match.end()
                break
        else:
            pos += 1
    if last_end < len(string):
        yield "unknown", string[last_end:]

def parse_arguments():
    parser = argparse.ArgumentParser(description="The TinyVM Assembler")
    parser.add_argument("file", metavar="FILE", help="source file to assemble")
    parser.add_argument("-o", metavar="OUTFILE", default="tvmimage.bin", help="name of the output memory image (default: tvmimage.bin)")
    return parser.parse_args()

def main():
    args = parse_arguments()
    with open(args.file) as f:
        for tok, text in tokenize(f.read()):
            print(tok + ": " + text)

if __name__ == "__main__":
    main()
