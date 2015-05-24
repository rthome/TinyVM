#include "vm.hpp"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>

// // //
// PRIVATE HELPER FUNCTIONS
// // //

#define sp(ctx) (ctx->registers[SP])
#define ip(ctx) (ctx->registers[IP])
#define regv(ctx, r) (ctx->registers[(r)])

namespace
{
	inline int pop(VMContext *c)
	{
		return c->stack[--sp(c)];
	}

	inline void push(VMContext *c, int val)
	{
		if (sp(c) > 0 && sp(c) >= VMContext::MAX_STACK_SIZE)
		{
			std::cout << "Stack overflow! Aborting.." << std::endl;
			quick_exit(EXIT_FAILURE);
		}
		c->stack[++sp(c)] = val;
	}

	inline int consume_word(VMContext *c)
	{
		return c->program[ip(c)++];
	}

	void print_stack(const VMContext *c)
	{
		std::cout << "Stack dump @ ic " << c->ic;
		for (size_t i = 0; i < VMContext::MAX_STACK_SIZE; i++)
		{
			if (i % 10 == 0)
				printf("\n  %03d: ", i);
			printf("%010d ", c->stack[i]);
		}
		printf("\n");
	}

	void print_registers(const VMContext *c)
	{
		std::cout << "Register state dump @ ic " << c->ic << std::endl;
		std::cout << "  R0 = " << regv(c, R0) << std::endl;
		std::cout << "  R1 = " << regv(c, R1) << std::endl;
		std::cout << "  R2 = " << regv(c, R2) << std::endl;
		std::cout << "  R3 = " << regv(c, R3) << std::endl;
		std::cout << "  R4 = " << regv(c, R4) << std::endl;
		std::cout << "  R5 = " << regv(c, R5) << std::endl;
		std::cout << "  R6 = " << regv(c, R6) << std::endl;
		std::cout << "  R7 = " << regv(c, R7) << std::endl;
		std::cout << "  R8 = " << regv(c, R8) << std::endl;
		std::cout << "  R9 = " << regv(c, R9) << std::endl;
		std::cout << "  IP = " << regv(c, IP) << std::endl;
		std::cout << "  SP = " << regv(c, SP) << std::endl;
	}
}

// // //
// VM IMPLEMENTATION
// // //

VMContext *vm_create_context()
{
	auto ctx = new VMContext;
	vm_reset_context(ctx);
	return ctx;
}

void vm_reset_context(VMContext *ctx)
{
	memset(ctx->program, 0, sizeof(ctx->program));
	memset(ctx->stack, 0, sizeof(ctx->stack));
	memset(ctx->registers, 0, sizeof(ctx->registers));
	sp(ctx) = -1;
	ctx->running = true;
}

void vm_destroy_context(VMContext *ctx)
{
	delete ctx;
}

void vm_load_program(VMContext *ctx, const int *progbuf, size_t n)
{
	if (n > VMContext::MAX_PROGRAM_SIZE)
		quick_exit(EXIT_FAILURE);
	memcpy(ctx->program, progbuf, n);
}

int vm_fetch(VMContext *ctx)
{
	if (ip(ctx) >= VMContext::MAX_PROGRAM_SIZE)
	{
		std::cout << "IP overflow! Aborting.." << std::endl;
		quick_exit(EXIT_FAILURE);
	}
	return ctx->program[ip(ctx)];
}

void vm_eval(VMContext *ctx, InstructionSet instr)
{
	switch (instr)
	{
	case PUSH: {
		push(ctx, consume_word(ctx));
		break;
	}
	case PSHR: {
		int reg = consume_word(ctx);
		push(ctx, ctx->registers[reg]);
		break;
	}
	case POP: {
		int reg = consume_word(ctx);
		int val = pop(ctx);
		ctx->registers[reg] = val;
		break;
	}
	case ADD: {
		int ra = consume_word(ctx);
		int rb = consume_word(ctx);
		int rc = consume_word(ctx);
		int val = regv(ctx, rb) + regv(ctx, rc);
		ctx->registers[ra] = val;
		break;
	}
	case SET: {
		int reg = consume_word(ctx);
		int val = consume_word(ctx);
		ctx->registers[reg] = val;
		break;
	}
	case MOV: {
		int ra = consume_word(ctx);
		int rb = consume_word(ctx);
		ctx->registers[ra] = ctx->registers[rb];
		break;
	}
	case HALT:
		ctx->running = false;
		break;
	case NOP:
		// Do nothing
		break;
	case DBG:
		print_registers(ctx);
		print_stack(ctx);
		break;
	default:
		break;
	}

	ctx->ic++;
}
