// license:BSD-3-Clause
// copyright-holders:Miguel Angel Horna
#pragma once

#ifndef __MULTIPCM_H__
#define __MULTIPCM_H__

class multipcm_device : public device_t,
						public device_sound_interface,
						public device_memory_interface
{
public:
	multipcm_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~multipcm_device() {}

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

	void set_bank(UINT32 leftoffs, UINT32 rightoffs);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	const address_space_config  m_space_config;

private:
	struct sample_t
	{
		UINT32 m_start;
		UINT32 m_loop;
		UINT32 m_end;
		UINT8 m_attack_reg;
		UINT8 m_decay1_reg;
		UINT8 m_decay2_reg;
		UINT8 m_decay_level;
		UINT8 m_release_reg;
		UINT8 m_key_rate_scale;
		UINT8 m_lfo_vibrato_reg;
		UINT8 m_lfo_amplitude_reg;
	};

	enum state_t
	{
		ATTACK,
		DECAY1,
		DECAY2,
		RELEASE
	};

	struct envelope_gen_t
	{
		INT32 m_volume;
		state_t m_state;
		INT32 step;
		//step vals
		INT32 m_attack_rate;     // Attack
		INT32 m_decay1_rate;    // Decay1
		INT32 m_decay2_rate;    // Decay2
		INT32 m_release_rate;     // Release
		INT32 m_decay_level;     // Decay level
	};

	struct lfo_t
	{
		UINT16 m_phase;
		UINT32 m_phase_step;
		INT32 *m_table;
		INT32 *m_scale;
	};


	struct slot_t
	{
		UINT8 m_slot_index;
		UINT8 m_regs[8];
		bool m_playing;
		sample_t *m_sample;
		UINT32 m_base;
		UINT32 m_offset;
		UINT32 m_step;
		UINT32 m_pan;
		UINT32 m_total_level;
		UINT32 m_dest_total_level;
		INT32 m_total_level_step;
		INT32 m_prev_sample;
		envelope_gen_t m_envelope_gen;
		lfo_t m_pitch_lfo; // Pitch lfo
		lfo_t m_amplitude_lfo; // AM lfo
	};

	// internal state
	sound_stream *m_stream;
	sample_t *m_samples;            // Max 512 samples
	slot_t *m_slots;
	UINT32 m_cur_slot;
	UINT32 m_address;
	UINT32 m_bank_right;
	UINT32 m_bank_left;
	float m_rate;

	UINT32 *m_attack_step;
	UINT32 *m_decay_release_step;   // Envelope step tables
	UINT32 *m_freq_step_table;      // Frequency step table

	direct_read_data *m_direct;

	INT32 *m_left_pan_table;
	INT32 *m_right_pan_table;
	INT32 *m_linear_to_exp_volume;
	INT32 *m_total_level_steps;

	INT32 *m_pitch_table;
	INT32 **m_pitch_scale_tables;
	INT32 *m_amplitude_table;
	INT32 **m_amplitude_scale_tables;

	UINT32 value_to_fixed(const UINT32 bits, const float value);

	// Internal LFO functions
	void lfo_init();
	void lfo_compute_step(lfo_t *lfo, UINT32 lfo_frequency, UINT32 LFOS, INT32 amplitude_lfo);
	INT32 pitch_lfo_step(lfo_t *lfo);
	INT32 amplitude_lfo_step(lfo_t *lfo);

	// Internal envelope functions
	INT32 envelope_generator_update(slot_t *slot);
	void envelope_generator_calc(slot_t *slot);
	UINT32 get_rate(UINT32 *steps, UINT32 rate, UINT32 val);

	void write_slot(slot_t *slot, INT32 reg, UINT8 data);

	INT16 clamp_to_int16(INT32 value);

	static const UINT32 TL_SHIFT;

	static const INT32 VALUE_TO_CHANNEL[32];

	static const UINT32 EG_SHIFT;
	static const double BASE_TIMES[64];

	static const UINT32 LFO_SHIFT;
	static const float LFO_FREQ[8];
	static const float PHASE_SCALE_LIMIT[8];
	static const float AMPLITUDE_SCALE_LIMIT[8];
};

extern const device_type MULTIPCM;


#endif /* __MULTIPCM_H__ */
