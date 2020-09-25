// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Couriersud,Derrick Renaud,Frank Palazzolo,Jonathan Gevaryahu
#pragma once

#ifndef MAME_SOUND_FLT_BIQUAD_H
#define MAME_SOUND_FLT_BIQUAD_H

#pragma once

// we need the M_SQRT2 constant
#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

// display debug info about the filters
#undef FLT_BIQUAD_DEBUG_SETUP

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> filter_biquad_device

class filter_biquad_device : public device_t, public device_sound_interface
{
public:
	enum
	{
		LOWPASS1P = 0,
		HIGHPASS1P,
		LOWPASS,
		HIGHPASS,
		BANDPASS,
		NOTCH,
		PEAK,
		LOWSHELF,
		HIGHSHELF
	};

	filter_biquad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration
	filter_biquad_device &setup(int type, double fc, double q, double gain)
	{
		m_type = type;
		m_fc = fc;
		m_q = q;
		m_gain = gain;
		return *this;
	}

	filter_biquad_device &filter_biquad_setup(int type, double fc, double q, double gain)
	{
		m_stream->update();
		setup(type, fc, q, gain);
		recalc();
		return *this;
	}

	/* Setup a biquad filter structure based on an op-amp sallen-key lowpass circuit.
	*
	*                   .----------------------------.
	*                   |                            |
	*                  ---  c2                       |
	*                  ---                           |
	*                   |                            |
	*            r1     |   r2                |\     |
	*   In >----ZZZZ----+--ZZZZ---+--------+  | \    |
	*                             |        '--|+ \   |
	*                            ---  c1      |   >--+------> out
	*                            ---       .--|- /   |
	*                             |        |  | /    |
	*                            gnd       |  |/     |
	*                                      |         |
	*                                      |   r4    |
	*                                      +--ZZZZ---'
	*                                      |
	*                                      Z
	*                                      Z r3
	*                                      Z
	*                                      |
	*                                     gnd
	*/
	filter_biquad_device &opamp_sk_lowpass_setup(double r1, double r2, double r3, double r4, double c1, double c2)
	{
		if ((r1 == 0) || (r2 == 0) || (r3 == 0) || (r4 == 0) || (c1 == 0) || (c2 == 0))
		{
			fatalerror("filter_biquad_device::opamp_sk_lowpass_setup() - no parameters can be 0; parameters were: r1: %f, r2: %f, r3: %f, r4: %f, c1: %f, c2: %f", r1, r2, r3, r4, c1, c2); /* Filter can not be setup.  Undefined results. */
		}

		// note: if R3 doesn't exist, pass a value of RES_M(999.99) or the like, i.e. an 'infinite resistor'
		double const gain = (r3 + r4) / r3;
		double const fc = 1.0 / (2 * M_PI * sqrt(r1 * r2 * c1 * c2));
		double const q = sqrt(r1 * r2 * c1 * c2) / ((r1 * c1) + (r2 * c1) + ((r2 * c2) * (1.0 - gain)));
#ifdef FLT_BIQUAD_DEBUG_SETUP
		fprintf(stderr,"filter_biquad_device::opamp_sk_lowpass_setup() yields: fc = %f, Q = %f, gain = %f\n", fc, q, gain); fflush(stderr);
#endif
		return setup(LOWPASS, fc, q, gain);
	}

	/* Setup a biquad filter structure based on an op-amp multifeedback lowpass circuit.
	* NOTE: vRef is not needed to setup filter.
	*
	*                             .--------+---------.
	*                             |        |         |
	*                             Z       --- c2     |
	*                             Z r3    ---        |
	*                             Z        |         |
	*            r1               |   r2   |  |\     |
	*   In >----ZZZZ----+---------+--ZZZZ--+  | \    |
	*                   |                  '--|- \   |
	*                  ---  c1                |   >--+------> out
	*                  ---                 .--|+ /
	*                   |                  |  | /
	*                  gnd        vRef >---'  |/
	*
	*/
	filter_biquad_device &opamp_mfb_lowpass_setup(double r1, double r2, double r3, double c1, double c2)
	{
		if ((r1 == 0) || (r2 == 0) || (r3 == 0) || (c2 == 0))
		{
			fatalerror("filter_biquad_device::opamp_mfb_lowpass_setup() - only c1 can be 0; parameters were: r1: %f, r2: %f, r3: %f, c1: %f, c2: %f", r1, r2, r3, c1, c2); /* Filter can not be setup.  Undefined results. */
		}

		double const gain = -r3 / r1;
		double fc, q = (M_SQRT2/2);
		if (c1 == 0) // set C1 to 0 to run this filter in a degraded single pole mode where C1 was left off the filter entirely. Certain williams boards seem to have omitted C1, presumably by accident.
		{
			fc = (r1 * r3) / (2 * M_PI * ((r1 * r2) + (r1 * r3) + (r2 * r3)) * r3 * c2);
#ifdef FLT_BIQUAD_DEBUG_SETUP
			fprintf(stderr,"filter_biquad_device::opamp_mfb_lowpass_setup() in degraded mode yields: fc = %f, Q = %f(ignored), gain = %f\n", fc, q, gain); fflush(stderr);
#endif
			return setup(LOWPASS1P, fc, q, gain);
		}
		else
		{
			fc = 1.0 / (2 * M_PI * sqrt(r2 * r3 * c1 * c2));
			q = sqrt(r2 * r3 * c1 * c2) / ((r3 * c2) + (r2 * c2) + ((r2 * c2) * -gain));
#ifdef FLT_BIQUAD_DEBUG_SETUP
			fprintf(stderr,"filter_biquad_device::opamp_mfb_lowpass_setup() yields: fc = %f, Q = %f, gain = %f\n", fc, q, gain); fflush(stderr);
#endif
			return setup(LOWPASS, fc, q, gain);
		}
	}

