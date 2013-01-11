/****************************************************************************
 *
 * Phoenix sound hardware simulation - still very ALPHA!
 *
 * If you find errors or have suggestions, please mail me.
 * Juergen Buchmueller <pullmoll@t-online.de>
 *
 ****************************************************************************/


#include "emu.h"
#include "sound/tms36xx.h"
#include "includes/phoenix.h"

/****************************************************************************
 * 4006
 * Dual 4-bit and dual 5-bit serial-in serial-out shift registers.
 *
 *          +----------+
 *      1D5 |1  +--+ 14| VCC
 *     /1Q4 |2       13| 1Q1
 *      CLK |3       12| 2Q0
 *      2D4 |4  4006 11| 2Q0
 *      3D4 |5       10| 3Q0
 *      4D5 |6        9| 4Q0
 *      GND |7        8| 4Q1
 *          +----------+
 *
 * [This information is part of the GIICM]
 *
 * Pin 8 and 9 are connected to an EXOR gate and the inverted
 * output (EXNOR) is fed back to pin 1 (and the pseudo polynome output).
 *
 *      1D5          1Q1  2D4       2Q0  3D4       3Q0  4D5      4Q1 4Q0
 *      +--+--+--+--+--+  +--+--+--+--+  +--+--+--+--+  +--+--+--+--+--+
 *   +->| 0| 1| 2| 3| 4|->| 5| 6| 7| 8|->| 9|10|11|12|->|13|14|15|16|17|
 *   |  +--+--+--+--+--+  +--+--+--+--+  +--+--+--+--+  +--+--+--+--+--+
 *   |                                           ____             |  |
 *   |                                          /    |------------+  |
 *   +-----------------------------------------|EXNOR|               |
 *                                              \____|---------------+
 *
 ****************************************************************************/

#define VMIN    0
#define VMAX    32767

struct c_state
{
	INT32 counter;
	INT32 level;
};

struct n_state
{
	INT32 counter;
	INT32 polyoffs;
	INT32 polybit;
	INT32 lowpass_counter;
	INT32 lowpass_polybit;
};

struct phoenix_sound_state
{
	struct c_state      m_c24_state;
	struct c_state      m_c25_state;
	struct n_state      m_noise_state;
	UINT8               m_sound_latch_a;
	sound_stream *      m_channel;
	UINT32 *                m_poly18;
	device_t *m_discrete;
	device_t *m_tms;
};

INLINE phoenix_sound_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == PHOENIX);

	return (phoenix_sound_state *)downcast<phoenix_sound_device *>(device)->token();
}

INLINE int update_c24(phoenix_sound_state *state, int samplerate)
{
	/*
	 * Noise frequency control (Port B):
	 * Bit 6 lo charges C24 (6.8u) via R51 (330) and when
	 * bit 6 is hi, C24 is discharged through R52 (20k)
	 * in approx. 20000 * 6.8e-6 = 0.136 seconds
	 */
	#define C24 6.8e-6
	#define R49 1000
	#define R51 330
	#define R52 20000

	c_state *c24_state = &state->m_c24_state;

	if( state->m_sound_latch_a & 0x40 )
	{
		if (c24_state->level > VMIN)
		{
			c24_state->counter -= (int)((c24_state->level - VMIN) / (R52 * C24));
			if( c24_state->counter <= 0 )
			{
				int n = -c24_state->counter / samplerate + 1;
				c24_state->counter += n * samplerate;
				if( (c24_state->level -= n) < VMIN)
					c24_state->level = VMIN;
			}
		}
	}
	else
	{
		if (c24_state->level < VMAX)
		{
			c24_state->counter -= (int)((VMAX - c24_state->level) / ((R51+R49) * C24));
			if( c24_state->counter <= 0 )
			{
				int n = -c24_state->counter / samplerate + 1;
				c24_state->counter += n * samplerate;
				if( (c24_state->level += n) > VMAX)
					c24_state->level = VMAX;
			}
		}
	}
	return VMAX - c24_state->level;
}

