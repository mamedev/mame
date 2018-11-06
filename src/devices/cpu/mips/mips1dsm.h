// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    r3kdasm.c
    Disassembler for the portable R3000 emulator.
    Written by Aaron Giles

***************************************************************************/

#ifndef MAME_CPU_MIPS_MIPS1DSM_H
#define MAME_CPU_MIPS_MIPS1DSM_H

#pragma once

class mips1_disassembler : public util::disasm_interface
{
public:
	mips1_disassembler() = default;
	virtual ~mips1_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const reg[32];
	static const char *const cpreg[4][32];
	static const char *const ccreg[4][32];
	std::string signed_16bit(int16_t val);
	uint32_t dasm_cop(uint32_t pc, int cop, uint32_t op, std::ostream &stream);
	uint32_t dasm_cop1(uint32_t pc, uint32_t op, std::ostream &stream);


};

#endif // MAME_CPU_MIPS_MIPS1DSM_H
