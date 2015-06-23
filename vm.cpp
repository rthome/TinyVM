#include "vm.hpp"

#include <cstring>
#include <cmath>
#include <cstdlib>

#include <iostream>

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
		if (ctx->registers[SP] >= ctx->registers[SBP])
			vm_error(ctx, "Stack overflow");
		ctx->registers[SP]++;
		return stack_top(ctx);
	}

	inline vmword* stack_dec(VMContext *ctx)
	{
		if (ctx->registers[SP] <= 0)
			vm_error(ctx, "Stack underflow");
		ctx->registers[SP]--;
		return stack_top(ctx);
	}

	inline void stack_push(VMContext *ctx, vmword word)
	{
		*stack_inc(ctx) = word;
	}

	inline vmword stack_pop(VMContext *ctx)
	{
		auto word = *stack_top(ctx);
		stack_dec(ctx);
		return word;
	}

	////////
	// Operand helper functions
	////////

	// Assign value to location operand at Index position points to. Does not support literal operands.
	template<size_t Index>
	void operand_assign_at(VMContext *ctx, const Instruction *instr, vmword value)
	{
		static_assert(Index < 3, "Operand index must be less than 3.");

		auto operand = instr->operands[Index];
		auto mode = instr->addressing[Index];

		if (mode & AM_LITERAL)
			vm_error(ctx, "Trying to assign to a literal operand.");

		vmword *target;
		if (mode & AM_REGISTER)
			target = (ctx->registers + operand);
		else if (mode & AM_MEMORY)
			target = (ctx->memory + operand);

		if (mode & AM_INDIRECT)
			target = (ctx->memory + *target);

		*target = value;
	}

	// Fetch operand value at Index position, following indirections
	template<size_t Index>
	vmword operand_fetch(const VMContext *ctx, const Instruction *instr)
	{
		static_assert(Index < 3, "Operand index must be less than 3.");

		auto operand = instr->operands[Index];
		auto mode = instr->addressing[Index];

		vmword value;
		if (mode & AM_LITERAL)
			value = operand;
		else if (mode & AM_MEMORY)
			value = ctx->memory[operand];
		else if (mode & AM_REGISTER)
			value = ctx->registers[operand];

		if (mode & AM_INDIRECT)
			value = ctx->memory[value];

		return value;
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
	switch (instr->opcode)
	{
	case OP_NOP: {
		break;
	}
	case OP_HALT: {
		ctx->running = false;
		break;
	}
	case OP_DBG: {
		break;
	}
	case OP_PRNT: {
		break;
	}
	case OP_PUSH: {
		auto op = operand_fetch<0>(ctx, instr);
		stack_push(ctx, op);
		break;
	}
	case OP_POP: {
		auto val = stack_pop(ctx);
		operand_assign_at<0>(ctx, instr, val);
		break;
	}
	case OP_ADD: {
		// TODO: operate on signed value here?
		auto a = operand_fetch<1>(ctx, instr);
		auto b = operand_fetch<2>(ctx, instr);
		auto val = a + b;
		operand_assign_at<0>(ctx, instr, val);
		break;
	}
	case OP_INC: {
		// TODO: operate on signed value here?
		auto a = operand_fetch<0>(ctx, instr);
		auto val = a + 1;
		operand_assign_at<0>(ctx, instr, val);
		break;
	}
	case OP_DEC: {
		// TODO: operate on signed value here?
		auto a = operand_fetch<0>(ctx, instr);
		auto val = a - 1;
		operand_assign_at<0>(ctx, instr, val);
		break;
	}
	case OP_MOV: {
		auto a = operand_fetch<1>(ctx, instr);
		operand_assign_at<0>(ctx, instr, a);
		break;
	}
	case OP_JMP: {
		auto a = operand_fetch<0>(ctx, instr);
		ctx->registers[IP] = a;
		break;
	}
    case OP_JEQ: {
        auto a = operand_fetch<0>(ctx, instr);
        auto b = operand_fetch<1>(ctx, instr);
        auto c = operand_fetch<2>(ctx, instr);
        if (b == c)
            operand_assign_at<0>(ctx, instr, a);
        break;
    }
    case OP_JNE: {
        auto a = operand_fetch<0>(ctx, instr);
        auto b = operand_fetch<1>(ctx, instr);
        auto c = operand_fetch<2>(ctx, instr);
        if (b != c)
            operand_assign_at<0>(ctx, instr, a);
        break;
    }

	default:
		vm_error(ctx, "Unknown opcode");
	}
	ctx->registers[IC]++;
}
