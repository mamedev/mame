// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8hd.h

    H8-300H base cpu emulation, disassembler

***************************************************************************/

#ifndef MAME_CPU_H8_H8HD_H
#define MAME_CPU_H8_H8HD_H

#pragma once

#include "h8d.h"

class h8h_disassembler : public h8_disassembler
{
public:
	h8h_disassembler();
	virtual ~h8h_disassembler() = default;

protected:
	static const disasm_entry disasm_entries[];
};

#endif
