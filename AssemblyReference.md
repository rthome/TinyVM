TinyVM Reference
=========================

This document is the "official" reference manual for the TinyVM execution environment. it comes in three parts:

1. A general guide to the TinyVM machine.
2. A reference for the human-readable TinyVM assembly language.
3. A reference for the instructions that are available in TinyVM.

Introduction
------------

TinyVM is a virtual machine written out of curiosity about virtual machines and machine code. it doesn't do anything really new and borrows heavily from other CPU architectures (mostly x86).

The goal is to implement a simple virtual machine with CPU, memory, and some IO capabilities, and a clean and easy-to-use instruction set and assembly language.

TinyVM is implemented in C++ and should run on pretty much any operating system that has a compiler for C++11. There are no dependencies beyond the C and C++ standard libraries.

TinyVM Machine Guide
--------------------

This section introduces the TinyVm machine and the resources available to programs that run on it.

### Overview ###

The TinyVM machine implements a von Neumann-architecture, so data and code sit in the same memory space.

Memory consists of 2^14 (16384) 64-bit words, or 131072 bytes. Memory addressing is word by word, indexed from 0 to 16383.

There is a machine-supported stack, usually occupying the first 2048 words of memory. This can be controlled with the SBP (stack base pointer) register. The stack grows downwards in memory, towards address 0, to prevent stack overflows from running wild over data and code in memory.

The machine has a number of registers, of which most can be used for general calculations, while some have special semantics. These are covered in the next section, "Registers".

### Registers ###

The TinyVm virtual machine has a total of 21 registers, all of which are 64 bits wide.

General purpose registers r0 through r15 are available for programs to use as they see fit.

There are also 6 special-purpose registers, each of which serves a well-defined role within the machine:

- rIP, the instruction pointer. 

	Always contains the address of the next instruction to be executed. Will be incremented by the execution engine and modified by various instructions (jump, for example).
- rIC, the instruction counter.

	Holds the number of instructions executed over the entire runtime of the machine. Will be incremented by 1 after each instruction is executed.
- rSP, the stack pointer.

	Holds the current size of the stack. Value is relative to rSBP. (Note that the stack grows towards 0 in memory, so to obtain the absolute address of the top of the stack, subtract rSP from rSBP.)
- rSBP, the stack base pointer.

	Holds the absolute address of the base of the stack. Defaults to 2048, but can be set to any value to set a custom maximum stack size. (An existing stack must be copied to adjust to the new base address, obviously.)
- rRMD, the remainder register.

	Holds the remainder after a DIV instruction has been executed.

### Instruction Encoding ###

This section documents the encoding and format of TinyVM instructions. For information about the individual instructions, refer to the Instruction Reference section. For information about the human-readable assembly representation, refer to the Assembly Syntax and Features section.

#### General

TinyVM uses a three-address code for most instructions, where the first operand is the target location for the result of the instruction. There is no accumulator register as a common target location for instruction results. This reduces the shuffling-around of data a lot.

All instructions in TinyVm are fixed in length, with one control word and three operand words. This gives a total instruction length of 4 * 64 = 256 bit. Some instructions have less than 3 operands, but the simplicity of a fixed-length instruction format is deemed more important than the potential memory savings.

#### In-Memory Encoding

In memory, instructions are made up of 4 consecutive words in memory. The first word is the control word, while the other 3 words are operands to the instruction.

The control word contains the instruction opcode (32bit), instruction flags (8bit), and 3 addressing modes (1 for each operand, 8bit each).

The opcode takes up bits 64-33 of the control word. The instruction flags take up bits 32-25. The addressing modes take up bits 24-17 (for operand 1), 16-9 (for operand 2), and 8-1 (for operand 3), respectively.

A control word:

	64                               32       24       16       8        0
	+--------------------------------+--------+--------+--------+--------+
	|                                |        |        |        |        |
	|            Opcode              | Flags  |  AM 1  |  AM 2  |  AM 3  |
	|                                |        |        |        |        |
	+--------------------------------+--------+--------+--------+--------+

#### Addressing Modes

