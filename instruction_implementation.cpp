#include "instruction_implementation.hpp"

#include "instruction.hpp"
#include "vm.hpp"

#define IMPL_NAME(name) op_##name##_impl
#define INSTRUCTION_IMPL(name) void IMPL_NAME(name)(VMContext *ctx, const Instruction *instr)

////////
// Anonymous namespace for helper functions
////////
namespace
{
    ////////
    // Stack helper functions
    ////////

    inline vmword* stack_top(VMContext *ctx)
    {
        auto sp = ctx->registers[SP];
        if (sp == -1)
            sp++;
        auto stack_base = ctx->memory + ctx->registers[SBP];
        auto top = stack_base - sp;
        return top;
    }

    inline vmword* stack_inc(VMContext *ctx)
    {
        if (ctx->registers[SP] >= ctx->registers[SBP])
            vm_error(ctx, "Stack overflow");
        ctx->registers[SP]++;
        return stack_top(ctx);
    }

    inline vmword* stack_dec(VMContext *ctx)
    {
        if (ctx->registers[SP] <= 0)
            vm_error(ctx, "Stack underflow");
        ctx->registers[SP]--;
        return stack_top(ctx);
    }

    inline void stack_push(VMContext *ctx, vmword word)
    {
        *stack_inc(ctx) = word;
    }

    inline vmword stack_pop(VMContext *ctx)
    {
        auto word = *stack_top(ctx);
        stack_dec(ctx);
        return word;
    }

    ////////
    // Operand helper functions
    ////////

    // Assign value to location operand at Index position points to. Does not support literal operands.
    template<size_t Index>
    void operand_assign_at(VMContext *ctx, const Instruction *instr, vmword value)
    {
        static_assert(Index < 3, "Operand index must be less than 3.");

        auto operand = instr->operands[Index];
        auto mode = instr->addressing[Index];

        if (mode & AM_LITERAL)
            vm_error(ctx, "Trying to assign to a literal operand.");

        vmword *target;
        if (mode & AM_REGISTER)
            target = (ctx->registers + operand);
        else if (mode & AM_MEMORY)
            target = (ctx->memory + operand);

        if (mode & AM_INDIRECT)
            target = (ctx->memory + *target);

        *target = value;
    }

    // Fetch operand value at Index position, following indirections
    template<size_t Index>
    vmword operand_fetch(const VMContext *ctx, const Instruction *instr)
    {
        static_assert(Index < 3, "Operand index must be less than 3.");

        auto operand = instr->operands[Index];
        auto mode = instr->addressing[Index];

        vmword value;
        if (mode & AM_LITERAL)
            value = operand;
        else if (mode & AM_MEMORY)
            value = ctx->memory[operand];
        else if (mode & AM_REGISTER)
            value = ctx->registers[operand];

        if (mode & AM_INDIRECT)
            value = ctx->memory[value];

        return value;
    }
}

////////
// Anonymous namespace for instruction implementations
////////
namespace
{
    INSTRUCTION_IMPL(nop)
    {

    }

    INSTRUCTION_IMPL(dbg)
    {

    }

    INSTRUCTION_IMPL(halt)
    {
        ctx->running = false;
    }

    INSTRUCTION_IMPL(prnt)
    {

    }

    INSTRUCTION_IMPL(push)
    {
        auto op = operand_fetch<0>(ctx, instr);
        stack_push(ctx, op);
    }

    INSTRUCTION_IMPL(pop)
    {
        auto val = stack_pop(ctx);
        operand_assign_at<0>(ctx, instr, val);
    }

    INSTRUCTION_IMPL(add)
    {
        // TODO: operate on signed value here?
        auto a = operand_fetch<1>(ctx, instr);
        auto b = operand_fetch<2>(ctx, instr);
        auto val = a + b;
        operand_assign_at<0>(ctx, instr, val);
    }

    INSTRUCTION_IMPL(inc)
    {
        // TODO: operate on signed value here?
        auto a = operand_fetch<0>(ctx, instr);
        auto val = a + 1;
        operand_assign_at<0>(ctx, instr, val);
    }

    INSTRUCTION_IMPL(dec)
    {
        // TODO: operate on signed value here?
        auto a = operand_fetch<0>(ctx, instr);
        auto val = a - 1;
        operand_assign_at<0>(ctx, instr, val);
    }

    INSTRUCTION_IMPL(mov)
    {
        auto a = operand_fetch<1>(ctx, instr);
        operand_assign_at<0>(ctx, instr, a);
    }

    INSTRUCTION_IMPL(jmp)
    {
        auto a = operand_fetch<0>(ctx, instr);
        ctx->registers[IP] = a;
    }

    INSTRUCTION_IMPL(jeq)
    {
        auto a = operand_fetch<0>(ctx, instr);
        auto b = operand_fetch<1>(ctx, instr);
        auto c = operand_fetch<2>(ctx, instr);
        if (b == c)
            operand_assign_at<0>(ctx, instr, a);
    }

    INSTRUCTION_IMPL(jne)
    {
        auto a = operand_fetch<0>(ctx, instr);
        auto b = operand_fetch<1>(ctx, instr);
        auto c = operand_fetch<2>(ctx, instr);
        if (b != c)
            operand_assign_at<0>(ctx, instr, a);
    }
}

void prepare_instruction_table(instr_func *buffer)
{
    buffer[OP_NOP] = &IMPL_NAME(nop);
    buffer[OP_DBG] = &IMPL_NAME(dbg);
    buffer[OP_HALT] = &IMPL_NAME(halt);
    buffer[OP_PRNT] = &IMPL_NAME(prnt);
    buffer[OP_PUSH] = &IMPL_NAME(push);
    buffer[OP_POP] = &IMPL_NAME(pop);
    buffer[OP_ADD] = &IMPL_NAME(add);
    buffer[OP_INC] = &IMPL_NAME(inc);
    buffer[OP_DEC] = &IMPL_NAME(dec);
    buffer[OP_MOV] = &IMPL_NAME(mov);
    buffer[OP_JMP] = &IMPL_NAME(jmp);
    buffer[OP_JEQ] = &IMPL_NAME(jeq);
    buffer[OP_JNE] = &IMPL_NAME(jne);
}
