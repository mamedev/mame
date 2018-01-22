// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************
 *
 *   es5510.c - Ensoniq ES5510 (ESP) emulation
 *   by Christian Brunschen
 *
 ***************************************************************************/

#ifndef MAME_CPU_ES5510_ES5510DASM_H
#define MAME_CPU_ES5510_ES5510DASM_H

#pragma once

class es5510_disassembler : public util::disasm_interface
{
public:
	es5510_disassembler() = default;
	virtual ~es5510_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
