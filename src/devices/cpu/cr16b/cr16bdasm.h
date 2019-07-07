// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_CR16B_CR16BDASM_H
#define MAME_CPU_CR16B_CR16BDASM_H 1

#pragma once

class cr16b_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	cr16b_disassembler();

	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	enum cr16_arch
	{
		CR16A,
		CR16B
	};

	cr16b_disassembler(cr16_arch arch);

	// internal helpers
	void format_reg(std::ostream &stream, u8 reg);
	void format_rpair(std::ostream &stream, u8 reg);
	virtual void format_rproc(std::ostream &stream, u8 reg);
	void format_short_imm(std::ostream &stream, u8 imm);
	void format_short_imm_unsigned(std::ostream &stream, u8 imm, bool i);
	void format_short_imm_decimal(std::ostream &stream, u8 imm);
	void format_medium_imm(std::ostream &stream, u16 imm);
	void format_medium_imm_unsigned(std::ostream &stream, u16 imm, bool i);
	void format_medium_imm_decimal(std::ostream &stream, u16 imm);
	void format_imm21(std::ostream &stream, u32 imm);
	void format_disp5(std::ostream &stream, u8 disp);
	void format_disp16(std::ostream &stream, u16 disp);
	void format_disp18(std::ostream &stream, u32 disp);
	void format_abs18(std::ostream &stream, u32 addr);
	void format_pc_disp5(std::ostream &stream, offs_t pc, u8 disp);
	void format_pc_disp9(std::ostream &stream, offs_t pc, u16 disp);
	void format_pc_disp17(std::ostream &stream, offs_t pc, u32 disp);
	void format_pc_disp21(std::ostream &stream, offs_t pc, u32 disp);
	void format_excp_vector(std::ostream &stream, u8 vec);

private:
	// static tables
	static const char *const s_cc[14];

	// architecture version
	const cr16_arch m_arch;
};

class cr16a_disassembler : public cr16b_disassembler
{
public:
	// construction/destruction
	cr16a_disassembler();

protected:
	virtual void format_rproc(std::ostream &stream, u8 reg) override;
};

#endif // MAME_CPU_CR16B_CR16BDASM_H
