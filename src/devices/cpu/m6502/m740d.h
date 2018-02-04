// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m740d.h

    Mitsubishi M740 series (M507xx/M509xx), disassembler

***************************************************************************/

#ifndef MAME_CPU_M6502_M740D_H
#define MAME_CPU_M6502_M740D_H

#pragma once

#include "m6502d.h"

class m740_disassembler : public m6502_base_disassembler
{
public:
	struct config {
		virtual ~config() = default;
		virtual u32 get_state_base() const = 0;
	};

	m740_disassembler(config *conf);
	virtual ~m740_disassembler() = default;

protected:
	virtual u32 get_instruction_bank() const override;

private:
	static const disasm_entry disasm_entries[0x200];
	config *conf;
};

#endif
