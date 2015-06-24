#include "vm.hpp"
#include "instruction_support.hpp"

void run_vm_context(VMContext *ctx)
{
    ctx->running = true;
    while (ctx->running)
    {
        auto instr = vm_fetch_decode(ctx);
        vm_execute(ctx, &instr);
    }
}

void load_example(VMContext *ctx)
{
    // Euclid's algorithm
    // Inputs in R0 and R1
    // Result in R0
    InstructionData program[] =
    {
        // jmp main
        vmi_encode_instr_1(OP_JMP, OF_NORMAL, AM_LITERAL, 1056),

        // 1036:
        // R2 = R1
        vmi_encode_instr_2(OP_MOV, OF_NORMAL, AM_REGISTER, R2, AM_REGISTER, R1),
        // R1 = R0 mod R1
        vmi_encode_instr_3(OP_MOD, OF_NORMAL, AM_REGISTER, R1, AM_REGISTER, R0, AM_REGISTER, R1),
        // R0 = R2
        vmi_encode_instr_2(OP_MOV, OF_NORMAL, AM_REGISTER, R0, AM_REGISTER, R2),
        // continue while R1 != 0
        vmi_encode_instr_3(OP_JNZ, OF_NORMAL, AM_LITERAL, 1036, AM_REGISTER, R1, AM_LITERAL, 0),
        // return
        vmi_encode_instr_0(OP_RET),

        // 1056 main:
        // Set inputs gcd(1071, 462) = 21
        vmi_encode_instr_2(OP_MOV, OF_NORMAL, AM_REGISTER, R0, AM_LITERAL, 1071),
        vmi_encode_instr_2(OP_MOV, OF_NORMAL, AM_REGISTER, R1, AM_LITERAL, 462),
        vmi_encode_instr_1(OP_CALL, OF_NORMAL, AM_LITERAL, 1036),
        vmi_encode_instr_0(OP_HALT),
    };
    vm_load_program(ctx, program, 10);
}

int main()
{
	auto ctx = vm_create();
    vm_init_stack(ctx, 1024);
    vm_init_programbase(ctx, 1032);

    load_example(ctx);

    run_vm_context(ctx);
    vm_destroy(ctx);

    return 0;
}

