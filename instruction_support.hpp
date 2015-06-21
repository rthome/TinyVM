#pragma once

#include "instruction.hpp"

// Assemble a number of instructions into a buffer of vmwords. buffer needs to be at least 4*count vmwords in size.
inline void vmi_assemble(vmword *buffer, const InstructionData *instructions, size_t count);

// Make a nullary instruction
inline Instruction vmi_make_instr_0(Opcode opcode, OpcodeFlags flags);
inline InstructionData vmi_encode_instr_0(Opcode opcode, OpcodeFlags flags);

// Make a unary instruction
inline Instruction vmi_make_instr_1(Opcode opcode, OpcodeFlags flags,
    AddressingMode am0, vmword op0);
inline InstructionData vmi_encode_instr_1(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0);

// Make a binary instruction
inline Instruction vmi_make_instr_2(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0,
	AddressingMode am1, vmword op1);
inline InstructionData vmi_encode_instr_2(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0,
	AddressingMode am1, vmword op1);

// Make a tertiary instruction
inline Instruction vmi_make_instr_3(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0,
	AddressingMode am1, vmword op1,
	AddressingMode am2, vmword op2);
inline InstructionData vmi_encode_instr_3(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0,
	AddressingMode am1, vmword op1,
	AddressingMode am2, vmword op2);
