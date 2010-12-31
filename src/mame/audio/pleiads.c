/****************************************************************************
 *
 * Sound hardware for Pleiades, Naughty Boy and Pop Flamer.
 *
 * If you find errors or have suggestions, please mail me.
 * Juergen Buchmueller <pullmoll@t-online.de>
 *
 ****************************************************************************/
#include "emu.h"
#include "streams.h"
#include "sound/tms36xx.h"
#include "audio/pleiads.h"

#define VMIN	0
#define VMAX	32767

/* fixed 8kHz clock */
#define TONE1_CLOCK  8000

struct t_state
{
	int counter;
	int output;
	int max_freq;
};

struct c_state
{
	int counter;
	int level;
	double charge_time;
	double discharge_time;
};

struct n_state
{
	int counter;
	int polyoffs;
	int freq;
};

typedef struct _pleiads_sound_state pleiads_sound_state;
struct _pleiads_sound_state
{
	device_t *tms;
	sound_stream *channel;

	int sound_latch_a;
	int sound_latch_b;
	int sound_latch_c;	/* part of the videoreg_w latch */

	UINT32 *poly18;
	int polybit;

	t_state tone1;
	t_state tone2;
	t_state tone3;
	t_state tone4;

	c_state pa5;
	c_state pa6;
	c_state pb4;
	c_state pc4;
	c_state pc5;

	n_state noise;

	int pa5_resistor;
	int pc5_resistor;
	int polybit_resistor;
	int opamp_resistor;
};

INLINE pleiads_sound_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == PLEIADS || device->type() == POPFLAME || device->type() == NAUGHTYB);

	return (pleiads_sound_state *)downcast<legacy_device_base *>(device)->token();
}


/*****************************************************************************
 * Tone #1 is a fixed 8 kHz signal divided by 1 to 15.
 *****************************************************************************/
INLINE int tone1(pleiads_sound_state *state, int samplerate)
{
	if( (state->sound_latch_a & 15) != 15 )
	{
		state->tone1.counter -= TONE1_CLOCK;
		while( state->tone1.counter <= 0 )
		{
			state->tone1.counter += samplerate;
			if( ++state->tone1.max_freq == 16 )
			{
				state->tone1.max_freq = state->sound_latch_a & 15;
				state->tone1.output ^= 1;
			}
		}
	}
	return state->tone1.output ? VMAX : -VMAX;
}

/*****************************************************************************
 * Tones #2 and #3 are coming from the upper 556 chip
 * It's labelled IC96 in Pop Flamer, 4D(??) in Naughty Boy.
 * C68 controls the frequencies of tones #2 and #3 (V/C inputs)
 *****************************************************************************/
INLINE int update_pb4(pleiads_sound_state *state, int samplerate)
{
	/* bit 4 of latch B: charge 10uF (C28/C68) through 10k (R19/R25) */
	if( state->sound_latch_b & 0x10 )
	{
		if( state->pb4.level < VMAX )
		{
			state->pb4.counter -= (int)((VMAX - state->pb4.level) / state->pb4.charge_time);
			if( state->pb4.counter <= 0 )
			{
				int n = (-state->pb4.counter / samplerate) + 1;
				state->pb4.counter += n * samplerate;
				if( (state->pb4.level += n) > VMAX )
					state->pb4.level = VMAX;
			}
		}
	}
	else
	{
		if( state->pb4.level > VMIN )
		{
			state->pb4.counter -= (int)((state->pb4.level - VMIN) / state->pb4.discharge_time);
			if( state->pb4.counter <= 0 )
			{
				int n = (-state->pb4.counter / samplerate) + 1;
				state->pb4.counter += n * samplerate;
				if( (state->pb4.level -= n) < VMIN)
					state->pb4.level = VMIN;
			}
		}
	}
	return state->pb4.level;
}

