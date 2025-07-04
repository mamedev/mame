// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#pragma once

#ifndef MAME_EMU_AUDIO_EFFECTS_REVERB_H
#define MAME_EMU_AUDIO_EFFECTS_REVERB_H

#include "aeffect.h"

#include <random>

class audio_effect_reverb : public audio_effect
{
public:
	audio_effect_reverb(speaker_device *speaker, u32 sample_rate, audio_effect *def);
	virtual ~audio_effect_reverb() = default;

	virtual int type() const override { return REVERB; }
	virtual void apply(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest) override;
	virtual void config_load(util::xml::data_node const *ef_node) override;
	virtual void config_save(util::xml::data_node *ef_node) const override;
	virtual void default_changed() override;

	void set_mode(u32 mode);
	void set_early_tap_setup(u32 index);
	void set_early_damping(double cutoff);
	void set_stereo_width(double width);
	void set_early_room_size(double size);
	void set_late_room_size(double size);
	void set_late_spin(double speed);
	void set_late_wander(double wander);
	void set_late_diffusion(double value);
	void set_late_damping(double cutoff);
	void set_late_predelay(double delay);
	void set_late_global_decay(float value);
	void set_dry_level(double level);
	void set_early_level(double level);
	void set_late_level(double level);
	void set_early_to_late_level(double level);

	u32    mode() const                { return m_mode; }
	u32    early_tap_setup() const     { return m_early_tap_setup; }
	double early_damping() const       { return m_early_damping; }
	double stereo_width() const        { return m_stereo_width; }
	double early_room_size() const     { return m_early_room_size; }
	double late_room_size() const      { return m_late_room_size; }
	double late_spin() const           { return m_late_spin; }
	double late_wander() const         { return m_late_wander; }
	double late_diffusion() const      { return m_late_diffusion; }
	double late_damping() const        { return m_late_damping; }
	double late_predelay() const       { return m_late_predelay; }
	float  late_global_decay() const   { return m_late_global_decay; }
	double dry_level() const           { return m_dry_level; }
	double early_level() const         { return m_early_level; }
	double late_level() const          { return m_late_level; }
	double early_to_late_level() const { return m_early_to_late_level; }

	bool isset_mode() const                { return m_isset_mode; }
	bool isset_early_tap_setup() const     { return m_isset_early_tap_setup; }
	bool isset_early_damping() const       { return m_isset_early_damping; }
	bool isset_stereo_width() const        { return m_isset_stereo_width; }
	bool isset_early_room_size() const     { return m_isset_early_room_size; }
	bool isset_late_room_size() const      { return m_isset_late_room_size; }
	bool isset_late_spin() const           { return m_isset_late_spin; }
	bool isset_late_wander() const         { return m_isset_late_wander; }
	bool isset_late_diffusion() const      { return m_isset_late_diffusion; }
	bool isset_late_damping() const        { return m_isset_late_damping; }
	bool isset_late_predelay() const       { return m_isset_late_predelay; }
	bool isset_late_global_decay() const   { return m_isset_late_global_decay; }
	bool isset_dry_level() const           { return m_isset_dry_level; }
	bool isset_early_level() const         { return m_isset_early_level; }
	bool isset_late_level() const          { return m_isset_late_level; }
	bool isset_early_to_late_level() const { return m_isset_early_to_late_level; }

	void reset_mode();
	void reset_early_tap_setup();
	void reset_early_damping();
	void reset_stereo_width();
	void reset_early_room_size();
	void reset_late_room_size();
	void reset_late_spin();
	void reset_late_wander();
	void reset_late_diffusion();
	void reset_late_damping();
	void reset_late_predelay();
	void reset_late_global_decay();
	void reset_dry_level();
	void reset_early_level();
	void reset_late_level();
	void reset_early_to_late_level();
	void reset_all();

	static u32 preset_count();
	static const char *preset_name(u32 id);
	static u32 early_tap_setup_count();
	static const char *early_tap_setup_name(u32 id);

