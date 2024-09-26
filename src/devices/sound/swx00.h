// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWX00, rompler/dsp/cpu combo, audio support

#ifndef DEVICES_SOUND_SWX00_H
#define DEVICES_SOUND_SWX00_H

#pragma once

#include "meg.h"
#include "dirom.h"

class swx00_sound_device : public device_t, public device_sound_interface, public device_rom_interface<24, 1, -1, ENDIANNESS_BIG>
{
public:
	swx00_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	sound_stream *m_stream;

	static const std::array<s32, 0x80> attack_linear_step;
	static const std::array<s32, 0x20> decay_linear_step;
	static const std::array<s32, 16> panmap;
	static const std::array<u8, 4> dpcm_offset;
	std::array<s32,  0x80> m_global_step;
	std::array<s16, 0x100> m_dpcm;

	std::array<u16,  0x20> m_sample_start;
	std::array<u16,  0x20> m_sample_end;
	std::array<u32,  0x20> m_sample_address;
	std::array<u16,  0x20> m_sample_pitch;

	std::array<u16,  0x20> m_attack;
	std::array<u16,  0x20> m_decay;

	std::array<u16,  0x20> m_glo_pan;
	std::array<u16,  0x20> m_rev_dry;
	std::array<u16,  0x20> m_cho_var;

	std::array<u32,  0x40> m_dsp_offsets;
	std::array<u16, 0x120> m_dsp_values;

	std::array<s32,  0x20> m_sample_pos;
	std::array<s32,  0x20> m_envelope_level;
	std::array<s32,  0x20> m_glo_level_cur;
	std::array<s32,  0x20> m_pan_l;
	std::array<s32,  0x20> m_pan_r;
	std::array<bool, 0x20> m_active, m_decay_on, m_decay_done;
	std::array<s16, 0x20>  m_dpcm_current;
	std::array<s16, 0x20>  m_dpcm_next;
	std::array<u32, 0x20>  m_dpcm_address;
	std::array<s32, 0x20>  m_dpcm_sum;

	u32 m_keyon;
	u32 m_rom_address;
	u16 m_rom_read_status;
	u8 m_state_sel;
	u8 m_dsp_offh, m_dsp_valh;

	// Generic catch-all
	u16 snd_r(offs_t offset);
	void snd_w(offs_t offset, u16 data, u16 mem_mask);

	inline auto &rchan(address_map &map, int idx) {
		return map(idx*0x40, idx*0x40+1).select(0x3e);
	}

	u16 sample_start_r(offs_t chan);
	void sample_start_w(offs_t chan, u16 data, u16 mem_mask);
	u16 sample_end_r(offs_t chan);
	void sample_end_w(offs_t chan, u16 data, u16 mem_mask);
	u16 sample_address_h_r(offs_t chan);
	u16 sample_address_l_r(offs_t chan);
	void sample_address_h_w(offs_t chan, u16 data, u16 mem_mask);
	void sample_address_l_w(offs_t chan, u16 data, u16 mem_mask);
	u16 sample_pitch_r(offs_t chan);
	void sample_pitch_w(offs_t chan, u16 data, u16 mem_mask);

	u16 glo_pan_r(offs_t chan);
	void glo_pan_w(offs_t chan, u16 data, u16 mem_mask);
	u16 rev_dry_r(offs_t chan);
	void rev_dry_w(offs_t chan, u16 data, u16 mem_mask);
	u16 cho_var_r(offs_t chan);
	void cho_var_w(offs_t chan, u16 data, u16 mem_mask);

	u16 attack_r(offs_t chan);
	void attack_w(offs_t chan, u16 data, u16 mem_mask);
	u16 decay_r(offs_t chan);
	void decay_w(offs_t chan, u16 data, u16 mem_mask);

	void keyon_w(offs_t offset, u8 data);
	void keyon_commit_w(u8);

	void dsp_offh_w(u8 data);
	void dsp_offl_w(offs_t reg, u8 data);
	void dsp_valh_w(u8 data);
	void dsp_vall_w(offs_t reg, u8 data);

	void rom_read_adrh_w(offs_t, u16 data, u16 mem_mask);
	void rom_read_adrl_w(offs_t, u16 data, u16 mem_mask);
	u16 rom_read_status_r();
	u16 rom_read_r();
	void rom_read_w(u16);

	void state_sel_w(u8 data);
	u8 state_r();

	static bool istep(s32 &value, s32 limit, s32 step);
	static bool fpstep(s32 &value, s32 limit, s32 step);
	static s32 fpadd(s32 value, s32 step);
	static s32 fpsub(s32 value, s32 step);
	static s32 fpapply(s32 value, s32 sample);
};

DECLARE_DEVICE_TYPE(SWX00_SOUND, swx00_sound_device)

#endif
