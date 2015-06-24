#include "instruction_support.hpp"

#include <cstring>
#include <cstdio>

#include "vm.hpp"

void vmi_load_memory_image(const void *data, VMContext *ctx)
{
    memcpy(ctx->memory, data, sizeof(ctx->memory));
}

bool vmi_load_memory_image_file(const char *filename, VMContext *ctx)
{
    auto fp = fopen(filename, "r");
    if (fp == nullptr)
        return false;
    auto read = fread(ctx->memory, sizeof(vmword), VM_MEMORY_SIZE, fp);
    fclose(fp);
    if (read == VM_MEMORY_SIZE)
        return true;
    else
        return false;
}

bool vmi_save_memory_image_file(const char *filename, const VMContext *ctx)
{
    auto fp = fopen(filename, "w");
    if (fp == nullptr)
        return false;
    auto written = fwrite(ctx->memory, sizeof(vmword), VM_MEMORY_SIZE, fp);
    fclose(fp);
    if (written == VM_MEMORY_SIZE)
        return true;
    else
        return false;
}

// Make a nullary instruction
Instruction vmi_make_instr_0(Opcode opcode, OpcodeFlags flags)
{
	return Instruction{ opcode, flags };
}

InstructionData vmi_encode_instr_0(Opcode opcode, OpcodeFlags flags)
{
	auto instr = vmi_make_instr_0(opcode, flags);
	return vmi_encode(&instr);
}

// Make a unary instruction
Instruction vmi_make_instr_1(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0)
{
	Instruction instr{ opcode, flags };
	instr.addressing[0] = am0;
	instr.operands[0] = op0;
	return instr;
}

InstructionData vmi_encode_instr_1(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0)
{
	auto instr = vmi_make_instr_1(opcode, flags, am0, op0);
	return vmi_encode(&instr);
}

// Make a binary instruction
Instruction vmi_make_instr_2(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0,
	AddressingMode am1, vmword op1)
{
	Instruction instr{ opcode, flags };
	instr.addressing[0] = am0;
	instr.addressing[1] = am1;
	instr.operands[0] = op0;
	instr.operands[1] = op1;
	return instr;
}

InstructionData vmi_encode_instr_2(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0,
	AddressingMode am1, vmword op1)
{
	auto instr = vmi_make_instr_2(opcode, flags, am0, op0, am1, op1);
	return vmi_encode(&instr);
}

// Make a tertiary instruction
Instruction vmi_make_instr_3(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0,
	AddressingMode am1, vmword op1,
	AddressingMode am2, vmword op2)
{
	Instruction instr{ opcode, flags };
	instr.addressing[0] = am0;
	instr.addressing[1] = am1;
	instr.addressing[2] = am2;
	instr.operands[0] = op0;
	instr.operands[1] = op1;
	instr.operands[2] = op2;
	return instr;
}

InstructionData vmi_encode_instr_3(Opcode opcode, OpcodeFlags flags,
	AddressingMode am0, vmword op0,
	AddressingMode am1, vmword op1,
	AddressingMode am2, vmword op2)
{
	auto instr = vmi_make_instr_3(opcode, flags, am0, op0, am1, op1, am2, op2);
	return vmi_encode(&instr);
}
