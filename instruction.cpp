#include "instruction.hpp"

#include <cstring>

namespace
{
    inline vmword encode_controlword(const DecodedInstruction *instr)
    {
        vmword control = 0;
        control |= (uint64_t)instr->opcode        << 32;
        control |= (uint64_t)instr->flags         << 24;
        control |= (uint64_t)instr->addressing[0]  << 16;
        control |= (uint64_t)instr->addressing[1]  << 8;
        control |= (uint64_t)instr->addressing[2];
        return control;
    }

    inline void decode_controlword(vmword control, DecodedInstruction *target)
    {
        target->opcode        =         (Opcode)(control & 0xffffffff00000000ULL);
        target->flags         =    (OpcodeFlags)(control & 0x00000000ff000000ULL);
        target->addressing[0] = (AddressingMode)(control & 0x0000000000ff0000ULL);
        target->addressing[1] = (AddressingMode)(control & 0x000000000000ff00ULL);
        target->addressing[2] = (AddressingMode)(control & 0x00000000000000ffULL);
    }
}

VMInstruction vm_encode_instruction(const DecodedInstruction *instr)
{
	VMInstruction encoded_instruction;
	encoded_instruction.words[0] = encode_controlword(instr);
    memcpy(encoded_instruction.words + 1, instr->operands, 3);
    return encoded_instruction;
}

DecodedInstruction vm_decode_instruction(const VMInstruction *instr)
{
    DecodedInstruction decoded_instruction;
    decode_controlword(instr->words[0], &decoded_instruction);
    memcpy(decoded_instruction.operands, instr->words + 1, 3);
    return decoded_instruction;
}

VMInstruction vm_make_instruction(Opcode op, OpcodeFlags flags,
                               AddressingMode am0, vmint op0,
                               AddressingMode am1, vmint op1,
                               AddressingMode am2, vmint op2)
{
    DecodedInstruction decoded_instr;
    decoded_instr.opcode = op;
    decoded_instr.flags = flags;
    decoded_instr.addressing[0] = am0;
    decoded_instr.addressing[1] = am1;
    decoded_instr.addressing[2] = am2;
    decoded_instr.operands[0] = op0;
    decoded_instr.operands[1] = op1;
    decoded_instr.operands[2] = op2;
    return vm_encode_instruction(&decoded_instr);
}

VMValidationResult vm_validate_instruction(const DecodedInstruction *instr)
{
	VMValidationResult result;
	result.ok = false;

	// Check if opcode is ok
	if (instr->opcode >= INSTRUCTION_COUNT)
	{
		result.msg = "invalid opcode";
		goto end;
	}

	// Check if addressing modes are ok
	for (size_t i = 0; i < 3; i++)
	{
		auto addrm = instr->addressing[i];
		// Make sure any one mode is selected
		auto count = (addrm & ADDR_LITERAL) + (addrm & ADDR_MEMORY) + (addrm & ADDR_REGISTER);
		if (count != 1)
		{
			result.msg = "invalid addressing modes";
			goto end;
		}

		// ADDR_LITERAL plus ADDR_INDIRECT is forbidden
		if (addrm & ADDR_INDIRECT && addrm & ADDR_LITERAL)
		{
			result.msg = "indirect addressing with a literal value";
			goto end;
		}
	}

	result.ok = true;
end:
	return result;
}