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
	tlcs900_disassembler();
	template <size_t N>
	tlcs900_disassembler(std::pair<u16, char const *> const (&symbols)[N])
		: tlcs900_disassembler(symbols, N)
	{
	}

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	std::pair<u16, char const *> const *m_symbols;
	std::size_t m_symbol_count;

	tlcs900_disassembler(std::pair<u16, char const *> const symbols[], std::size_t symbol_count)
		: m_symbols(symbols), m_symbol_count(symbol_count)
	{
	}

	template <typename T> std::string address(T offset, int size) const;
};

#endif // MAME_CPU_TLCS900_DASM900_H
