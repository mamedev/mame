// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_M88000_M88000D_H
#define MAME_CPU_M88000_M88000D_H

#pragma once

#include <map>

class m88000_disassembler : public util::disasm_interface
{
protected:
	enum class addressing
	{
		TRIADIC,
		FP,
		IMM6,
		BITFIELD,
		SIMM16,
		IMM16,
		CR,
		SI16_GRF,
		SI16_XRF,
		SCALED,
		JUMP,
		VEC9,
		D16,
		D26,
		NONE
	};

	struct instruction
	{
		u32 mask;
		const char *mnemonic;
		addressing mode;
	};

	// construction/destruction
	m88000_disassembler(const std::map<u32, const m88000_disassembler::instruction> &ops);

	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	// control register helpers
	virtual void format_gcr(std::ostream &stream, u8 cr) = 0;
	virtual void format_fcr(std::ostream &stream, u8 cr) = 0;

private:
	void format_condition(std::ostream &stream, u8 cnd);

	// disassembly helpers
	offs_t dasm_triadic(std::ostream &stream, const char *mnemonic, u32 inst);
	offs_t dasm_fp(std::ostream &stream, const char *mnemonic, u32 inst);
	offs_t dasm_imm6(std::ostream &stream, const char *mnemonic, u32 inst);
	offs_t dasm_bitfield(std::ostream &stream, const char *mnemonic, u32 inst);
	offs_t dasm_simm16(std::ostream &stream, const char *mnemonic, u32 inst);
	offs_t dasm_imm16(std::ostream &stream, const char *mnemonic, u32 inst);
	offs_t dasm_cr(std::ostream &stream, const char *mnemonic, u32 inst);
	offs_t dasm_si16(std::ostream &stream, const char *mnemonic, u32 inst, bool xrf);
	offs_t dasm_scaled(std::ostream &stream, const char *mnemonic, u32 inst);
	offs_t dasm_jump(std::ostream &stream, const char *mnemonic, u32 inst);
	offs_t dasm_vec9(std::ostream &stream, const char *mnemonic, u32 inst);
	offs_t dasm_d16(std::ostream &stream, const char *mnemonic, u32 inst, offs_t pc);
	offs_t dasm_d26(std::ostream &stream, const char *mnemonic, u32 inst, offs_t pc);
	offs_t dasm_none(std::ostream &stream, const char *mnemonic, u32 inst);
	offs_t dasm_illop(std::ostream &stream, u32 inst);

	const std::map<u32, const instruction> &m_ops;
};

class mc88100_disassembler : public m88000_disassembler
{
public:
	// construction/destruction
	mc88100_disassembler();

protected:
	// control register helpers
	virtual void format_gcr(std::ostream &stream, u8 cr) override;
	virtual void format_fcr(std::ostream &stream, u8 cr) override;

private:
	// decode map
	static const std::map<u32, const instruction> s_ops;
};

class mc88110_disassembler : public m88000_disassembler
{
public:
	// construction/destruction
	mc88110_disassembler();

protected:
	// control register helpers
	virtual void format_gcr(std::ostream &stream, u8 cr) override;
	virtual void format_fcr(std::ostream &stream, u8 cr) override;

private:
	// decode map
	static const std::map<u32, const instruction> s_ops;
};

#endif // MAME_CPU_M88000_M88000D_H
