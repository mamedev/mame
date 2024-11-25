// license:BSD-3-Clause
// copyright-holders:hap
/*

  National Semiconductor COPS(MM57 MCU family) disassembler

*/

#ifndef MAME_CPU_COPS1_COPS1D_H
#define MAME_CPU_COPS1_COPS1D_H

#pragma once


class cops1_common_disassembler : public util::disasm_interface
{
public:
	cops1_common_disassembler();
	virtual ~cops1_common_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual u32 interface_flags() const override { return NONLINEAR_PC | PAGED; }
	virtual u32 page_address_bits() const override { return 6; }
	virtual offs_t pc_linear_to_real(offs_t pc) const override { return (pc & ~0x3f) | m_l2r[pc & 0x3f]; }
	virtual offs_t pc_real_to_linear(offs_t pc) const override { return (pc & ~0x3f) | m_r2l[pc & 0x3f]; }

protected:
	enum e_mnemonics : unsigned;
	static const char *const s_name[];
	static const u8 s_bits[];
	static const u32 s_flags[];

	u8 m_l2r[0x40];
	u8 m_r2l[0x40];

	offs_t increment_pc(offs_t pc);
	offs_t common_disasm(const u8 *lut_opmap, std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params);
};

class mm5799_disassembler : public cops1_common_disassembler
{
public:
	mm5799_disassembler() = default;
	virtual ~mm5799_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 mm5799_opmap[0x100];

};

#endif // MAME_CPU_COPS1_COPS1D_H
