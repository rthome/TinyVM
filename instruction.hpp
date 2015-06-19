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

// Flags affecting execution of an instruction
// 8 bits reserved
enum OpcodeFlags
{

};

// Flags for the interpretation of instruction operands
// 8 bits reserved
enum AddressingMode
{
    ADDRESSING_INDIRECT = 1, // Operand's target holds address of actual operand value in memory
    ADDRESSING_REGISTER = 2, // Operand is register number. Mutually exclusive with ADDRESSING_MEMORY and ADDRESSING_LITERAL
    ADDRESSING_MEMORY   = 4, // Operand is memory address. Mutually exclusive with ADDRESSING_REGISTER and ADDRESSING_LITERAL
    ADDRESSING_LITERAL  = 8, // Operand is a literal value. Mutually exclusive with every other addressing mode
};

// Structure for encoded instructions
//
// 4 vmwords: control, operands 0 through 2
// +----------------+----------------+----------------+----------------+
// |    control     |    operand0    |    operand1    |    operand2    | = 256bit
// +----------------+----------------+----------------+----------------+
// 0                                                                  255
// All words are unsigned 64-bit integers, but the operands will be interpreted
// as 2s complement signed integers during execution
//
// The control word is divided in 5 fields: opcode, flags, and 3 addressing mode flags
// +--------------------------------+--------+--------+--------+--------+
// |            Opcode              | Flags  |  AM0   |  AM1   |  AM2   | = 64bit
// +--------------------------------+--------+--------+--------+--------+
// 0                                                                   63
//              32bit                  8bit     8bit     8bit     8bit
union VMInstruction
{
    vmword words[4];
    vmword control, op0, op1, op2;
};

// Structure for decoded instructions
struct DecodedInstruction
{
	Opcode opcode;
	OpcodeFlags flags;
    AddressingMode addressing[3];
    vmint operands[3];
};

VMInstruction vm_encode_instruction(const DecodedInstruction *instr);
DecodedInstruction vm_decode_instruction(const VMInstruction *instr);
