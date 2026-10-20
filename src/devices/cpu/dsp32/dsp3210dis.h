// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    dsp3210dis.h
    Disassembler for the AT&T DSP3210.

***************************************************************************/

#ifndef MAME_CPU_DSP32_DSP3210DIS_H
#define MAME_CPU_DSP32_DSP3210DIS_H

#pragma once

class dsp3210_disassembler : public util::disasm_interface
{
public:
	dsp3210_disassembler() = default;
	virtual ~dsp3210_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_DSP32_DSP3210DIS_H