INLINE int update_c25(phoenix_sound_state *state, int samplerate)
{
	/*
	 * Bit 7 hi charges C25 (6.8u) over a R50 (1k) and R53 (330) and when
	 * bit 7 is lo, C25 is discharged through R54 (47k)
	 * in about 47000 * 6.8e-6 = 0.3196 seconds
	 */
	#define C25 6.8e-6
	#define R50 1000
	#define R53 330
	#define R54 47000

	c_state *c25_state = &state->m_c25_state;

	if( state->m_sound_latch_a & 0x80 )
	{
		if (c25_state->level < VMAX)
		{
			c25_state->counter -= (int)((VMAX - c25_state->level) / ((R50+R53) * C25));
			if( c25_state->counter <= 0 )
			{
				int n = -c25_state->counter / samplerate + 1;
				c25_state->counter += n * samplerate;
				if( (c25_state->level += n) > VMAX )
					c25_state->level = VMAX;
			}
		}
	}
	else
	{
		if (c25_state->level > VMIN)
		{
			c25_state->counter -= (int)((c25_state->level - VMIN) / (R54 * C25));
			if( c25_state->counter <= 0 )
			{
				int n = -c25_state->counter / samplerate + 1;
				c25_state->counter += n * samplerate;
				if( (c25_state->level -= n) < VMIN )
					c25_state->level = VMIN;
			}
		}
	}
	return c25_state->level;
}


INLINE int noise(phoenix_sound_state *state, int samplerate)
{
	int vc24 = update_c24(state, samplerate);
	int vc25 = update_c25(state, samplerate);
	int sum = 0, level, frequency;
	n_state *noise_state = &state->m_noise_state;

	/*
	 * The voltage levels are added and control I(CE) of transistor TR1
	 * (NPN) which then controls the noise clock frequency (linearily?).
	 * level = voltage at the output of the op-amp controlling the noise rate.
	 */
	if( vc24 < vc25 )
		level = vc24 + (vc25 - vc24) / 2;
	else
		level = vc25 + (vc24 - vc25) / 2;

	frequency = 588 + 6325 * level / 32768;

	/*
	 * NE555: Ra=47k, Rb=1k, C=0.05uF
	 * minfreq = 1.44 / ((47000+2*1000) * 0.05e-6) = approx. 588 Hz
	 * R71 (2700 Ohms) parallel to R73 (47k Ohms) = approx. 2553 Ohms
	 * maxfreq = 1.44 / ((2553+2*1000) * 0.05e-6) = approx. 6325 Hz
	 */
	noise_state->counter -= frequency;
	if( noise_state->counter <= 0 )
	{
		int n = (-noise_state->counter / samplerate) + 1;
		noise_state->counter += n * samplerate;
		noise_state->polyoffs = (noise_state->polyoffs + n) & 0x3ffff;
		noise_state->polybit = (state->m_poly18[noise_state->polyoffs>>5] >> (noise_state->polyoffs & 31)) & 1;
	}
	if (!noise_state->polybit)
		sum += vc24;

	/* 400Hz crude low pass filter: this is only a guess!! */
	noise_state->lowpass_counter -= 400;
	if( noise_state->lowpass_counter <= 0 )
	{
		noise_state->lowpass_counter += samplerate;
		noise_state->lowpass_polybit = noise_state->polybit;
	}
	if (!noise_state->lowpass_polybit)
		sum += vc25;

	return sum;
}

static STREAM_UPDATE( phoenix_sound_update )
{
	phoenix_sound_state *state = get_safe_token(device);
	int samplerate = device->machine().sample_rate();
	stream_sample_t *buffer = outputs[0];

	while( samples-- > 0 )
	{
		int sum = 0;
		sum = noise(state, samplerate) / 2;
		*buffer++ = sum < 32768 ? sum > -32768 ? sum : -32768 : 32767;
	}
}


/************************************************************************/
/* phoenix Sound System Analog emulation                                */
/*                                                                      */
/* NOTE: Sample Rate must be at least 44100 for proper emulation.       */
/*                                                                      */
/* April 2005, DR.                                                      */
/************************************************************************/

