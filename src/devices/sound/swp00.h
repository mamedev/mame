// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP00, rompler/dsp combo

#ifndef MAME_SOUND_SWP00_H
#define MAME_SOUND_SWP00_H

#pragma once

#include "meg.h"
#include "dirom.h"

class swp00_device : public device_t, public device_sound_interface, public device_rom_interface<24, 0, 0, ENDIANNESS_LITTLE>
{
public:
	swp00_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 33868800);

	void map(address_map &map) ATTR_COLD;
	void require_sync();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;
	virtual void rom_bank_pre_change() override;

private:
	struct streaming_block {
		static const std::array<s16, 256> dpcm_expand;
		static const std::array<s32, 8> max_value;

		u16 m_phase;
		u16 m_start;
		u16 m_loop;
		u32 m_address;
		u16 m_pitch;
		u8 m_format;

		s32 m_pos;
		s32 m_pos_dec;
		s16 m_dpcm_s0, m_dpcm_s1;
		u32 m_dpcm_pos;
		s32 m_dpcm_delta;

		bool m_first, m_done;
		s16 m_last;

		void clear();
		void keyon();
		std::pair<s16, bool> step(memory_access<24, 0, 0, ENDIANNESS_LITTLE>::cache &wave, s32 fmod);

		template<int sel> void phase_w(u8 data);
		template<int sel> void start_w(u8 data);
		template<int sel> void loop_w(u8 data);
		template<int sel> void address_w(u8 data);
		template<int sel> void pitch_w(u8 data);
		void format_w(u8 data);

		template<int sel> u8 phase_r() const;
		template<int sel> u8 start_r() const;
		template<int sel> u8 loop_r() const;
		template<int sel> u8 address_r() const;
		template<int sel> u8 pitch_r() const;
		u8 format_r() const;

		u32 sample_address_get();

		void read_16(memory_access<24, 0, 0, ENDIANNESS_LITTLE>::cache &wave, s16 &val0, s16 &val1);
		void read_12(memory_access<24, 0, 0, ENDIANNESS_LITTLE>::cache &wave, s16 &val0, s16 &val1);
		void read_8 (memory_access<24, 0, 0, ENDIANNESS_LITTLE>::cache &wave, s16 &val0, s16 &val1);
		void read_8c(memory_access<24, 0, 0, ENDIANNESS_LITTLE>::cache &wave, s16 &val0, s16 &val1);

		void dpcm_step(u8 input);

		std::string describe() const;
	};

	struct envelope_block {
		// Hardware values readable through internal read on variable 2, do not change
		enum {
			ATTACK     = 0,
			DECAY      = 2,
			DECAY_DONE = 3,
		};

		u8 m_attack_speed;
		u8 m_attack_level;
		u8 m_decay_speed;
		u8 m_decay_level;
		s32 m_envelope_level;
		u8  m_envelope_mode;

		void clear();
		void keyon();
		u8 status() const;
		bool active() const;
		u16 step(u32 sample_counter);
		void trigger_release();

		void attack_speed_w(u8 data);
		void attack_level_w(u8 data);
		void decay_speed_w(u8 data);
		void decay_level_w(u8 data);

		u8 attack_speed_r() const;
		u8 attack_level_r() const;
		u8 decay_speed_r() const;
		u8 decay_level_r() const;
	};

	struct filter_block {
		enum {
			SWEEP_NONE,
			SWEEP_UP,
			SWEEP_DONE,
		};

		s32 m_q, m_b, m_l;
		u16 m_k, m_k_target;
		u16 m_info;
		u8 m_speed, m_sweep;

		void clear();
		void keyon();
		s32 step(s16 input, s32 lmod, u32 sample_counter);
		u8 status() const;
		template<int sel> void info_w(u8 data);
		template<int sel> u8 info_r();
		void speed_w(u8 data);
		u8 speed_r();
	};

	struct lfo_block {
		u32 m_counter;
		u8 m_speed, m_lamod, m_fmod;

		void clear();
		void keyon();
		std::tuple<u32, s32, s32> step();

		void lamod_w(u8 data);
		u8 lamod_r();
		void speed_w(u8 data);
		u8 speed_r();
		void fmod_w(u8 data);
		u8 fmod_r();
	};

	struct mixer_block {
		u16 m_cglo, m_cpanl, m_cpanr;
		u16 m_tglo, m_tpanl, m_tpanr;
		u8 m_glo, m_pan, m_dry, m_rev, m_cho, m_var;

		void clear();
		void keyon();
		u8 status_glo() const;
		u8 status_panl() const;
		u8 status_panr() const;
		void step(s32 sample, u16 envelope, u16 amod,
				  s32 &dry_l, s32 &dry_r, s32 &rev, s32 &cho_l, s32 &cho_r, s32 &var_l, s32 &var_r);
		static s32 volume_apply(s32 level, s32 sample);

		void glo_w(u8 data);
		void pan_w(u8 data);
		void dry_w(u8 data);
		void rev_w(u8 data);
		void cho_w(u8 data);
		void var_w(u8 data);

		u8 glo_r();
		u8 pan_r();
		u8 dry_r();
		u8 rev_r();
		u8 cho_r();
		u8 var_r();
	};

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
	bool m_require_sync;

	std::array<streaming_block, 0x20> m_streaming;
	std::array<envelope_block,  0x20> m_envelope;
	std::array<filter_block,    0x20> m_filter;
	std::array<lfo_block,       0x20> m_lfo;
	std::array<mixer_block,     0x20> m_mixer;

	// MEG reverb memory
	std::array<s32, 0x20000> m_rev_buffer;
	std::array<s32, 0x8000>  m_cho_buffer;
	std::array<s32, 0x20000> m_var_buffer;

	// MEG registers
	std::array<u16, 0x40>  m_offset;
	std::array<u16, 0xc0>  m_const;

	// Control registers
	u32 m_sample_counter;
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
	void lfo_lamod_w(offs_t offset, u8 data);
	u8 lfo_lamod_r(offs_t offset);
	void lfo_speed_w(offs_t offset, u8 data);
	u8 lfo_speed_r(offs_t offset);
	void lfo_fmod_w(offs_t offset, u8 data);
	u8 lfo_fmod_r(offs_t offset);
	void rev_w(offs_t offset, u8 data);
	u8 rev_r(offs_t offset);
	void dry_w(offs_t offset, u8 data);
	u8 dry_r(offs_t offset);
	void cho_w(offs_t offset, u8 data);
	u8 cho_r(offs_t offset);
	void var_w(offs_t offset, u8 data);
	u8 var_r(offs_t offset);
	void glo_w(offs_t offset, u8 data);
	u8 glo_r(offs_t offset);
	void pan_w(offs_t offset, u8 data);
	u8 pan_r(offs_t offset);
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
	template<int sel> void phase_w(offs_t offset, u8 data);
	template<int sel> u8 phase_r(offs_t offset);
	template<int sel> void start_w(offs_t offset, u8 data);
	template<int sel> u8 start_r(offs_t offset);
	template<int sel> void loop_w(offs_t offset, u8 data);
	template<int sel> u8 loop_r(offs_t offset);
	template<int sel> void address_w(offs_t offset, u8 data);
	template<int sel> u8 address_r(offs_t offset);
	void format_w(offs_t offset, u8 data);
	u8 format_r(offs_t offset);

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
	static u16 interpolation_step(u32 speed, u32 sample_counter);
	static bool istep(s32 &value, s32 limit, s32 step);
	static bool fpstep(s32 &value, s32 limit, s32 step);
	static s32 fpadd(s32 value, s32 step);
	static s32 fpsub(s32 value, s32 step);
	static s32 fpapply(s32 value, s32 sample);
	static s32 lpffpapply(s32 value, s32 sample);
	static s32 volume_apply(s32 level, s32 sample);

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

#endif // MAME_SOUND_SWP00_H
