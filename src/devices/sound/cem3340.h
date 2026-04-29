// license:BSD-3-Clause
// copyright-holders:m1macrophage

#ifndef MAME_SOUND_CEM3340_H
#define MAME_SOUND_CEM3340_H

#pragma once

DECLARE_DEVICE_TYPE(CEM3340, cem3340_device)

// A CEM3340 voltage-controlled oscillator.
//
// TODO:
// - Hard and soft sync inputs.
// - Linear FM input.
// - High frequency tracking input.
// - Hardcoded a lot of external components to the recommended values.
class cem3340_device : public device_t, public device_sound_interface
{
public:
	enum input
	{
		INPUT_FREQ = 0,
		INPUT_PW
	};

	enum output
	{
		OUTPUT_TRIANGLE = 0,
		OUTPUT_RAMP,
		OUTPUT_PULSE
	};

	// cf - timing capacitor between pin 11 and GND.
	// rr - reference current resistor between pin 13 and V+.
	cem3340_device(const machine_config &mconfig, const char *tag, device_t *owner, float cf, float rr) ATTR_COLD;
	cem3340_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) ATTR_COLD;

	cem3340_device &set_freq_cc(float freq_cc);  // Frequency control current.
	cem3340_device &set_pw_cv(float pw_cv);  // Pulse width control voltage.

	float get_freq();

protected:
	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	void set_freq_cc_internal(float freq_cc);
	void set_pw_cv_internal(float pw_cv);

	float poly_blep(float phase) const;
	float poly_blamp(float phase) const;

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
};

#endif  // MAME_SOUND_CEM3340_H