static const discrete_555_desc phoenix_effect1_555 =
{
	DISC_555_OUT_COUNT_F_X,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_555_desc phoenix_effect2_555 =
{
	DISC_555_OUT_ENERGY,
	5,      // B+ voltage of 555
	DEFAULT_555_CHARGE,
	4.0     // loaded output voltage
};

static const discrete_comp_adder_table phoenix_effect2_cap_sel =
{
	DISC_COMP_P_CAPACITOR,
	CAP_U(0.01),    // C18
	2,
	{CAP_U(0.47), CAP_U(1)} // C16, C17
};

static const discrete_mixer_desc phoenix_effect2_mixer1 =
{
	DISC_MIXER_IS_RESISTOR,
	{RES_K(10), RES_K(5.1) + RES_K(5.1), RES_K(5)}, // R42, R45+R46, internal 555 R
	{0},            // No variable resistor nodes
	{0},            // No caps
	0,              // No rI
	RES_K(10),      // internal 555
	0,0,            // No Filter
	0,              // not used in resistor network
	1   // final gain
};

static const discrete_mixer_desc phoenix_effect2_mixer2 =
{
	DISC_MIXER_IS_RESISTOR,
	{RES_K(5.1), RES_K(5.1)},   // R45, R46
	{0},            // No variable resistor nodes
	{0},            // No caps
	0,              // No rI
	0,              // No rF
	0,0,            // No Filter
	0,              // not used in resistor network
	1   // final gain
};

static const discrete_mixer_desc phoenix_effect2_mixer3 =
{
	DISC_MIXER_IS_RESISTOR,
	{RES_K(10), RES_K(5.1), RES_K(5)},  // R42, R46, internal 555 R
	{0},            // No variable resistor nodes
	{0},            // No caps
	0,              // No rI
	RES_K(10),      // internal 555
	0,0,            // No Filter
	0,              // not used in resistor network
	1   // final gain
};

static const discrete_mixer_desc phoenix_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{RES_K(10+47), RES_K(10+20), RES_K(20), RES_K(20)}, // R19+R21, R38+R47, R67, R68
	{0},            // No variable resistor nodes
	{CAP_U(10), CAP_U(10), CAP_U(.1), CAP_U(10)},       // C6, C31, C29, C30
	0,              // No rI
	RES_K(10),      // VR1
	0,              // No Filter
	CAP_U(10),      // C32
	0,              // not used in resistor network
	40000   // final gain
};

/* Nodes - Inputs */
#define PHOENIX_EFFECT_1_DATA       NODE_01
#define PHOENIX_EFFECT_1_FREQ       NODE_02
#define PHOENIX_EFFECT_1_FILT       NODE_03
#define PHOENIX_EFFECT_2_DATA       NODE_04
#define PHOENIX_EFFECT_2_FREQ       NODE_05
#define PHOENIX_EFFECT_3_EN         NODE_06
#define PHOENIX_EFFECT_4_EN         NODE_07
/* Nodes - Sounds */
#define PHOENIX_EFFECT_1_SND        NODE_10
#define PHOENIX_EFFECT_2_SND        NODE_11
#define PHOENIX_EFFECT_3_SND        0
#define PHOENIX_EFFECT_4_SND        0


