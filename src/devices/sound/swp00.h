// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP00, rompler/dsp combo

#ifndef DEVICES_SOUND_SWP00_H
#define DEVICES_SOUND_SWP00_H

#pragma once

#include "meg.h"
#include "dirom.h"

class swp00_device : public device_t, public device_sound_interface, public device_rom_interface<24, 0, 0, ENDIANNESS_LITTLE>
{
public:
	swp00_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 33868800);

	void map(address_map &map);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
	virtual void rom_bank_pre_change() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	//  required_device<meg_embedded_device> m_meg;

	sound_stream *m_stream;

	static const std::array<s32, 0x80> attack_linear_step;
	static const std::array<s32, 0x20> decay_linear_step;
	static const std::array<s32, 16> panmap;
	std::array<s32, 0x80> m_global_step;
	std::array<s16, 0x100> m_sample_log8;

	static const std::array<u32, 4> lfo_shape_centered_saw;
	static const std::array<u32, 4> lfo_shape_centered_tri;
	static const std::array<u32, 4> lfo_shape_offset_saw;
	static const std::array<u32, 4> lfo_shape_offset_tri;

	std::array<u16, 0x40> m_intreg;
	std::array<u16, 0x300> m_fpreg;

	std::array<u16, 0x20>  m_lpf_info;
	std::array<u8,  0x20>  m_lpf_speed;
	std::array<u8,  0x20>  m_lfo_famod_depth;
	std::array<u8,  0x20>  m_rev_level;
	std::array<u8,  0x20>  m_dry_level;
	std::array<u8,  0x20>  m_cho_level;
	std::array<u8,  0x20>  m_var_level;
	std::array<u8,  0x20>  m_glo_level;
	std::array<u8,  0x20>  m_panning;
	std::array<u8,  0x20>  m_attack_speed;
	std::array<u8,  0x20>  m_attack_level;
	std::array<u8,  0x20>  m_decay_speed;
	std::array<u8,  0x20>  m_decay_level;
	std::array<u16, 0x20>  m_pitch;
	std::array<u16, 0x20>  m_sample_start;
	std::array<u16, 0x20>  m_sample_end;
	std::array<u8,  0x20>  m_sample_dec_and_format;
	std::array<u32, 0x20>  m_sample_address;
	std::array<u8,  0x20>  m_lfo_step;
	std::array<u8,  0x20>  m_lfo_pmod_depth;

	std::array<u32, 0x20>  m_lfo_phase;
	std::array<s32, 0x20>  m_sample_pos;
	std::array<u32, 0x20>  m_sample_increment;
	std::array<s32, 0x20>  m_envelope_level;
	std::array<s32, 0x20>  m_glo_level_cur;
	std::array<s32, 0x20>  m_pan_l;
	std::array<s32, 0x20>  m_pan_r;
	std::array<s32, 0x20>  m_lpf_feedback;
	std::array<s32, 0x20>  m_lpf_target_value;
	std::array<s32, 0x20>  m_lpf_value;
	std::array<s32, 0x20>  m_lpf_timer;
	std::array<s32, 0x20>  m_lpf_ha;
	std::array<s32, 0x20>  m_lpf_hb;	
	std::array<bool, 0x20> m_active, m_decay, m_decay_done, m_lpf_done;

	u16 m_waverom_val;
	u8 m_waverom_access;
	u8 m_state_adr;

	// Voice control

	template<int sel> void lpf_info_w(offs_t offset, u8 data);
	template<int sel> u8 lpf_info_r(offs_t offset);
	void lpf_speed_w(offs_t offset, u8 data);
	u8 lpf_speed_r(offs_t offset);
	void lfo_famod_depth_w(offs_t offset, u8 data);
	u8 lfo_famod_depth_r(offs_t offset);
	void rev_level_w(offs_t offset, u8 data);
	u8 rev_level_r(offs_t offset);
	void dry_level_w(offs_t offset, u8 data);
	u8 dry_level_r(offs_t offset);
	void cho_level_w(offs_t offset, u8 data);
	u8 cho_level_r(offs_t offset);
	void var_level_w(offs_t offset, u8 data);
	u8 var_level_r(offs_t offset);
	void glo_level_w(offs_t offset, u8 data);
	u8 glo_level_r(offs_t offset);
	void panning_w(offs_t offset, u8 data);
	u8 panning_r(offs_t offset);
	void attack_speed_w(offs_t offset, u8 data);
	u8 attack_speed_r(offs_t offset);
	void attack_level_w(offs_t offset, u8 data);
	u8 attack_level_r(offs_t offset);
	void decay_speed_w(offs_t offset, u8 data);
	u8 decay_speed_r(offs_t offset);
	void decay_level_w(offs_t offset, u8 data);
	u8 decay_level_r(offs_t offset);
	template<int sel> void pitch_w(offs_t offset, u8 data);
	template<int sel> u8 pitch_r(offs_t offset);
	template<int sel> void sample_start_w(offs_t offset, u8 data);
	template<int sel> u8 sample_start_r(offs_t offset);
	template<int sel> void sample_end_w(offs_t offset, u8 data);
	template<int sel> u8 sample_end_r(offs_t offset);
	void sample_dec_and_format_w(offs_t offset, u8 data);
	u8 sample_dec_and_format_r(offs_t offset);
	template<int sel> void sample_address_w(offs_t offset, u8 data);
	template<int sel> u8 sample_address_r(offs_t offset);
	void lfo_step_w(offs_t offset, u8 data);
	u8 lfo_step_r(offs_t offset);
	void lfo_pmod_depth_w(offs_t offset, u8 data);
	u8 lfo_pmod_depth_r(offs_t offset);

	// Internal state access
	u8 state_r();
	void state_adr_w(u8 data);

	// MEG variables
	void intreg_w(offs_t offset, u8 data);
	u8 intreg_r(offs_t offset);
	void fpreg_w(offs_t offset, u8 data);
	u8 fpreg_r(offs_t offset);

	// Control registers
	void keyon(int chan);
	template<int sel> void keyon_w(u8 data);

	void waverom_access_w(u8 data);
	u8 waverom_access_r();
	u8 waverom_val_r();

	// Generic catch-all
	u8 snd_r(offs_t offset);
	void snd_w(offs_t offset, u8 data);

	inline auto &rany(address_map &map, int chan, int idx) {
		int slot = (chan << 6) | idx;
		return map(slot, slot);
	}

	inline auto &rchan(address_map &map, int idx) {
		int slot = ((idx & 0x3e) << 5) | (idx & 1);
		return map(slot, slot).select(0x3e);
	}

	// Other methods
	static bool istep(s32 &value, s32 limit, s32 step);
	static bool fpstep(s32 &value, s32 limit, s32 step);
	static s32 fpadd(s32 value, s32 step);
	static s32 fpsub(s32 value, s32 step);
	static s16 fpapply(s32 value, s16 sample);
	static s16 lpffpapply(s32 value, s16 sample);
};

DECLARE_DEVICE_TYPE(SWP00, swp00_device)

#endif
