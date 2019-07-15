// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    57002dsm.h

    TMS57002 "DASP" emulator.

***************************************************************************/

#ifndef MAME_CPU_TMS57002_57002DSM_H
#define MAME_CPU_TMS57002_57002DSM_H

#pragma once

class tms57002_disassembler : public util::disasm_interface
{
public:
	tms57002_disassembler();
	virtual ~tms57002_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static std::string get_memadr(u32 opcode, char type);

};

#endif
