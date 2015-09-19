#include "instruction_implementation.hpp"

#include "instruction.hpp"
#include "vm.hpp"

#include <random>
#include <chrono>

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

    enum OperandIndex
    {
        O_A = 0,
        O_B = 1,
        O_C = 2,
    };

    // Assign value to location operand at Index position points to. Does not support literal operands.
    template<OperandIndex Index>
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
    template<OperandIndex Index>
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

    INSTRUCTION_IMPL(halt)
    {
        ctx->running = false;
    }

    INSTRUCTION_IMPL(push)
    {
        auto a = operand_fetch<O_A>(ctx, instr);
        stack_push(ctx, a);
    }

    INSTRUCTION_IMPL(pop)
    {
        auto val = stack_pop(ctx);
        operand_assign_at<O_A>(ctx, instr, val);
    }

    INSTRUCTION_IMPL(add)
    {
        // TODO: operate on signed value here?
        auto b = operand_fetch<O_B>(ctx, instr);
        auto c = operand_fetch<O_C>(ctx, instr);
        auto val = b + c;
        operand_assign_at<O_A>(ctx, instr, val);
    }

	INSTRUCTION_IMPL(sub)
	{
		// TODO: operate on signed value here?
        auto b = operand_fetch<O_B>(ctx, instr);
        auto c = operand_fetch<O_C>(ctx, instr);
        auto val = b - c;
        operand_assign_at<O_A>(ctx, instr, val);
	}

	INSTRUCTION_IMPL(mul)
	{
		// TODO: operate on signed value here?
        auto b = operand_fetch<O_B>(ctx, instr);
        auto c = operand_fetch<O_C>(ctx, instr);
        auto val = b * c;
        operand_assign_at<O_A>(ctx, instr, val);
	}

	INSTRUCTION_IMPL(div)
	{
		// TODO: operate on signed value here?
        auto b = operand_fetch<O_B>(ctx, instr);
        auto c = operand_fetch<O_C>(ctx, instr);
		auto val = b / c;
		auto rem = b % c;
        operand_assign_at<O_A>(ctx, instr, val);
		ctx->registers[RMD] = rem;
	}

	INSTRUCTION_IMPL(shl)
	{
		auto b = operand_fetch<O_B>(ctx, instr);
		auto c = operand_fetch<O_C>(ctx, instr);
		auto val = b << c;
		operand_assign_at<O_A>(ctx, instr, val);
	}

	INSTRUCTION_IMPL(shr)
	{
		auto b = operand_fetch<O_B>(ctx, instr);
		auto c = operand_fetch<O_C>(ctx, instr);
		auto val = b >> c;
		operand_assign_at<O_A>(ctx, instr, val);
	}

    INSTRUCTION_IMPL(mod)
    {
        auto b = operand_fetch<O_B>(ctx, instr);
        auto c = operand_fetch<O_C>(ctx, instr);
        auto rem = b % c;
        operand_assign_at<O_A>(ctx, instr, rem);
    }

    INSTRUCTION_IMPL(inc)
    {
        // TODO: operate on signed value here?
        auto a = operand_fetch<O_A>(ctx, instr);
        auto val = a + 1;
        operand_assign_at<O_A>(ctx, instr, val);
    }

    INSTRUCTION_IMPL(dec)
    {
        // TODO: operate on signed value here?
        auto a = operand_fetch<O_A>(ctx, instr);
        auto val = a - 1;
        operand_assign_at<O_A>(ctx, instr, val);
    }

	INSTRUCTION_IMPL(not)
	{
		// TODO: operate on signed value here?
        auto a = operand_fetch<O_A>(ctx, instr);
		auto na = ~a;
        operand_assign_at<O_A>(ctx, instr, na);
	}

	INSTRUCTION_IMPL(cmp)
	{
		auto b = operand_fetch<O_B>(ctx, instr);
		auto c = operand_fetch<O_C>(ctx, instr);
		vmword result;
		if (c < b)
			result = (vmword)-1;
		else if (c > b)
			result = 1;
		else
			result = 0;
		operand_assign_at<O_A>(ctx, instr, result);
	}

    INSTRUCTION_IMPL(mov)
    {
        auto b = operand_fetch<O_B>(ctx, instr);
        operand_assign_at<O_A>(ctx, instr, b);
    }

	INSTRUCTION_IMPL(call)
	{
        auto a = operand_fetch<O_A>(ctx, instr);
		stack_push(ctx, ctx->registers[IP]);
		ctx->registers[IP] = a;
	}

	INSTRUCTION_IMPL(ret)
	{
		auto ip = stack_pop(ctx);
		ctx->registers[IP] = ip;
	}

    INSTRUCTION_IMPL(jmp)
    {
        auto a = operand_fetch<O_A>(ctx, instr);
        ctx->registers[IP] = a;
    }

    INSTRUCTION_IMPL(jeq)
    {
        auto a = operand_fetch<O_A>(ctx, instr);
        auto b = operand_fetch<O_B>(ctx, instr);
        auto c = operand_fetch<O_C>(ctx, instr);
        if (b == c)
			ctx->registers[IP] = a;
    }

    INSTRUCTION_IMPL(jne)
    {
        auto a = operand_fetch<O_A>(ctx, instr);
        auto b = operand_fetch<O_B>(ctx, instr);
        auto c = operand_fetch<O_C>(ctx, instr);
        if (b != c)
			ctx->registers[IP] = a;
    }

	INSTRUCTION_IMPL(jnz)
	{
        auto a = operand_fetch<O_A>(ctx, instr);
        auto b = operand_fetch<O_B>(ctx, instr);
		if (b != 0)
			ctx->registers[IP] = a;
	}

    INSTRUCTION_IMPL(rdrand)
    {
        static std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());

        auto min = operand_fetch<O_B>(ctx, instr);
        auto max = operand_fetch<O_C>(ctx, instr);
        if (min == max && min == 0)
            max = UINT64_MAX;
        std::uniform_int_distribution<vmword> distribution(min, max);
        vmword value = distribution(generator);
        operand_assign_at<O_A>(ctx, instr, value);
    }
}

