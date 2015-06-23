#include "vm.hpp"
#include "instruction_support.hpp"

int main()
{
	// Euclid's algorithm
	// Inputs in R0 and R1
	// Result in R0
	InstructionData program[] = 
	{
		// jmp main
		vmi_encode_instr_1(OP_JMP, OF_NORMAL, AM_LITERAL, 2204),

		// 2180:
		// R2 = R1
		vmi_encode_instr_2(OP_MOV, OF_NORMAL, AM_REGISTER, R2, AM_REGISTER, R1),
		// R1 = R0 mod R1
		vmi_encode_instr_3(OP_DIV, OF_NORMAL, AM_REGISTER, R1, AM_REGISTER, R0, AM_REGISTER, R1),
		vmi_encode_instr_2(OP_MOV, OF_NORMAL, AM_REGISTER, R1, AM_REGISTER, RMD),
		// R0 = R2
		vmi_encode_instr_2(OP_MOV, OF_NORMAL, AM_REGISTER, R0, AM_REGISTER, R2),
		// continue while R1 != 0
		vmi_encode_instr_3(OP_JNZ, OF_NORMAL, AM_LITERAL, 2180, AM_REGISTER, R1, AM_LITERAL, 0),
		// return
		vmi_encode_instr_0(OP_RET),

		// 2204 main:
		// Set inputs gcd(1071, 462) = 21
		vmi_encode_instr_2(OP_MOV, OF_NORMAL, AM_REGISTER, R0, AM_LITERAL, 1071),
		vmi_encode_instr_2(OP_MOV, OF_NORMAL, AM_REGISTER, R1, AM_LITERAL, 462),
		vmi_encode_instr_1(OP_CALL, OF_NORMAL, AM_LITERAL, 2180),
		vmi_encode_instr_0(OP_HALT),
	};

	auto ctx = vm_create();
	vm_init_stack(ctx, 2048);
	vm_init_programbase(ctx, 2176);

	vm_load_program(ctx, program, 11);

	ctx->running = true;
	while (ctx->running)
	{
		auto instr = vm_fetch_decode(ctx);
		vm_execute(ctx, &instr);
	}
	vm_destroy(ctx);

    return 0;
}

