/***************************************************************************

    Microprose sound hardware

    Bit of a rush job - needs to be implemented properly eventually

***************************************************************************/

#include "emu.h"
#include "sound/upd7759.h"
#include "includes/micro3d.h"


#define MM5837_CLOCK		100000


/*************************************
 *
 *  Type definitions
 *
 *************************************/

struct biquad
{
	double a0, a1, a2;		/* Numerator coefficients */
	double b0, b1, b2;		/* Denominator coefficients */
};

struct lp_filter
{
	float *history;
	float *coef;
	double fs;
	biquad ProtoCoef[2];
};

struct filter_state
{
	double		capval;
	double		exponent;
};

struct noise_state
{
	union
	{
		struct
		{
			UINT8 vcf;
			UINT8 vcq;
			UINT8 vca;
			UINT8 pan;
		};
		UINT8 dac[4];
	};

	float				gain;
	UINT32				noise_shift;
	UINT8				noise_value;
	UINT8				noise_subcount;

	filter_state		noise_filters[4];
	lp_filter			filter;
	sound_stream		*stream;
};


/*************************************
 *
 *  Pink noise filtering
 *
 *************************************/

/* Borrowed from segasnd.c */
INLINE void configure_filter(filter_state *state, double r, double c)
{
	state->capval = 0;
	state->exponent = 1.0 - exp(-1.0 / (r * c * 2000000/8));
}

INLINE double step_rc_filter(filter_state *state, double input)
{
	state->capval += (input - state->capval) * state->exponent;
	return state->capval;
}

INLINE double step_cr_filter(filter_state *state, double input)
{
	double result = (input - state->capval);
	state->capval += (input - state->capval) * state->exponent;
	return result;
}


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

	iir->coef = (float *)auto_alloc_array_clear(machine, float, 4 * 2 + 1);
	iir->fs = fs;
	iir->history = (float *)auto_alloc_array_clear(machine, float, 2 * 2);
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

	float *coef = iir->coef + 1;

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

void micro3d_noise_sh_w(running_machine &machine, UINT8 data)
{
	micro3d_state *state = machine.driver_data<micro3d_state>();

	if (~data & 8)
	{
		device_t *device = machine.device(data & 4 ? "noise_2" : "noise_1");
		noise_state *nstate = (noise_state *)downcast<micro3d_sound_device *>(device)->token();

		if (state->m_dac_data != nstate->dac[data & 3])
		{
			double q;
			double fc;

			nstate->stream->update();

			nstate->dac[data & 3] = state->m_dac_data;

			if (nstate->vca == 255)
				nstate->gain = 0;
			else
				nstate->gain = exp(-(float)(nstate->vca) / 25.0) * 10.0;

			q = 0.75/255 * (255 - nstate->vcq) + 0.1;
			fc = 4500.0/255 * (255 - nstate->vcf) + 100;

			recompute_filter(&nstate->filter, nstate->gain, q, fc);
		}
	}
}

INLINE noise_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == MICRO3D);

	return (noise_state *)downcast<micro3d_sound_device *>(device)->token();
}

