// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mips3dsm.c
    Disassembler for the portable MIPS 3 emulator.
    Written by Aaron Giles

***************************************************************************/

#ifndef MAME_CPU_MIPS_MIPS3DSM_H
#define MAME_CPU_MIPS_MIPS3DSM_H

#pragma once

class mips3_disassembler : public util::disasm_interface
{
public:
	mips3_disassembler() = default;
	virtual ~mips3_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	offs_t dasm_one(std::ostream &stream, offs_t pc, u32 op);

private:
	static const char *const reg[32];
	static const char *const cacheop[32];
	static const char *const cpreg[4][32];
	static const char *const ccreg[4][32];
	inline std::string signed_16bit(int16_t val);
	uint32_t dasm_cop0(uint32_t pc, uint32_t op, std::ostream &stream);
	uint32_t dasm_cop1(uint32_t pc, uint32_t op, std::ostream &stream);
	uint32_t dasm_cop1x(uint32_t pc, uint32_t op, std::ostream &stream);
	uint32_t dasm_cop2(uint32_t pc, uint32_t op, std::ostream &stream);

};

#endif
