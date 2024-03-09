// license:BSD-3-Clause
// copyright-holders:hap, Jonathan Gevaryahu
/*

  Sharp SM5xx MCU family disassembler

*/

#ifndef MAME_CPU_SM510_SM510D_H
#define MAME_CPU_SM510_SM510D_H

#pragma once

class sm510_common_disassembler : public util::disasm_interface
{
public:
	sm510_common_disassembler();
	virtual ~sm510_common_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual u32 interface_flags() const override { return NONLINEAR_PC | PAGED2LEVEL; }
	virtual u32 page_address_bits() const override { return 6; }
	virtual u32 page2_address_bits() const override { return 4; }
	virtual offs_t pc_linear_to_real(offs_t pc) const override { return (pc & ~0x3f) | m_l2r6[pc & 0x3f]; }
	virtual offs_t pc_real_to_linear(offs_t pc) const override { return (pc & ~0x3f) | m_r2l6[pc & 0x3f]; }

protected:
	enum e_mnemonics : unsigned;
	static const char *const s_mnemonics[];
	static const u8 s_bits[];
	static const u32 s_flags[];

	u8 m_l2r6[0x40];
	u8 m_r2l6[0x40];
	u8 m_l2r7[0x80];
	u8 m_r2l7[0x80];

	offs_t increment_pc(offs_t pc, u8 pclen);
	offs_t common_disasm(const u8 *lut_mnemonic, const u8 *lut_extended, std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params);
};

class sm510_disassembler : public sm510_common_disassembler
{
public:
	sm510_disassembler() = default;
	virtual ~sm510_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 sm510_mnemonic[0x100];

};

class sm511_disassembler : public sm510_common_disassembler
{
public:
	sm511_disassembler() = default;
	virtual ~sm511_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 sm511_mnemonic[0x100];
	static const u8 sm511_extended[0x10];
};

class sm500_disassembler : public sm510_common_disassembler
{
public:
	sm500_disassembler() = default;
	virtual ~sm500_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 sm500_mnemonic[0x100];
	static const u8 sm500_extended[0x10];
};

class sm5a_disassembler : public sm510_common_disassembler
{
public:
	sm5a_disassembler() = default;
	virtual ~sm5a_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 sm5a_mnemonic[0x100];
	static const u8 sm5a_extended[0x10];
};

class sm530_disassembler : public sm510_common_disassembler
{
public:
	sm530_disassembler() = default;
	virtual ~sm530_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	virtual u32 interface_flags() const override { return NONLINEAR_PC | PAGED; }

private:
	static const u8 sm530_mnemonic[0x100];
};

class sm590_disassembler : public sm510_common_disassembler
{
public:
	sm590_disassembler() = default;
	virtual ~sm590_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	virtual u32 page_address_bits() const override { return 7; }
	virtual u32 page2_address_bits() const override { return 2; }
	virtual offs_t pc_linear_to_real(offs_t pc) const override { return (pc & ~0x7f) | m_l2r7[pc & 0x7f]; }
	virtual offs_t pc_real_to_linear(offs_t pc) const override { return (pc & ~0x7f) | m_r2l7[pc & 0x7f]; }

private:
	static const u8 sm590_mnemonic[0x100];
};

#endif // MAME_CPU_SM510_SM510D_H
