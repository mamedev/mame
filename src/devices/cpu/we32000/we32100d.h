// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_WE32000_WE32100D_H
#define MAME_CPU_WE32000_WE32100D_H

#pragma once

class we32100_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	we32100_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const s_rnames[16];

	// internal helpers
	void format_signed(std::ostream &stream, s32 x);
	void dasm_am(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, u8 n, bool dst, bool spectype);
	void dasm_src(std::ostream &stream, offs_t &pc, const data_buffer &opcodes);
	void dasm_srcw(std::ostream &stream, offs_t &pc, const data_buffer &opcodes);
	void dasm_dst(std::ostream &stream, offs_t &pc, const data_buffer &opcodes);
	void dasm_dstw(std::ostream &stream, offs_t &pc, const data_buffer &opcodes);
	void dasm_ea(std::ostream &stream, offs_t &pc, offs_t ppc, const data_buffer &opcodes);
	void dasm_sr(std::ostream &stream, offs_t &pc, const data_buffer &opcodes);
	void dasm_bdisp(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool byte);
	offs_t dasm_30xx(std::ostream &stream, offs_t &pc, const data_buffer &opcodes);
};

#endif // MAME_CPU_WE32000_WE32100D_H
