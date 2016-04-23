// license:BSD-3-Clause
// copyright-holders:Derrick Renaud
/***************************************************************************
    polepos.c
    Sound handler
****************************************************************************/
#include "emu.h"
#include "machine/rescap.h"
#include "namco52.h"
#include "namco54.h"
#include "includes/polepos.h"

#define OUTPUT_RATE         24000

#define POLEPOS_R166        1000.0
#define POLEPOS_R167        2200.0
#define POLEPOS_R168        4700.0

/* resistor values when shorted by 4066 running at 5V */
#define POLEPOS_R166_SHUNT  1.0/(1.0/POLEPOS_R166 + 1.0/250)
#define POLEPOS_R167_SHUNT  1.0/(1.0/POLEPOS_R166 + 1.0/250)
#define POLEPOS_R168_SHUNT  1.0/(1.0/POLEPOS_R166 + 1.0/250)

static const double volume_table[8] =
{
	(POLEPOS_R168_SHUNT + POLEPOS_R167_SHUNT + POLEPOS_R166_SHUNT + 2200) / 10000,
	(POLEPOS_R168_SHUNT + POLEPOS_R167_SHUNT + POLEPOS_R166       + 2200) / 10000,
	(POLEPOS_R168_SHUNT + POLEPOS_R167       + POLEPOS_R166_SHUNT + 2200) / 10000,
	(POLEPOS_R168_SHUNT + POLEPOS_R167       + POLEPOS_R166       + 2200) / 10000,
	(POLEPOS_R168       + POLEPOS_R167_SHUNT + POLEPOS_R166_SHUNT + 2200) / 10000,
	(POLEPOS_R168       + POLEPOS_R167_SHUNT + POLEPOS_R166       + 2200) / 10000,
	(POLEPOS_R168       + POLEPOS_R167       + POLEPOS_R166_SHUNT + 2200) / 10000,
	(POLEPOS_R168       + POLEPOS_R167       + POLEPOS_R166       + 2200) / 10000
};

static const double r_filt_out[3] = {RES_K(4.7), RES_K(7.5), RES_K(10)};
static const double r_filt_total = 1.0 / (1.0/RES_K(4.7) + 1.0/RES_K(7.5) + 1.0/RES_K(10));

/* Max filter order */
#define FILTER_ORDER_MAX 51

/* Define to use integer calculation */
#define FILTER_USE_INT

#ifdef FILTER_USE_INT
typedef int filter_real;
#define FILTER_INT_FRACT 15 /* fractional bits */
#else
typedef double filter_real;
#endif

struct filter
{
	filter_real xcoeffs[(FILTER_ORDER_MAX+1)/2];
	unsigned order;
};

struct filter_state
{
	unsigned prev_mac;
	filter_real xprev[FILTER_ORDER_MAX];
};

/* Insert a value in the filter state */
static inline void filter_insert(filter* f, filter_state* s, filter_real x) {
	/* next state */
	++s->prev_mac;
	if (s->prev_mac >= f->order)
		s->prev_mac = 0;

	/* set x[0] */
	s->xprev[s->prev_mac] = x;
}

/* Filter types */
#define FILTER_LOWPASS      0
#define FILTER_HIGHPASS     1
#define FILTER_BANDPASS     2

#define Q_TO_DAMP(q)    (1.0/q)

/* Setup the filter context based on the passed filter type info.
 * type - 1 of the 3 defined filter types
 * fc   - center frequency
 * d    - damp = 1/Q
 * gain - overall filter gain. Set to 1 if not needed.
 */
static void filter2_setup(device_t *device, int type, double fc, double d, double gain,
					filter2_context *filter2);


/* Reset the input/output voltages to 0. */
static void filter2_reset(filter2_context *filter2);


/* Step the filter.
 * x0 is the new input, which needs to be set before stepping.
 * y0 is the new filter output.
 */
static void filter2_step(filter2_context *filter2);


