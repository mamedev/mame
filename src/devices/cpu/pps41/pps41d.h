// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell PPS-4/1 disassembler

*/

#ifndef MAME_CPU_PPS41_PPS41D_H
#define MAME_CPU_PPS41_PPS41D_H

#pragma once


class pps41_common_disassembler : public util::disasm_interface
{
public:
	pps41_common_disassembler();
	virtual ~pps41_common_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual u32 interface_flags() const override { return NONLINEAR_PC | PAGED; }
	virtual u32 page_address_bits() const override { return 6; }
	virtual offs_t pc_linear_to_real(offs_t pc) const override { return (pc & ~0x3f) | m_l2r[pc & 0x3f]; }
	virtual offs_t pc_real_to_linear(offs_t pc) const override { return (pc & ~0x3f) | m_r2l[pc & 0x3f]; }

protected:
	// opcode mnemonics
	enum e_mnemonics
	{
		// MM76/shared
		em_ILL /* 0! */,
		em_XAB, em_LBA, em_LB, em_EOB2,
		em_SB, em_RB, em_SKBF,
		em_XAS, em_LSA,
		em_L, em_X, em_XDSK, em_XNSK,
		em_A, em_AC, em_ACSK, em_ASK, em_COM, em_RC, em_SC, em_SKNC, em_LAI, em_AISK,
		em_RT, em_RTSK, em_T, em_NOP, em_TL, em_TM, em_TML, em_TR,
		em_SKMEA, em_SKBEI, em_SKAEI,
		em_SOS, em_ROS, em_SKISL, em_IBM, em_OB, em_IAM, em_OA, em_IOS, em_I1, em_I2C, em_INT1H, em_DIN1, em_INT0L, em_DIN0, em_SEG1, em_SEG2,

		// MM78 differences
		em_INT0H, em_INT1L, em_SAG, em_EOB3, em_TAB,
		em_I1SK, em_IX, em_OX, em_LXA, em_XAX, em_IOA,
		em_TLB, em_TMLB
	};

	static const char *const s_name[];
	static const u8 s_bits[];
	static const u32 s_flags[];

	u8 m_l2r[0x40];
	u8 m_r2l[0x40];

	offs_t increment_pc(offs_t pc);
	offs_t common_disasm(const u8 *lut_opmap, std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params);
};

class mm76_disassembler : public pps41_common_disassembler
{
public:
	mm76_disassembler() = default;
	virtual ~mm76_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 mm76_opmap[0x100];

};

class mm78_disassembler : public pps41_common_disassembler
{
public:
	mm78_disassembler() = default;
	virtual ~mm78_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 mm78_opmap[0x100];

};

#endif // MAME_CPU_PPS41_PPS41D_H
