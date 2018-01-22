// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*****************************************************************************
 *
 *   4004dasm.cpp
 *
 *   Intel MCS-40 CPU Disassembly
 *
 *****************************************************************************/

#ifndef MAME_CPU_MCS40_MCS40DASM_H
#define MAME_CPU_MCS40_MCS40DASM_H

#pragma once

class mcs40_disassembler : public util::disasm_interface
{
public:
	enum class level
	{
		I4004,
		I4040
	};

	mcs40_disassembler(level lvl, unsigned pcmask);
	virtual ~mcs40_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	virtual u32 interface_flags() const override { return PAGED; }
	virtual u32 page_address_bits() const override { return 12; }

private:
	enum class format
	{
		ILL,
		SIMPLE,
		IMM4,
		REG,
		REGPAGE,
		PAIR,
		PAIRIMM,
		ABS,
		PAGE,
		COND,
		EXT
	};

	struct op
	{
		format m_format;
		level m_level;
		char const *m_name;
		op const *m_ext;
	};

	static op const f_opx_0[16];
	static op const f_opx_2[16];
	static op const f_opx_3[16];
	static op const f_opx_io[16];
	static op const f_opx_f[16];
	static op const f_ops[16];
	static char const *const f_cond[16];

	level m_lvl;
	unsigned m_pcmask;
};

class i4004_disassembler : public mcs40_disassembler
{
public:
	i4004_disassembler();
	virtual ~i4004_disassembler() = default;
};

class i4040_disassembler : public mcs40_disassembler
{
public:
	i4040_disassembler();
	virtual ~i4040_disassembler() = default;
};

#endif
