// license:BSD-3-Clause
// copyright-holders:Ville Linde
// TMS32082 PP Disassembler

#ifndef MAME_CPU_TMS32082_DIS_PP_H
#define MAME_CPU_TMS32082_DIS_PP_H

#pragma once

class tms32082_pp_disassembler : public util::disasm_interface
{
public:
	tms32082_pp_disassembler() = default;
	virtual ~tms32082_pp_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static char const *const REG_NAMES[128];
	static char const *const CONDITION_CODES[16];
	static char const *const TRANSFER_SIZE[4];


	std::ostream *output;


	std::string format_address_mode(int mode, int areg, int s, int limx);
	void format_transfer(uint64_t op);
	void format_alu_op(int aluop, int a, const char *dst_text, const char *a_text, const char *b_text, const char *c_text);
};

#endif
