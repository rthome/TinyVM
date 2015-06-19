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
    encoded_instruction.control = encode_controlword(instr);
    memcpy(encoded_instruction.words + 1, instr->operands, 3);
    return encoded_instruction;
}

DecodedInstruction vm_decode_instruction(const VMInstruction *instr)
{
    DecodedInstruction decoded_instruction;
    decode_controlword(instr->control, &decoded_instruction);
    memcpy(decoded_instruction.operands, instr->words + 1, 3);
    return decoded_instruction;
}
