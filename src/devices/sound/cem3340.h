// license:BSD-3-Clause
// copyright-holders:m1macrophage

#ifndef MAME_SOUND_CEM3340_H
#define MAME_SOUND_CEM3340_H

#pragma once

DECLARE_DEVICE_TYPE(CEM3340, cem3340_device)

// A CEM3340 voltage-controlled oscillator.
//
// A triangle-core oscillator with triangle, ramp and variable width pulse outputs.
//
// *** Sync ***
//
// The CEM3340 supports multiple types of sync, some of which are unusual. At
// the moment, only the "conventional hard sync" setup is emulated (figure 5 in
// the datasheet), which is one that's commonly used. In this mode, a negative
// pulse will unconditionally switch the direction of the triangle wave, from
// which the other waveforms are derived.
//
// In order to produce anti-aliased synced waveforms, we need to know the exact
// time the sync pulse occured. It is not enough to know at which sample it
// occured. Therefore the sync pulse itself cannot be provided as a streaming
// input. Instead, we use the frequency of the sync oscillator as an input. The
// implementation will then keep track of when the sync events occur, based on
// the provided frequency.
//
// TODO:
// - Other types of sync.
// - Linear FM input.
// - High frequency tracking input.
// - A lot of external components are hardcoded to the recommended values. Make
//   them configurable as needed.
class cem3340_device : public device_t, public device_sound_interface
{
public:
	enum input
	{
		INPUT_FREQ = 0,
		INPUT_PW,
		// Frequency of the sync oscillator used for "conventional" hard sync.
		INPUT_SYNC_FREQ,
	};

	enum output
	{
		OUTPUT_TRIANGLE = 0,
		OUTPUT_RAMP,
		OUTPUT_PULSE,
		// Typically connected to INPUT_SYNC_FREQ of the synchronized oscillator.
		OUTPUT_FREQ,
	};

	// cf - timing capacitor between pin 11 and GND.
	// rr - reference current resistor between pin 13 and V+.
	cem3340_device(const machine_config &mconfig, const char *tag, device_t *owner, float cf, float rr) ATTR_COLD;
	cem3340_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) ATTR_COLD;

	cem3340_device &set_freq_cc(float freq_cc);  // Frequency control current.
	cem3340_device &set_pw_cv(float pw_cv);  // Pulse width control voltage.

	void set_sync_enabled(bool enabled);

	float freq();  // in Hz
	u32 sample_rate() const { return m_stream->sample_rate(); }

	// Returns the time remaining for the ramp waveform to cross the specified
	// voltage, in an upwards direction. Also works for 0V.
	attotime ramp_time_to_thresh(float threshold);

protected:
	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	void set_freq_cc_internal(float freq_cc);
	void set_pw_cv_internal(float pw_cv);

	float poly_blep(float phase) const;
	std::pair<float, float> poly_blep_corrections(
		const std::array<std::pair<float, float>, 5>& disc, int n) const;

	float poly_blamp(float phase) const;
	std::pair<float, float> poly_blamp_corrections(
		const std::array<std::pair<float, float>, 5>& disc, int n) const;

	static float ramp_wave(float phase);
	float pulse_wave(float phase) const;

	static float will_wrap(float phase, float step);
	float will_wrap(float phase) const;

	static float time_to_reset(float phase, float step);
	float time_to_reset(float phase) const;

	float midpulse_phase(float phase) const;
	float tritop_phase(float phase) const;

	// configuration
	const float m_cf;
	const float m_rr;
	sound_stream *m_stream;

	// state
	float m_freq_cc;
	float m_freq;
	float m_pw_cv;
	float m_pw;
	float m_step;
	float m_phase;  // 0-1
	float m_ramp_correction;
	float m_pulse_correction;
	float m_triangle_correction;
	bool m_sync;
	float m_sync_phase;  // 0-1
	float m_sync_step;
};

#endif  // MAME_SOUND_CEM3340_H
