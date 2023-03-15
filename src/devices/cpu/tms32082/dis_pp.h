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
	static char const *const CONDITION_CODES[16];
	static char const *const TRANSFER_SIZE[4];

	std::string m_alu_condition;
	std::string m_alu_operation;
	std::string m_parallel_condition;
	std::string m_parallel_transfer;

	int m_src1bank;
	int m_dstbank;

	std::string format_alu_op(int aluop, int a, const std::string& dst_text, std::string& a_text, std::string& b_text, std::string& c_text);

	void parallel_transfer(uint64_t op);
	std::string make_mem_transfer(int mode, int dst, int a, bool scale, int size, int le, int imm, int x);
	std::string make_ea(int mode, int areg, bool scale, int size, int offset, int xreg);
	std::string make_condition(int cond, int ncvz);

	std::string get_reg_name(int reg, bool read);
	std::string make_field_move(bool d, bool e, int size, int itm);
};

#endif
