#include "vm.hpp"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>

// // //
// PRIVATE HELPER FUNCTIONS
// // //

#define sp(ctx) ((ctx)->registers[SP])
#define sbp(ctx) ((ctx)->registers[SBP])
#define ip(ctx) ((ctx)->registers[IP])
#define ic(ctx) ((ctx)->registers[IC])

#define regv(ctx, r) ((ctx)->registers[(r)])
#define stack(ctx) ((ctx)->memory + sbp(ctx)) // base address of the stack for the given context
#define stackv(ctx, addr) *(stack(ctx) - (addr)) // contents of address on stack
#define stackp(ctx) stackv(ctx, sp(ctx)) // contents of address pointed to by stack pointer

namespace
{
	inline vmword pop(VMContext *c)
	{
		if (sp(c) < 0)
			vm_crash(c, "Stack underflow! Aborting..");

		vmword val = stackp(c); // get top of stack
		sp(c)--; // decrement stack pointer
		return val;
	}

	inline void push(VMContext *c, vmword val)
	{
		if (sp(c) > 0 && sp(c) > sbp(c))
			vm_crash(c, "Stack overflow! Aborting..");
		
		sp(c)++; // increment stack pointer
		stackp(c) = val; // set value
	}

	void print_stack(const VMContext *c)
	{
        std::cout << "Stack dump @ ic " << ic(c);
		for (size_t i = sbp(c); i >= 0; i--)
		{
			if (i % 5 == 0)
				printf("\n  %04d: ", sbp(c) - i);
			printf("%020d ", stackv(c, i));
		}
		printf("\n");
	}

	void print_registers(const VMContext *c)
	{
        std::cout << "Register state dump @ ic " << ic(c) << std::endl;
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
        std::cout << "  R10 = " << regv(c, R10) << std::endl;
        std::cout << "  R11 = " << regv(c, R11) << std::endl;
        std::cout << "  R12 = " << regv(c, R12) << std::endl;
        std::cout << "  R13 = " << regv(c, R13) << std::endl;
        std::cout << "  R14 = " << regv(c, R14) << std::endl;
        std::cout << "  R15 = " << regv(c, R15) << std::endl;
        std::cout << "  IC = " << regv(c, IC) << std::endl;
		std::cout << "  IP = " << regv(c, IP) << std::endl;
		std::cout << "  SP = " << regv(c, SP) << std::endl;
		std::cout << "  SBP = " << regv(c, SBP) << std::endl;
	}

	vmword fetch_operand(const VMContext *ctx, const DecodedInstruction *instr, size_t op)
	{
		auto opval = instr->operands[op];
		auto addrm = instr->addressing[op];

		vmword fetched_val;
		switch (addrm)
		{
		case ADDR_LITERAL:
			fetched_val = opval;
		case ADDR_REGISTER:
			fetched_val = ctx->registers[opval];
			break;
		case ADDR_MEMORY:
			fetched_val = ctx->memory[opval];
			break;
		}

		// Perform indirection if needed
		if (addrm & ADDR_INDIRECT)
		{
			if (addrm & ADDR_REGISTER || addrm & ADDR_MEMORY)
				fetched_val = ctx->memory[fetched_val];
		}

		return fetched_val;
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
	memset(ctx->memory, 0, sizeof(ctx->memory));
	memset(ctx->registers, 0, sizeof(ctx->registers));
	sp(ctx) = -1;
	sbp(ctx) = 127;

	ctx->running = true;
}

void vm_destroy_context(VMContext *ctx)
{
	delete ctx;
}

void vm_crash(VMContext *ctx, const char *msg)
{
	std::cout << msg << std::endl;
	quick_exit(EXIT_FAILURE);
}

void vm_set_program_base(VMContext *ctx, vmword value)
{
	ip(ctx) = value;
}

void vm_load_program(VMContext *ctx, const vmword *progbuf, size_t n)
{
    auto program_base_ptr = ctx->memory + ip(ctx);
    if (ip(ctx) + n >= VM_MEMORY_SIZE)
        vm_crash(ctx, "Memory upper limit exceeded while loading program! Aborting...");
    memcpy(program_base_ptr, progbuf, n);
}

DecodedInstruction vm_fetch_decode(VMContext *ctx)
{
    if (ip(ctx) >= VM_MEMORY_SIZE)
		vm_crash(ctx, "IP running past memory limit! Aborting..");

	auto addr = reinterpret_cast<VMInstruction *>(ctx->memory + ip(ctx));
	auto instr = vm_decode_instruction(addr);

	return instr;
}

void vm_eval(VMContext *ctx, DecodedInstruction *instr)
{
	switch (instr->opcode)
	{
	case PUSH: {
		push(ctx, fetch_operand(ctx, instr, 0));
		break;
	}
	// TODO: Implement instructions
	default:
		vm_crash(ctx, "Unknown opcode encountered");
		break;
	}

    ctx->registers[IC]++;
}