/* Setup a filter2 structure based on an op-amp multipole bandpass circuit.
 * NOTE: If r2 is not used then set to 0.
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
static void filter_opamp_m_bandpass_setup(device_t *device, double r1, double r2, double r3, double c1, double c2,
					filter2_context *filter2);


static void filter2_setup(device_t *device, int type, double fc, double d, double gain,
					filter2_context *filter2)
{
	int sample_rate = device->machine().sample_rate();
	double w;   /* cutoff freq, in radians/sec */
	double w_squared;
	double den; /* temp variable */
	double two_over_T = 2*sample_rate;
	double two_over_T_squared = two_over_T * two_over_T;

	/* calculate digital filter coefficents */
	/*w = 2.0*M_PI*fc; no pre-warping */
	w = sample_rate*2.0*tan(M_PI*fc/sample_rate); /* pre-warping */
	w_squared = w*w;

	den = two_over_T_squared + d*w*two_over_T + w_squared;

	filter2->a1 = 2.0*(-two_over_T_squared + w_squared)/den;
	filter2->a2 = (two_over_T_squared - d*w*two_over_T + w_squared)/den;

	switch (type)
	{
		case FILTER_LOWPASS:
			filter2->b0 = filter2->b2 = w_squared/den;
			filter2->b1 = 2.0*(filter2->b0);
			break;
		case FILTER_BANDPASS:
			filter2->b0 = d*w*two_over_T/den;
			filter2->b1 = 0.0;
			filter2->b2 = -(filter2->b0);
			break;
		case FILTER_HIGHPASS:
			filter2->b0 = filter2->b2 = two_over_T_squared/den;
			filter2->b1 = -2.0*(filter2->b0);
			break;
		default:
			device->logerror("filter2_setup() - Invalid filter type for 2nd order filter.");
			break;
	}

	filter2->b0 *= gain;
	filter2->b1 *= gain;
	filter2->b2 *= gain;
}


/* Reset the input/output voltages to 0. */
static void filter2_reset(filter2_context *filter2)
{
	filter2->x0 = 0;
	filter2->x1 = 0;
	filter2->x2 = 0;
	filter2->y0 = 0;
	filter2->y1 = 0;
	filter2->y2 = 0;
}


/* Step the filter. */
static void filter2_step(filter2_context *filter2)
{
	filter2->y0 = -filter2->a1 * filter2->y1 - filter2->a2 * filter2->y2 +
					filter2->b0 * filter2->x0 + filter2->b1 * filter2->x1 + filter2->b2 * filter2->x2;
	filter2->x2 = filter2->x1;
	filter2->x1 = filter2->x0;
	filter2->y2 = filter2->y1;
	filter2->y1 = filter2->y0;
}


/* Setup a filter2 structure based on an op-amp multipole bandpass circuit. */
static void filter_opamp_m_bandpass_setup(device_t *device, double r1, double r2, double r3, double c1, double c2,
					filter2_context *filter2)
{
	double  r_in, fc, d, gain;

	if (r1 == 0)
	{
		device->logerror("filter_opamp_m_bandpass_setup() - r1 can not be 0");
		return; /* Filter can not be setup.  Undefined results. */
	}

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

	fc = 1.0 / (2 * M_PI * sqrt(r_in * r3 * c1 * c2));
	d = (c1 + c2) / sqrt(r3 / r_in * c1 * c2);
	gain *= -r3 / r_in * c2 / (c1 + c2);

	filter2_setup(device, FILTER_BANDPASS, fc, d, gain, filter2);
}


// device type definition
const device_type POLEPOS = &device_creator<polepos_sound_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  polepos_sound_device - constructor
//-------------------------------------------------

