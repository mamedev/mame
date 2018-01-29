// license:BSD-3-Clause
// copyright-holders:Miguel Angel Horna
#ifndef MAME_SOUND_MULTIPCM_H
#define MAME_SOUND_MULTIPCM_H

#pragma once

class multipcm_device : public device_t,
						public device_sound_interface,
						public device_rom_interface
{
public:
	multipcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_rom_interface overrides
	virtual void rom_bank_updated() override;

private:
	struct sample_t
	{
		uint32_t m_start;
		uint32_t m_loop;
		uint32_t m_end;
		uint8_t m_attack_reg;
		uint8_t m_decay1_reg;
		uint8_t m_decay2_reg;
		uint8_t m_decay_level;
		uint8_t m_release_reg;
		uint8_t m_key_rate_scale;
		uint8_t m_lfo_vibrato_reg;
		uint8_t m_lfo_amplitude_reg;
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
		int32_t m_volume;
		state_t m_state;
		int32_t step;
		//step vals
		int32_t m_attack_rate;     // Attack
		int32_t m_decay1_rate;    // Decay1
		int32_t m_decay2_rate;    // Decay2
		int32_t m_release_rate;     // Release
		int32_t m_decay_level;     // Decay level
	};

	struct lfo_t
	{
		uint16_t m_phase;
		uint32_t m_phase_step;
		int32_t *m_table;
		int32_t *m_scale;
	};

	struct slot_t
	{
		uint8_t m_slot_index;
		uint8_t m_regs[8];
		bool m_playing;
		sample_t m_sample;
		uint32_t m_base;
		uint32_t m_offset;
		uint32_t m_step;
		uint32_t m_pan;
		uint32_t m_total_level;
		uint32_t m_dest_total_level;
		int32_t m_total_level_step;
		int32_t m_prev_sample;
		envelope_gen_t m_envelope_gen;
		lfo_t m_pitch_lfo; // Pitch lfo
		lfo_t m_amplitude_lfo; // AM lfo
	};

	// internal state
	sound_stream *m_stream;
	slot_t *m_slots;
	uint32_t m_cur_slot;
	uint32_t m_address;
	float m_rate;

	uint32_t *m_attack_step;
	uint32_t *m_decay_release_step;   // Envelope step tables
	uint32_t *m_freq_step_table;      // Frequency step table

	int32_t *m_left_pan_table;
	int32_t *m_right_pan_table;
	int32_t *m_linear_to_exp_volume;
	int32_t *m_total_level_steps;

	int32_t *m_pitch_table;
	int32_t **m_pitch_scale_tables;
	int32_t *m_amplitude_table;
	int32_t **m_amplitude_scale_tables;

	uint32_t value_to_fixed(const uint32_t bits, const float value);

	void init_sample(sample_t *sample, uint32_t index);

	// Internal LFO functions
	void lfo_init();
	void lfo_compute_step(lfo_t *lfo, uint32_t lfo_frequency, uint32_t LFOS, int32_t amplitude_lfo);
	int32_t pitch_lfo_step(lfo_t *lfo);
	int32_t amplitude_lfo_step(lfo_t *lfo);

	// Internal envelope functions
	int32_t envelope_generator_update(slot_t *slot);
	void envelope_generator_calc(slot_t *slot);
	uint32_t get_rate(uint32_t *steps, uint32_t rate, uint32_t val);

	void write_slot(slot_t *slot, int32_t reg, uint8_t data);

	int16_t clamp_to_int16(int32_t value);

	static constexpr uint32_t TL_SHIFT = 12;

	static const int32_t VALUE_TO_CHANNEL[32];

	static constexpr uint32_t EG_SHIFT = 16;
	static const double BASE_TIMES[64];

	static constexpr uint32_t LFO_SHIFT = 8;
	static const float LFO_FREQ[8];
	static const float PHASE_SCALE_LIMIT[8];
	static const float AMPLITUDE_SCALE_LIMIT[8];
};

DECLARE_DEVICE_TYPE(MULTIPCM, multipcm_device)

#endif // MAME_SOUND_MULTIPCM_H
