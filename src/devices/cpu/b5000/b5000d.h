// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B5000 family MCU disassembler

*/

#ifndef MAME_CPU_B5000_B5000D_H
#define MAME_CPU_B5000_B5000D_H

#pragma once


class b5000_common_disassembler : public util::disasm_interface
{
public:
	b5000_common_disassembler();
	virtual ~b5000_common_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual u32 interface_flags() const override { return NONLINEAR_PC | PAGED; }
	virtual u32 page_address_bits() const override { return 6; }
	virtual offs_t pc_linear_to_real(offs_t pc) const override { return (pc & ~0x3f) | m_l2r[pc & 0x3f]; }
	virtual offs_t pc_real_to_linear(offs_t pc) const override { return (pc & ~0x3f) | m_r2l[pc & 0x3f]; }

protected:
	// opcode mnemonics
	enum e_mnemonics
	{
		em_ILL,
		em_NOP, em_RSC, em_SC, em_TC, em_TAM,
		em_LAX, em_ADX, em_COMP, em_ATB, em_ATBZ,
		em_LDA, em_EXC0, em_EXCP, em_EXCM, em_ADD,
		em_LB0, em_LB7, em_LB8, em_LB9, em_LB10, em_LB11,
		em_RSM, em_SM, em_TM,
		em_TL, em_TRA0, em_TRA1, em_RET,
		em_TKB, em_TKBS, em_TDIN, em_READ, em_KSEG, em_MTD
	};

	static const char *const s_name[];
	static const u8 s_bits[];
	static const u32 s_flags[];

	u8 m_l2r[0x40];
	u8 m_r2l[0x40];

	offs_t increment_pc(offs_t pc);
	offs_t common_disasm(const u8 *lut_opmap, std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params);
};

class b5000_disassembler : public b5000_common_disassembler
{
public:
	b5000_disassembler() = default;
	virtual ~b5000_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 b5000_opmap[0x100];

};

class b5500_disassembler : public b5000_common_disassembler
{
public:
	b5500_disassembler() = default;
	virtual ~b5500_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 b5500_opmap[0x100];

};

#endif // MAME_CPU_B5000_B5000D_H