	u32 default_preset() { return m_default_preset_id; }
	u32 find_current_preset();
	void load_preset(u32 id);

private:
	struct preset {
		const char *name;
		u32 early_tap_setup;
		double early_damping;
		double stereo_width;
		double early_room_size;
		double late_room_size;
		double late_spin;
		double late_wander;
		double late_diffusion;
		double late_damping;
		double late_predelay;
		float late_global_decay;
		double dry_level;
		double early_level;
		double late_level;
		double early_to_late_level;
	};

	static const preset presets[];

	enum { INPUT_DIFFUSION_ALLPASS = 10, CROSS_DIFFUSION_ALLPASS = 4 };

	struct early_reverb_tap_map {
		const char *name;
		u32 m_count[2];
		float m_delay[2][18];
		float m_gain[2][18];
	};

	static const early_reverb_tap_map tap_maps[15];

	struct delay_buffer {
		enum { D_MASK = 8191 };
		sample_t m_samples[D_MASK+1];
		u32 m_index;

		void clear();
		void push(sample_t value);
		sample_t get(u32 dist) const;
		sample_t geti(float dist) const; // Get with interpolation
		delay_buffer() { clear(); }
	};

	struct delay {
		delay_buffer m_delay_buffer;
		std::vector<u32> m_taps;
		void clear();
		void set_tap(u32 tap, u32 dist);
		void push(sample_t value);
		sample_t get(u32 tap) const;
		sample_t process(sample_t value) { push(value); return get(0); }
		delay() { clear(); }
	};

	struct iir1h {
		sample_t m_y1;
		void clear();
		iir1h() { clear(); }
	};

	struct iir1 {
		float m_a2, m_b1, m_b2;
		sample_t process(iir1h &h, sample_t x0);
		void prepare_lpf(double cutoff, u32 sample_rate);
		void prepare_hpf(double cutoff, u32 sample_rate);
	};

	struct iir2h {
		sample_t m_x1, m_x2, m_y1, m_y2;
		void clear();
		iir2h() { clear(); }
	};

	struct iir2 {
		float m_a1, m_a2, m_b0, m_b1, m_b2;
		sample_t process(iir2h &h, sample_t x0);
		void prepare_lpf(double cutoff, double bw, u32 sample_rate);
		void prepare_apf(double cutoff, double bw, u32 sample_rate);
	};

	struct dccuth {
		float m_y1, m_y2;
		void clear();
		dccuth() { clear(); }
	};

	struct dccut {
		float m_gain;
		sample_t process(dccuth &h, sample_t x0);
		void prepare(double cutoff, u32 sample_rate);
	};

	struct pink {
		enum { SIZE = 1 << 15 };
		std::mt19937 m_rng;
		std::uniform_real_distribution<sample_t> m_dis;
		sample_t m_buffer[SIZE];
		u32 m_index;

		void clear();
		pink() : m_rng(std::random_device()()), m_dis(-1, 1) { clear(); }
		sample_t process();
	};

	struct lfo {
		float m_c, m_s, m_rc, m_rs;
		u32 m_count;

		void clear();
		lfo() : m_rc(1), m_rs(0) { clear(); }
		void prepare(double speed, u32 sample_rate);
		float process();
	};

	struct allpass {
		delay_buffer m_delay_buffer;
		float m_gain;
		u32 m_delay;

		void clear();
		sample_t process(sample_t input);
		void set_gain(float base);
		void set_delay(u32 base);
		allpass() { clear(); }
	};

	struct allpass_m {
		delay_buffer m_delay_buffer;
		float m_base_gain;
		float m_base_delay, m_mod_delay;

		void clear();
		sample_t process(sample_t input, float delay_mod, float gain_mod);
		void set_gain(float base);
		void set_delay(float base, float mod);
		allpass_m() { clear(); }
	};