INLINE int tone23(pleiads_sound_state *state, int samplerate)
{
	int level = VMAX - update_pb4(state, samplerate);
	int sum = 0;

	/* bit 5 = low: tone23 disabled */
	if( (state->sound_latch_b & 0x20) == 0 )
		return sum;

	/* modulate timers from the upper 556 with the voltage on Cxx on PB4. */
	if( level < VMAX )
	{
		state->tone2.counter -= state->tone2.max_freq * level / 32768;
		if( state->tone2.counter <= 0 )
		{
			int n = (-state->tone2.counter / samplerate) + 1;
			state->tone2.counter += n * samplerate;
			state->tone2.output = (state->tone2.output + n) & 1;
		}

		state->tone3.counter -= state->tone3.max_freq*1/3 + state->tone3.max_freq*2/3 * level / 33768;
		if( state->tone3.counter <= 0 )
		{
			int n = (-state->tone2.counter / samplerate) + 1;
			state->tone3.counter += samplerate;
			state->tone3.output = (state->tone3.output + n) & 1;
		}
	}

	sum += (state->tone2.output) ? VMAX : -VMAX;
	sum += (state->tone3.output) ? VMAX : -VMAX;

	return sum / 2;
}

/*****************************************************************************
 * Tone #4 comes from upper half of the lower 556 (IC98 in Pop Flamer)
 * It's modulated by the voltage at C49, which is then divided between
 * 0V or 5V, depending on the polynome output bit.
 * The tone signal gates two signals (bits 5 of latches A and C), but
 * these are also swept between two levels (C52 and C53 in Pop Flamer).
 *****************************************************************************/
INLINE int update_c_pc4(pleiads_sound_state *state, int samplerate)
{
	#define PC4_MIN (int)(VMAX * 7 / 50)

	/* bit 4 of latch C: (part of videoreg_w) hi? */
	if (state->sound_latch_c & 0x10)
	{
		if (state->pc4.level < VMAX)
		{
			state->pc4.counter -= (int)((VMAX - state->pc4.level) / state->pc4.charge_time);
			if( state->pc4.counter <= 0 )
			{
				int n = (-state->pc4.counter / samplerate) + 1;
				state->pc4.counter += n * samplerate;
				if( (state->pc4.level += n) > VMAX )
					state->pc4.level = VMAX;
			}
		}
	}
	else
	{
		if (state->pc4.level > PC4_MIN)
		{
			state->pc4.counter -= (int)((state->pc4.level - PC4_MIN) / state->pc4.discharge_time);
			if( state->pc4.counter <= 0 )
			{
				int n = (-state->pc4.counter / samplerate) + 1;
				state->pc4.counter += n * samplerate;
				if( (state->pc4.level -= n) < PC4_MIN )
					state->pc4.level = PC4_MIN;
			}
		}
	}
	return state->pc4.level;
}

INLINE int update_c_pc5(pleiads_sound_state *state, int samplerate)
{
	/* bit 5 of latch C: charge or discharge C52 */
	if (state->sound_latch_c & 0x20)
	{
		if (state->pc5.level < VMAX)
		{
			state->pc5.counter -= (int)((VMAX - state->pc5.level) / state->pc5.charge_time);
			if( state->pc5.counter <= 0 )
			{
				int n = (-state->pc5.counter / samplerate) + 1;
				state->pc5.counter += n * samplerate;
				if( (state->pc5.level += n) > VMAX )
					state->pc5.level = VMAX;
			}
		}
	}
	else
	{
		if (state->pc5.level > VMIN)
		{
			state->pc5.counter -= (int)((state->pc5.level - VMIN) / state->pc5.discharge_time);
			if( state->pc5.counter <= 0 )
			{
				int n = (-state->pc5.counter / samplerate) + 1;
				state->pc5.counter += samplerate;
				if( (state->pc5.level -= n) < VMIN )
					state->pc5.level = VMIN;
			}
		}
	}
	return state->pc5.level;
}

