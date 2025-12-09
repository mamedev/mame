// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_SOUND_CEM3394_H
#define MAME_SOUND_CEM3394_H

#pragma once


class cem3394_device : public device_t, public device_sound_interface
{
public:
	// inputs
	// Each of these CV inputs can either be specified with `set_voltage()`, or
	// provided via a sound stream.
	enum
	{
		AUDIO_INPUT = 0,  // not valid for set_voltage()
		VCO_FREQUENCY,
		MODULATION_AMOUNT,
		WAVE_SELECT,
		PULSE_WIDTH,
		MIXER_BALANCE,
		FILTER_RESONANCE,
		FILTER_FREQUENCY,
		FINAL_GAIN,
		INPUT_COUNT
	};

	cem3394_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;

	// The constructor will initialize components values to those recommended
	// in the datasheet. configure() can be used to change those.
	// r_vco: Pin 1 - Resistor to VEE. Sets reference current for the VCO internals.
	// c_vco: Pin 4 - VCO timing capacitor.
	// c_vcf: Pin 12 (or 13, or 14, they should be equal) - VCF capacitor.
	// c_ac: Pin 17 - AC-coupling capacitor on the VCF output.
	cem3394_device &configure(double r_vco, double c_vco, double c_vcf, double c_ac) ATTR_COLD;

	cem3394_device &configure_limit_pw(bool limit_pw) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

public:
	// Set the voltage going to a particular parameter
	void set_voltage(int input, double voltage);

	// Requesting a streaming voltage will force a stream update.
	double get_voltage(int input);

	// Get the translated parameter associated with the given input as follows:
	//    VCO_FREQUENCY:      frequency in Hz
	//    MODULATION_AMOUNT:  scale factor, 0.0 to 2.0
	//    WAVE_SELECT:        voltage from this line
	//    PULSE_WIDTH:        width fraction, from 0.0 to 1.0
	//    MIXER_BALANCE:      balance, from -1.0 to 1.0
	//    FILTER_RESONANCE:   resonance, from 0.0 to 1.0
	//    FILTER_FREQUENCY:   frequency, in Hz
	//    FINAL_GAIN:         gain, in dB
	// Requesting a parameter associated with a streaming input will force a
	// stream update.
	double get_parameter(int input);

private:
	double compute_db(double voltage);
	sound_stream::sample_t compute_db_volume(double voltage);

	void set_voltage_internal(int input, double voltage);

	double filter(double input, double cutoff);
	double hpf(double input);

	// device configuration, not needed in save state
	sound_stream *m_stream;           // our stream
	double m_inv_sample_rate;         // 1/current sample rate
	double m_vco_zero_freq;           // frequency of VCO at 0.0V
	double m_filter_zero_freq;        // frequency of filter at 0.0V
	double m_hpf_k;                   // RC filter coefficient for AC coupling
	bool m_limit_pw;                  // whether to clamp the pulse width.

	// device state

	double m_values[INPUT_COUNT];     // raw values of registers
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

	double m_hpf_mem;                 // AC coupling filter memory
};

DECLARE_DEVICE_TYPE(CEM3394, cem3394_device)

#endif // MAME_SOUND_CEM3394_H
