// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    dsp56dsm.c
    Disassembler for the portable Motorola/Freescale dsp56k emulator.
    Written by Andrew Gardner

***************************************************************************/

#ifndef MAME_CPU_DSP56K_DSP56DSM_H
#define MAME_CPU_DSP56K_DSP56DSM_H

#pragma once

class dsp56k_disassembler : public util::disasm_interface
{
public:
	dsp56k_disassembler() = default;
	virtual ~dsp56k_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