INLINE int update_c_pa5(pleiads_sound_state *state, int samplerate)
{
	/* bit 5 of latch A: charge or discharge C63 */
	if (state->sound_latch_a & 0x20)
	{
		if (state->pa5.level < VMAX)
		{
			state->pa5.counter -= (int)((VMAX - state->pa5.level) / state->pa5.charge_time);
			if( state->pa5.counter <= 0 )
			{
				int n = (-state->pa5.counter / samplerate) + 1;
				state->pa5.counter += n * samplerate;
				if( (state->pa5.level += n) > VMAX )
					state->pa5.level = VMAX;
			}
		}
	}
	else
	{
		if (state->pa5.level > VMIN)
		{
			state->pa5.counter -= (int)((state->pa5.level - VMIN) / state->pa5.discharge_time);
			if( state->pa5.counter <= 0 )
			{
				int n = (-state->pa5.counter / samplerate) + 1;
				state->pa5.counter += samplerate;
				if( (state->pa5.level -= n) < VMIN )
					state->pa5.level = VMIN;
			}
		}
	}
	return state->pa5.level;
}

INLINE int tone4(pleiads_sound_state *state, int samplerate)
{
	int level = update_c_pc4(state, samplerate);
	int vpc5 = update_c_pc5(state, samplerate);
	int vpa5 = update_c_pa5(state, samplerate);
	int sum;

	/* Two resistors divide the output voltage of the op-amp between
     * polybit = 0: 0V and level: x * opamp_resistor / (opamp_resistor + polybit_resistor)
     * polybit = 1: level and 5V: x * polybit_resistor / (opamp_resistor + polybit_resistor)
     */
	if (state->polybit)
		level = level + (VMAX - level) * state->opamp_resistor / (state->opamp_resistor + state->polybit_resistor);
	else
		level = level * state->polybit_resistor / (state->opamp_resistor + state->polybit_resistor);

	state->tone4.counter -= state->tone4.max_freq * level / 32768;
	if( state->tone4.counter <= 0 )
	{
		int n = (-state->tone4.counter / samplerate) + 1;
		state->tone4.counter += n * samplerate;
		state->tone4.output = (state->tone4.output + n) & 1;
	}

	/* mix the two signals */
	sum = vpc5 * state->pa5_resistor / (state->pa5_resistor + state->pc5_resistor) +
		  vpa5 * state->pc5_resistor / (state->pa5_resistor + state->pc5_resistor);

	return (state->tone4.output) ? sum : -sum;
}

/*****************************************************************************
 * Noise comes from a shift register (4006) hooked up just like in Phoenix.
 * Difference: the clock frequecy is toggled between two values only by
 * bit 4 of latch A. The output of the first shift register can be zapped(?)
 * by some control line (IC87 in Pop Flamer: not yet implemented)
 *****************************************************************************/
INLINE int update_c_pa6(pleiads_sound_state *state, int samplerate)
{
	/* bit 6 of latch A: charge or discharge C63 */
	if (state->sound_latch_a & 0x40)
	{
		if (state->pa6.level < VMAX)
		{
			state->pa6.counter -= (int)((VMAX - state->pa6.level) / state->pa6.charge_time);
			if( state->pa6.counter <= 0 )
			{
				int n = (-state->pa6.counter / samplerate) + 1;
				state->pa6.counter += n * samplerate;
				if( (state->pa6.level += n) > VMAX )
					state->pa6.level = VMAX;
			}
		}
	}
	else
	{
		/* only discharge of poly bit is active */
		if (state->polybit && state->pa6.level > VMIN)
		{
			/* discharge 10uF through 10k -> 0.1s */
			state->pa6.counter -= (int)((state->pa6.level - VMIN) / 0.1);
			if( state->pa6.counter <= 0 )
			{
				int n = (-state->pa6.counter / samplerate) + 1;
				state->pa6.counter += n * samplerate;
				if( (state->pa6.level -= n) < VMIN )
					state->pa6.level = VMIN;
			}
		}
	}
	return state->pa6.level;
}


