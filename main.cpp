#include "vm.hpp"

const int program[] = 
{
	SET, 0, 10,
	SET, 1, 20,
	ADD, 2, 1, 0,
	PRNT, 2,
	PSHR, 2,
	SET, 2, 0,
	PRNT, 2,
	NOP,
	NOP,
	NOP,
	HALT,
};

int main()
{
	VMContext *ctx = vm_create_context();
	vm_load_program(ctx, program, sizeof(program));

	while (ctx->running)
	{
		int instr = vm_fetch(ctx);
		ctx->registers[IP]++;
		vm_eval(ctx, (InstructionSet)instr);
	}

	vm_destroy_context(ctx);

    return 0;
}

