// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Microprose sound hardware

    Bit of a rush job - needs to be implemented properly eventually

***************************************************************************/

#include "emu.h"
#include "micro3d_a.h"


#define MM5837_CLOCK        100000


/*************************************
 *
 *  Pink noise filtering
 *
 *************************************/

/* Borrowed from segasnd.c */
inline void micro3d_sound_device::m3d_filter_state::configure(double r, double c)
{
	capval = 0;
	exponent = 1.0 - exp(-1.0 / (r * c * 2000000/8));
}

#if 0
inline double micro3d_sound_device::m3d_filter_state::step_rc_filter(double input)
{
	capval += (input - capval) * exponent;
	return capval;
}

inline double micro3d_sound_device::m3d_filter_state::step_cr_filter(double input)
{
	double const result = input - capval;
	capval += result * exponent;
	return result;
}
#endif


/*************************************
 *
 *  SSM2047 simulation
 *
 *************************************/

void micro3d_sound_device::lp_filter::init(double fsval)
{
	/* Section 1 */
	proto_coef[0].a0 = 1.0;
	proto_coef[0].a1 = 0;
	proto_coef[0].a2 = 0;
	proto_coef[0].b0 = 1.0;
	proto_coef[0].b1 = 0.765367;
	proto_coef[0].b2 = 1.0;

	/* Section 2 */
	proto_coef[1].a0 = 1.0;
	proto_coef[1].a1 = 0;
	proto_coef[1].a2 = 0;
	proto_coef[1].b0 = 1.0;
	proto_coef[1].b1 = 1.847759;
	proto_coef[1].b2 = 1.0;

	std::fill(std::begin(coef), std::end(coef), 0);
	fs = fsval;
	std::fill(std::begin(history), std::end(history), 0);
}

static void prewarp(double *a0, double *a1, double *a2,double fc, double fs)
{
	double wp, pi;

	pi = 4.0 * atan(1.0);
	wp = 2.0 * fs * tan(pi * fc / fs);

	*a2 = *a2 / (wp * wp);
	*a1 = *a1 / wp;
}

static void bilinear(double a0, double a1, double a2,
				double b0, double b1, double b2,
				double *k, double fs, float *coef)
{
	double ad, bd;

	ad = 4. * a2 * fs * fs + 2. * a1 * fs + a0;
	bd = 4. * b2 * fs * fs + 2. * b1* fs + b0;

	*k *= ad/bd;

	*coef++ = (2. * b0 - 8. * b2 * fs * fs) / bd;
	*coef++ = (4. * b2 * fs * fs - 2. * b1 * fs + b0) / bd;

	*coef++ = (2. * a0 - 8. * a2 * fs * fs) / ad;
	*coef = (4. * a2 * fs * fs - 2. * a1 * fs + a0) / ad;
}

void micro3d_sound_device::lp_filter::recompute(double k, double q, double fc)
{
	float *c = &coef[1];

	for (int nInd = 0; nInd < 2; nInd++)
	{
		double a0 = proto_coef[nInd].a0;
		double a1 = proto_coef[nInd].a1;
		double a2 = proto_coef[nInd].a2;

		double b0 = proto_coef[nInd].b0;
		double b1 = proto_coef[nInd].b1 / q;
		double b2 = proto_coef[nInd].b2;

		prewarp(&a0, &a1, &a2, fc, fs);
		prewarp(&b0, &b1, &b2, fc, fs);
		bilinear(a0, a1, a2, b0, b1, b2, &k, fs, c);

		c += 4;
	}

	coef[0] = k;
}

void micro3d_sound_device::noise_sh_w(u8 data)
{
	if (~data & 8)
	{
		if (m_dac_data != m_dac[data & 3])
		{
			double q;
			double fc;

			m_stream->update();

			m_dac[data & 3] = m_dac_data;

			if (m_dac[VCA] == 255)
				m_gain = 0;
			else
				m_gain = expf(-(float)(m_dac[VCA]) / 25.0f) * 10.0f;

			q = 0.75/255 * (255 - m_dac[VCQ]) + 0.1;
			fc = 4500.0/255 * (255 - m_dac[VCF]) + 100;

			m_filter.recompute(m_gain, q, fc);
		}
	}
}


