// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_SOUND_CEM3394_H
#define MAME_SOUND_CEM3394_H

#pragma once


class cem3394_device : public device_t, public device_sound_interface
{
public:
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

	cem3394_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_vco_zero_freq(double freq) { m_vco_zero_freq = freq; }
	void set_filter_zero_freq(double freq) { m_filter_zero_freq = freq; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

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
	stream_buffer::sample_t compute_db_volume(double voltage);

private:
	double filter(double input, double cutoff);

	sound_stream *m_stream;           // our stream
	double m_vco_zero_freq;           // frequency of VCO at 0.0V
	double m_filter_zero_freq;        // frequency of filter at 0.0V

	double m_values[8];               // raw values of registers
	u8 m_wave_select;                 // flags which waveforms are enabled

	double m_volume;                  // linear overall volume (0-1)
	double m_mixer_internal;          // linear internal volume (0-1)
	double m_mixer_external;          // linear external volume (0-1)

	double m_vco_position;            // current VCO frequency position (always < 1.0)
	double m_vco_step;                // per-sample VCO step

	double m_filter_frequency;        // baseline filter frequency
	double m_filter_modulation;       // depth of modulation (up to 1.0)
	double m_filter_resonance;        // depth of modulation (up to 1.0)
	double m_filter_in[4];            // filter input history
	double m_filter_out[4];           // filter output history

	double m_pulse_width;             // fractional pulse width

	double m_inv_sample_rate;         // 1/current sample rate
};

DECLARE_DEVICE_TYPE(CEM3394, cem3394_device)

#endif // MAME_SOUND_CEM3394_H