void prepare_instruction_table(instr_func *buffer)
{
    buffer[OP_NOP] = &IMPL_NAME(nop);
    buffer[OP_HALT] = &IMPL_NAME(halt);
    buffer[OP_PUSH] = &IMPL_NAME(push);
    buffer[OP_POP] = &IMPL_NAME(pop);
    buffer[OP_ADD] = &IMPL_NAME(add);
	buffer[OP_SUB] = &IMPL_NAME(sub);
	buffer[OP_MUL] = &IMPL_NAME(mul);
	buffer[OP_DIV] = &IMPL_NAME(div);
	buffer[OP_SHL] = &IMPL_NAME(shl);
	buffer[OP_SHR] = &IMPL_NAME(shr);
    buffer[OP_MOD] = &IMPL_NAME(mod);
	buffer[OP_NOT] = &IMPL_NAME(not);
    buffer[OP_INC] = &IMPL_NAME(inc);
    buffer[OP_DEC] = &IMPL_NAME(dec);
	buffer[OP_CMP] = &IMPL_NAME(cmp);
    buffer[OP_MOV] = &IMPL_NAME(mov);
	buffer[OP_CALL] = &IMPL_NAME(call);
	buffer[OP_RET] = &IMPL_NAME(ret);
    buffer[OP_JMP] = &IMPL_NAME(jmp);
    buffer[OP_JEQ] = &IMPL_NAME(jeq);
	buffer[OP_JNE] = &IMPL_NAME(jne);
	buffer[OP_JNZ] = &IMPL_NAME(jnz);
    buffer[OP_RDRAND] = &IMPL_NAME(rdrand);
}
