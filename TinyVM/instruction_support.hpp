#pragma once

#include "instruction.hpp"

// Forward-declare VMContext
struct VMContext;

// Load a memory image (VM_MEMORY_SIZE vmwords) from a memory location or a file
// Exactly VM_MEMORY_SIZE * sizeof(vmword) bytes will be read
// vmi_load_memory_image_file returns true if successful in loading the file, false otherwise
void vmi_load_memory_image(const void *data, VMContext *ctx);
bool vmi_load_memory_image_file(const char *filename, VMContext *ctx);

// Save content of ctx's memory to file
// Return true in case of success, false otherwise
bool vmi_save_memory_image_file(const char *filename, const VMContext *ctx);

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
