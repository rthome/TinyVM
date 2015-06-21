#include "vm.hpp"

#include <cstring>
#include <cmath>

namespace
{
	////////
	// Stack helper functions
	////////

	inline vmword* stack_top(VMContext *ctx)
	{
		auto sp = ctx->registers[SP];
		if (sp == -1)
			sp++;
		auto stack_base = ctx->memory + ctx->registers[SBP];
		auto top = stack_base - sp;
		return top;
	}

	inline vmword* stack_inc(VMContext *ctx)
	{
		// TODO: Check stack boundaries
		ctx->registers[SP]++;
		return stack_top(ctx);
	}

	inline vmword* stack_dec(VMContext *ctx)
	{
		// TODO: Check stack boundaries
		ctx->registers[SP]--;
		return stack_top(ctx);
	}

	inline void stack_push(VMContext *ctx, vmword word)
	{
		*stack_inc(ctx) = word;
	}
}

VMContext* vm_create()
{
	VMContext *ctx = new VMContext;
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
	ctx->registers[SP] = -1;
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

Instruction vm_fetch_decode(VMContext *ctx)
{
	auto data_address = reinterpret_cast<InstructionData*>(ctx->memory + ctx->registers[IP]);
	auto instr = vmi_decode(data_address);
	ctx->registers[IP] += 4;
	return instr;
}

void vm_execute(VMContext *ctx, const Instruction *instr)
{
	switch (instr->opcode)
	{
	case OP_NOP:
		break;
	case OP_PUSH: {

		break;
	}
	}
	ctx->registers[IC]++;
}