// license:BSD-3-Clause
// copyright-holders:Aaron Giles,m1macrophage
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

	// optional streaming inputs
	struct stream_inputs
	{
		device_sound_interface *ext_input = nullptr;      // pin 9
		device_sound_interface *filt_freq_cv = nullptr;   // pin 16
		device_sound_interface *final_gain_cv = nullptr;  // pin 18
	};

	cem3394_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;
	cem3394_device(const machine_config &mconfig, const char *tag, device_t *owner, const components &comps, const stream_inputs &inputs) ATTR_COLD;

	// control voltage setters
	// Cannot not be called when the corresponding CV is configured as a
	// streaming input (see stream_inputs).
	void set_vco_freq_cv(double cv);       // pin 2
	void set_mod_amount_cv(double cv);     // pin 5
	void set_wave_select_cv(double cv);    // pin 6
	void set_pulse_width_cv(double cv);    // pin 7
	void set_mixer_balance_cv(double cv);  // pin 10
	void set_filt_res_cv(double cv);       // pin 11
	void set_filt_freq_cv(double cv);      // pin 16
	void set_final_gain_cv(double cv);     // pin 18

	// internal state accessors
	// Note that these do not return the raw CVs. They return values derived
	// from CVs. See per-function comments.
	// Calling these might trigger a stream update.
	double vco_freq();    // in Hz
	double filt_res();    // feedback gain (0: no resonance, ~4: self-oscillation. Could be > 4)
	double filt_freq();   // in Hz
	double final_gain();  // linear (0: quiet, 1: no attenuation)

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	double compute_db(double voltage);
	sound_stream::sample_t compute_db_volume(double voltage);

	void set_filt_freq_cv_internal(double cv);
	void set_final_gain_cv_internal(double cv);
	void update_osc_mix();

	float stream_op_filter_freq(float voltage);
	float stream_op_amp_gain(float voltage);

	// device configuration, not needed in save state
	const stream_inputs m_stream_inputs;  // streaming inputs
	const components m_components;        // external components
	const double m_vco_zero_freq;         // frequency of VCO at 0.0V
	const double m_filter_zero_freq;      // frequency of filter at 0.0V
	bool m_initialized;                   // parameters initialized
	sound_stream *m_stream;               // our stream

	required_device<va_vco_device> m_vco;
	optional_device<va_const_device> m_filt_freq;
	required_device<va_vca_device> m_filt_fm;
	required_device<va_lpf4_device> m_vcf;
	required_device<va_vca_device> m_vca;

	// device state

	// raw control voltages for the chip's parameters
	double m_vco_freq_cv;
	double m_mod_amount_cv;
	double m_wave_select_cv;
	double m_pulse_width_cv;
	double m_mixer_balance_cv;
	double m_filt_res_cv;
	double m_filt_freq_cv;
	double m_final_gain_cv;

	// parameters derived from control voltages
	bool m_tri;                       // triangle waveform enabled
	bool m_saw;                       // sawtooth waveform enabled
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
