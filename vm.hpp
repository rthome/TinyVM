#pragma once

enum InstructionSet
{
	// Legend:
	//   a,b,c - literal value
	//   r,s,t - register index

	NOP,  // NOP        Do nothing (still increment ic)
    PUSH, // PUSH a     Push a onto the stack
	PSHR, // PSHR r     Push content of r onto the stack
    POP,  // POP r      Pop the top of the stack into r
    ADD,  // ADD r s t  Add contents of s and t and put the result into r
    SET,  // SET r a    Set r to a
	MOV,  // MOV r s    Copy value of s into r
    HALT, // HALT       Stop execution
	DBG,  // DBG        Print state information
	PRNT, // PRNT r     Print content of r, formatted as a number      

	INSTRUCTION_COUNT,
};

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

	REGISTER_COUNT,
};

struct VMContext
{
	static const size_t MAX_PROGRAM_SIZE = 256;
	static const size_t MAX_STACK_SIZE = 128;

	bool running = true;
	unsigned long long ic = 0; // instruction count

	int program[MAX_PROGRAM_SIZE]; // Program memory
	int stack[MAX_STACK_SIZE]; // Stack
	int registers[REGISTER_COUNT]; // Register file
};

// Create a new VM context
VMContext *vm_create_context();

// Reset a context to its initial state
void vm_reset_context(VMContext *ctx);

// Destroy a context
void vm_destroy_context(VMContext *ctx);

// Load a program into the given context
// n = number of bytes to load
void vm_load_program(VMContext *ctx, const int *progbuf, size_t n);

// Fetch the next instruction
int vm_fetch(VMContext *ctx);

// Evaluate an instruction given a VM context
void vm_eval(VMContext *ctx, InstructionSet instr);
