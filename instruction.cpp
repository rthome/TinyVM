#include "instruction.hpp"

#include <cstring>
#include <cstdint>

namespace
{
	vmword encode_controlword(const Instruction *instr)
	{
		return (static_cast<uint32_t>(instr->opcode) << 32)
			| (static_cast<uint8_t>(instr->flags) << 24)
			| (static_cast<uint8_t>(instr->addressing[0]) << 16)
			| (static_cast<uint8_t>(instr->addressing[1]) << 8)
			| static_cast<uint8_t>(instr->addressing[2]);
	}

	void decode_controlword(vmword word, Instruction *dst)
	{
		dst->opcode = static_cast<Opcode>(word >> 32);
		dst->flags = static_cast<OpcodeFlags>((word >> 24) & 0xff);
		dst->addressing[0] = static_cast<AddressingMode>((word >> 16) & 0xff);
		dst->addressing[1] = static_cast<AddressingMode>((word >> 8) & 0xff);
		dst->addressing[2] = static_cast<AddressingMode>(word & 0xff);
	}
}

InstructionData vmi_encode(const Instruction *instr)
{
	InstructionData data;
	data.words[0] = encode_controlword(instr);
	for (size_t i = 0; i < 3; i++)
		data.words[i + 1] = instr->operands[i];
	return data;
}

Instruction vmi_decode(const InstructionData *data)
{
	Instruction instr;
	decode_controlword(data->words[0], &instr);
	for (size_t i = 0; i < 3; i++)
		instr.operands[i] = data->words[i + 1];
	return instr;
}

InstructionData vmi_read(const vmword *memory, size_t offset)
{
	InstructionData data;
	auto adjusted_addr = memory + offset;
	memcpy(data.words, adjusted_addr, sizeof(data.words));
	return data;
}