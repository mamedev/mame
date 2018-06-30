// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    xavixd.h

***************************************************************************/

#ifndef MAME_CPU_M6502_XAVIXD_H
#define MAME_CPU_M6502_XAVIXD_H

#pragma once

#include "m6502d.h"

class xavix_disassembler : public m6502_base_disassembler
{
public:
	xavix_disassembler();
	virtual ~xavix_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
