// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8xc196.h

    MCS96, c196 branch, the enhanced 16 bits bus version

***************************************************************************/

#ifndef MAME_CPU_MCS96_I8XC196D_H
#define MAME_CPU_MCS96_I8XC196D_H

#include "mcs96d.h"

class i8xc196_disassembler : public mcs96_disassembler
{
public:
	i8xc196_disassembler();
	virtual ~i8xc196_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
