// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/**********************************************************
 *   Xerox AltoII disassembler
 *
 **********************************************************/

#ifndef MAME_CPU_ALTO2_ALTO2DSM_H
#define MAME_CPU_ALTO2_ALTO2DSM_H

#pragma once

class alto2_disassembler : public util::disasm_interface
{
public:
	alto2_disassembler() = default;
	virtual ~alto2_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const taskname[16];
	static const char *const regname[32];
	static const char *const t_bus_alu[16];
	static uint16_t const_prom[];

	std::string addrname(int a) const;
};

#endif