/*************************************
 *
 *  Initialisation
 *
 *************************************/


DEFINE_DEVICE_TYPE(MICRO3D_SOUND, micro3d_sound_device, "micro3d_sound", "Microprose Custom Sound")

micro3d_sound_device::micro3d_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MICRO3D_SOUND, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_gain(0),
		m_noise_shift(0),
		m_noise_value(0),
		m_noise_subcount(0),
		m_stream(nullptr)

{
	memset(m_dac, 0, sizeof(u8)*4);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void micro3d_sound_device::device_start()
{
	/* Allocate the stream */
	m_stream = stream_alloc(0, 2, machine().sample_rate());
	m_filter.init(machine().sample_rate());

	m_noise_filters[0].configure(2.7e3 + 2.7e3, 1.0e-6);
	m_noise_filters[1].configure(2.7e3 + 1e3, 0.30e-6);
	m_noise_filters[2].configure(2.7e3 + 270, 0.15e-6);
	m_noise_filters[3].configure(2.7e3 + 0, 0.082e-6);
//  m_noise_filters[4].configure(33e3, 0.1e-6);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void micro3d_sound_device::device_reset()
{
	m_noise_shift = 0x15555;
	m_dac[0] = 255;
	m_dac[1] = 255;
	m_dac[2] = 255;
	m_dac[3] = 255;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void micro3d_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	lp_filter *iir = &m_filter;
	float pan_l, pan_r;

	auto &fl = outputs[0];
	auto &fr = outputs[1];

	/* Clear the buffers */
	fl.fill(0);
	fr.fill(0);

	if (m_gain == 0)
		return;

	pan_l = (float)(255 - m_dac[PAN]) / 255.0f;
	pan_r = (float)(m_dac[PAN]) / 255.0f;

	for (int sampindex = 0; sampindex < fl.samples(); sampindex++)
	{
		unsigned int i;
		float *hist1_ptr,*hist2_ptr,*coef_ptr;
		float output,new_hist,history1,history2;
		float input, white;
		int step;

		/* Update the noise source */
		for (step = 2000000 / (2000000/8); step >= m_noise_subcount; step -= m_noise_subcount)
		{
			m_noise_shift = (m_noise_shift << 1) | (((m_noise_shift >> 13) ^ (m_noise_shift >> 16)) & 1);
			m_noise_value = (m_noise_shift >> 16) & 1;
			m_noise_subcount = 2000000 / MM5837_CLOCK;
		}
		m_noise_subcount -= step;
		input = (float)m_noise_value - 0.5f;
		white = input;

		/* Pink noise filtering */
		m_noise_filters[0].capval = 0.99765 * m_noise_filters[0].capval + input * 0.0990460;
		m_noise_filters[1].capval = 0.96300 * m_noise_filters[1].capval + input * 0.2965164;
		m_noise_filters[2].capval = 0.57000 * m_noise_filters[2].capval + input * 1.0526913;
		input = m_noise_filters[0].capval + m_noise_filters[1].capval + m_noise_filters[2].capval + input * 0.1848;

		input += white;
		input *= 200.0f;

		coef_ptr = &iir->coef[0];

		hist1_ptr = &iir->history[0];
		hist2_ptr = hist1_ptr + 1;

		/* 1st number of coefficients array is overall input scale factor, * or filter gain */
		output = input * (*coef_ptr++);

		for (i = 0 ; i < 2; i++)
		{
			history1 = *hist1_ptr;
			history2 = *hist2_ptr;

			output = output - history1 * (*coef_ptr++);
			new_hist = output - history2 * (*coef_ptr++);

			output = new_hist + history1 * (*coef_ptr++);
			output = output + history2 * (*coef_ptr++);

			*hist2_ptr++ = *hist1_ptr;
			*hist1_ptr++ = new_hist;
			hist1_ptr++;
			hist2_ptr++;
		}
		output *= 3.5f / 32768.f;

		fl.put_clamp(sampindex, output * pan_l, 1.0);
		fr.put_clamp(sampindex, output * pan_r, 1.0);
	}
}
