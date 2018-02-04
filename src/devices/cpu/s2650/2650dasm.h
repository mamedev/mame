// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/***************************************************************************
 *
 *  Portable Signetics 2650 disassembler
 *
 *  Written by J. Buchmueller (pullmoll@t-online.de)
 *  for the MAME project
 *
 **************************************************************************/

#ifndef MAME_CPU_S2650_2650DASM_H
#define MAME_CPU_S2650_2650DASM_H

#pragma once

class s2650_disassembler : public util::disasm_interface
{
public:
	struct config {
		virtual ~config() = default;
		virtual bool get_z80_mnemonics_mode() const = 0;
	};

	s2650_disassembler(config *conf);
	virtual ~s2650_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const int rel[0x100];
	static const char cc[4];

	void add(std::string &buf, std::string str);
	std::string SYM(int addr);
	std::string IMM(offs_t pc, const data_buffer &params);
	std::string IMM_PSL(offs_t pc, const data_buffer &params);
	std::string IMM_PSU(offs_t pc, const data_buffer &params);
	std::string REL(offs_t pc, const data_buffer &params);
	std::string REL0(offs_t pc, const data_buffer &params);
	std::string ABS(int load, int r, offs_t pc, const data_buffer &params);
	std::string ADR(offs_t pc, const data_buffer &params);

	config *m_config;
};

#endif
