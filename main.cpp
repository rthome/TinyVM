#include "vm.hpp"

const int program[] = 
{
	PUSH, 5,
	PUSH, 6,
	DBG,
	SET, 0, 22,
	PUSH, 10,
	POP, 1,
	POP, 2,
	ADD, 0, 1, 2,
	POP, 4,
	DBG,
	NOP,
	HALT
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

