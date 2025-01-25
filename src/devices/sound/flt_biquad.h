// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Couriersud,Derrick Renaud,Frank Palazzolo,Jonathan Gevaryahu
#ifndef MAME_SOUND_FLT_BIQUAD_H
#define MAME_SOUND_FLT_BIQUAD_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> filter_biquad_device

class filter_biquad_device : public device_t, public device_sound_interface
{
public:
	enum class biquad_type : int
	{
		LOWPASS1P = 0,
		HIGHPASS1P,
		LOWPASS,
		HIGHPASS,
		BANDPASS,
		NOTCH,
		PEAK,
		LOWSHELF,
		HIGHSHELF,
		RAWPARAMS
	};

	struct biquad_params
	{
		biquad_type type;
		double fc;
		double q;
		double gain;
	};

	filter_biquad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// set up the filter with the specified filter parameters and return a pointer to the new device
	filter_biquad_device& setup(biquad_type type, double fc, double q, double gain);
	filter_biquad_device& setup(biquad_params p);
	// modify an existing instance with new filter parameters
	void modify(biquad_type type, double fc, double q, double gain);
	void modify(biquad_params p);

	// set up the filter with raw biquad coefficients
	filter_biquad_device& setup_raw(double a1, double a2, double b0, double b1, double b2);
	void modify_raw(double a1, double a2, double b0, double b1, double b2);

	// Helper setup functions to create common filters representable by biquad filters:
	// (and, as needed, modify/update/recalc helpers)

	// universal calculator for both Sallen-Key low-pass and high-pass
	biquad_params opamp_sk_lphp_calc(biquad_type type, double r1, double r2, double r3, double r4, double c1, double c2);

	// Sallen-Key low-pass
	filter_biquad_device& opamp_sk_lowpass_setup(double r1, double r2, double r3, double r4, double c1, double c2);
	void opamp_sk_lowpass_modify(double r1, double r2, double r3, double r4, double c1, double c2);

	// Sallen-Key high-pass
	filter_biquad_device& opamp_sk_highpass_setup(double r1, double r2, double r3, double r4, double c1, double c2);
	void opamp_sk_highpass_modify(double r1, double r2, double r3, double r4, double c1, double c2);

	// TODO when needed: Sallen-Key band-pass (there are several versions of this in the 1955 Sallen-Key paper)

	// Multiple-Feedback low-pass
	filter_biquad_device& opamp_mfb_lowpass_setup(double r1, double r2, double r3, double c1, double c2);
	void opamp_mfb_lowpass_modify(double r1, double r2, double r3, double c1, double c2);
	biquad_params opamp_mfb_lowpass_calc(double r1, double r2, double r3, double c1, double c2);

	// Multiple-Feedback band-pass
	filter_biquad_device& opamp_mfb_bandpass_setup(double r1, double r2, double r3, double c1, double c2);

	// Multiple-Feedback high-pass
	filter_biquad_device& opamp_mfb_highpass_setup(double r1, double r2, double c1, double c2, double c3);

	// Differentiator band-pass
	filter_biquad_device& opamp_diff_bandpass_setup(double r1, double r2, double c1, double c2);
	void opamp_diff_bandpass_modify(double r1, double r2, double c1, double c2);
	biquad_params opamp_diff_bandpass_calc(double r1, double r2, double c1, double c2);


protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	void recalc();
	void step();

	sound_stream*  m_stream;
	biquad_type    m_type;
	int            m_last_sample_rate;
	double         m_fc;
	double         m_q;
	double         m_gain;

	stream_buffer::sample_t m_input;
	double m_w0, m_w1, m_w2; /* w[k], w[k-1], w[k-2], current and previous intermediate values */
	stream_buffer::sample_t m_output;
	double m_a1, m_a2; /* digital filter coefficients, denominator */
	double m_b0, m_b1, m_b2;  /* digital filter coefficients, numerator */
};

DECLARE_DEVICE_TYPE(FILTER_BIQUAD, filter_biquad_device)

#endif // MAME_SOUND_FLT_BIQUAD_H
