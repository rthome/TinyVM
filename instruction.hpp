#pragma once

#include "vmtypes.hpp"

// Instruction Opcodes
// 32 bit reserved
// Other restrictions:
//  - Target addresses for jumps must be instruction-aligned, i.e. 4-word-aligned
enum Opcode
{
	// Legend:
	//   a, b, c - operands (interpret as "value of")

	OP_NOP,  // NOP          Do nothing (still increment ic)
	OP_HALT, // HALT         Stop execution     
	OP_PUSH, // PUSH a       Push a onto the stack
	OP_POP,  // POP a        Pop the top of the stack into a
	OP_ADD,  // ADD a b c    a = b + c
	OP_SUB,  // SUB a b c    a = b - c
	OP_MUL,  // MUL a b c    a = b * c
    OP_DIV,  // DIV a b c    a = b / c, remainder is placed in RMD register
	OP_SHL,  // SHL a b c    a = b << c
	OP_SHR,  // SHR a b c    a = b >> c
    OP_MOD,  // MOD a b c    a = b mod c
    OP_INC,  // INC a        a = a + 1
	OP_DEC,  // DEC a        a = a - 1
	OP_NOT,  // NOT a        Set a to its bitwise complement
	OP_CMP,  // CMP a b c    Compare b and c, put result in a (-1 if c is less than b, 0 if equal, 1 if c is greater than b)
	OP_MOV,  // MOV a b      a = b
	OP_CALL, // CALL a       Jump to a, but save IP
	OP_RET,  // RET          Return from CALL, restoring IP
    OP_JMP,  // JMP a        Jump to a
    OP_JEQ,  // JEQ a b c    Jump to a if b and c are equal
    OP_JNE,  // JNE a b c    Jump to a if b and c are not equal
	OP_JNZ,  // JNZ a b      Jump to a if b is not zero

	INSTRUCTION_COUNT,
};

enum OpcodeFlags
{
	OF_NORMAL = 0, // No special behavior
};

enum AddressingMode
{
	AM_INDIRECT = 1, // Operand target contains location of operand value in memory
	AM_LITERAL  = 2, // Operand value is a literal
	AM_MEMORY   = 4, // Operand points to a memory word
	AM_REGISTER = 8, // Operand is a register index
};

struct InstructionData
{
	vmword words[4];
};

struct Instruction
{
	Opcode opcode;
	OpcodeFlags flags;
	AddressingMode addressing[3];
	vmword operands[3];
};

// Encode the instruction into 4 vmwords
InstructionData vmi_encode(const Instruction *intr);

// Decode the given instruction data
Instruction vmi_decode(const InstructionData *data);

// Read instruction data from memory
InstructionData vmi_read(const vmword *memory, size_t offset = 0);