Operand values in encoded instructions will be interpreted differently in TinyVM, depending on the Addressing Mode for the operand. There are a total of 6 Addressing Modes available which will influence instruction execution.

There are 2 locations that operands can refer to, memory addresses and register indices, and a special indirect addressing flag. Operands can also be marked as literal values.

1. Literal values.

	Interpret the operand as a literal value instead of as an address.
2. Memory Address

	Interpret the operand as a memory address that contains the operand value.
3. Register Index

	Interpret the operand as a register index that contains the operand value.

For each instruction, there must be exactly one addressing mode bit set.

If indirect addressing is enabled, the value of the operand is interpreted as the address in memory that contains the actual operand value. This allows for a more compact expression of programs. Indirect addressing can be used together with all 3 Addressing Modes.

Addressing Modes are encoded as bitfields, with the following values:

1: Indirect addressing  
2: Literal value  
4: Memory addressing  
8: Register addressing  

Examples:

	2 = Literal
	3 = Literal | Indirect
	9 = Register | Indirect
	6 = Literal | Memory (Illegal!)

Assembly Syntax and Features
----------------------------

### Grammar

This section defines the syntax of the TinyVM  assembly language using a formal grammar in EBNF notation.

	Line            = [ (SpecifierLine | InstructionLine) ], [ Comment ] ;
	SpecifierLine   = Specifier, [ SpecifierOperand ] ;
	InstructionLine = [ Label ], [ Identifier, [ Operand ], [ Operand ], [Operand] ] ;

	Specifier        = ".", Identifier ;
	SpecifierOperand = Number ;
	Label            = Identifier, ":" ;
	Comment          = ";", { ? Any character ? }
	
	Operand         = DirectOperand | IndirectOperand ;
	IndirectOperand = "[", DirectOperand, "]" ;
	DirectOperand   = RegisterOperand | AddressOperand | LiteralOperand ;
	RegisterOperand = "r", Number | "rIP" | "rIC" | "rSP" | "rSBP" | "rRMD" ;
	AddressOperand  = Number ;
	LiteralOperand  = "#", Number ;
	
	Number     = DecNumber | HexNumber ;
	Identifier = Character, { Character } ;
	
	HexNumber  = "h", NNHexDigit, { HexDigit } | "h0" ;
	HexDigit   = "0" | NNHexDigit ;
	NNHexDigit = NNDigit | "A" | "B" | "C" | "D" | "E" | "F" | "a" | "b" | "d" | "d" | "e" | "f" ;
	
	DecNumber = NonNullDigit, { Digit } | "0" ;
	Digit     = "0" | NNDigit ;
	NNDigit   = "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
	
	Character = "A" | ... | "Z" | "a" | ... | "z" ;

Instruction Reference
---------------------

This section documents each instruction available inside TinyVM. Instructions are grouped into rough categories:

1. Machine Support - anything that doesn't really fit into any other category.
2. Arithmetic - anything that calculates with numbers.
3. Logic - anything that deals with comparisons or bit operations.
4. Flow control - jumps and subroutine support.

Note: Effective operand values are referred to as A, B, and C for operands 1, 2, and 3, respectively.

### Machine Support ###

#### NOP (No Operation)

This instruction does nothing.

Opcode: 0  
Applicable flags: None  
Operand count: 0  

#### HALT

Halts execution of the VM.

Opcode: 1  
Applicable flags: None  
Operand count: 0  

#### PUSH

Push the value of A onto the stack and increment the stack pointer.

Opcode: 2  
Applicable flags: None  
Operand count: 1  

1. The value to push onto the stack

#### POP

Pop the value on top of the stack into A and decrement the stack pointer.

Opcode: 3  
Applicable flags: None  
Operand count: 1  

1. Target location for the popped value. Must not be a literal.

#### MOV (Move)

Move B into A. (A = B)

Opcode: 15  
Applicable flags: None  
Operand count: 2  

1. Target location for the result of the operation. Must not be a literal.
2. The value to move.

### Arithmetic ###

#### ADD (Addition)

Add B and C and put the result into A. (A = B + C)

Opcode: 4  
Applicable flags: None  
Operand count: 3  

