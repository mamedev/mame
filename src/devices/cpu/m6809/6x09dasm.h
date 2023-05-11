// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Sean Riddle,Tim Lindner
/*****************************************************************************

    a 6809/6309/Konami opcode disassembler

    Based on:
        6309dasm.c - a 6309 opcode disassembler
        Version 1.0 5-AUG-2000
        Copyright Tim Lindner

    and

        6809dasm.c - a 6809 opcode disassembler
        Version 1.4 1-MAR-95
        Copyright Sean Riddle

    Thanks to Franklin Bowen for bug fixes, ideas

    Freely distributable on any medium given all copyrights are retained
    by the author and no charge greater than $7.00 is made for obtaining
    this software

    Please send all bug reports, update ideas and data files to:
    sriddle@ionet.net and tlindner@ix.netcom.com

*****************************************************************************/

#ifndef MAME_CPU_M6809_6X09DASM_H
#define MAME_CPU_M6809_6X09DASM_H

#include <set>

#pragma once

class m6x09_base_disassembler : public util::disasm_interface
{
protected:
	class opcodeinfo;

public:
	// General, undocumented, or 6309 only?
	enum m6x09_instruction_level : uint8_t
	{
		M6x09_GENERAL = 1,
		M6809_UNDOCUMENTED = 2,
		HD6309_EXCLUSIVE = 4
	};

	m6x09_base_disassembler(const opcodeinfo *opcodes, size_t opcode_count, uint32_t level);

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	enum m6x09_addressing_mode : uint8_t
	{
		INH,                // Inherent
		PSHS, PSHU,         // Push
		PULS, PULU,         // Pull
		DIR,                // Direct
		DIR_IM,             // Direct in memory (6309 only)
		IND,                // Indexed
		REL,                // Relative (8 bit)
		LREL,               // Long relative (16 bit)
		EXT,                // Extended
		IMM,                // Immediate
		IMM_RR,             // Register-to-register
		IMM_BW,             // Bitwise operations (6309 only)
		IMM_TFM,            // Transfer from memory (6309 only)
		PG2,                // Switch to page 2 opcodes
		PG3                 // Switch to page 3 opcodes
	};

	// Opcode structure
	class opcodeinfo
	{
	public:
		constexpr opcodeinfo(uint16_t opcode, uint8_t operand_length, const char *name, m6x09_addressing_mode mode, uint32_t level, unsigned flags = 0)
			: m_opcode(opcode), m_operand_length(operand_length), m_mode(mode), m_level(level), m_flags(flags), m_name(name)
		{
		}

		uint16_t opcode() const { return m_opcode; }
		uint8_t operand_length() const { return m_operand_length; }
		m6x09_addressing_mode mode() const { return m_mode; }
		uint32_t level() const { return m_level; }
		unsigned flags() const { return m_flags; }
		const char *name() const { return m_name; }

		struct compare
		{
			using is_transparent = void;
			bool operator()(opcodeinfo const& lhs, opcodeinfo const& rhs) const;
			bool operator()(uint16_t opcode, opcodeinfo const& rhs) const;
			bool operator()(opcodeinfo const& lhs, uint16_t opcode) const;
		};

	private:
		uint16_t                m_opcode;         // 8-bit opcode value
		uint8_t                 m_operand_length; // Opcode length in bytes
		m6x09_addressing_mode   m_mode;           // Addressing mode
		uint8_t                 m_level;          // General, or 6309 only?
		unsigned                m_flags;          // Disassembly flags
		const char *            m_name;           // Opcode name
	};

	static const char *const m6x09_regs[5];
	static const char *const m6x09_btwregs[5];
	static const char *const hd6309_tfmregs[16];
	static const char *const tfm_s[];

	virtual void indexed(std::ostream &stream, uint8_t pb, const data_buffer &params, offs_t &p) = 0;
	virtual void register_register(std::ostream &stream, uint8_t pb) = 0;

private:
	std::set<opcodeinfo, opcodeinfo::compare> m_opcodes;

	uint32_t m_level;
	uint16_t m_page;

	const opcodeinfo *fetch_opcode(const data_buffer &opcodes, offs_t &p);

	void assert_hd6309_exclusive()
	{
		if (!(m_level & HD6309_EXCLUSIVE))
			throw false;
	}
};

class m6x09_disassembler : public m6x09_base_disassembler
{
public:
	m6x09_disassembler(uint32_t level, const char teregs[16][4]);

protected:
	virtual void indexed(std::ostream &stream, uint8_t pb, const data_buffer &params, offs_t &p) override;
	virtual void register_register(std::ostream &stream, uint8_t pb) override;

private:
	static const opcodeinfo m6x09_opcodes[];
	const std::array<std::array<char, 4>, 16> &m_teregs;
};


class konami_disassembler : public m6x09_base_disassembler
{
public:
	konami_disassembler();

protected:
	virtual void indexed(std::ostream &stream, uint8_t pb, const data_buffer &params, offs_t &p) override;
	virtual void register_register(std::ostream &stream, uint8_t pb) override;

private:
	static const opcodeinfo konami_opcodes[];
};

class m6809_disassembler : public m6x09_disassembler
{
public:
	m6809_disassembler();

private:
	static const char m6809_teregs[16][4];
};

class hd6309_disassembler : public m6x09_disassembler
{
public:
	hd6309_disassembler();

private:
	static const char hd6309_teregs[16][4];
};

#endif