	/* Setup a biquad filter structure based on an op-amp multipole bandpass circuit.
	* NOTE: If r2 is not used then set it to 0 ohms.
	*       vRef is not needed to setup filter.
	*
	*                             .--------+---------.
	*                             |        |         |
	*                            --- c1    Z         |
	*                            ---       Z r3      |
	*                             |        Z         |
	*            r1               |  c2    |  |\     |
	*   In >----ZZZZ----+---------+--||----+  | \    |
	*                   Z                  '--|- \   |
	*                   Z r2                  |   >--+------> out
	*                   Z                  .--|+ /
	*                   |                  |  | /
	*                  gnd        vRef >---'  |/
	*
	*/
	filter_biquad_device &opamp_mfb_bandpass_setup(double r1, double r2, double r3, double c1, double c2)
	{
		if ((r1 == 0) || (r3 == 0) || (c1 == 0) || (c2 == 0))
		{
			fatalerror("filter_biquad_device::opamp_mfb_bandpass_setup() - only r2 can be 0; parameters were: r1: %f, r2: %f, r3: %f, c1: %f, c2: %f", r1, r2, r3, c1, c2); /* Filter can not be setup.  Undefined results. */
		}

		double r_in, gain;

		if (r2 == 0)
		{
			gain = 1;
			r_in = r1;
		}
		else
		{
			gain = r2 / (r1 + r2);
			r_in = 1.0 / (1.0/r1 + 1.0/r2);
		}

		double const fc = 1.0 / (2 * M_PI * sqrt(r_in * r3 * c1 * c2));
		double const q = sqrt(r3 / r_in * c1 * c2) / (c1 + c2);
		gain *= -r3 / r_in * c2 / (c1 + c2);
#ifdef FLT_BIQUAD_DEBUG_SETUP
		fprintf(stderr,"filter_biquad_device::opamp_mfb_bandpass_setup() yields: fc = %f, Q = %f, gain = %f\n", fc, q, gain); fflush(stderr);
#endif
		return setup(BANDPASS, fc, q, gain);
	}

	/* Setup a biquad filter structure based on an op-amp multifeedback highpass circuit.
	* NOTE: vRef is not needed to setup filter.
	*
	*                             .--------+---------.
	*                             |        |         |
	*                            --- c3    Z         |
	*                            ---       Z r2      |
	*                             |        Z         |
	*            c1               |   c2   |  |\     |
	*   In >-----||-----+---------+---||---+  | \    |
	*                   Z                  '--|- \   |
	*                   Z r1                  |   >--+------> out
	*                   Z                  .--|+ /
	*                   |                  |  | /
	*                  gnd        vRef >---'  |/
	*
	*/
	filter_biquad_device &opamp_mfb_highpass_setup(double r1, double r2, double c1, double c2, double c3)
	{
		if ((r1 == 0) || (r2 == 0) || (c1 == 0) || (c2 == 0) || (c3 == 0))
		{
			fatalerror("filter_biquad_device::opamp_mfb_highpass_setup() - no parameters can be 0; parameters were: r1: %f, r2: %f, c1: %f, c2: %f, c3: %f", r1, r2, c1, c2, c3); /* Filter can not be setup.  Undefined results. */
		}
		// TODO: if c1 is 0/shorted, should the circuit should work with a gain of 1 in a first order mode?

		double const gain = -c1 / c3;
		double const fc = 1.0 / (2 * M_PI * sqrt(c2 * c3 * r1 * r2));
		double const q = sqrt(c2 * c3 * r1 * r2) / ((c2 * r1) + (c3 * r1) + ((c3 * r1) * -gain));
#ifdef FLT_BIQUAD_DEBUG_SETUP
		fprintf(stderr,"filter_biquad_device::opamp_mfb_highpass_setup() yields: fc = %f, Q = %f, gain = %f\n", fc, q, gain); fflush(stderr);
#endif
		return setup(HIGHPASS, fc, q, gain);
	}

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	void recalc();
	void step();

// some sane defaults for a highpass filter with a cutoff at 16hz, same as flt_rc's 'ac' mode.
private:
	sound_stream*  m_stream;
	int            m_type = HIGHPASS;
	int            m_last_sample_rate;
	double         m_fc = 16.0;
	double         m_q = M_SQRT2 / 2.0;
	double         m_gain = 1.0;

	stream_buffer::sample_t m_input = 0.0;
	double m_w0 = 0.0, m_w1 = 0.0, m_w2 = 0.0;  /* w[k], w[k-1], w[k-2], current and previous intermediate values */
	stream_buffer::sample_t m_output = 0.0;
	double m_a1 = 0.0, m_a2 = 0.0;              /* digital filter coefficients, denominator */
	double m_b0 = 1.0, m_b1 = 0.0, m_b2 = 0.0;  /* digital filter coefficients, numerator */
};

DECLARE_DEVICE_TYPE(FILTER_BIQUAD, filter_biquad_device)

#endif // MAME_SOUND_FLT_BIQUAD_H
