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

	OP_NOP,  // NOP        Do nothing (still increment ic)
	OP_HALT, // HALT       Stop execution     
	OP_PUSH, // PUSH a     Push a onto the stack
	OP_POP,  // POP a      Pop the top of the stack into a
	OP_ADD,  // ADD a b c  Add b and c and put the result into a
	OP_INC,  // INC a      Increment a
	OP_DEC,  // DEC a      Decrement a
	OP_MOV,  // MOV a b    Copy b into a
    OP_JMP,  // JMP a      Jump to a
    OP_JEQ,  // JEQ a b c  Jump to a if b and c are equal
    OP_JNE,  // JNE a b c  Jump to a if b and c are not equal

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
