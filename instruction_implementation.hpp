#pragma once

// Forward declarations
struct VMContext;
struct Instruction;

// Function prototype definition for instruction implementations
typedef void (*instr_func)(VMContext*, const Instruction*);

// Fill buffer with implementations for VM instructions. Buffer must be at least of size Opcode::INSTRUCTION_COUNT.
void prepare_instruction_table(instr_func *buffer);
