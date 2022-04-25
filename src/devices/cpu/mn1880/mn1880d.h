// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_MN1880_MN1880D_H
#define MAME_CPU_MN1880_MN1880D_H

#pragma once

class mn1880_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	mn1880_disassembler();

protected:
	mn1880_disassembler(const char *const *inst_names);

	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// instruction mnemonic table
	static const char *const s_inst_names[256];

	// internal helpers
	void format_direct(std::ostream &stream, u8 da) const;
	void format_direct16(std::ostream &stream, u16 da) const;
	void format_direct_bp(std::ostream &stream, u8 da, u8 bp) const;
	void format_indirect_bp(std::ostream &stream, const char *ptr, u8 bp) const;
	void format_direct_masked(std::ostream &stream, u8 da, u8 mask) const;
	void format_indirect_masked(std::ostream &stream, const char *ptr, u8 mask) const;
	void format_imm(std::ostream &stream, u8 imm) const;
	void format_imm4(std::ostream &stream, u8 imm) const;
	void format_imm16(std::ostream &stream, u16 imm) const;
	void format_rel(std::ostream &stream, u16 base, s8 disp) const;
	void format_abs4k(std::ostream &stream, u16 label) const;
	void format_abs64k(std::ostream &stream, u16 label) const;
	void dasm_operands(std::ostream &stream, u8 opcode, offs_t &pc, offs_t &flags, const data_buffer &opcodes);

	const char *const *m_inst_names;
};

class mn1870_disassembler : public mn1880_disassembler
{
public:
	// construction/destruction
	mn1870_disassembler();

private:
	// instruction mnemonic table
	static const char *const s_inst_names[256];
};

class mn1860_disassembler : public mn1880_disassembler
{
public:
	// construction/destruction
	mn1860_disassembler();

private:
	// instruction mnemonic table
	static const char *const s_inst_names[256];
};

#endif // MAME_CPU_MN1880_MN1880D_H
