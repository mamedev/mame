// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    dsp56dsm.cpp
    Disassembler for the portable Motorola/Freescale DSP56156 emulator.
    Written by Andrew Gardner

***************************************************************************/

#ifndef MAME_CPU_DSP56156_DSP56156DSM_H
#define MAME_CPU_DSP56156_DSP56156DSM_H

#pragma once

class dsp56156_disassembler : public util::disasm_interface
{
public:
	dsp56156_disassembler() = default;
	virtual ~dsp56156_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
