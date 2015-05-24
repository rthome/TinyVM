#pragma once

#include "vmtypes.hpp"
#include "instruction.hpp"

enum Registers
{
	R0, // General-purpose registers
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	R8,
	R9,

	IP, // Instruction pointer
	SP, // Stack pointer
	SBP, // Stack base pointer

	REGISTER_COUNT,
};

struct VMContext
{
	static const size_t MEMORY_SIZE = 4096;

	bool running = true;
	size_t ic = 0; // Instruction counter

	vmword registers[REGISTER_COUNT];
	vmword memory[MEMORY_SIZE];
};

// Create a new VM context
VMContext *vm_create_context();

// Reset a context to its initial state
void vm_reset_context(VMContext *ctx);

// Destroy a context
void vm_destroy_context(VMContext *ctx);

// Set the instruction pointer to value
void vm_set_program_base(VMContext *ctx, vmword value);

// Load a program into the given context at ip
// n = number of bytes to load
void vm_load_program(VMContext *ctx, const vmword *progbuf, size_t n);

// Fetch the next instruction
vmword vm_fetch(VMContext *ctx);

// Evaluate an instruction given a VM context
void vm_eval(VMContext *ctx, Opcode instr);
