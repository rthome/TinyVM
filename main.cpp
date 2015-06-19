#include "vm.hpp"
#include "instruction.hpp"

#define INSTR vm_make_instruction

const VMInstruction program[] =
{
    INSTR(PUSH, OF_NORMAL, ADDR_LITERAL, 1),
    INSTR(PUSH, OF_NORMAL, ADDR_LITERAL, 2),
    INSTR(PUSH, OF_NORMAL, ADDR_LITERAL, 3),
    INSTR(SET, OF_NORMAL, ADDR_REGISTER, 0, ADDR_LITERAL, 10),
    INSTR(SET, OF_NORMAL, ADDR_REGISTER, 1, ADDR_LITERAL, 20),
    INSTR(ADD, OF_NORMAL, ADDR_REGISTER, 2, ADDR_REGISTER, 1, ADDR_REGISTER, 0),
    INSTR(PRNT, OF_NORMAL, ADDR_REGISTER, 2),
    INSTR(PUSH, OF_NORMAL, ADDR_REGISTER, 2),
    INSTR(SET, OF_NORMAL, ADDR_REGISTER, 2, ADDR_LITERAL, 0),
    INSTR(PRNT, OF_NORMAL, ADDR_REGISTER, 2),
    INSTR(PUSH, OF_NORMAL, ADDR_REGISTER, 3),
    INSTR(PUSH, OF_NORMAL, ADDR_REGISTER, 4),
    INSTR(NOP),
    INSTR(DEC, OF_NORMAL, ADDR_REGISTER, 0),
    INSTR(NOP),
    INSTR(NOP),
    INSTR(HALT),
};

int main()
{
	VMContext *ctx = vm_create_context();
	vm_set_program_base(ctx, 128);
    vm_load_program(ctx, (vmword*)program, sizeof(program));

	while (ctx->running)
	{
		auto instr = vm_fetch(ctx);
		vm_eval(ctx, (Opcode)instr);
	}

	vm_destroy_context(ctx);

    return 0;
}

