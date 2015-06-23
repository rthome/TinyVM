#include "vm.hpp"

#include <cstring>
#include <cstdlib>

#include <iostream>


VMContext* vm_create()
{
	VMContext *ctx = new VMContext;
    prepare_instruction_table(ctx->instr_table);
	vm_reset(ctx);
	return ctx;
}

void vm_destroy(VMContext *ctx)
{
	delete ctx;
}

void vm_reset(VMContext *ctx)
{
	ctx->running = false;
	memset(ctx->memory, 0, sizeof(ctx->memory));
	memset(ctx->registers, 0, sizeof(ctx->registers));
}

void vm_init_stack(VMContext *ctx, size_t stacksize)
{
	ctx->registers[SP] = 0;
	ctx->registers[SBP] = stacksize;
}

void vm_init_programbase(VMContext *ctx, vmword location)
{
	ctx->registers[IP] = location;
}

void vm_load_program(VMContext *ctx, InstructionData *data, size_t count)
{
	auto ctx_program_addr = ctx->memory + ctx->registers[IP];
	memcpy(ctx_program_addr, data, count * sizeof(InstructionData));
}

void vm_error(VMContext *ctx, const char *message)
{
	ctx->running = false;
	// TODO: Add some more information
	std::cout << "Error caught: " << message << std::endl;
	exit(-1);
}

Instruction vm_fetch_decode(VMContext *ctx)
{
	auto data_address = reinterpret_cast<InstructionData*>(ctx->memory + ctx->registers[IP]);
	auto instr = vmi_decode(data_address);
	ctx->registers[IP] += 4;
	return instr;
}

void vm_execute(VMContext *ctx, const Instruction *instr)
{
    auto instr_impl = ctx->instr_table[instr->opcode];
    instr_impl(ctx, instr);
	ctx->registers[IC]++;
}
