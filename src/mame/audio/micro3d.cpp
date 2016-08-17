// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Microprose sound hardware

    Bit of a rush job - needs to be implemented properly eventually

***************************************************************************/

#include "emu.h"
#include "includes/micro3d.h"


#define MM5837_CLOCK        100000


/*************************************
 *
 *  Pink noise filtering
 *
 *************************************/

/* Borrowed from segasnd.c */
static inline void configure_filter(m3d_filter_state *state, double r, double c)
{
	state->capval = 0;
	state->exponent = 1.0 - exp(-1.0 / (r * c * 2000000/8));
}

#if 0
static inline double step_rc_filter(m3d_filter_state *state, double input)
{
	state->capval += (input - state->capval) * state->exponent;
	return state->capval;
}

static inline double step_cr_filter(m3d_filter_state *state, double input)
{
	double result = (input - state->capval);
	state->capval += (input - state->capval) * state->exponent;
	return result;
}
#endif


/*************************************
 *
 *  SSM2047 simulation
 *
 *************************************/

static void filter_init(running_machine &machine, lp_filter *iir, double fs)
{
	/* Section 1 */
	iir->ProtoCoef[0].a0 = 1.0;
	iir->ProtoCoef[0].a1 = 0;
	iir->ProtoCoef[0].a2 = 0;
	iir->ProtoCoef[0].b0 = 1.0;
	iir->ProtoCoef[0].b1 = 0.765367;
	iir->ProtoCoef[0].b2 = 1.0;

	/* Section 2 */
	iir->ProtoCoef[1].a0 = 1.0;
	iir->ProtoCoef[1].a1 = 0;
	iir->ProtoCoef[1].a2 = 0;
	iir->ProtoCoef[1].b0 = 1.0;
	iir->ProtoCoef[1].b1 = 1.847759;
	iir->ProtoCoef[1].b2 = 1.0;

	iir->coef = make_unique_clear<float[]>(4 * 2 + 1);
	iir->fs = fs;
	iir->history = make_unique_clear<float[]>(2 * 2);
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

static void recompute_filter(lp_filter *iir, double k, double q, double fc)
{
	int nInd;
	double a0, a1, a2, b0, b1, b2;

	float *coef = iir->coef.get() + 1;

	for (nInd = 0; nInd < 2; nInd++)
	{
		a0 = iir->ProtoCoef[nInd].a0;
		a1 = iir->ProtoCoef[nInd].a1;
		a2 = iir->ProtoCoef[nInd].a2;

		b0 = iir->ProtoCoef[nInd].b0;
		b1 = iir->ProtoCoef[nInd].b1 / q;
		b2 = iir->ProtoCoef[nInd].b2;

		prewarp(&a0, &a1, &a2, fc, iir->fs);
		prewarp(&b0, &b1, &b2, fc, iir->fs);
		bilinear(a0, a1, a2, b0, b1, b2, &k, iir->fs, coef);

		coef += 4;
	}

	iir->coef[0] = k;
}

void micro3d_sound_device::noise_sh_w(UINT8 data)
{
	micro3d_state *state = machine().driver_data<micro3d_state>();

	if (~data & 8)
	{
		if (state->m_dac_data != m_dac[data & 3])
		{
			double q;
			double fc;

			m_stream->update();

			m_dac[data & 3] = state->m_dac_data;

			if (m_dac[VCA] == 255)
				m_gain = 0;
			else
				m_gain = expf(-(float)(m_dac[VCA]) / 25.0f) * 10.0f;

			q = 0.75/255 * (255 - m_dac[VCQ]) + 0.1;
			fc = 4500.0/255 * (255 - m_dac[VCF]) + 100;

			recompute_filter(&m_filter, m_gain, q, fc);
		}
	}
}


/*************************************
 *
 *  Initialisation
 *
 *************************************/


const device_type MICRO3D = &device_creator<micro3d_sound_device>;

micro3d_sound_device::micro3d_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MICRO3D, "Microprose Audio Custom", tag, owner, clock, "micro3d_sound", __FILE__),
		device_sound_interface(mconfig, *this),
		m_gain(0),
		m_noise_shift(0),
		m_noise_value(0),
		m_noise_subcount(0),
		m_stream(nullptr)

