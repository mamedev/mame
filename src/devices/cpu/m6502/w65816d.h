// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    w65816d.h

    WDC W65C816S, disassembler

    The 65816 decodes differently depending on the emulation flag and the M
    and X width flags, so the disassembler needs the CPU's current instruction
    bank the same way m740_disassembler does.

***************************************************************************/

#ifndef MAME_CPU_M6502_W65816D_H
#define MAME_CPU_M6502_W65816D_H

#pragma once

#include "m6502d.h"

class w65816_disassembler : public m6502_base_disassembler
{
public:
	struct config {
		virtual ~config() = default;
		virtual u32 get_state_base() const = 0;
	};

	w65816_disassembler(config *conf);
	virtual ~w65816_disassembler() = default;

protected:
	virtual u32 get_instruction_bank() const override;

private:
	static const disasm_entry disasm_entries[0x500];
	config *m_conf;
};

#endif // MAME_CPU_M6502_W65816D_H
