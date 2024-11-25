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

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
	virtual void rom_bank_pre_change() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	template<size_t size> struct delay_block {
		swp00_device *m_swp;
		std::array<s32, size> &m_buffer;

		delay_block(swp00_device *swp, std::array<s32, size> &buffer);
		s32 r(int offreg) const;
		s32 rlfo(int offreg, u32 phase, s32 delta_phase, int levelreg) const;
		s32 rlfo2(int offreg, s32 offset) const;
		void w(int offreg, s32 value) const;
	};

	sound_stream *m_stream;

	static const std::array<s32, 0x80> attack_linear_step;
	static const std::array<s32, 0x20> decay_linear_step;
	static const std::array<s32, 16> panmap;
	static const std::array<u8, 4> dpcm_offset;
	std::array<s32, 0x80> m_global_step;
	std::array<s16, 0x100> m_dpcm;

	static const std::array<u32, 4> lfo_shape_centered_saw;
	static const std::array<u32, 4> lfo_shape_centered_tri;
	static const std::array<u32, 4> lfo_shape_offset_saw;
	static const std::array<u32, 4> lfo_shape_offset_tri;

	// MEG reverb memory
	std::array<s32, 0x20000> m_rev_buffer;
	std::array<s32, 0x8000>  m_cho_buffer;
	std::array<s32, 0x20000> m_var_buffer;

	// MEG registers
	std::array<u16, 0x40>  m_offset;
	std::array<u16, 0xc0>  m_const;

	// AWM registers
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
	std::array<u8,  0x20>  m_sample_dpcm_and_format;
	std::array<u32, 0x20>  m_sample_address;
	std::array<u8,  0x20>  m_lfo_step;
	std::array<u8,  0x20>  m_lfo_pmod_depth;

	std::array<u32, 0x20>  m_lfo_phase;
	std::array<s32, 0x20>  m_sample_pos;
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
	std::array<s16, 0x20>  m_dpcm_current;
	std::array<s16, 0x20>  m_dpcm_next;
	std::array<u32, 0x20>  m_dpcm_address;
	std::array<s32, 0x20>  m_dpcm_sum;

	u16 m_waverom_val;
	u8 m_waverom_access;
	u8 m_state_adr;
	u8 m_meg_control;

	// MEG state
	u32 m_buffer_offset;
	s32 m_rev_vol, m_cho_vol, m_var_vol;

	u32 m_var_lfo_phase;

	s32 m_var_lfo_h_1, m_var_lfo_h_2;
	s32 m_var_lfo1a, m_var_lfo2a, m_var_lfo3a, m_var_lfo4a;

	s32 m_var_filter_1, m_var_filter_2, m_var_filter_3;
	s32 m_var_filter_l_1, m_var_filter_l_2, m_var_filter_l_3;
	s32 m_var_filter_r_1, m_var_filter_r_2, m_var_filter_r_3;
	s32 m_var_filter2_1, m_var_filter2_2a, m_var_filter2_2b, m_var_filter2_3a, m_var_filter2_3b, m_var_filter2_4;
	s32 m_var_filter3_1, m_var_filter3_2;

	s32 m_var_filterp_l_1, m_var_filterp_l_2, m_var_filterp_l_3;
	s32 m_var_filterp_l_4, m_var_filterp_l_5, m_var_filterp_l_6;
	s32 m_var_filterp_r_1, m_var_filterp_r_2, m_var_filterp_r_3;
	s32 m_var_filterp_r_4, m_var_filterp_r_5, m_var_filterp_r_6;

	s32 m_var_h1, m_var_h2, m_var_h3, m_var_h4;

	u32 m_cho_lfo_phase;
	s32 m_cho_filter_l_1, m_cho_filter_l_2, m_cho_filter_l_3;
	s32 m_cho_filter_r_1, m_cho_filter_r_2, m_cho_filter_r_3;

	s32 m_rev_filter_1, m_rev_filter_2, m_rev_filter_3;
	s32 m_rev_hist_a, m_rev_hist_b, m_rev_hist_c, m_rev_hist_d;


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
	void sample_dpcm_and_format_w(offs_t offset, u8 data);
	u8 sample_dpcm_and_format_r(offs_t offset);
	template<int sel> void sample_address_w(offs_t offset, u8 data);
	template<int sel> u8 sample_address_r(offs_t offset);
	void lfo_step_w(offs_t offset, u8 data);
	u8 lfo_step_r(offs_t offset);
	void lfo_pmod_depth_w(offs_t offset, u8 data);
	u8 lfo_pmod_depth_r(offs_t offset);

	void slot8_w(offs_t offset, u8 data);
	void slot9_w(offs_t offset, u8 data);

	// Internal state access
	u8 state_r();
	void state_adr_w(u8 data);

	// MEG
	void offset_w(offs_t offset, u8 data);
	u8 offset_r(offs_t offset);
	void const_w(offs_t offset, u8 data);
	u8 const_r(offs_t offset);
	void meg_control_w(u8 data);
	u8 meg_control_r();

	// Control registers
	void keyon(int chan);
	template<int sel> void keyon_w(u8 data);

	void waverom_access_w(u8 data);
	u8 waverom_access_r();
	u8 waverom_val_r();

	// Generic catch-all
	u8 snd_r(offs_t offset);
	void snd_w(offs_t offset, u8 data);

	inline auto &rctrl(address_map &map, int idx) {
		return map(idx, idx);
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
	static s32 fpapply(s32 value, s32 sample);
	static s32 lpffpapply(s32 value, s32 sample);

	s32 rext(int reg) const;
	static s32 m7v(s32 value, s32 mult);
	s32 m7(s32 value, int reg) const;
	static s32 m9v(s32 value, s32 mult);
	s32 m9(s32 value, int reg) const;
	s32 lfo_get_step(int reg) const;
	void lfo_step(u32 &phase, int reg) const;
	static u32 lfo_wrap(s32 phase, s32 delta);
	static s32 lfo_saturate(s32 phase);
	s32 lfo_wrap_reg(s32 phase, int deltareg) const;
	void filtered_lfo_step(s32 &position, s32 phase, int deltareg, int postdeltareg, int scalereg, int feedbackreg);
	s32 lfo_mod(s32 phase, int scalereg) const;
	s32 lfo_scale(s32 phase, int scalereg) const;

	s32 alfo(u32 phase, s32 delta_phase, int levelreg, int offsetreg, bool sub) const;

	s32 sx(int reg) const;
	double sx7(int reg) const;
	double sx9(int reg) const;

	static s32 saturate(s32 value);
};

DECLARE_DEVICE_TYPE(SWP00, swp00_device)

#endif
