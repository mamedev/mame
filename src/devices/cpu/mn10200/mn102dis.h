// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, hap
/*
    Panasonic MN10200 disassembler
*/

#ifndef MAME_CPU_MN10200_MN102DIS_H
#define MAME_CPU_MN10200_MN102DIS_H

#pragma once

class mn10200_disassembler : public util::disasm_interface
{
public:
	mn10200_disassembler() = default;
	virtual ~mn10200_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	u32 r24(const data_buffer &opcodes, offs_t pc);
	std::string i8str(s8 v);
	std::string i16str(int16_t v);
	std::string i24str(u32 v);
};

#endif
