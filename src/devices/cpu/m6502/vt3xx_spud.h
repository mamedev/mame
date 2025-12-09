// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    vt3xx_spud.h

***************************************************************************/

#ifndef MAME_CPU_M6502_VT3XX_SPUD_H
#define MAME_CPU_M6502_VT3XX_SPUD_H

#pragma once

#include "m6502d.h"

class vt3xx_spu_disassembler : public m6502_base_disassembler
{
public:
	vt3xx_spu_disassembler();
	virtual ~vt3xx_spu_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
