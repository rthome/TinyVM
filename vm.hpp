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
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,

	IP, // Instruction pointer
    IC, // Instruction counter
	SP, // Stack pointer
	SBP, // Stack base pointer

    VM_REGISTER_COUNT,
};

const size_t VM_MEMORY_SIZE = 16384;

struct VMContext
{
	bool running = true;

    vmword registers[VM_REGISTER_COUNT];
    vmword memory[VM_MEMORY_SIZE];
};

// Create a new VM context
VMContext *vm_create_context();

// Reset a context to its initial state
void vm_reset_context(VMContext *ctx);

// Destroy a context
void vm_destroy_context(VMContext *ctx);

// Crash the VM instantly
void vm_crash(VMContext *ctx, const char *msg);

// Set the instruction pointer to value
void vm_set_program_base(VMContext *ctx, vmword value);

// Load a program into the given context at ip
// n = number of bytes to load
void vm_load_program(VMContext *ctx, const vmword *progbuf, size_t n);

// Fetch the next instruction and decode it
DecodedInstruction vm_fetch_decode(VMContext *ctx);

// Evaluate an instruction given a VM context
void vm_eval(VMContext *ctx, DecodedInstruction *instr);