INLINE int noise(pleiads_sound_state *state, int samplerate)
{
	int c_pa6_level = update_c_pa6(state, samplerate);
	int sum = 0;

	/*
     * bit 4 of latch A: noise counter rate modulation?
     * CV2 input of lower 556 is connected via 2k resistor
     */
	if ( state->sound_latch_a & 0x10 )
		state->noise.counter -= state->noise.freq * 2 / 3; /* ????? */
	else
		state->noise.counter -= state->noise.freq * 1 / 3; /* ????? */

	if( state->noise.counter <= 0 )
	{
		int n = (-state->noise.counter / samplerate) + 1;
		state->noise.counter += n * samplerate;
		state->noise.polyoffs = (state->noise.polyoffs + n) & 0x3ffff;
		state->polybit = (state->poly18[state->noise.polyoffs>>5] >> (state->noise.polyoffs & 31)) & 1;
	}

	/* The polynome output bit is used to gate bits 6 + 7 of
     * sound latch A through the upper half of a 4066 chip.
     * Bit 6 is sweeping a capacitor between 0V and 4.7V
     * while bit 7 is connected directly to the 4066.
     * Both outputs are then filtered, bit 7 even twice,
     * but it's beyond me what the filters there are doing...
     */
	if (state->polybit)
	{
		sum += c_pa6_level;
		/* bit 7 is connected directly */
		if (state->sound_latch_a & 0x80)
			sum += VMAX;
	}
	else
	{
		sum -= c_pa6_level;
		/* bit 7 is connected directly */
		if (state->sound_latch_a & 0x80)
			sum -= VMAX;
	}

	return sum / 2;
}

static STREAM_UPDATE( pleiads_sound_update )
{
	pleiads_sound_state *state = get_safe_token(device);
	int rate = device->machine->sample_rate;
	stream_sample_t *buffer = outputs[0];

	while( samples-- > 0 )
	{
		int sum = tone1(state, rate)/2 + tone23(state, rate)/2 + tone4(state, rate) + noise(state, rate);
		*buffer++ = sum < 32768 ? sum > -32768 ? sum : -32768 : 32767;
	}
}

WRITE8_DEVICE_HANDLER( pleiads_sound_control_a_w )
{
	pleiads_sound_state *state = get_safe_token(device);

	if (data == state->sound_latch_a)
		return;

	logerror("pleiads_sound_control_b_w $%02x\n", data);

	stream_update(state->channel);
	state->sound_latch_a = data;
}

WRITE8_DEVICE_HANDLER( pleiads_sound_control_b_w )
{
	pleiads_sound_state *state = get_safe_token(device);

	/*
     * pitch selects one of 4 possible clock inputs
     * (actually 3, because IC2 and IC3 are tied together)
     * write note value to TMS3615; voice b1 & b2
     */
	int note = data & 15;
	int pitch = (data >> 6) & 3;

	if (data == state->sound_latch_b)
		return;

	logerror("pleiads_sound_control_b_w $%02x\n", data);

	if (pitch == 3)
		pitch = 2;	/* 2 and 3 are the same */

	tms36xx_note_w(state->tms, pitch, note);

	stream_update(state->channel);
	state->sound_latch_b = data;
}

/* two bits (4 + 5) from the videoreg_w latch go here */
WRITE8_DEVICE_HANDLER( pleiads_sound_control_c_w )
{
	pleiads_sound_state *state = get_safe_token(device);

	if (data == state->sound_latch_c)
		return;

	logerror("pleiads_sound_control_c_w $%02x\n", data);
	stream_update(state->channel);
	state->sound_latch_c = data;
}

static DEVICE_START( common_sh_start )
{
	pleiads_sound_state *state = get_safe_token(device);
	int i, j;
	UINT32 shiftreg;

	state->pc4.level = PC4_MIN;
	state->tms = device->machine->device("tms");
	state->poly18 = auto_alloc_array(device->machine, UINT32, 1ul << (18-5));

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
		state->poly18[i] = bits;
	}

	state->channel = stream_create(device, 0, 1, device->machine->sample_rate, NULL, pleiads_sound_update);
}