	struct allpass_md {
		delay_buffer m_delay_buffer;
		float m_base_gain, m_decay;
		float m_base_delay, m_mod_delay;

		void clear();
		sample_t process(sample_t input, float delay_mod, float gain_mod);
		void set_gain(float base);
		void set_delay(float base, float mod);
		void set_decay(float base);
		allpass_md() { clear(); }
	};

	struct allpass2 {
		delay_buffer m_delay_buffer_1, m_delay_buffer_2;
		float m_gain_1, m_gain_2, m_decay_1, m_decay_2;
		u32 m_delay_1, m_delay_2;
		std::vector<u32> m_taps_1, m_taps_2;

		void clear();
		sample_t process(sample_t input);
		void set_gain_1(float base);
		void set_gain_2(float base);
		void set_delays(u32 base_1, u32 base_2);
		void set_decay_1(float base);
		void set_decay_2(float base);
		void set_tap_1(u32 tap, u32 dist);
		void set_tap_2(u32 tap, u32 dist);
		sample_t get_1(u32 tap) const;
		sample_t get_2(u32 tap) const;
		allpass2() { clear(); }
	};

	struct allpass3m {
		delay_buffer m_delay_buffer_1, m_delay_buffer_2, m_delay_buffer_3;
		float m_gain_1, m_gain_2, m_gain_3, m_decay_1, m_decay_2, m_decay_3;
		u32 m_base_delay_1, m_mod_delay_1, m_delay_2, m_delay_3;
		std::vector<u32> m_taps_1, m_taps_2, m_taps_3;

		void clear();
		sample_t process(sample_t input, float delay_mod);
		void set_gain_1(float base);
		void set_gain_2(float base);
		void set_gain_3(float base);
		void set_delays(u32 base_1, u32 mod_1, u32 base_2, u32 base_3);
		void set_decay_1(float base);
		void set_decay_2(float base);
		void set_decay_3(float base);
		void set_tap_1(u32 tap, u32 dist);
		void set_tap_2(u32 tap, u32 dist);
		void set_tap_3(u32 tap, u32 dist);
		sample_t get_1(u32 tap) const;
		sample_t get_2(u32 tap) const;
		sample_t get_3(u32 tap) const;
		allpass3m() { clear(); }
	};

	struct comb {
		delay_buffer m_delay_buffer;
		sample_t m_filter_history;
		u32 m_size;

		void clear();
		sample_t process(sample_t input, float feedback);
		void set_size(u32 size);
		comb() { clear(); }
	};

	enum ch_type_t { T_MONO, T_LEFT, T_RIGHT };

	u32    m_mode;
	u32    m_early_tap_setup;
	double m_early_damping;
	double m_stereo_width;
	double m_early_room_size;
	double m_late_room_size;
	double m_late_spin;
	double m_late_wander;
	double m_late_diffusion;
	double m_late_damping;
	double m_late_predelay;
	float  m_late_global_decay;
	double m_dry_level;
	double m_early_level;
	double m_late_level;
	double m_early_to_late_level;

	bool m_isset_mode;
	bool m_isset_early_tap_setup;
	bool m_isset_early_damping;
	bool m_isset_stereo_width;
	bool m_isset_early_room_size;
	bool m_isset_late_room_size;
	bool m_isset_late_spin;
	bool m_isset_late_wander;
	bool m_isset_late_diffusion;
	bool m_isset_late_damping;
	bool m_isset_late_predelay;
	bool m_isset_late_global_decay;
	bool m_isset_dry_level;
	bool m_isset_early_level;
	bool m_isset_late_level;
	bool m_isset_early_to_late_level;

	std::vector<ch_type_t> m_ch_type;
	std::vector<int> m_ch_pair;

	iir1 m_early_lpf, m_early_hpf;
	iir2 m_early_diffusion_allpass, m_early_cross_allpass;
	dccut m_late_dccut;
	pink m_late_noise;
	lfo m_late_lfo1, m_late_lfo2;
	iir1 m_late_lfo1_lpf, m_late_lfo2_lpf, m_late_input_damping, m_late_damping_1;
	iir2 m_late_bass, m_late_damping_2, m_late_output_lpf;

