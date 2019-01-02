// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    xavix2000d.h

***************************************************************************/

#ifndef MAME_CPU_M6502_XAVIX2000D_H
#define MAME_CPU_M6502_XAVIX2000D_H

#pragma once

#include "m6502d.h"

class xavix2000_disassembler : public m6502_base_disassembler
{
public:
	xavix2000_disassembler();
	virtual ~xavix2000_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
