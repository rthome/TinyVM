#pragma once

#include "vmtypes.hpp"
#include "instruction.hpp"
#include "instruction_implementation.hpp"

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
    instr_func instr_table[INSTRUCTION_COUNT];

    vmword registers[VM_REGISTER_COUNT];
    vmword memory[VM_MEMORY_SIZE];
};

// Create a new vm context and reset it
VMContext* vm_create();

// Destroy the given vm context
void vm_destroy(VMContext *ctx);

// Reset the given vm context, basically zeroing everything
void vm_reset(VMContext *ctx);

// Initialize the stack by setting sp and sbp and zeroing the stack
void vm_init_stack(VMContext *ctx, vmword stacksize);

// Set ip to the given value
void vm_init_programbase(VMContext *ctx, vmword location);

// Load the given number of instructions into memory, starting at ip
void vm_load_program(VMContext *ctx, InstructionData *data, size_t count);

// Report an error and stop execution
void vm_error(VMContext *ctx, const char *message);

// Fetch and decode the next instruction
Instruction vm_fetch_decode(VMContext *ctx);

// Execute the given instruction
void vm_execute(VMContext *ctx, const Instruction *instr);