DISCRETE_SOUND_START(phoenix)
	/************************************************/
	/* Input register mapping for phoenix           */
	/************************************************/
	DISCRETE_INPUT_DATA (PHOENIX_EFFECT_1_DATA)
	DISCRETE_INPUT_LOGIC(PHOENIX_EFFECT_1_FREQ)
	DISCRETE_INPUT_LOGIC(PHOENIX_EFFECT_1_FILT)
	DISCRETE_INPUT_DATA (PHOENIX_EFFECT_2_DATA)
	DISCRETE_INPUT_DATA (PHOENIX_EFFECT_2_FREQ)
	DISCRETE_INPUT_LOGIC(PHOENIX_EFFECT_3_EN)
	DISCRETE_INPUT_LOGIC(PHOENIX_EFFECT_4_EN)

	/************************************************/
	/* Effect 1                                     */
	/* - shield, bird explode, level 3&4 siren,     */
	/* - level 5 spaceship                          */
	/************************************************/
	/* R22 has been confirmed on real boards as 470 ohm, not 47k in schematics  */
	DISCRETE_RCDISC4(NODE_20,                   /* IC52 output pin 7 */
						1,                          /* ENAB */
						PHOENIX_EFFECT_1_FREQ,      /* Input to O.C. inverter */
						470,                        /* R22 */
						RES_K(100),             /* R23 */
						RES_K(33),                  /* R24 */
						CAP_U(6.8),             /* C7 */
						12,                     /* 12V supply */
						1)                          /* Circuit type 1 */
	DISCRETE_555_ASTABLE_CV(NODE_21,            /* IC20 pin 6 */
							1,                  /* ENAB */
							RES_K(47),          /* R25 */
							RES_K(47),          /* R26 */
							CAP_U(.001),        /* C8 */
							NODE_20,            /* IC48 pin 5 input */
							&phoenix_effect1_555)
	/* LS163 counts rising edge, but the LS14 inverts that */
	DISCRETE_NOTE(NODE_22,                      /* IC21 pin 5 output */
					1,                          /* ENAB */
					NODE_21,                        /* IC13 pin 2 clock input */
					PHOENIX_EFFECT_1_DATA,      /* Pre-load data */
					0x0f,                           /* Maximum count of first counter 0-15 (IC13) */
					1,                          /* Maximum count of second counter 0-1 (IC21) */
					DISC_CLK_BY_COUNT | DISC_OUT_IS_ENERGY) /* Module is clocked externally and we anti-alias output */
	/* When FILT is enabled, the effect is filtered.
	 * While the R20 does decrease the amplitude a little, its main purpose
	 * is to discharge C5 when the filter is disabled. */
	DISCRETE_SWITCH(NODE_23,
					1,                          /* ENAB */
					PHOENIX_EFFECT_1_FILT,
					DEFAULT_TTL_V_LOGIC_1,
					DEFAULT_TTL_V_LOGIC_1 * RES_K(100) / (RES_K(10) + RES_K(100)))  /* R20, R19 */
	DISCRETE_MULTIPLY(NODE_24,
						NODE_22,
						NODE_23)
	DISCRETE_RCFILTER(NODE_25,
						NODE_24,
						1.0/(1.0/RES_K(10) + 1.0/RES_K(100)),   /* R19, R20 */
						CAP_U(.047))                            /* C5 */
	DISCRETE_SWITCH(PHOENIX_EFFECT_1_SND,
					1,                          /* ENAB */
					PHOENIX_EFFECT_1_FILT,
					NODE_24,                    /* non-filtered */
					NODE_25)                    /* filtered */

	/************************************************/
	/* Effect 2                                     */
	/* - bird flying, bird/phoenix/spaceship hit    */
	/* - phoenix wing hit                           */
	/************************************************/
	DISCRETE_COMP_ADDER(NODE_30,                /* total capacitance of selected capacitors */
						PHOENIX_EFFECT_2_FREQ,  /* passed selection bits */
						&phoenix_effect2_cap_sel)
	/* Part of the frequency select also effects the gain */
	DISCRETE_TRANSFORM2(NODE_31,                /* 0/1 state of PHOENIX_EFFECT_2_FREQ high bit */
						PHOENIX_EFFECT_2_FREQ, 2, "01&1/") // get bit 0x02
	DISCRETE_SWITCH(NODE_32,                    /* voltage level */
					1,                          /* ENAB */
					NODE_31,                    /* PHOENIX_EFFECT_2_FREQ high bit determines voltage level */
					DEFAULT_TTL_V_LOGIC_1,
					DEFAULT_TTL_V_LOGIC_1 / 2)
	DISCRETE_555_ASTABLE(NODE_33,               /* pin 3 output of IC44 */
							1,                      /* ENAB */
							RES_K(47),              /* R40 */
							RES_K(100),         /* R41 */
							NODE_30,                /* C16, C17, C18 combined */
							&phoenix_effect2_555)
	/* C20 has been confirmed on real boards as 1uF, not 10uF in schematics  */
	DISCRETE_555_ASTABLE(NODE_34,               /* pin 3 output of IC51 */
							1,                      /* ENAB */
							RES_K(510),         /* R23 */
							RES_K(510),         /* R24 */
							CAP_U(1),               /* C20 */
							&phoenix_effect2_555)
	/* R45 & R46 have been confirmed on real boards as 5.1k, not 51k in schematics  */
	/* We need to work backwards here and calculate the voltage at the junction of R42 & R46 */
	/* If you remove C22 from the real PCB, you can WAVELOG NODE_35 with a gain of 1000 and compare
	 * it against the junction of R42 & R46 on a real PCB. */
	DISCRETE_MIXER3(NODE_35,                    /* Voltage at junction of R42 & R46 with C22 removed */
					1,                          /* ENAB */
					NODE_33,                    /* output from IC44 */
					NODE_34,                    /* output from IC51 */
					5,                          /* B+ connected internally to pin 5 of 555 */
					&phoenix_effect2_mixer1)
	/* Then calculate the voltage going to C22 */
	/* If you remove C22 from the real PCB, you can WAVELOG NODE_36 with a gain of 1000 and compare
	 * it against the junction of R45 & R46 on a real PCB. */
	DISCRETE_MIXER2(NODE_36,                    /* Voltage at junction of R45 & R46 with C22 removed */
					1,                          /* ENAB */
					NODE_34,                    /* pin 3 output of IC51 */
					NODE_35,                    /* Voltage at junction of R42 & R46 with C22 removed */
					&phoenix_effect2_mixer2)
	/* C22 charging is R45 in parallel with R46, R42 and the 555 CV internal resistance */
	DISCRETE_RCFILTER(NODE_37,
						NODE_36,
						1.0/ (1.0/RES_K(5.1) + (1.0/(RES_K(5.1) + 1.0/(1.0/RES_K(10) + 1.0/RES_K(5) + 1.0/RES_K(10)) ))),
						CAP_U(100)) /* R45, R46, R42, internal 555 Rs, C22 */
	/* Now mix from C22 on */
	/* You can WAVELOG NODE_38 with a gain of 1000 and compare it against IC50 pin 5 on a real PCB. */
	DISCRETE_MIXER3(NODE_38,                    /* control voltage to pin 5 of IC50 */
					1,                          /* ENAB */
					NODE_33,                    /* pin 3 output of IC44 */
					NODE_37,                    /* voltage on C22 */
					5,                          /* IC50 internally connected to B+ */
					&phoenix_effect2_mixer3)
	DISCRETE_555_ASTABLE_CV(NODE_39,            /* IC20 pin 8 output */
							1,                  /* ENAB */
							RES_K(20),          /* R47 */
							RES_K(20),          /* R48 */
							CAP_U(0.001),       /* C23 */
							NODE_38,            /* IC50 pin 5 input */
							&phoenix_effect1_555)
	DISCRETE_NOTE(NODE_40,                      /* IC21 pin 9 output */
					1,                          /* ENAB */
					NODE_39,                        /* IC14 pin 2 clock input */
					PHOENIX_EFFECT_2_DATA,      /* Pre-load data */
					0x0f,                           /* Maximum count of first counter 0-15 (IC14) */
					1,                          /* Maximum count of second counter 0-1 (IC21) */
					DISC_CLK_BY_COUNT | DISC_OUT_IS_ENERGY)
	DISCRETE_MULTIPLY(PHOENIX_EFFECT_2_SND,
						NODE_40,                    /* IC21 pin 9 output */
						NODE_32)                    /* voltage level selected by high bit of PHOENIX_EFFECT_2_FREQ */

	/************************************************/
	/* Combine all sound sources.                   */
	/************************************************/
	DISCRETE_MIXER4(NODE_90,
					1,                          /* ENAB */
					PHOENIX_EFFECT_1_SND,
					PHOENIX_EFFECT_2_SND,
					PHOENIX_EFFECT_3_SND,
					PHOENIX_EFFECT_4_SND,
					&phoenix_mixer)

	DISCRETE_OUTPUT(NODE_90, 1)
