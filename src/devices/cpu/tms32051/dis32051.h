// license:BSD-3-Clause
// copyright-holders:Ville Linde

#ifndef MAME_CPU_TMS32051_DIS32051_H
#define MAME_CPU_TMS32051_DIS32051_H

#pragma once

class tms32051_disassembler : public util::disasm_interface
{
public:
	tms32051_disassembler() = default;
	virtual ~tms32051_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const zl_condition_codes[];
	static const char *const cv_condition_codes[16];
	static const char *const tp_condition_codes[4];
	uint16_t FETCH(offs_t &npc, const data_buffer &opcodes);
	std::string GET_ADDRESS(int addr_mode, int address);
	std::string GET_SHIFT(int shift);
	void print_condition_codes(bool pp, int zl, int cv, int tp);
	void dasm_group_be(uint16_t opcode, offs_t &npc, const data_buffer &opcodes);
	void dasm_group_bf(uint16_t opcode, offs_t &npc, const data_buffer &opcodes);

	std::ostream *output;
};

#endif
