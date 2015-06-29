TinyVM Reference
=========================

This document is the "official" reference manual for the TinyVM execution environment. it comes in three parts:

1. A general guide to the TinyVM machine.
2. A reference for the human-readable TinyVM assembly language.
3. A reference for the instrcuctions that are available in TinyVM.

TinyVM Machine Guide
--------------------

TBD

### Overview ###

Assembly Syntax and Features
----------------------------

TBD

Instruction Reference
---------------------

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

#### MOV (Move)

Move B into A. (A = B)

Opcode: 15  
Applicable flags: None  
Operand count: 2  

1. Target location for the result of the operation. Must not be a literal.
2. The value to move.

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