{
		memset(m_dac, 0, sizeof(UINT8)*4);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void micro3d_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void micro3d_sound_device::device_start()
{
	/* Allocate the stream */
	m_stream = machine().sound().stream_alloc(*this, 0, 2, machine().sample_rate());
	filter_init(machine(), &m_filter, machine().sample_rate());

	configure_filter(&m_noise_filters[0], 2.7e3 + 2.7e3, 1.0e-6);
	configure_filter(&m_noise_filters[1], 2.7e3 + 1e3, 0.30e-6);
	configure_filter(&m_noise_filters[2], 2.7e3 + 270, 0.15e-6);
	configure_filter(&m_noise_filters[3], 2.7e3 + 0, 0.082e-6);
//  configure_filter(&m_noise_filters[4], 33e3, 0.1e-6);
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

void micro3d_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	lp_filter *iir = &m_filter;
	float pan_l, pan_r;

	stream_sample_t *fl = &outputs[0][0];
	stream_sample_t *fr = &outputs[1][0];

	/* Clear the buffers */
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));
	memset(outputs[1], 0, samples * sizeof(*outputs[1]));

	if (m_gain == 0)
		return;

	pan_l = (float)(255 - m_dac[PAN]) / 255.0f;
	pan_r = (float)(m_dac[PAN]) / 255.0f;

	while (samples--)
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

		coef_ptr = iir->coef.get();

		hist1_ptr = iir->history.get();
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
		output *= 3.5f;

		/* Clip */
		if (output > 32767)
			output = 32767;
		else if (output < -32768)
			output  = -32768;

		*fl++ = output * pan_l;
		*fr++ = output * pan_r;
	}
}

/***************************************************************************

    8031 port mappings:

    Port 1                          Port 2
    =======                         ======
    0: S/H sel A     (O)            0:
    1: S/H sel B     (O)            1:
    2: S/H sel C     (O)            2: uPD bank select (O)
    3: S/H en        (O)            3: /uPD busy       (I)
    4: DS1267 data   (O)            4: /uPD reset      (O)
    5: DS1267 clock  (O)            5: Watchdog reset  (O)
    6: /DS1267 reset (O)            6:
    7: Test SW       (I)            7:

***************************************************************************/


WRITE8_MEMBER(micro3d_state::micro3d_snd_dac_a)
{
	m_dac_data = data;
}

WRITE8_MEMBER(micro3d_state::micro3d_snd_dac_b)
{
	/* TODO: This controls upd7759 volume */
}

WRITE8_MEMBER(micro3d_state::micro3d_sound_io_w)
{
	m_sound_port_latch[offset] = data;

	switch (offset)
	{
		case 0x01:
		{
			micro3d_sound_device *noise = (data & 4) ? m_noise_2 : m_noise_1;
			noise->noise_sh_w(data);
			break;
		}
		case 0x03:
		{
			m_upd7759->set_bank_base((data & 0x4) ? 0x20000 : 0);
			m_upd7759->reset_w((data & 0x10) ? 0 : 1);
			break;
		}
	}
}

READ8_MEMBER(micro3d_state::micro3d_sound_io_r)
{
	switch (offset)
	{
		case 0x01:  return (m_sound_port_latch[offset] & 0x7f) | m_sound_sw->read();
		case 0x03:  return (m_sound_port_latch[offset] & 0xf7) | (m_upd7759->busy_r() ? 0x08 : 0);
		default:    return 0;
	}
}

WRITE8_MEMBER(micro3d_state::micro3d_upd7759_w)
{
	m_upd7759->port_w(space, 0, data);
	m_upd7759->start_w(0);
	m_upd7759->start_w(1);
}
