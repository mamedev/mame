// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

    gt913d.h

    GT913 base cpu emulation, disassembler

***************************************************************************/

#ifndef MAME_CPU_H8_GT913D_H
#define MAME_CPU_H8_GT913D_H

#pragma once

#include "h8d.h"

class gt913_disassembler : public h8_disassembler
{
public:
	gt913_disassembler();
	virtual ~gt913_disassembler() = default;

protected:
	static const disasm_entry disasm_entries[];
};

#endif
