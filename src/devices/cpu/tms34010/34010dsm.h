// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*
 *   A TMS34010 disassembler
 *
 *   This code written by Zsolt Vasvari for the MAME project
 *
 */

#ifndef MAME_CPU_TMS34010_34010DSM_H
#define MAME_CPU_TMS34010_34010DSM_H

#pragma once

class tms34010_disassembler : public util::disasm_interface
{
public:
	tms34010_disassembler(bool is_34020);
	virtual ~tms34010_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	bool m_is_34020;
	uint8_t rf;
	uint16_t op,rs,rd;

	uint16_t r16(offs_t &pos, const data_buffer &opcodes);
	uint32_t r32(offs_t &pos, const data_buffer &opcodes);
	void print_reg(std::ostream &stream, uint8_t reg);
	void print_src_reg(std::ostream &stream);
	void print_des_reg(std::ostream &stream);
	void print_src_des_reg(std::ostream &stream);
	void print_word_parm(std::ostream &stream, offs_t &pos, const data_buffer &params);
	void print_word_parm_1s_comp(std::ostream &stream, offs_t &pos, const data_buffer &params);
	void print_long_parm(std::ostream &stream, offs_t &pos, const data_buffer &params);
	void print_long_parm_1s_comp(std::ostream &stream, offs_t &pos, const data_buffer &params);
	void print_constant(std::ostream &stream);
	void print_constant_1_32(std::ostream &stream);
	void print_constant_1s_comp(std::ostream &stream);
	void print_constant_2s_comp(std::ostream &stream);
	void print_relative(std::ostream &stream, offs_t pc, offs_t &pos, const data_buffer &params);
	void print_relative_8bit(std::ostream &stream, offs_t pc);
	void print_relative_5bit(std::ostream &stream, offs_t pc);
	void print_field(std::ostream &stream);
	void print_condition_code(std::ostream &stream);
	void print_reg_list_range(std::ostream &stream, int8_t first, int8_t last);
	void print_reg_list(std::ostream &stream, uint16_t rev, offs_t &pos, const data_buffer &params);
};

#endif
