// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Felipe Sanches
/*******************************************************************

Toshiba TLCS-900/H disassembly

*******************************************************************/

#ifndef MAME_CPU_TLCS900_DASM900_H
#define MAME_CPU_TLCS900_DASM900_H

#pragma once

#include <utility>


class tlcs900_disassembler : public util::disasm_interface
{
public:
	// Control register symbol entry for model-specific LDC operand names
	struct cr_sym { u8 size; u8 encoding; char const *name; };

	tlcs900_disassembler();
	template <size_t N>
	tlcs900_disassembler(std::pair<u16, char const *> const (&symbols)[N])
		: tlcs900_disassembler(symbols, N)
	{
	}
	template <size_t N, size_t CRN>
	tlcs900_disassembler(std::pair<u16, char const *> const (&symbols)[N], cr_sym const (&cr_symbols)[CRN])
		: tlcs900_disassembler(symbols, N, cr_symbols, CRN)
	{
	}

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	std::pair<u16, char const *> const *m_symbols;
	std::size_t m_symbol_count;
	cr_sym const *m_cr_symbols;
	std::size_t m_cr_symbol_count;

	tlcs900_disassembler(std::pair<u16, char const *> const symbols[], std::size_t symbol_count)
		: m_symbols(symbols), m_symbol_count(symbol_count), m_cr_symbols(nullptr), m_cr_symbol_count(0)
	{
	}
	tlcs900_disassembler(std::pair<u16, char const *> const symbols[], std::size_t symbol_count,
						 cr_sym const cr_symbols[], std::size_t cr_symbol_count)
		: m_symbols(symbols), m_symbol_count(symbol_count), m_cr_symbols(cr_symbols), m_cr_symbol_count(cr_symbol_count)
	{
	}

	template <typename T> std::string address(T offset, int size) const;
	char const *cr_name(u8 size, u8 encoding) const;
};

#endif // MAME_CPU_TLCS900_DASM900_H
