// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_CPU_M6502_R65C19D_H
#define MAME_CPU_M6502_R65C19D_H

#pragma once

#include "m6502d.h"

class r65c19_disassembler : public m6502_base_disassembler
{
public:
	r65c19_disassembler();
	virtual ~r65c19_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
