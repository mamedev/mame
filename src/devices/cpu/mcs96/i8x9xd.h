// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8x9x.h

    MCS96, 8x9x branch, the original version

***************************************************************************/

#ifndef MAME_CPU_MCS96_I8X9XD_H
#define MAME_CPU_MCS96_I8X9XD_H

#include "mcs96d.h"

class i8x9x_disassembler : public mcs96_disassembler
{
public:
	i8x9x_disassembler();
	virtual ~i8x9x_disassembler() = default;

protected:
	virtual std::string regname8(uint8_t reg, bool is_dest) const override;
	virtual std::string regname16(uint8_t reg, bool is_dest) const override;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
