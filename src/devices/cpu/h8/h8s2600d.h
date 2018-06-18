// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8s2600d.h

    H8S-2600 base cpu emulation, disassembler

***************************************************************************/

#ifndef MAME_CPU_H8_H8S2600D_H
#define MAME_CPU_H8_H8S2600D_H

#pragma once

#include "h8d.h"

class h8s2600_disassembler : public h8_disassembler
{
public:
	h8s2600_disassembler();
	virtual ~h8s2600_disassembler() = default;

protected:
	static const disasm_entry disasm_entries[];
};

#endif
