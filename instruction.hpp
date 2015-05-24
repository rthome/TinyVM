#pragma once

#include "vmtypes.hpp"

enum Opcode
{
	// Legend:
	//   a,b,c - literal value
	//   r,s,t - register index
	//   x,y,z - memory address

	NOP,  // NOP        Do nothing (still increment ic)
	PUSH, // PUSH a     Push a onto the stack
	PSHR, // PSHR r     Push content of r onto the stack
	POP,  // POP r      Pop the top of the stack into a register
	POPM, // POPM x     Pop the top of the stack into memory
	ADD,  // ADD r s t  Add contents of s and t and put the result into r
	INC,  // INC r      Increment contents of r
	DEC,  // DEC r      Decrement contents of r
	SET,  // SET r a    Set r to a
	MOV,  // MOV r s    Copy value of s into r
	MOVM, // MOVM r x   Copy from register to memory (r to x)
	MOVR, // MOVR x r   Copy from memory to register (x to r)
	HALT, // HALT       Stop execution
	DBG,  // DBG        Print state information
	PRNT, // PRNT r     Print content of r, formatted as a number      

	INSTRUCTION_COUNT,
};

enum OpcodeFlags
{

};

#define INSTRUCTION // TODO: create encoding macro

struct VMInstruction
{
	Opcode opcode;
	OpcodeFlags flags;
	vmword operands[3];
};

vmword encode_instruction();
VMInstruction decode_instruction();