DISCRETE_SOUND_END

WRITE8_DEVICE_HANDLER( phoenix_sound_control_a_w )
{
	phoenix_sound_state *state = get_safe_token(device);

	discrete_sound_w(state->m_discrete, space, PHOENIX_EFFECT_2_DATA, data & 0x0f);
	discrete_sound_w(state->m_discrete, space, PHOENIX_EFFECT_2_FREQ, (data & 0x30) >> 4);
#if 0
	/* future handling of noise sounds */
	discrete_sound_w(state->m_discrete, space, PHOENIX_EFFECT_3_EN  , data & 0x40);
	discrete_sound_w(state->m_discrete, space, PHOENIX_EFFECT_4_EN  , data & 0x80);
#endif
	state->m_channel->update();
	state->m_sound_latch_a = data;
}

static void register_state(device_t *device)
{
	phoenix_sound_state *state = get_safe_token(device);

	device->save_item(NAME(state->m_sound_latch_a));
	device->save_item(NAME(state->m_c24_state.counter));
	device->save_item(NAME(state->m_c24_state.level));
	device->save_item(NAME(state->m_c25_state.counter));
	device->save_item(NAME(state->m_c25_state.level));
	device->save_item(NAME(state->m_noise_state.counter));
	device->save_item(NAME(state->m_noise_state.polybit));
	device->save_item(NAME(state->m_noise_state.polyoffs));
	device->save_item(NAME(state->m_noise_state.lowpass_counter));
	device->save_item(NAME(state->m_noise_state.lowpass_polybit));
	device->save_pointer(NAME(state->m_poly18), (1ul << (18-5)));
}

