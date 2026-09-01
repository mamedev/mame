// license:BSD-3-Clause
// copyright-holders:superctr
#ifndef MAME_SOUND_ROLAND_XPD_H
#define MAME_SOUND_ROLAND_XPD_H

#pragma once

class roland_xp_disassembler : public util::disasm_interface
{
public:
	// the coefficient of a slot lives in a second memory the disassembler cannot address
	class info
	{
	public:
		virtual ~info() = default;

		virtual u16 xpd_cram_r(offs_t address) const = 0;
		virtual int xpd_ramp_base() const = 0;
	};

	roland_xp_disassembler(info *inf = nullptr) : m_info(inf) { }
	virtual ~roland_xp_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	info *m_info;

	bool has_coefficient() const { return m_info != nullptr; }
	u16 coefficient_word(offs_t pc) const { return m_info ? m_info->xpd_cram_r(pc) : 0; }
	int ramp_base() const { return m_info ? m_info->xpd_ramp_base() : 0xf0; }
	std::string coefficient(offs_t pc) const;
	std::string constant(offs_t pc) const;

	static bool continuation(offs_t pc, const data_buffer &opcodes);
	static void append(std::string &r, const std::string &e);
	static void function(std::string &r, int fn, int mode, const std::string &k);
	void parallel(std::string &r, offs_t pc, int st, int word) const;
};

#endif // MAME_SOUND_ROLAND_XPD_H
