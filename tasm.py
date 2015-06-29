#!/usr/bin/env python3

# TinyVM assembler
# Usage: tasm FILE
#   FILE: Text file containing injstructions to be assembled
#   Output to stdout

# TinyVM assembly short reference:
#   General form: OP A B C, where OP is an opcode mnemonic and A, B, and C are operands
#   Instructions have 0, 1, 2, or 3 operands
#
#   Addressing modes:
#     One of:
#     - Register addressing:  rX, where X is one of 0-15, SP, SBP, IP, IC, RMD
#     - Memory addressing: X, where X is a memory address (see below)
#     - Literal: #X, where X is any 64-bit integer
#
#   Indirect addressing:
#     Register and memory addressing modes can use indirect addressing.
#     In this mode, the value of the operand is interpreted as 
#     the address in memory where the actual operand value is located.
#     Syntax: [X], where X is an operand in register of memory addressing mode
#
#   Memory address format
#     X (decimal) or hX (hexadecimal), where X can be any memory address (0-16384)
#
#   Labels
#     Any line can start with a label of the form
#       label: OP A B C
#       or, without an instruction on the same line,
#       label:
#     The label can be substituted for any memory address (direct or indirect addressing),
#     and will reference the address of the next instruction after the label
#
#   base specifier
#     A specifier of the form
#       base X
#     where X is a memory address, can be used to set the absolute address in memory
#     where the following instructions will be assembled in memory.
#     If no base is given, instructions are assembled starting from word 3000
#

import array, sys

VM_MEMORY_WORDS = 16384

# Table of instructions to opcode and operand count
INSTRUCTION_TABLE = {
    "nop": (0, 0),
    "halt": (1, 0),
    "push": (2, 1),
    "pop": (3, 1),
    "add": (4, 3),
    "sub": (5, 3),
}

class Assembler(object):
    def __init__(self):
        self.memory = array.array('Q', [0]*VM_MEMORY_WORDS)
        self.current_base = 0

    def assemble(file):
        return self.memory

    def parse_line(self, line):
        pass

if __name__ == "__main__":
    if len(sys.argv) < 3:
        raise Exception("No input and output file given")
    inputfile, outputfile = sys.argv[1], sys.argv[2]

    
    assembler = Assembler()
    with open(inputfile) as file:
        assembler.assemble(file)

    with open(outputfile, "w") as file:
        assembler.tofile(file)