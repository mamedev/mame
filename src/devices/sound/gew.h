// license:BSD-3-Clause
// copyright-holders:Miguel Angel Horna
#ifndef MAME_SOUND_GEW_H
#define MAME_SOUND_GEW_H

#pragma once

#include "dirom.h"

#define MULTIPCM_LOG_SAMPLES    0

#if MULTIPCM_LOG_SAMPLES
#include <map>
#endif

class gew_pcm_device : public device_t,
						public device_sound_interface,
						public device_rom_interface<22, 0, 0, ENDIANNESS_BIG>
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

protected:
	gew_pcm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock,
		uint32_t voices, uint32_t clock_divider);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

	struct sample_t
	{
		uint32_t m_start = 0;
		uint32_t m_loop = 0;
		uint32_t m_end = 0;
		uint8_t m_attack_reg = 0;
		uint8_t m_decay1_reg = 0;
		uint8_t m_decay2_reg = 0;
		uint8_t m_decay_level = 0;
		uint8_t m_release_reg = 0;
		uint8_t m_key_rate_scale = 0;
		uint8_t m_lfo_vibrato_reg = 0;
		uint8_t m_lfo_amplitude_reg = 0;
		uint8_t m_format = 0;
	};

	enum class state_t : u8
	{
		ATTACK,
		DECAY1,
		DECAY2,
		RELEASE
	};

	struct envelope_gen_t
	{
		int32_t m_volume = 0;
		state_t m_state = state_t::ATTACK;
		uint8_t m_reverb = 0;
		int32_t step = 0;
		//step vals
		int32_t m_attack_rate = 0;  // Attack
		int32_t m_decay1_rate = 0;  // Decay1
		int32_t m_decay2_rate = 0;  // Decay2
		int32_t m_release_rate = 0; // Release
		int32_t m_decay_level = 0;  // Decay level
	};

	struct lfo_t
	{
		uint16_t m_phase = 0;
		uint32_t m_phase_step = 0;
		int32_t *m_table = nullptr;
		int32_t *m_scale = nullptr;
	};

	struct slot_t
	{
		uint8_t m_regs[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		bool m_playing = false;
		sample_t m_sample;
		uint32_t m_offset = 0;
		uint8_t m_octave = 0;
		uint16_t m_pitch = 0;
		uint32_t m_step = 0;
		bool m_reverse = false;
		uint32_t m_pan = 0;
		uint32_t m_total_level = 0;
		uint32_t m_dest_total_level = 0;
		int32_t m_total_level_step = 0;
		int32_t m_prev_sample = 0;
		envelope_gen_t m_envelope_gen;
		uint8_t m_lfo_frequency;
		lfo_t m_pitch_lfo; // Pitch lfo
		uint8_t m_vibrato = 0;
		lfo_t m_amplitude_lfo; // AM lfo
		uint8_t m_tremolo = 0;
	};

	uint32_t value_to_fixed(const uint32_t bits, const float value);

	void retrigger_sample(slot_t &slot);
	void update_step(slot_t &slot);

	static constexpr uint32_t TL_SHIFT = 12;
	static constexpr uint32_t EG_SHIFT = 16;
	static constexpr uint32_t LFO_SHIFT = 8;

	// Internal LFO functions
	void lfo_init();
	void lfo_compute_step(lfo_t &lfo, uint32_t lfo_frequency, uint32_t LFOS, int32_t amplitude_lfo);
	int32_t pitch_lfo_step(lfo_t &lfo);
	int32_t amplitude_lfo_step(lfo_t &lfo);

	// Internal envelope functions
	void envelope_generator_init(const double (&rates)[64], double attack_decay_ratio);
	int32_t envelope_generator_update(slot_t &slot);
	void envelope_generator_calc(slot_t &slot);
	uint32_t get_rate(uint32_t *steps, int32_t rate, uint32_t val);

#if MULTIPCM_LOG_SAMPLES
	void dump_sample(slot_t &slot);
	std::map<uint32_t, bool> m_logged_map;
#endif

	static const double BASE_TIMES[64];

	static const float LFO_FREQ[8];
	static const float PHASE_SCALE_LIMIT[8];
	static const float AMPLITUDE_SCALE_LIMIT[8];

	// internal state
	sound_stream* m_stream;
	std::unique_ptr<slot_t[]> m_slots;
	float m_rate;
	uint32_t m_voices;
	uint32_t m_clock_divider;

	std::unique_ptr<uint32_t[]> m_attack_step;
	std::unique_ptr<uint32_t[]> m_decay_release_step;   // Envelope step tables
	std::unique_ptr<uint32_t[]> m_freq_step_table;      // Frequency step table

	std::unique_ptr<int32_t[]> m_left_pan_table;
	std::unique_ptr<int32_t[]> m_right_pan_table;
	std::unique_ptr<int32_t[]> m_linear_to_exp_volume;
	std::unique_ptr<int32_t[]> m_total_level_steps;

	std::unique_ptr<int32_t[]> m_pitch_table;
	std::unique_ptr<int32_t[]> m_pitch_scale_tables[8];
	std::unique_ptr<int32_t[]> m_amplitude_table;
	std::unique_ptr<int32_t[]> m_amplitude_scale_tables[8];
};

#endif // MAME_SOUND_GEW_H
