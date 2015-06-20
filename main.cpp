#include "vm.hpp"

int main()
{
	auto ctx = vm_create();
	vm_init_stack(ctx, 2048);
	vm_init_programbase(ctx, 2176);

	// Load program...

	ctx->running = true;
	while (ctx->running)
	{
		auto instr = vm_fetch_decode(ctx);
		vm_execute(ctx, &instr);
	}
	vm_destroy(ctx);

    return 0;
}

