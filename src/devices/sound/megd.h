// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha MEG - Multiple effects generator
//
// Audio dsp dedicated to effects generation
//
// Disassembler

#ifndef DEVICES_SOUND_MEGD_H
#define DEVICES_SOUND_MEGD_H

#pragma once

class meg_disassembler : public util::disasm_interface
{
public:
	class info {
	public:
		virtual u16 fp_r(u16 address) const = 0;
		virtual u16 offset_r(u16 address) const = 0;
	};

	meg_disassembler(info *inf = nullptr);

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	info *m_info;

	std::string gfp(offs_t address) const;
	std::string goffset(offs_t address) const;

	static inline u32 b(u64 opc, u32 start, u32 count);
	static inline void append(std::string &r, std::string e);
};

#endif
