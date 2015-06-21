#include "vm.hpp"
#include "instruction_support.hpp"

int main()
{
	InstructionData program[] = 
	{
		vmi_encode_instr_0(OP_NOP),
		vmi_encode_instr_0(OP_HALT),
	};

	auto ctx = vm_create();
	vm_init_stack(ctx, 2048);
	vm_init_programbase(ctx, 2176);

	vm_load_program(ctx, program, 2);

	ctx->running = true;
	while (ctx->running)
	{
		auto instr = vm_fetch_decode(ctx);
		vm_execute(ctx, &instr);
	}
	vm_destroy(ctx);

    return 0;
}

