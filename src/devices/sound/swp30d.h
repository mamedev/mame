// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP30 - Multiple effects generator subpart
//
// Audio dsp dedicated to effects generation
//
// Disassembler

#ifndef MAME_SOUND_SWP30D_H
#define MAME_SOUND_SWP30D_H

#pragma once

class swp30_disassembler : public util::disasm_interface
{
public:
	class info {
	public:
		virtual u16 swp30d_const_r(u16 address) const = 0;
		virtual u16 swp30d_offset_r(u16 address) const = 0;
	};

	swp30_disassembler(info *inf = nullptr);

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	info *m_info;

	std::string gconst(offs_t address) const;
	std::string goffset(offs_t address) const;

	static inline u32 b(u64 opc, u32 start, u32 count);
	static inline void append(std::string &r, const std::string &e);
};

#endif // MAME_SOUND_SWP30D_H
