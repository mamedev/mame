// license:BSD-3-Clause
// copyright-holders:grubbyplaya

#ifndef MAME_CPU_T6M53_T6M53D_H
#define MAME_CPU_T6M53_T6M53D_H

#pragma once

class t6m53_disassembler : public util::disasm_interface
{
public:
	t6m53_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static u16 bit_address(u16 op);
	static u16 ef(u16 op);

	static unsigned bit_number(u16 op);
	static const char *size_suffix(u16 op);
	static const char *compare_name(u16 op);

	static void print_location(std::ostream &stream, u16 location);
	static void print_bit_location(std::ostream &stream, u16 op);
	static void print_target(std::ostream &stream, u16 target);
	static void print_branch(std::ostream &stream, u16 op, u16 target, offs_t &flags);

	static bool has_branch(u16 op);
};

#endif // MAME_CPU_T6M53_T6M53D_H