static DEVICE_START( pleiads_sound )
{
	pleiads_sound_state *state = get_safe_token(device);

	/* The real values are _unknown_!
     * I took the ones from Naughty Boy / Pop Flamer
     */

	/* charge 10u?? (C??) through 330K?? (R??) -> 3.3s */
	state->pa5.charge_time = 3.3;

	/* discharge 10u?? (C??) through 220k?? (R??) -> 2.2s */
	state->pa5.discharge_time = 2.2;

	/* charge 2.2uF?? through 330?? -> 0.000726s */
	state->pa6.charge_time = 0.000726;

	/* discharge 2.2uF?? through 10k?? -> 0.22s */
	state->pa6.discharge_time = 0.022;

    /* 10k and 10uF */
	state->pb4.charge_time = 0.1;
	state->pb4.discharge_time = 0.1;

	/* charge C49 (22u?) via R47 (2k?) and R48 (1k)
     * time constant (1000+2000) * 22e-6 = 0.066s */
	state->pc4.charge_time = 0.066;

	/* discharge C49 (22u?) via R48 (1k) and diode D1
     * time constant 1000 * 22e-6 = 0.022s */
	state->pc4.discharge_time = 0.022;

	/* charge 10u?? through 330 -> 0.0033s */
	state->pc5.charge_time = 0.0033;

	/* discharge 10u?? through ??k (R??) -> 0.1s */
	state->pc5.discharge_time = 0.1;

	/* both in K */
	state->pa5_resistor = 33;
	state->pc5_resistor = 47;

	/* upper 556 upper half: Ra=10k??, Rb=200k??, C=0.01uF?? -> 351Hz */
	state->tone2.max_freq = 351;

	/* upper 556 lower half: Ra=47k??, Rb=100k??, C=0.01uF?? -> 582Hz */
	state->tone3.max_freq = 582;

	/* lower 556 upper half: Ra=33k??, Rb=100k??, C=0.0047uF??
       freq = 1.44 / ((33000+2*100000) * 0.0047e-6) = approx. 1315 Hz */
	state->tone4.max_freq = 1315;

	/* how to divide the V/C voltage for tone #4 */
	state->polybit_resistor = 47;
	state->opamp_resistor = 20;

	/* lower 556 lower half: Ra=100k??, Rb=1k??, C=0.01uF??
      freq = 1.44 / ((100000+2*1000) * 0.01e-6) = approx. 1412 Hz */
	state->noise.freq = 1412;	/* higher noise rate than popflame/naughtyb??? */

	DEVICE_START_CALL(common_sh_start);
}

