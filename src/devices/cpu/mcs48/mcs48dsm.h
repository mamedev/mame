// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mcs48dsm.c

    Simple MCS-48/UPI-41 disassembler.
    Written by Aaron Giles

***************************************************************************/


#ifndef MAME_CPU_MCS48_MCS48DASM_H
#define MAME_CPU_MCS48_MCS48DASM_H

#pragma once

class mcs48_disassembler : public util::disasm_interface
{
public:
	mcs48_disassembler(bool upi41);
	virtual ~mcs48_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	bool m_upi41;
};

#endif
