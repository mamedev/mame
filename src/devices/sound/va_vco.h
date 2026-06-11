// license:BSD-3-Clause
// copyright-holders:m1macrophage

#ifndef MAME_SOUND_VA_VCO_H
#define MAME_SOUND_VA_VCO_H

#pragma once

DECLARE_DEVICE_TYPE(VA_VCO, va_vco_device)

// A virtual analog oscillator, with support for multiple waveforms and sync.
//
// By default, the output will be in the range [-1, 1], and the frequency and
// pulse width control signals should be in Hz and a fraction, respectively.
// The control signals can either be set by functions, or they can be provided
// as input streams.
//
// Subclasses can override the "ctrl2*()" methods to match the control inputs
// (e.g. voltage, current) of real oscillator devices, and they can use the
// configure_*() methods to tailor the oscillator's behavior.
//
// This class can also be used directly in drivers that emulate audio at the
// block-diagram-level, rather than at the circuit-level. No need to worry about
// voltages and currents in that case.
//
// *** Sync ***
//
// Currently, this device supports one type of hard sync: switching the
// direction of the underlying triangle wave in a triangle-core oscillator.
//
// To avoid aliasing, we need to know the exact time the sync event occurred. It
// is not enough to know the sample it occurred at. Therefore the sync signal
// itself cannot be provided as a streaming input. Instead, we use the frequency
// of the sync oscillator as the input. The implementation will then keep track
// of when the sync events occur, based on that provided frequency.
//
class va_vco_device : public device_t, public device_sound_interface
{
public:
	enum input
	{
		// Frequency in Hz. Subclasses can change the control type by overriding ctrl2freq().
		INPUT_FREQ_CTRL = 0,
		// Pulse width fraction (0-1). Subclasses can change the control type by overriding ctrl2pw().
		INPUT_PW_CTRL,
		// Frequency of the sync oscillator.
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

	va_vco_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	va_vco_device &configure_ramp_range(float minval, float maxval) ATTR_COLD;
	va_vco_device &configure_pulse_range(float minval, float maxval) ATTR_COLD;
	va_vco_device &configure_triangle_range(float minval, float maxval) ATTR_COLD;

	// Sets the frequency in Hz. Subclasses can change the input type (e.g. to
	// a control current) by overriding ctrl2freq(). Cannot be used when
	// INPUT_FREQ_CTRL is connected.
	void set_freq_ctrl(float freq_ctrl);

	// Sets the pulse width fraction (0-1). Subclasses can change the input type
	// (e.g. to a control voltage) by overriding ctrl2pw(). Cannot be used when
	// INPUT_PW_CTRL is connected.
	void set_pw_ctrl(float pw_ctrl);

	void set_sync_enabled(bool enabled);

	float freq();  // in Hz
	u32 sample_rate() const { return m_stream->sample_rate(); }

	// Returns the time remaining for the ramp waveform to cross the specified
	// voltage, in an upwards direction. Also works for 0V.
	attotime ramp_time_to_thresh(float threshold);

protected:
	va_vco_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

	// Subclasses can override these, to support voltage or current control of
	// the oscillator frequency and pulse width.
	virtual float ctrl2freq(float freq_ctrl) const;
	virtual float ctrl2pw(float pw_ctrl) const;

private:
	void set_freq_ctrl_internal(float freq_cc);
	void set_pw_ctrl_internal(float pw_cv);

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
	sound_stream *m_stream;
	float m_ramp_min;
	float m_ramp_max;
	float m_pulse_min;
	float m_pulse_max;
	float m_tri_min;
	float m_tri_max;

	// state
	float m_freq_ctrl;
	float m_freq;
	float m_pw_ctrl;
	float m_pw;
	float m_step;
	float m_phase;  // 0-1
	float m_ramp_correction;
	float m_pulse_correction;
	float m_tri_correction;
	bool m_sync;
	float m_sync_phase;  // 0-1
	float m_sync_step;
};

#endif  // MAME_SOUND_VA_VCO_H
