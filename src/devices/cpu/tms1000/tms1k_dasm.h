// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*

  TMS0980/TMS1000-family disassembler

*/

#ifndef MAME_CPU_TMS1000_TMS1K_DASM_H
#define MAME_CPU_TMS1000_TMS1K_DASM_H

#pragma once

class tms1000_base_disassembler : public util::disasm_interface
{
public:
	tms1000_base_disassembler(const u8 *lut_mnemonic, bool opcode_9bits, int pc_bits);
	virtual ~tms1000_base_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual u32 interface_flags() const override { return NONLINEAR_PC | PAGED; }
	virtual u32 page_address_bits() const override { return m_pc_bits; }
	virtual offs_t pc_linear_to_real(offs_t pc) const override;
	virtual offs_t pc_real_to_linear(offs_t pc) const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	enum e_mnemonics : unsigned;
	static const char *const s_mnemonic[];
	static const u32 s_flags[];
	static const u8 s_bits[];
	static const u8 i2_value[4];
	static const u8 i3_value[8];
	static const u8 i4_value[16];

	const u8 *m_lut_mnemonic;
	bool m_opcode_9bits;
	int m_pc_bits;

	std::unique_ptr<u8[]> m_l2r;
	std::unique_ptr<u8[]> m_r2l;
};

class tms1000_disassembler : public tms1000_base_disassembler
{
public:
	tms1000_disassembler();
	virtual ~tms1000_disassembler() = default;

private:
	static const u8 tms1000_mnemonic[256];
};

class tms1100_disassembler : public tms1000_base_disassembler
{
public:
	tms1100_disassembler();
	virtual ~tms1100_disassembler() = default;

	virtual u32 interface_flags() const override { return NONLINEAR_PC | PAGED2LEVEL; }
	virtual u32 page2_address_bits() const override { return 4; }

protected:
	tms1100_disassembler(const u8 *lut_mnemonic, bool opcode_9bits, int pc_bits);

private:
	static const u8 tms1100_mnemonic[256];
};

class tms1400_disassembler : public tms1100_disassembler
{
public:
	tms1400_disassembler();
	virtual ~tms1400_disassembler() = default;

private:
	static const u8 tms1400_mnemonic[256];
};

class tms2100_disassembler : public tms1100_disassembler
{
public:
	tms2100_disassembler();
	virtual ~tms2100_disassembler() = default;

private:
	static const u8 tms2100_mnemonic[256];
};

class tms2400_disassembler : public tms1100_disassembler
{
public:
	tms2400_disassembler();
	virtual ~tms2400_disassembler() = default;

private:
	static const u8 tms2400_mnemonic[256];
};

class smc1102_disassembler : public tms1100_disassembler
{
public:
	smc1102_disassembler();
	virtual ~smc1102_disassembler() = default;

private:
	static const u8 smc1102_mnemonic[256];
};

class tms0980_disassembler : public tms1000_base_disassembler
{
public:
	tms0980_disassembler();
	virtual ~tms0980_disassembler() = default;

private:
	static const u8 tms0980_mnemonic[512];
};

class tp0320_disassembler : public tms1000_base_disassembler
{
public:
	tp0320_disassembler();
	virtual ~tp0320_disassembler() = default;

private:
	static const u8 tp0320_mnemonic[512];
};

#endif // MAME_CPU_TMS1000_TMS1K_DASM_H
