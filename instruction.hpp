#pragma once

#include "vmtypes.hpp"

// Instruction Opcodes
// 32 bit reserved
enum Opcode
{
	// Legend:
	//   a,b,c - literal value
	//   r,s,t - register index
	//   x,y,z - memory address

	NOP,  // NOP        Do nothing (still increment ic)
	PUSH, // PUSH a     Push a onto the stack
	POP,  // POP r      Pop the top of the stack into a register
	ADD,  // ADD r s t  Add contents of s and t and put the result into r
	INC,  // INC r      Increment contents of r
	DEC,  // DEC r      Decrement contents of r
	SET,  // SET r a    Set r to a
	MOV,  // MOV r s    Copy value of s into r
	HALT, // HALT       Stop execution
	DBG,  // DBG        Print state information
	PRNT, // PRNT r     Print content of r, formatted as a number      

	INSTRUCTION_COUNT,
};

enum OpcodeFlags
{
	OF_NORMAL, // No special behavior
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
