#pragma once

#include "instruction.hpp"

// Make a nullary instruction
Instruction vmi_make_instr_0(Opcode opcode, OpcodeFlags flags = OF_NORMAL);
InstructionData vmi_encode_instr_0(Opcode opcode, OpcodeFlags flags = OF_NORMAL);

// Make a unary instruction
Instruction vmi_make_instr_1(Opcode opcode, OpcodeFlags flags,
    AddressingMode am0, vmword op0);
InstructionData vmi_encode_instr_1(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0);

// Make a binary instruction
Instruction vmi_make_instr_2(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0,
	AddressingMode am1, vmword op1);
InstructionData vmi_encode_instr_2(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0,
	AddressingMode am1, vmword op1);

// Make a tertiary instruction
Instruction vmi_make_instr_3(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0,
	AddressingMode am1, vmword op1,
	AddressingMode am2, vmword op2);
InstructionData vmi_encode_instr_3(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0,
	AddressingMode am1, vmword op1,
	AddressingMode am2, vmword op2);
