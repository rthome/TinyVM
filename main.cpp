#include "vm.hpp"

const VMInstruction program[] =
{
    vm_make_instruction(PUSH, OF_NORMAL, ADDR_LITERAL, 1),
    vm_make_instruction(PUSH, OF_NORMAL, ADDR_LITERAL, 2),
    vm_make_instruction(PUSH, OF_NORMAL, ADDR_LITERAL, 3),
    vm_make_instruction(SET, OF_NORMAL, ADDR_REGISTER, 0, ADDR_LITERAL, 10),
    vm_make_instruction(SET, OF_NORMAL, ADDR_REGISTER, 1, ADDR_LITERAL, 20),
    vm_make_instruction(ADD, OF_NORMAL, ADDR_REGISTER, 2, ADDR_REGISTER, 1, ADDR_REGISTER, 0),
    vm_make_instruction(PRNT, OF_NORMAL, ADDR_REGISTER, 2),
    vm_make_instruction(PUSH, OF_NORMAL, ADDR_REGISTER, 2),
    vm_make_instruction(SET, OF_NORMAL, ADDR_REGISTER, 2, ADDR_LITERAL, 0),
    vm_make_instruction(PRNT, OF_NORMAL, ADDR_REGISTER, 2),
    vm_make_instruction(PUSH, OF_NORMAL, ADDR_REGISTER, 3),
    vm_make_instruction(PUSH, OF_NORMAL, ADDR_REGISTER, 4),
    vm_make_instruction(NOP),
    vm_make_instruction(DEC, OF_NORMAL, ADDR_REGISTER, 0),
    vm_make_instruction(NOP),
    vm_make_instruction(NOP),
    vm_make_instruction(HALT),
};

int main()
{
	VMContext *ctx = vm_create_context();
	vm_set_program_base(ctx, 128);
    vm_load_program(ctx, (vmword*)program, sizeof(program));

	while (ctx->running)
	{
		auto instr = vm_fetch_decode(ctx);
		auto validation = vm_validate_instruction(&instr);
		if (!validation.ok)
			vm_crash(ctx, validation.msg);
		vm_eval(ctx, &instr);
	}

	vm_destroy_context(ctx);

    return 0;
}

