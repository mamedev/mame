// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

    Sanyo LC8670 disassembler

******************************************************************************/

#ifndef MAME_CPU_LC8670_LC8670DSM_H
#define MAME_CPU_LC8670_LC8670DSM_H

#pragma once

class lc8670_disassembler : public util::disasm_interface
{
public:
	lc8670_disassembler() = default;
	virtual ~lc8670_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	void dasm_arg(uint8_t op, std::ostream &buffer, offs_t pc, int arg, const data_buffer &opcodes, offs_t &pos);
};

#endif
