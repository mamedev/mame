// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell A/B5000 family MCU disassembler

*/

#ifndef MAME_CPU_RW5000_RW5000D_H
#define MAME_CPU_RW5000_RW5000D_H

#pragma once


class rw5000_common_disassembler : public util::disasm_interface
{
public:
	rw5000_common_disassembler();
	virtual ~rw5000_common_disassembler() = default;

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

class a5000_disassembler : public rw5000_common_disassembler
{
public:
	a5000_disassembler() = default;
	virtual ~a5000_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 a5000_opmap[0x100];
};

class a5500_disassembler : public rw5000_common_disassembler
{
public:
	a5500_disassembler() = default;
	virtual ~a5500_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 a5500_opmap[0x100];
};

class b5000_disassembler : public rw5000_common_disassembler
{
public:
	b5000_disassembler() = default;
	virtual ~b5000_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 b5000_opmap[0x100];
};

class b5500_disassembler : public rw5000_common_disassembler
{
public:
	b5500_disassembler() = default;
	virtual ~b5500_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 b5500_opmap[0x100];
};

class b6000_disassembler : public rw5000_common_disassembler
{
public:
	b6000_disassembler() = default;
	virtual ~b6000_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 b6000_opmap[0x100];
};

class b6100_disassembler : public rw5000_common_disassembler
{
public:
	b6100_disassembler() = default;
	virtual ~b6100_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const u8 b6100_opmap[0x100];
};

#endif // MAME_CPU_RW5000_RW5000D_H
