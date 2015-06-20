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

// Flags affecting execution of an instruction
// 8 bits reserved
enum OpcodeFlags
{
    OF_NORMAL, // Specifies normal execution
};

// Flags for the interpretation of instruction operands
// 8 bits reserved
enum AddressingMode
{
    ADDR_INDIRECT = 1, // Operand's target holds address of actual operand value in memory
    ADDR_REGISTER = 2, // Operand is register number. Mutually exclusive with ADDR_MEMORY and ADDR_LITERAL
    ADDR_MEMORY   = 4, // Operand is memory address. Mutually exclusive with ADDR_REGISTER and ADDR_LITERAL
    ADDR_LITERAL  = 8, // Operand is a literal value. Mutually exclusive with every other addressing mode
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
struct VMInstruction
{
	vmword words[4];
};

// Structure for decoded instructions
struct DecodedInstruction
{
	Opcode opcode;
	OpcodeFlags flags;
    AddressingMode addressing[3];
    vmint operands[3];
};

// Result for instruction validation API
struct VMValidationResult
{
	bool ok;
	const char *msg;
};

// Encode an instruction
VMInstruction vm_encode_instruction(const DecodedInstruction *instr);

// Decode an instruction
DecodedInstruction vm_decode_instruction(const VMInstruction *instr);

// Helper function for "inline" creation of encoded instruction
VMInstruction vm_make_instruction(Opcode op, OpcodeFlags flags = OF_NORMAL,
                                  AddressingMode am0 = (AddressingMode)0, vmint op0 = 0,
                                  AddressingMode am1 = (AddressingMode)0, vmint op1 = 0,
                                  AddressingMode am2 = (AddressingMode)0, vmint op2 = 0);

// Validate an instruction
VMValidationResult vm_validate_instruction(const DecodedInstruction *instr);