1. Target location for the result of the operation. Must not be a literal.
2. First summand.
3. Second summand.

#### SUB (Subtraction)

Subtract C from B and put the result into A. (A = B - C)

Opcode: 5  
Applicable flags: None  
Operand count: 3  

1. Target location for the result of the operation. Must not be a literal.
2. The minuend.
3. The subtrahend.

#### MUL (Multiplication)

Multipy A by B and put the result into A. (A = B * C)

Opcode: 6  
Applicable flags: None  
Operand count: 3  

1. Target location for the result of the operation. Must not be a literal.
2. The first factor.
3. The second factor.

#### DIV (Division)

Divide B by C and put the quotient into A. The remainder is put into the remainder register. (A = B / C, rRMD = B % C)

Opcode: 7  
Applicable flags: None  
Operand count: 3  

1. Target location for the result of the operation. Must not be a literal.
2. The numerator.
3. The denominator.

#### MOD (Modulus)

Take the modulus (remainder) of B and C and put the result into A. (A = B % C)

Opcode: 10  
Applicable flags: None  
Operand count: 3  

1. Target location for the result of the operation. Must not be a literal.
2. The numerator.
3. The denominator.

### Logic ###

#### SHL (Shift Left)

Shift B to the left by C bits and put the resulting value into A. (A = B << C)

Opcode: 8  
Applicable flags: None  
Operand count: 3  

1. Target location for the result of the operation. Must not be a literal.
2. The value to be shifted.
3. The number of bits to shift by.

#### SHR (Shift Right)

Shift B to the right by C bits and put the resulting value into A. (A = B >> C)

Opcode: 9  
Applicable flags: None  
Operand count: 3  

1. Target location for the result of the operation. Must not be a literal.
2. The value to be shifted.
3. The number of bits to shift by.

#### INC (Increment)

Increment A by 1. (A = A + 1)

Opcode: 11  
Applicable flags: None  
Operand count: 1  

1. Source and target for the operation. Must not be a literal.

#### DEC (Decrement)

Decrement A by 1. (A = A - 1)

Opcode: 12  
Applicable flags: None  
Operand count: 1  

1. Source and target for the operation. Must not be a literal.

#### NOT

Take the bitwise complement of B and put it into A. (A = ~B)

Opcode: 13  
Applicable flags: None  
Operand count: 2  

1. Target location for the result of the operation. Must not be a literal.
2. The value to negate.

#### CMP (Compare)

Compare B and C and put the result into A. A has one of the following values after this instruction was executed:

- 0, if B and C are equal.
- -1, if B is greater than C (as an unsigned integer, i.e. 18446744073709551616)
- 1, if B is less than C

Opcode: 14  
Applicable flags: None  
Operand count: 3  

1. Target location for the result of the operation. Must not be a literal.
2. The first value.
3. The second value.

### Flow Control ###

#### CALL

Jump to a subroutine at A and put the current instruction pointer onto the stack.

Use the RET instruction to return from a CALL.

Opcode: 16  
Applicable flags: None  
Operand count: 1

1. The address of the subroutine to execute

#### RET (Return)

Return from a subroutine, using the top of the stack as the new instruction pointer.

Use the CALL instruction to jump into a subroutine.

Opcode: 17  
Applicable flags: None  
Operand count: 0  

#### JMP (Jump)

Jump to A and continue execution there.

Opcode: 18  
Applicable flags: None  
Operand count: 1  

1. Jump target location.

#### JEQ (Jump-if-equal)

Jump to A and continue execution there **if** B and C are equal.

Opcode: 19  
Applicable flags: None  
Operand count: 3  

1. Jump target location.
2. First value to compare.
3. Second value to compare.

#### JNE (Jump-if-not-equal)

Jump to A and continue execution there **if** B and C are *not* equal.

Opcode: 20  
Applicable flags: None  
Operand count: 3  

1. Jump target location.
2. First value to compare.
3. Second value to compare.

#### JNZ (Jump-if-not-zero)

Jump to A and continue execution there **if** B is not zero.

Opcode: 21  
Applicable flags: None  
Operand count: 2  

1. Jump target location.
2. Value to compare to zero.