WRITE8_DEVICE_HANDLER( phoenix_sound_control_b_w )
{
	phoenix_sound_state *state = get_safe_token(device);

	discrete_sound_w(state->m_discrete, space, PHOENIX_EFFECT_1_DATA, data & 0x0f);
	discrete_sound_w(state->m_discrete, space, PHOENIX_EFFECT_1_FILT, data & 0x20);
	discrete_sound_w(state->m_discrete, space, PHOENIX_EFFECT_1_FREQ, data & 0x10);

	/* update the tune that the MM6221AA is playing */
	mm6221aa_tune_w(state->m_tms, data >> 6);
}

static DEVICE_START( phoenix_sound )
{
	phoenix_sound_state *state = get_safe_token(device);
	int i, j;
	UINT32 shiftreg;

	state->m_sound_latch_a = 0;
	memset(&state->m_c24_state, 0, sizeof(state->m_c24_state));
	memset(&state->m_c25_state, 0, sizeof(state->m_c25_state));
	memset(&state->m_noise_state, 0, sizeof(state->m_noise_state));

	state->m_discrete = device->machine().device("discrete");
	state->m_tms = device->machine().device("tms");

	state->m_poly18 = auto_alloc_array(device->machine(), UINT32, 1ul << (18-5));

	shiftreg = 0;
	for( i = 0; i < (1ul << (18-5)); i++ )
	{
		UINT32 bits = 0;
		for( j = 0; j < 32; j++ )
		{
			bits = (bits >> 1) | (shiftreg << 31);
			if( ((shiftreg >> 16) & 1) == ((shiftreg >> 17) & 1) )
				shiftreg = (shiftreg << 1) | 1;
			else
				shiftreg <<= 1;
		}
		state->m_poly18[i] = bits;
	}

	state->m_channel = device->machine().sound().stream_alloc(*device, 0, 1, device->machine().sample_rate(), 0, phoenix_sound_update);

	register_state(device);
}

const device_type PHOENIX = &device_creator<phoenix_sound_device>;

phoenix_sound_device::phoenix_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PHOENIX, "Phoenix Custom", tag, owner, clock),
		device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(phoenix_sound_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void phoenix_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void phoenix_sound_device::device_start()
{
	DEVICE_START_NAME( phoenix_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void phoenix_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}