static STREAM_UPDATE( micro3d_stream_update )
{
	noise_state *state = (noise_state *)param;
	lp_filter *iir = &state->filter;
	float pan_l, pan_r;

	stream_sample_t *fl = &outputs[0][0];
	stream_sample_t *fr = &outputs[1][0];

	/* Clear the buffers */
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));
	memset(outputs[1], 0, samples * sizeof(*outputs[1]));

	if (state->gain == 0)
		return;

	pan_l = (float)(255 - state->pan) / 255.0;
	pan_r = (float)(state->pan) / 255.0;

	while (samples--)
	{
		unsigned int i;
		float *hist1_ptr,*hist2_ptr,*coef_ptr;
		float output,new_hist,history1,history2;
		float input, white;
		int step;

		/* Update the noise source */
		for (step = 2000000 / (2000000/8); step >= state->noise_subcount; step -= state->noise_subcount)
		{
			state->noise_shift = (state->noise_shift << 1) | (((state->noise_shift >> 13) ^ (state->noise_shift >> 16)) & 1);
			state->noise_value = (state->noise_shift >> 16) & 1;
			state->noise_subcount = 2000000 / MM5837_CLOCK;
		}
		state->noise_subcount -= step;
		input = (float)state->noise_value - 0.5;
		white = input;

		/* Pink noise filtering */
		state->noise_filters[0].capval = 0.99765 * state->noise_filters[0].capval + input * 0.0990460;
		state->noise_filters[1].capval = 0.96300 * state->noise_filters[1].capval + input * 0.2965164;
		state->noise_filters[2].capval = 0.57000 * state->noise_filters[2].capval + input * 1.0526913;
		input = state->noise_filters[0].capval + state->noise_filters[1].capval + state->noise_filters[2].capval + input * 0.1848;

		input += white;
		input *= 200.0f;

		coef_ptr = iir->coef;

		hist1_ptr = iir->history;
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
		output *= 3.5;

		/* Clip */
		if (output > 32767)
			output = 32767;
		else if (output < -32768)
			output  = -32768;

		*fl++ = output * pan_l;
		*fr++ = output * pan_r;
	}
}


/*************************************
 *
 *  Initialisation
 *
 *************************************/

static DEVICE_START( micro3d_sound )
{
	running_machine &machine = device->machine();
	noise_state *state = get_safe_token(device);

	/* Allocate the stream */
	state->stream = device->machine().sound().stream_alloc(*device, 0, 2, machine.sample_rate(), state, micro3d_stream_update);
	filter_init(machine, &state->filter, machine.sample_rate());

	configure_filter(&state->noise_filters[0], 2.7e3 + 2.7e3, 1.0e-6);
	configure_filter(&state->noise_filters[1], 2.7e3 + 1e3, 0.30e-6);
	configure_filter(&state->noise_filters[2], 2.7e3 + 270, 0.15e-6);
	configure_filter(&state->noise_filters[3], 2.7e3 + 0, 0.082e-6);
//  configure_filter(&state->noise_filters[4], 33e3, 0.1e-6);
}

static DEVICE_RESET( micro3d_sound )
{
	noise_state *state = get_safe_token(device);
	state->noise_shift = 0x15555;
	state->dac[0] = 255;
	state->dac[1] = 255;
	state->dac[2] = 255;
	state->dac[3] = 255;
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
			micro3d_noise_sh_w(machine(), data);
			break;
		}
		case 0x03:
		{
			device_t *upd = machine().device("upd7759");
			upd7759_set_bank_base(upd, (data & 0x4) ? 0x20000 : 0);
			upd7759_reset_w(upd, (data & 0x10) ? 0 : 1);
			break;
		}
	}
}

READ8_MEMBER(micro3d_state::micro3d_sound_io_r)
{
	switch (offset)
	{
		case 0x01:	return (m_sound_port_latch[offset] & 0x7f) | ioport("SOUND_SW")->read();
		case 0x03:	return (m_sound_port_latch[offset] & 0xf7) | (upd7759_busy_r(machine().device("upd7759")) ? 0x08 : 0);
		default:	return 0;
	}
}

WRITE8_MEMBER(micro3d_state::micro3d_upd7759_w)
{
	device_t *device = machine().device("upd7759");
	upd7759_port_w(device, space, 0, data);
	upd7759_start_w(device, 0);
	upd7759_start_w(device, 1);
}


const device_type MICRO3D = &device_creator<micro3d_sound_device>;

micro3d_sound_device::micro3d_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MICRO3D, "Microprose Custom", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(noise_state);
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
	DEVICE_START_NAME( micro3d_sound )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void micro3d_sound_device::device_reset()
{
	DEVICE_RESET_NAME( micro3d_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void micro3d_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