	std::vector<iir1h> m_early_lpf_h, m_early_hpf_h;
	std::vector<iir2h> m_early_diffusion_allpass_h, m_early_cross_allpass_h;
	std::vector<dccuth> m_late_dccut_h;
	iir1h m_late_lfo1_lpf_h, m_late_lfo2_lpf_h;
	std::vector<delay_buffer> m_early_delays;
	std::vector<delay_buffer> m_early_xdelays;
	std::vector<std::vector<allpass_m>> m_late_input_diffusion;
	std::vector<std::vector<allpass>> m_late_cross_diffusion;
	std::vector<iir1h> m_late_input_damping_h, m_late_damping_1_h;
	std::vector<iir2h> m_late_bass_h, m_late_damping_2_h, m_late_output_lpf_h;
	std::vector<allpass_md> m_late_step_1, m_late_step_3;
	std::vector<allpass2> m_late_step_5;
	std::vector<allpass3m> m_late_step_7;
	std::vector<delay> m_late_step_2, m_late_step_4, m_late_step_6, m_late_step_8;
	std::vector<comb> m_late_comb;
	std::vector<delay> m_late_final_delay;

	std::vector<sample_t> m_early_in, m_early_wet, m_early_out;
	std::vector<sample_t> m_late_in, m_late_diff, m_late_cross, m_late_cross2, m_late_pre_out, m_late_out;

	float m_wet1, m_wet2;
	float m_early_room_size_ratio;

	float m_late_room_size_ratio;
	float m_late_wander_actual;
	float m_late_modulation_noise_1, m_late_modulation_noise_2;
	float m_late_crossfeed;
	float m_late_decay_0, m_late_decay_1, m_late_decay_2, m_late_decay_3, m_late_decay_f;
	float m_late_loop_decay, m_late_bass_boost;

	float m_actual_dry_level, m_actual_early_level, m_actual_late_level, m_actual_early_to_late_level;

	u32 m_early_tap_dists[2][18];
	u32 m_early_xdelays_dist;

	const preset *m_default_preset;
	u32 m_default_preset_id;

	void commit_early_tap_setup();
	void commit_early_damping();
	void commit_stereo_width();
	void commit_early_room_size();
	void commit_late_room_size();
	void commit_late_decay();
	void commit_late_spin();
	void commit_late_wander();
	void commit_late_diffusion();
	void commit_late_damping();
	void commit_late_predelay();
	void commit_dry_level();
	void commit_early_level();
	void commit_late_level();
	void commit_early_to_late_level();

	void set_early_hpf(double cutoff);
	void set_early_diffusion_ap(double cutoff, double bw);
	void set_early_cross_ap(double cutoff, double bw);
	void set_early_multichannel_delay(double delay);

	void set_late_dccut(double cutoff);
	void set_late_spin_limit_1(double cutoff);
	void set_late_spin_limit_2(double cutoff);
	void set_late_modulation_noise_1(double mod);
	void set_late_modulation_noise_2(double mod);
	void set_late_diffusion_1(double value);
	void set_late_diffusion_2(double value);
	void set_late_diffusion_3(double value);
	void set_late_diffusion_4(double value);
	void set_late_input_damping(double cutoff);
	void set_late_damping_2(double cutoff, double bw);
	void set_late_decay_0(float value);
	void set_late_decay_1(float value);
	void set_late_decay_2(float value);
	void set_late_decay_3(float value);
	void set_late_decay_f(float value);
	void set_late_bass_allpass(double cutoff, double bw);
	void set_late_spin_to_wander(double value);

	static u32 find_preset(std::string name);

	static bool is_prime(u32 value);
	static u32 find_prime(u32 v);
};

#endif
