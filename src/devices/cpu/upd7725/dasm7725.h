// license:BSD-3-Clause
// copyright-holders:R. Belmont,byuu
/***************************************************************************

    dasm7725.c
    Disassembler for the portable uPD7725 emulator.
    Written by byuu
    MAME conversion by R. Belmont

***************************************************************************/

#ifndef MAME_CPU_UPD7725_DASM7725_H
#define MAME_CPU_UPD7725_DASM7725_H

#pragma once

class necdsp_disassembler : public util::disasm_interface
{
public:
	necdsp_disassembler() = default;
	virtual ~necdsp_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
