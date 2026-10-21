// license:BSD-3-Clause
// copyright-holders:R. Belmont,byuu,Jonathan Gevaryahu
/***************************************************************************

    dasm7720.h
    Disassembler for the portable uPD7720 emulator.
    Written by byuu
    MAME conversion by R. Belmont
    Adapted for uPD7720 by Lord Nightmare

***************************************************************************/

#ifndef MAME_CPU_UPD7725_DASM7720_H
#define MAME_CPU_UPD7725_DASM7720_H

#pragma once

class upd7720_disassembler : public util::disasm_interface
{
public:
	upd7720_disassembler() = default;
	virtual ~upd7720_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