DEVICE_GET_INFO( pleiads_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(pleiads_sound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(pleiads_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Pleiads Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


static DEVICE_START( naughtyb_sound )
{
	pleiads_sound_state *state = get_safe_token(device);

	/* charge 10u??? through 330K (R??) -> 3.3s */
	state->pa5.charge_time = 3.3;

	/* discharge 10u through 220k (R??) -> 2.1s */
	state->pa5.discharge_time = 2.2;

	/* charge 2.2uF through 330 -> 0.000726s */
	state->pa6.charge_time = 0.000726;

	/* discharge 2.2uF through 10K -> 0.022s */
	state->pa6.discharge_time = 0.022;

	/* 10k and 10uF */
	state->pb4.charge_time = 0.1;
	state->pb4.discharge_time = 0.1;

	/* charge 10uF? (C??) via 3k?? (R??) and 2k?? (R28?)
     * time constant (3000+2000) * 10e-6 = 0.05s */
	state->pc4.charge_time = 0.05 * 10;

	/* discharge 10uF? (C??) via 2k?? R28??  and diode D?
     * time constant 2000 * 10e-6 = 0.02s */
	state->pc4.discharge_time = 0.02 * 10;

	/* charge 10u through 330 -> 0.0033s */
	state->pc5.charge_time = 0.0033;

	/* discharge 10u through ??k (R??) -> 0.1s */
	state->pc5.discharge_time = 0.1;

	/* both in K */
	state->pa5_resistor = 100;
	state->pc5_resistor = 78;

	/* upper 556 upper half: 10k, 200k, 0.01uF -> 351Hz */
	state->tone2.max_freq = 351;

	/* upper 556 lower half: 47k, 200k, 0.01uF -> 322Hz */
	state->tone3.max_freq = 322;

	/* lower 556 upper half: Ra=33k, Rb=100k, C=0.0047uF
       freq = 1.44 / ((33000+2*100000) * 0.0047e-6) = approx. 1315 Hz */
	state->tone4.max_freq = 1315;

	/* how to divide the V/C voltage for tone #4 */
	state->polybit_resistor = 47;
	state->opamp_resistor = 20;

	/* lower 556 lower half: Ra=200k, Rb=1k, C=0.01uF
      freq = 1.44 / ((200000+2*1000) * 0.01e-6) = approx. 713 Hz */
	state->noise.freq = 713;

	DEVICE_START_CALL(common_sh_start);
}

DEVICE_GET_INFO( naughtyb_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(pleiads_sound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(naughtyb_sound);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Naughty Boy Custom");			break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


static DEVICE_START( popflame_sound )
{
	pleiads_sound_state *state = get_safe_token(device);

	/* charge 10u (C63 in Pop Flamer) through 330K -> 3.3s */
	state->pa5.charge_time = 3.3;

	/* discharge 10u (C63 in Pop Flamer) through 220k -> 2.2s */
	state->pa5.discharge_time = 2.2;

	/* charge 2.2uF through 330 -> 0.000726s */
	state->pa6.charge_time = 0.000726;

	/* discharge 2.2uF through 10K -> 0.022s */
	state->pa6.discharge_time = 0.022;

    /* 2k and 10uF */
	state->pb4.charge_time = 0.02;
	state->pb4.discharge_time = 0.02;

	/* charge 2.2uF (C49?) via R47 (100) and R48 (1k)
     * time constant (100+1000) * 2.2e-6 = 0.00242 */
	state->pc4.charge_time = 0.000242;

	/* discharge 2.2uF (C49?) via R48 (1k) and diode D1
     * time constant 1000 * 22e-6 = 0.0022s */
	state->pc4.discharge_time = 0.00022;

	/* charge 22u (C52 in Pop Flamer) through 10k -> 0.22s */
	state->pc5.charge_time = 0.22;

	/* discharge 22u (C52 in Pop Flamer) through ??k (R??) -> 0.1s */
	state->pc5.discharge_time = 0.1;

	/* both in K */
	state->pa5_resistor = 33;
	state->pc5_resistor = 47;

	/* upper 556 upper half: Ra=10k, Rb=100k, C=0.01uF -> 1309Hz */
	state->tone2.max_freq = 1309;

	/* upper 556 lower half: Ra=10k??, Rb=120k??, C=0.01uF -> 1108Hz */
	state->tone3.max_freq = 1108;

	/* lower 556 upper half: Ra=33k, Rb=100k, C=0.0047uF
       freq = 1.44 / ((33000+2*100000) * 0.0047e-6) = approx. 1315 Hz */
	state->tone4.max_freq = 1315;

	/* how to divide the V/C voltage for tone #4 */
	state->polybit_resistor = 20;
	state->opamp_resistor = 20;

	/* lower 556 lower half: Ra=200k, Rb=1k, C=0.01uF
      freq = 1.44 / ((200000+2*1000) * 0.01e-6) = approx. 713 Hz */
	state->noise.freq = 713;

	DEVICE_START_CALL(common_sh_start);
}

DEVICE_GET_INFO( popflame_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(pleiads_sound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(popflame_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Pop Flamer Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(PLEIADS, pleiads_sound);
DEFINE_LEGACY_SOUND_DEVICE(NAUGHTYB, naughtyb_sound);
DEFINE_LEGACY_SOUND_DEVICE(POPFLAME, popflame_sound);