polepos_sound_device::polepos_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, POLEPOS, "Pole Position Audio Custom", tag, owner, clock, "polepos_sound", __FILE__),
		device_sound_interface(mconfig, *this),
		m_current_position(0),
		m_sample_msb(0),
		m_sample_lsb(0),
		m_sample_enable(0),
		m_stream(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void polepos_sound_device::device_start()
{
	m_stream = stream_alloc(0, 1, OUTPUT_RATE);
	m_sample_msb = m_sample_lsb = 0;
	m_sample_enable = 0;

	/* setup the filters */
	filter_opamp_m_bandpass_setup(this, RES_K(220), RES_K(33), RES_K(390), CAP_U(.01),  CAP_U(.01),
									&m_filter_engine[0]);
	filter_opamp_m_bandpass_setup(this, RES_K(150), RES_K(22), RES_K(330), CAP_U(.0047),  CAP_U(.0047),
									&m_filter_engine[1]);
	/* Filter 3 is a little different.  Because of the input capacitor, it is
	 * a high pass filter. */
	filter2_setup(this, FILTER_HIGHPASS, 950, Q_TO_DAMP(.707), 1, &m_filter_engine[2]);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void polepos_sound_device::device_reset()
{
	int loop;
	for (loop = 0; loop < 3; loop++)
		filter2_reset(&m_filter_engine[loop]);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void polepos_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	UINT32 step, clock, slot;
	UINT8 *base;
	double volume, i_total;
	stream_sample_t *buffer = outputs[0];
	int loop;

	/* if we're not enabled, just fill with 0 */
	if (!m_sample_enable)
	{
		memset(buffer, 0, samples * sizeof(*buffer));
		return;
	}

	/* determine the effective clock rate */
	clock = (machine().device("maincpu")->unscaled_clock() / 16) * ((m_sample_msb + 1) * 64 + m_sample_lsb + 1) / (64*64);
	step = (clock << 12) / OUTPUT_RATE;

	/* determine the volume */
	slot = (m_sample_msb >> 3) & 7;
	volume = volume_table[slot];
	base = &machine().root_device().memregion("engine")->base()[slot * 0x800];

	/* fill in the sample */
	while (samples--)
	{
		m_filter_engine[0].x0 = (3.4 / 255 * base[(m_current_position >> 12) & 0x7ff] - 2) * volume;
		m_filter_engine[1].x0 = m_filter_engine[0].x0;
		m_filter_engine[2].x0 = m_filter_engine[0].x0;

		i_total = 0;
		for (loop = 0; loop < 3; loop++)
		{
			filter2_step(&m_filter_engine[loop]);
			/* The op-amp powered @ 5V will clip to 0V & 3.5V.
			 * Adjusted to vRef of 2V, we will clip as follows: */
			if (m_filter_engine[loop].y0 > 1.5) m_filter_engine[loop].y0 = 1.5;
			if (m_filter_engine[loop].y0 < -2)  m_filter_engine[loop].y0 = -2;

			i_total += m_filter_engine[loop].y0 / r_filt_out[loop];
		}
		i_total *= r_filt_total * 32000/2;  /* now contains voltage adjusted by final gain */

		*buffer++ = (int)i_total;
		m_current_position += step;
	}
}


/************************************/
/* Write LSB of engine sound        */
/************************************/
WRITE8_MEMBER( polepos_sound_device::polepos_engine_sound_lsb_w )
{
	/* Update stream first so all samples at old frequency are updated. */
	m_stream->update();
	m_sample_lsb = data & 62;
	m_sample_enable = data & 1;
}

/************************************/
/* Write MSB of engine sound        */
/************************************/
WRITE8_MEMBER( polepos_sound_device::polepos_engine_sound_msb_w )
{
	m_stream->update();
	m_sample_msb = data & 63;
}


/*************************************
 *
 *  Pole Position
 *
 *  Discrete sound emulation: Feb 2007, D.R.
 *
 *************************************/

/* nodes - sounds */
#define POLEPOS_CHANL1_SND      NODE_11
#define POLEPOS_CHANL2_SND      NODE_12
#define POLEPOS_CHANL3_SND      NODE_13
#define POLEPOS_CHANL4_SND      NODE_14

#define POLEPOS_54XX_DAC_R (1.0 / (1.0 / RES_K(47) + 1.0 / RES_K(22) + 1.0 / RES_K(10) + 1.0 / RES_K(4.7)))
static const discrete_dac_r1_ladder polepos_54xx_dac =
{
	4,               /* number of DAC bits */
						/* 54XX_0   54XX_1  54XX_2 */
	{ RES_K(47),     /* R124,    R136,   R152 */
		RES_K(22),   /* R120,    R132,   R142 */
		RES_K(10),   /* R119,    R131,   R138 */
		RES_K(4.7)}, /* R118,    R126,   R103 */
	0, 0, 0, 0       /* nothing extra */
};

#define POLEPOS_52XX_DAC_R (1.0 / (1.0 / RES_K(100) + 1.0 / RES_K(47) + 1.0 / RES_K(22) + 1.0 / RES_K(10)))
static const discrete_dac_r1_ladder polepos_52xx_dac =
{
	4,              /* number of DAC bits */
	{ RES_K(100),   /* R160 */
		RES_K(47),  /* R159 */
		RES_K(22),  /* R155 */
		RES_K(10)}, /* R154 */
	0, 0, 0, 0      /* nothing extra */
};

/*                           R117        R116         R117 */
#define POLEPOS_VREF (5.0 * (RES_K(1) / (RES_K(1.5) + RES_K(1))))

static const discrete_op_amp_filt_info polepos_chanl1_filt =
{
	POLEPOS_54XX_DAC_R + RES_K(22), /* R121 */
	0,                  /* no second input */
	RES_K(12),          /* R125 */
	0,                  /* not used */
	RES_K(120),         /* R122 */
	CAP_U(0.0022),      /* C27 */
	CAP_U(0.0022),      /* C28 */
	0,                  /* not used */
	POLEPOS_VREF,       /* vRef */
	5,                  /* vP */
	0                   /* vN */
};

static const discrete_op_amp_filt_info polepos_chanl2_filt =
{
	POLEPOS_54XX_DAC_R + RES_K(15), /* R133 */
	0,                  /* no second input */
	RES_K(15),          /* R137 */
	0,                  /* not used */
	RES_K(120),         /* R134 */
	CAP_U(0.022),       /* C29 */
	CAP_U(0.022),       /* C30 */
	0,                  /* not used */
	POLEPOS_VREF,       /* vRef */
	5,                  /* vP */
	0                   /* vN */
};

static const discrete_op_amp_filt_info polepos_chanl3_filt =
{
	POLEPOS_54XX_DAC_R + RES_K(22), /* R139 */
	0,                  /* no second input */
	RES_K(22),          /* R143 */
	0,                  /* not used */
	RES_K(180),         /* R140 */
	CAP_U(0.047),       /* C33 */
	CAP_U(0.047),       /* C34 */
	0,                  /* not used */
	POLEPOS_VREF,       /* vRef */
	5,                  /* vP */
	0                   /* vN */
};


DISCRETE_SOUND_START(polepos)

	/************************************************
	 * Input register mapping
	 ************************************************/
	DISCRETE_INPUT_DATA(NAMCO_54XX_0_DATA(NODE_01))
	DISCRETE_INPUT_DATA(NAMCO_54XX_1_DATA(NODE_01))
	DISCRETE_INPUT_DATA(NAMCO_54XX_2_DATA(NODE_01))
	DISCRETE_INPUT_DATA(NAMCO_52XX_P_DATA(NODE_04))

	/************************************************
	 * CHANL1 sound
	 ************************************************/
	DISCRETE_DAC_R1(NODE_20,
					NAMCO_54XX_2_DATA(NODE_01),
					4,          /* 4V - unmeasured*/
					&polepos_54xx_dac)
	DISCRETE_OP_AMP_FILTER(NODE_21,
					1,          /* ENAB */
					NODE_20,    /* INP0 */
					0,          /* INP1 - not used */
					DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &polepos_chanl1_filt)
	/* fake it so 0 is now vRef */
	DISCRETE_ADDER2(POLEPOS_CHANL1_SND,
					1,          /* ENAB */
					NODE_21, -POLEPOS_VREF)

	/************************************************
	 * CHANL2 sound
	 ************************************************/
	DISCRETE_DAC_R1(NODE_30,
					NAMCO_54XX_1_DATA(NODE_01),
					4,          /* 4V - unmeasured*/
					&polepos_54xx_dac)
	DISCRETE_OP_AMP_FILTER(NODE_31,
					1,          /* ENAB */
					NODE_30,    /* INP0 */
					0,          /* INP1 - not used */
					DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &polepos_chanl2_filt)
	/* fake it so 0 is now vRef */
	DISCRETE_ADDER2(POLEPOS_CHANL2_SND,
					1,          /* ENAB */
					NODE_31, -POLEPOS_VREF)

	/************************************************
	 * CHANL3 sound
	 ************************************************/
	DISCRETE_DAC_R1(NODE_40,
					NAMCO_54XX_0_DATA(NODE_01),
					4,          /* 4V - unmeasured*/
					&polepos_54xx_dac)
	DISCRETE_OP_AMP_FILTER(NODE_41,
					1,          /* ENAB */
					NODE_40,    /* INP0 */
					0,          /* INP1 - not used */
					DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &polepos_chanl3_filt)
	/* fake it so 0 is now vRef */
	DISCRETE_ADDER2(POLEPOS_CHANL3_SND,
					1,          /* ENAB */
					NODE_41, -POLEPOS_VREF)

	/************************************************
	 * CHANL4 sound
	 ************************************************/
	/* this circuit was simulated in SPICE and an equivalent filter circuit generated */
	DISCRETE_DAC_R1(NODE_50,
					NAMCO_52XX_P_DATA(NODE_04),
					4,          /* 4V - unmeasured*/
					&polepos_52xx_dac)
	/* fake it so 0 is now vRef */
	DISCRETE_ADDER2(NODE_51,
					1,          /* ENAB */
					NODE_50, -POLEPOS_VREF)
	DISCRETE_FILTER2(NODE_52,
					1,          /* ENAB */
					NODE_51,    /* INP0 */
					100,        /* FREQ */
					1.0 / 0.3,  /* DAMP */
					DISC_FILTER_HIGHPASS)
	DISCRETE_FILTER2(NODE_53,
					1,          /* ENAB */
					NODE_52,    /* INP0 */
					1200,       /* FREQ */
					1.0 / 0.8,  /* DAMP */
					DISC_FILTER_LOWPASS)
	DISCRETE_GAIN(NODE_54,
					NODE_53,    /* IN0 */
					0.5         /* overall filter GAIN */)
	/* clamp to the maximum of the op-amp shifted by vRef */
	DISCRETE_CLAMP(POLEPOS_CHANL4_SND,
					NODE_54,    /* IN0 */
					0,          /* MIN */
					5.0 - OP_AMP_VP_RAIL_OFFSET - POLEPOS_VREF) /* MAX */

	/************************************************
	 * Output
	 ************************************************/
	DISCRETE_OUTPUT(POLEPOS_CHANL1_SND, 32767/2)
	DISCRETE_OUTPUT(POLEPOS_CHANL2_SND, 32767/2)
	DISCRETE_OUTPUT(POLEPOS_CHANL3_SND, 32767/2)
	DISCRETE_OUTPUT(POLEPOS_CHANL4_SND, 32767/2)
DISCRETE_SOUND_END
