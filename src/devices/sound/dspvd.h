// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha DSPV
//
// Audio dsp dedicated to acoustic simulation
//
// Disassembler

#ifndef MAME_SOUND_DSPVD_H
#define MAME_SOUND_DSPVD_H

#pragma once

class dspv_disassembler : public util::disasm_interface
{
public:
	dspv_disassembler();

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_SOUND_DSPVD_H
