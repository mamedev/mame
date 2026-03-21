// license:BSD-3-Clause
// copyright-holders:m1macrophage

#ifndef MAME_SOUND_VA_VCF_H
#define MAME_SOUND_VA_VCF_H

#pragma once

DECLARE_DEVICE_TYPE(VA_LPF4, va_lpf4_device)

// A 4th order resonant low-pass filter.
//
// Cutoff frequency and resonance can either be provided by calling class
// methods, or via input streams.
//
// The frequency CV is in Hz, and the resonance CV is the feedback gain (0-4).
// The meaning of CV can be different in subclasses: it will typically match
// the type of inputs in the emulated hardware.
class va_lpf4_device : public device_t, public device_sound_interface
{
public:
	enum input_streams
	{
		INPUT_AUDIO = 0,
		INPUT_FREQ,
		INPUT_RES
	};

	va_lpf4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) ATTR_COLD;

	// Disables streams. Intended for using this filter in sound_stream_update()
	// of other devices. The device that embeds this should call process_sample()
	// for each sample in its stream, possibly after calling set_fixed_freq_cv()
	// and set_fixed_res_cv().
	va_lpf4_device &configure_streamless(u32 sample_rate) ATTR_COLD;

	va_lpf4_device &configure_input_gain(float gain) ATTR_COLD;

	// Bass gain compensation vs. resonance.
	// 0 - No compensation. Gain at lower frequencies will drop as resonance increases.
	// 1 - Gain at lower frequencies will remain constant as resonance increases.
	// The value, which can be between 0 and 1, will depend on the device being
	// emulated. The original moog filter and older ICs (e.g. CEM3320) do not
	// compensate. Newer designs and ICs (e.g. CEM3394, CEM3328) provide fixed
	// or configurable compensation.
	va_lpf4_device &configure_bass_gain_comp(float comp) ATTR_COLD;

	// Larger values result in more distortion and more of the (filtered)
	// signal making it through at full resonance. The "correct" value will
	// depend on the device being emulated, and finding it might require
	// experimentation. A decent starting point is: 1 / (PP / 2), where PP is
	// the peak-to-peak magnitude of the input signal.
	// TODO: Revisit and/or find a better way to determine this.
	va_lpf4_device &configure_drive(float drive) ATTR_COLD;

	// Note that the meaning of "CV" can change depending on the subclass being
	// instantiated.
	void set_fixed_freq_cv(float freq_cv);  // frequency in Hz
	void set_fixed_res_cv(float res_cv);  // feedback gain (typically 0-4, but can go over 4)

	float get_freq();  // Returns the cutoff frequency, in Hz.
	float get_res();  // Returns the feedback gain (0-4).

	// Processes a sample through the filter.
	// This only works when in "streamless" mode. See configure_streamless().
	sound_stream::sample_t process_sample(sound_stream::sample_t s);

protected:
	va_lpf4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	virtual float cv_to_freq(float freq_cv) const;
	virtual float cv_to_res(float res_cv) const;

	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream::sample_t process_sample_internal(sound_stream::sample_t s);

	u32 sample_rate() const;
	void recalc_res();
	void recalc_filter();

	sound_stream *m_stream;

	// Configuration, not needed in save state.
	u32 m_streamless_sample_rate;  // Ignored (and 0) when in streaming mode.
	float m_input_gain;
	float m_gain_comp;
	float m_drive;

	// Filter state.
	float m_fc;  // Cutoff frequency in Hz.
	float m_res;  // Feedback gain.
	struct filter_stage
	{
		float alpha = 1;
		float beta = 0;
		float state = 0;
	};
	std::array<filter_stage, 4> m_stages;
	float m_alpha0;
	float m_G4;
	float m_gain_comp_scale;
};

#endif  // MAME_SOUND_VA_VCF_H
