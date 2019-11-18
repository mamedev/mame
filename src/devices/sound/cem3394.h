// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_SOUND_CEM3394_H
#define MAME_SOUND_CEM3394_H

#pragma once

#define CEM3394_EXT_INPUT(_name) void _name(int count, short *buffer)


class cem3394_device : public device_t, public device_sound_interface
{
public:
	static constexpr unsigned SAMPLE_RATE = 44100*4;

	// inputs
	enum
	{
		VCO_FREQUENCY = 0,
		MODULATION_AMOUNT,
		WAVE_SELECT,
		PULSE_WIDTH,
		MIXER_BALANCE,
		FILTER_RESONANCE,
		FILTER_FREQENCY,
		FINAL_GAIN
	};

	typedef device_delegate<void (int count, short *buffer)> ext_input_delegate;

	cem3394_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename... T> void set_ext_input_callback(T &&... args) { m_ext_cb.set(std::forward<T>(args)...); }
	void set_vco_zero_freq(double freq) { m_vco_zero_freq = freq; }
	void set_filter_zero_freq(double freq) { m_filter_zero_freq = freq; }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	// Set the voltage going to a particular parameter
	void set_voltage(int input, double voltage);

	// Get the translated parameter associated with the given input as follows:
	//    VCO_FREQUENCY:      frequency in Hz
	//    MODULATION_AMOUNT:  scale factor, 0.0 to 2.0
	//    WAVE_SELECT:        voltage from this line
	//    PULSE_WIDTH:        width fraction, from 0.0 to 1.0
	//    MIXER_BALANCE:      balance, from -1.0 to 1.0
	//    FILTER_RESONANCE:   resonance, from 0.0 to 1.0
	//    FILTER_FREQENCY:    frequency, in Hz
	//    FINAL_GAIN:         gain, in dB
	double get_parameter(int input);

private:
	double compute_db(double voltage);
	uint32_t compute_db_volume(double voltage);

private:
	ext_input_delegate m_ext_cb; /* callback to generate external samples */

	sound_stream *m_stream;           /* our stream */
	double m_vco_zero_freq;           /* frequency of VCO at 0.0V */
	double m_filter_zero_freq;        /* frequency of filter at 0.0V */

	double m_values[8];               /* raw values of registers */
	uint8_t m_wave_select;              /* flags which waveforms are enabled */

	uint32_t m_volume;                  /* linear overall volume (0-256) */
	uint32_t m_mixer_internal;          /* linear internal volume (0-256) */
	uint32_t m_mixer_external;          /* linear external volume (0-256) */

	uint32_t m_position;                /* current VCO frequency position (0.FRACTION_BITS) */
	uint32_t m_step;                    /* per-sample VCO step (0.FRACTION_BITS) */

	uint32_t m_filter_position;         /* current filter frequency position (0.FRACTION_BITS) */
	uint32_t m_filter_step;             /* per-sample filter step (0.FRACTION_BITS) */
	uint32_t m_modulation_depth;        /* fraction of total by which we modulate (0.FRACTION_BITS) */
	int16_t m_last_ext;                 /* last external sample we read */

	uint32_t m_pulse_width;             /* fractional pulse width (0.FRACTION_BITS) */

	double m_inv_sample_rate;
	int m_sample_rate;

	std::unique_ptr<int16_t[]> m_mixer_buffer;
	std::unique_ptr<int16_t[]> m_external_buffer;
};

DECLARE_DEVICE_TYPE(CEM3394, cem3394_device)

#endif // MAME_SOUND_CEM3394_H
