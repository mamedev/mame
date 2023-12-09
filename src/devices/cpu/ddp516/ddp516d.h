// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_DDP516_DDP516D_H
#define MAME_CPU_DDP516_DDP516D_H

#pragma once

class ddp516_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	ddp516_disassembler();

protected:
	// util::disasm_interface overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	// formatting and disassembly helpers
	void format_disp(std::ostream &stream, u16 disp) const;
	offs_t dasm_skip(std::ostream &stream, u16 inst) const;
	offs_t dasm_generic(std::ostream &stream, u16 inst) const;
};

class prime_disassembler : public ddp516_disassembler
{
protected:
	// construction/destruction
	prime_disassembler(u8 mode);

	// util::disasm_interface overrides
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	// disassembly helpers
	offs_t dasm_generic_prime(std::ostream &stream, u16 inst) const;

private:
	const u8 c_mode_key;
};

class prime16s_disassembler : public prime_disassembler
{
public:
	// construction/destruction
	prime16s_disassembler();
};

class prime32s_disassembler : public prime_disassembler
{
public:
	// construction/destruction
	prime32s_disassembler();
};

class prime32r_disassembler : public prime_disassembler
{
public:
	// construction/destruction
	prime32r_disassembler();
};

class prime64r_disassembler : public prime_disassembler
{
public:
	// construction/destruction
	prime64r_disassembler();
};

class prime64v_disassembler : public prime_disassembler
{
public:
	// construction/destruction
	prime64v_disassembler();

protected:
	// util::disasm_interface overrides
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// formatting helper
	void format_ap(std::ostream &stream, u32 ap) const;
};

#endif // MAME_CPU_DDP516_DDP516D_H
