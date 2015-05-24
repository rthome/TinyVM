#include "vm.hpp"

const vmword program[] = 
{
	PUSH, 1,
	PUSH, 2,
	PUSH, 3,
	SET, 0, 10,
	SET, 1, 20,
	ADD, 2, 1, 0,
	PRNT, 2,
	PSHR, 2,
	SET, 2, 0,
	PRNT, 2,
	POP, 3,
	POP, 4,
	NOP,
	DEC, 0,
	NOP,
	NOP,
	HALT,
};

int main()
{
	VMContext *ctx = vm_create_context();
	vm_set_program_base(ctx, 128);
	vm_load_program(ctx, program, sizeof(program));

	while (ctx->running)
	{
		auto instr = vm_fetch(ctx);
		vm_eval(ctx, (Opcode)instr);
	}

	vm_destroy_context(ctx);

    return 0;
}

