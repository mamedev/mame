// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_SOUND_CEM3394_H
#define MAME_SOUND_CEM3394_H

#pragma once

#include "sound/va_ops.h"
#include "sound/va_vca.h"
#include "sound/va_vcf.h"
#include "sound/va_vco.h"

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

	// external component values
	struct components
	{
		// The default values below are the ones used in the datasheet.
		// Pin 1 - Resistor to VEE. Sets reference current for the VCO internals.
		double r_vco = 270E3;
		// Pin 4 - VCO timing capacitor.
		double c_vco = 2E-9;
		// Pin 12 (or 13, or 14, they should be equal) - VCF capacitor.
		double c_vcf = 33E-9;
		// Pin 17 - AC-coupling capacitor on the VCF output.
		double c_ac = 4.7E-6;
	};

	using input_array = std::array<device_sound_interface *, INPUT_COUNT>;

	cem3394_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;
	cem3394_device(const machine_config &mconfig, const char *tag, device_t *owner, const components &comps, const input_array& inputs) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

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
	void update_mix();

	float stream_op_filter_freq(float voltage);
	float stream_op_amp_gain(float voltage);

	// device configuration, not needed in save state
	const input_array m_stream_inputs;// streaming inputs
	const components m_components;    // external components
	const double m_vco_zero_freq;     // frequency of VCO at 0.0V
	const double m_filter_zero_freq;  // frequency of filter at 0.0V
	sound_stream *m_stream;           // our stream

	required_device<va_vco_device> m_vco;
	optional_device<va_const_device> m_filt_freq;
	required_device<va_vca_device> m_filt_fm;
	required_device<va_lpf4_device> m_vcf;
	required_device<va_vca_device> m_vca;

	// device state

	double m_values[INPUT_COUNT];     // raw values of registers

	u8 m_wave_select;                 // flags which waveforms are enabled
	double m_vco_frequency;           // current VCO frequency
	double m_pulse_width;             // fractional pulse width

	double m_volume;                  // linear overall volume (0-1)
	double m_mixer_internal;          // linear internal volume (0-1)
	double m_mixer_external;          // linear external volume (0-1)

	double m_filter_frequency;        // baseline filter frequency
	double m_filter_modulation;       // depth of modulation (up to 1.0)
	double m_filter_resonance;        // depth of modulation (up to 1.0)
};

DECLARE_DEVICE_TYPE(CEM3394, cem3394_device)

#endif // MAME_SOUND_CEM3394_H
