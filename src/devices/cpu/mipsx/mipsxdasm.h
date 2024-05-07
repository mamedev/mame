// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_MIPSX_MIPSXDASM_H
#define MAME_CPU_MIPSX_MIPSXDASM_H

#pragma once

class mipsx_disassembler : public util::disasm_interface
{
public:
	mipsx_disassembler() = default;
	virtual ~mipsx_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	int get_ty(u32 opcode);
	int get_op(u32 opcode);
	int get_src1(u32 opcode);
	int get_src2_dest(u32 opcode);
	int get_offset(u32 opcode);
	int get_sq(u32 opcode);
	int get_compute_dest(u32 opcode);
	int get_compute_compfunc(u32 opcode);
	int get_asr_amount(int shift);
	int get_sh_amount(int shift);

	const char *get_regname(u8 reg);
};

#endif
