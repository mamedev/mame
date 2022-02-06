// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP30/30B, rompler/dsp combo

#ifndef DEVICES_SOUND_SWP30_H
#define DEVICES_SOUND_SWP30_H

#pragma once

#include "meg.h"
#include "dirom.h"

class swp30_device : public device_t, public device_sound_interface, public device_rom_interface<25+2, 2, 0, ENDIANNESS_LITTLE>
{
public:
	swp30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 33868800);

	void map(address_map &map);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
	virtual void rom_bank_updated() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	enum {
		IDLE,
		ATTACK,
		DECAY,
		SUSTAIN,
		RELEASE
	};

	required_device<meg_embedded_device> m_meg;

	sound_stream *m_stream;

	s32 m_sample_increment[0x4000];
	s32 m_linear_attenuation[0x100];
	s16 m_sample_log8[0x100];

	u64 m_program[0x180];
	u64 m_keyon_mask;
	u32 m_pre_size[0x40], m_post_size[0x40], m_address[0x40];

	s32 m_sample_pos[0x40];
	s32 m_sample_history[0x40][2][2];
	u32 m_current_volume[0x40], m_target_volume[0x40];
	s32 m_step_volume[0x40];

	u32 m_waverom_adr, m_waverom_mode, m_waverom_val;
	u16 m_waverom_access;

	u16 m_program_pfp[0x180], m_program_pint[0x80], m_program_plfo[0x80];

	u16 m_base_volume[0x40], m_freq[0x40], m_pan[0x40], m_dry_rev[0x40], m_cho_var[0x40];
	u16 m_attack[0x40], m_decay[0x40], m_release[0x40];
	u16 m_lpf_cutoff[0x40], m_lpf_cutoff_inc[0x40], m_lpf_reso[0x40], m_hpf_cutoff[0x40];
	s16 m_eq_filter[0x40][6];
	u16 m_routing[0x40][3];
	u16 m_map[8];

	u16 m_internal_adr;

	u16 m_program_address;

	u8 m_mode[0x40];

	// AWM2 per-channel registers
	u16 lpf_cutoff_r(offs_t offset);
	void lpf_cutoff_w(offs_t offset, u16 data);
	u16 lpf_cutoff_inc_r(offs_t offset);
	void lpf_cutoff_inc_w(offs_t offset, u16 data);
	u16 hpf_cutoff_r(offs_t offset);
	void hpf_cutoff_w(offs_t offset, u16 data);
	u16 lpf_reso_r(offs_t offset);
	void lpf_reso_w(offs_t offset, u16 data);
	u16 attack_r(offs_t offset);
	void attack_w(offs_t offset, u16 data);
	u16 decay_r(offs_t offset);
	void decay_w(offs_t offset, u16 data);
	u16 release_r(offs_t offset);
	void release_w(offs_t offset, u16 data);
	template<int coef> u16 eq_filter_r(offs_t offset);
	template<int coef> void eq_filter_w(offs_t offset, u16 data);
	u16 base_volume_r(offs_t offset);
	void base_volume_w(offs_t offset, u16 data);
	u16 freq_r(offs_t offset);
	void freq_w(offs_t offset, u16 data);
	u16 pre_size_h_r(offs_t offset);
	u16 pre_size_l_r(offs_t offset);
	void pre_size_h_w(offs_t offset, u16 data);
	void pre_size_l_w(offs_t offset, u16 data);
	u16 post_size_h_r(offs_t offset);
	u16 post_size_l_r(offs_t offset);
	void post_size_h_w(offs_t offset, u16 data);
	void post_size_l_w(offs_t offset, u16 data);
	u16 address_h_r(offs_t offset);
	u16 address_l_r(offs_t offset);
	void address_h_w(offs_t offset, u16 data);
	void address_l_w(offs_t offset, u16 data);
	u16 pan_r(offs_t offset);
	void pan_w(offs_t offset, u16 data);
	u16 dry_rev_r(offs_t offset);
	void dry_rev_w(offs_t offset, u16 data);
	u16 cho_var_r(offs_t offset);
	void cho_var_w(offs_t offset, u16 data);
	u16 internal_adr_r();
	void internal_adr_w(u16 data);
	u16 internal_r();
	template<int sel> u16 routing_r(offs_t offset);
	template<int sel> void routing_w(offs_t offset, u16 data);

	// Envelope control
	void change_mode(int channel, u8 mode);

	// Control registers
	template<int sel> u16 keyon_mask_r();
	template<int sel> void keyon_mask_w(u16 data);
	u16 keyon_r();
	void keyon_w(u16);
	u16 prg_address_r();
	void prg_address_w(u16 data);
	template<int sel> u16 prg_r();
	template<int sel> void prg_w(u16 data);
	template<int sel> u16 map_r();
	template<int sel> void map_w(u16 data);
	template<int sel> void waverom_adr_w(u16 data);
	template<int sel> u16 waverom_adr_r();
	template<int sel> void waverom_mode_w(u16 data);
	template<int sel> u16 waverom_mode_r();
	void waverom_access_w(u16 data);
	u16 waverom_access_r();
	u16 waverom_busy_r();
	template<int sel> u16 waverom_val_r();

	// MEG registers
	template<int sel> u16 prg_fp_r(offs_t offset);
	template<int sel> void prg_fp_w(offs_t offset, u16 data);
	template<int sel> u16 prg_off_r(offs_t offset);
	template<int sel> void prg_off_w(offs_t offset, u16 data);
	template<int sel> u16 prg_lfo_r(offs_t offset);
	template<int sel> void prg_lfo_w(offs_t offset, u16 data);


	// Generic catch-all
	u16 snd_r(offs_t offset);
	void snd_w(offs_t offset, u16 data);

	inline auto &rchan(address_map &map, int idx) {
		return map(idx*2, idx*2+1).select(0x1f80);
	}

	inline auto &rctrl(address_map &map, int idx) {
		int slot = 0x40*(idx >> 1) | 0xe | (idx & 1);
		return map(slot*2, slot*2+1);
	}
};

DECLARE_DEVICE_TYPE(SWP30, swp30_device)

#endif
