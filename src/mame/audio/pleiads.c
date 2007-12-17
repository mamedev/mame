/****************************************************************************
 *
 * Sound hardware for Pleiades, Naughty Boy and Pop Flamer.
 *
 * If you find errors or have suggestions, please mail me.
 * Juergen Buchmueller <pullmoll@t-online.de>
 *
 ****************************************************************************/
#include "driver.h"
#include "streams.h"
#include "sound/custom.h"
#include "sound/tms36xx.h"
#include "includes/phoenix.h"

#define VMIN	0
#define VMAX	32767

static sound_stream * channel;

static int sound_latch_a;
static int sound_latch_b;
static int sound_latch_c;	/* part of the videoreg_w latch */

static UINT32 *poly18 = NULL;
static int polybit;

/* fixed 8kHz clock */
#define TONE1_CLOCK  8000

/* some resistor and capacitor dependent values which
   vary between the (otherwise identical) boards. */
static double pa5_charge_time;
static double pa5_discharge_time;

static double pa6_charge_time;
static double pa6_discharge_time;

static double pb4_charge_time;
static double pb4_discharge_time;

static double pc4_charge_time;
static double pc4_discharge_time;

static double pc5_charge_time;
static double pc5_discharge_time;

static int pa5_resistor;
static int pc5_resistor;

static int tone2_max_freq;
static int tone3_max_freq;
static int tone4_max_freq;
static int noise_freq;
static int polybit_resistor;
static int opamp_resistor;

/*****************************************************************************
 * Tone #1 is a fixed 8 kHz signal divided by 1 to 15.
 *****************************************************************************/
INLINE int tone1(int samplerate)
{
	static int counter, divisor, output;

	if( (sound_latch_a & 15) != 15 )
	{
		counter -= TONE1_CLOCK;
		while( counter <= 0 )
		{
			counter += samplerate;
			if( ++divisor == 16 )
			{
				divisor = sound_latch_a & 15;
				output ^= 1;
			}
		}
	}
	return output ? VMAX : -VMAX;
}

/*****************************************************************************
 * Tones #2 and #3 are coming from the upper 556 chip
 * It's labelled IC96 in Pop Flamer, 4D(??) in Naughty Boy.
 * C68 controls the frequencies of tones #2 and #3 (V/C inputs)
 *****************************************************************************/
INLINE int update_pb4(int samplerate)
{
	static int counter, level;

	/* bit 4 of latch B: charge 10uF (C28/C68) through 10k (R19/R25) */
	if( sound_latch_b & 0x10 )
	{
		if( level < VMAX )
		{
			counter -= (int)((VMAX - level) / pb4_charge_time);
			if( counter <= 0 )
			{
				int n = (-counter / samplerate) + 1;
				counter += n * samplerate;
				if( (level += n) > VMAX )
					level = VMAX;
			}
		}
	}
	else
	{
		if( level > VMIN )
		{
			counter -= (int)((level - VMIN) / pb4_discharge_time);
			if( counter <= 0 )
			{
				int n = (-counter / samplerate) + 1;
				counter += n * samplerate;
				if( (level -= n) < VMIN)
					level = VMIN;
			}
		}
	}
	return level;
}

INLINE int tone23(int samplerate)
{
	static int counter2, output2, counter3, output3;
	int level = VMAX - update_pb4(samplerate);
	int sum = 0;

	/* bit 5 = low: tone23 disabled */
	if( (sound_latch_b & 0x20) == 0 )
		return sum;

    /* modulate timers from the upper 556 with the voltage on Cxx on PB4. */
	if( level < VMAX )
	{
		counter2 -= tone2_max_freq * level / 32768;
		if( counter2 <= 0 )
		{
			int n = (-counter2 / samplerate) + 1;
			counter2 += n * samplerate;
			output2 = (output2 + n) & 1;
		}

		counter3 -= tone3_max_freq*1/3 + tone3_max_freq*2/3 * level / 33768;
		if( counter3 <= 0 )
		{
			int n = (-counter2 / samplerate) + 1;
			counter3 += samplerate;
			output3 = (output3 + n) & 1;
		}
	}

	sum += (output2) ? VMAX : -VMAX;
	sum += (output3) ? VMAX : -VMAX;

	return sum / 2;
}

/*****************************************************************************
 * Tone #4 comes from upper half of the lower 556 (IC98 in Pop Flamer)
 * It's modulated by the voltage at C49, which is then divided between
 * 0V or 5V, depending on the polynome output bit.
 * The tone signal gates two signals (bits 5 of latches A and C), but
 * these are also swept between two levels (C52 and C53 in Pop Flamer).
 *****************************************************************************/
INLINE int update_c_pc4(int samplerate)
{
	#define PC4_MIN (int)(VMAX * 7 / 50)

	static int counter, level = PC4_MIN;

	/* bit 4 of latch C: (part of videoreg_w) hi? */
	if (sound_latch_c & 0x10)
	{
		if (level < VMAX)
		{
			counter -= (int)((VMAX - level) / pc4_charge_time);
			if( counter <= 0 )
			{
				int n = (-counter / samplerate) + 1;
				counter += n * samplerate;
				if( (level += n) > VMAX )
					level = VMAX;
			}
		}
	}
	else
	{
		if (level > PC4_MIN)
		{
			counter -= (int)((level - PC4_MIN) / pc4_discharge_time);
			if( counter <= 0 )
			{
				int n = (-counter / samplerate) + 1;
				counter += n * samplerate;
				if( (level -= n) < PC4_MIN )
					level = PC4_MIN;
			}
		}
	}
	return level;
}

INLINE int update_c_pc5(int samplerate)
{
	static int counter, level;

	/* bit 5 of latch C: charge or discharge C52 */
	if (sound_latch_c & 0x20)
	{
		if (level < VMAX)
		{
			counter -= (int)((VMAX - level) / pc5_charge_time);
			if( counter <= 0 )
			{
				int n = (-counter / samplerate) + 1;
				counter += n * samplerate;
				if( (level += n) > VMAX )
					level = VMAX;
			}
		}
	}
	else
	{
		if (level > VMIN)
		{
			counter -= (int)((level - VMIN) / pc5_discharge_time);
			if( counter <= 0 )
			{
				int n = (-counter / samplerate) + 1;
				counter += samplerate;
				if( (level -= n) < VMIN )
					level = VMIN;
			}
		}
	}
	return level;
}

INLINE int update_c_pa5(int samplerate)
{
	static int counter, level;

	/* bit 5 of latch A: charge or discharge C63 */
	if (sound_latch_a & 0x20)
	{
		if (level < VMAX)
		{
			counter -= (int)((VMAX - level) / pa5_charge_time);
			if( counter <= 0 )
			{
				int n = (-counter / samplerate) + 1;
				counter += n * samplerate;
				if( (level += n) > VMAX )
					level = VMAX;
			}
		}
	}
	else
	{
		if (level > VMIN)
		{
			counter -= (int)((level - VMIN) / pa5_discharge_time);
			if( counter <= 0 )
			{
				int n = (-counter / samplerate) + 1;
				counter += samplerate;
				if( (level -= n) < VMIN )
					level = VMIN;
			}
		}
	}
	return level;
}

INLINE int tone4(int samplerate)
{
	static int counter, output;
	int level = update_c_pc4(samplerate);
	int vpc5 = update_c_pc5(samplerate);
	int vpa5 = update_c_pa5(samplerate);
	int sum;

	/* Two resistors divide the output voltage of the op-amp between
     * polybit = 0: 0V and level: x * opamp_resistor / (opamp_resistor + polybit_resistor)
     * polybit = 1: level and 5V: x * polybit_resistor / (opamp_resistor + polybit_resistor)
     */
	if (polybit)
		level = level + (VMAX - level) * opamp_resistor / (opamp_resistor + polybit_resistor);
	else
		level = level * polybit_resistor / (opamp_resistor + polybit_resistor);

	counter -= tone4_max_freq * level / 32768;
	if( counter <= 0 )
	{
		int n = (-counter / samplerate) + 1;
		counter += n * samplerate;
		output = (output + n) & 1;
	}

	/* mix the two signals */
	sum = vpc5 * pa5_resistor / (pa5_resistor + pc5_resistor) +
		  vpa5 * pc5_resistor / (pa5_resistor + pc5_resistor);

	return (output) ? sum : -sum;
}

/*****************************************************************************
 * Noise comes from a shift register (4006) hooked up just like in Phoenix.
 * Difference: the clock frequecy is toggled between two values only by
 * bit 4 of latch A. The output of the first shift register can be zapped(?)
 * by some control line (IC87 in Pop Flamer: not yet implemented)
 *****************************************************************************/
INLINE int update_c_pa6(int samplerate)
{
	static int counter, level;

	/* bit 6 of latch A: charge or discharge C63 */
	if (sound_latch_a & 0x40)
	{
		if (level < VMAX)
		{
			counter -= (int)((VMAX - level) / pa6_charge_time);
			if( counter <= 0 )
			{
				int n = (-counter / samplerate) + 1;
				counter += n * samplerate;
				if( (level += n) > VMAX )
					level = VMAX;
			}
		}
	}
	else
	{
		/* only discharge of poly bit is active */
		if (polybit && level > VMIN)
		{
			/* discharge 10uF through 10k -> 0.1s */
			counter -= (int)((level - VMIN) / 0.1);
			if( counter <= 0 )
			{
				int n = (-counter / samplerate) + 1;
				counter += n * samplerate;
				if( (level -= n) < VMIN )
					level = VMIN;
			}
		}
	}
	return level;
}


INLINE int noise(int samplerate)
{
	static int counter, polyoffs;
	int c_pa6_level = update_c_pa6(samplerate);
	int sum = 0;

	/*
     * bit 4 of latch A: noise counter rate modulation?
     * CV2 input of lower 556 is connected via 2k resistor
     */
	if ( sound_latch_a & 0x10 )
		counter -= noise_freq * 2 / 3; /* ????? */
	else
		counter -= noise_freq * 1 / 3; /* ????? */

	if( counter <= 0 )
	{
		int n = (-counter / samplerate) + 1;
		counter += n * samplerate;
		polyoffs = (polyoffs + n) & 0x3ffff;
		polybit = (poly18[polyoffs>>5] >> (polyoffs & 31)) & 1;
	}

	/* The polynome output bit is used to gate bits 6 + 7 of
     * sound latch A through the upper half of a 4066 chip.
     * Bit 6 is sweeping a capacitor between 0V and 4.7V
     * while bit 7 is connected directly to the 4066.
     * Both outputs are then filtered, bit 7 even twice,
     * but it's beyond me what the filters there are doing...
     */
	if (polybit)
	{
		sum += c_pa6_level;
		/* bit 7 is connected directly */
		if (sound_latch_a & 0x80)
			sum += VMAX;
	}
	else
	{
		sum -= c_pa6_level;
		/* bit 7 is connected directly */
		if (sound_latch_a & 0x80)
			sum -= VMAX;
	}

	return sum / 2;
}

static void pleiads_sound_update(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int length)
{
	int rate = Machine->sample_rate;
	stream_sample_t *buffer = outputs[0];

	while( length-- > 0 )
	{
		int sum = tone1(rate)/2 + tone23(rate)/2 + tone4(rate) + noise(rate);
		*buffer++ = sum < 32768 ? sum > -32768 ? sum : -32768 : 32767;
	}
}

WRITE8_HANDLER( pleiads_sound_control_a_w )
{
	if (data == sound_latch_a)
		return;

	logerror("pleiads_sound_control_b_w $%02x\n", data);

	stream_update(channel);
	sound_latch_a = data;
}

WRITE8_HANDLER( pleiads_sound_control_b_w )
{
	/*
     * pitch selects one of 4 possible clock inputs
     * (actually 3, because IC2 and IC3 are tied together)
     * write note value to TMS3615; voice b1 & b2
     */
	int note = data & 15;
	int pitch = (data >> 6) & 3;

	if (data == sound_latch_b)
		return;

	logerror("pleiads_sound_control_b_w $%02x\n", data);

	if (pitch == 3)
		pitch = 2;	/* 2 and 3 are the same */

	tms36xx_note_w(0, pitch, note);

	stream_update(channel);
	sound_latch_b = data;
}

/* two bits (4 + 5) from the videoreg_w latch go here */
WRITE8_HANDLER( pleiads_sound_control_c_w )
{
	if (data == sound_latch_c)
		return;

	logerror("pleiads_sound_control_c_w $%02x\n", data);
	stream_update(channel);
	sound_latch_c = data;
}

static void *common_sh_start(const struct CustomSound_interface *config, const char *name)
{
	int i, j;
	UINT32 shiftreg;

	poly18 = (UINT32 *)auto_malloc((1ul << (18-5)) * sizeof(UINT32));

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
		poly18[i] = bits;
	}

	channel = stream_create(0, 1, Machine->sample_rate, NULL, pleiads_sound_update);

	/* just a dummy alloc to make the caller happy */
	return auto_malloc(1);
}

void *pleiads_sh_start(int clock, const struct CustomSound_interface *config)
{
	/* The real values are _unknown_!
     * I took the ones from Naughty Boy / Pop Flamer
     */

	/* charge 10u?? (C??) through 330K?? (R??) -> 3.3s */
	pa5_charge_time = 3.3;

	/* discharge 10u?? (C??) through 220k?? (R??) -> 2.2s */
	pa5_discharge_time = 2.2;

	/* charge 2.2uF?? through 330?? -> 0.000726s */
	pa6_charge_time = 0.000726;

	/* discharge 2.2uF?? through 10k?? -> 0.22s */
	pa6_discharge_time = 0.022;

    /* 10k and 10uF */
	pb4_charge_time = 0.1;
	pb4_discharge_time = 0.1;

	/* charge C49 (22u?) via R47 (2k?) and R48 (1k)
     * time constant (1000+2000) * 22e-6 = 0.066s */
	pc4_charge_time = 0.066;

	/* discharge C49 (22u?) via R48 (1k) and diode D1
     * time constant 1000 * 22e-6 = 0.022s */
	pc4_discharge_time = 0.022;

	/* charge 10u?? through 330 -> 0.0033s */
	pc5_charge_time = 0.0033;

	/* discharge 10u?? through ??k (R??) -> 0.1s */
	pc5_discharge_time = 0.1;

	/* both in K */
	pa5_resistor = 33;
	pc5_resistor = 47;

	/* upper 556 upper half: Ra=10k??, Rb=200k??, C=0.01uF?? -> 351Hz */
	tone2_max_freq = 351;

	/* upper 556 lower half: Ra=47k??, Rb=100k??, C=0.01uF?? -> 582Hz */
	tone3_max_freq = 582;

	/* lower 556 upper half: Ra=33k??, Rb=100k??, C=0.0047uF??
       freq = 1.44 / ((33000+2*100000) * 0.0047e-6) = approx. 1315 Hz */
	tone4_max_freq = 1315;

	/* how to divide the V/C voltage for tone #4 */
	polybit_resistor = 47;
	opamp_resistor = 20;

	/* lower 556 lower half: Ra=100k??, Rb=1k??, C=0.01uF??
      freq = 1.44 / ((100000+2*1000) * 0.01e-6) = approx. 1412 Hz */
	noise_freq = 1412;	/* higher noise rate than popflame/naughtyb??? */

	return common_sh_start(config, "Custom (Pleiads)");
}

void *naughtyb_sh_start(int clock, const struct CustomSound_interface *config)
{
	/* charge 10u??? through 330K (R??) -> 3.3s */
	pa5_charge_time = 3.3;

	/* discharge 10u through 220k (R??) -> 2.1s */
	pa5_discharge_time = 2.2;

	/* charge 2.2uF through 330 -> 0.000726s */
	pa6_charge_time = 0.000726;

	/* discharge 2.2uF through 10K -> 0.022s */
	pa6_discharge_time = 0.022;

    /* 10k and 10uF */
	pb4_charge_time = 0.1;
	pb4_discharge_time = 0.1;

	/* charge 10uF? (C??) via 3k?? (R??) and 2k?? (R28?)
     * time constant (3000+2000) * 10e-6 = 0.05s */
	pc4_charge_time = 0.05 * 10;

	/* discharge 10uF? (C??) via 2k?? R28??  and diode D?
     * time constant 2000 * 10e-6 = 0.02s */
	pc4_discharge_time = 0.02 * 10;

	/* charge 10u through 330 -> 0.0033s */
	pc5_charge_time = 0.0033;

	/* discharge 10u through ??k (R??) -> 0.1s */
	pc5_discharge_time = 0.1;

	/* both in K */
	pa5_resistor = 100;
	pc5_resistor = 78;

	/* upper 556 upper half: 10k, 200k, 0.01uF -> 351Hz */
	tone2_max_freq = 351;

	/* upper 556 lower half: 47k, 200k, 0.01uF -> 322Hz */
	tone3_max_freq = 322;

	/* lower 556 upper half: Ra=33k, Rb=100k, C=0.0047uF
       freq = 1.44 / ((33000+2*100000) * 0.0047e-6) = approx. 1315 Hz */
	tone4_max_freq = 1315;

	/* how to divide the V/C voltage for tone #4 */
	polybit_resistor = 47;
	opamp_resistor = 20;

	/* lower 556 lower half: Ra=200k, Rb=1k, C=0.01uF
      freq = 1.44 / ((200000+2*1000) * 0.01e-6) = approx. 713 Hz */
	noise_freq = 713;

	return common_sh_start(config, "Custom (Naughty Boy)");
}

void *popflame_sh_start(int clock, const struct CustomSound_interface *config)
{
	/* charge 10u (C63 in Pop Flamer) through 330K -> 3.3s */
	pa5_charge_time = 3.3;

	/* discharge 10u (C63 in Pop Flamer) through 220k -> 2.2s */
	pa5_discharge_time = 2.2;

	/* charge 2.2uF through 330 -> 0.000726s */
	pa6_charge_time = 0.000726;

	/* discharge 2.2uF through 10K -> 0.022s */
	pa6_discharge_time = 0.022;

    /* 2k and 10uF */
	pb4_charge_time = 0.02;
	pb4_discharge_time = 0.02;

	/* charge 2.2uF (C49?) via R47 (100) and R48 (1k)
     * time constant (100+1000) * 2.2e-6 = 0.00242 */
	pc4_charge_time = 0.000242;

	/* discharge 2.2uF (C49?) via R48 (1k) and diode D1
     * time constant 1000 * 22e-6 = 0.0022s */
	pc4_discharge_time = 0.00022;

	/* charge 22u (C52 in Pop Flamer) through 10k -> 0.22s */
	pc5_charge_time = 0.22;

	/* discharge 22u (C52 in Pop Flamer) through ??k (R??) -> 0.1s */
	pc5_discharge_time = 0.1;

	/* both in K */
	pa5_resistor = 33;
	pc5_resistor = 47;

	/* upper 556 upper half: Ra=10k, Rb=100k, C=0.01uF -> 1309Hz */
	tone2_max_freq = 1309;

	/* upper 556 lower half: Ra=10k??, Rb=120k??, C=0.01uF -> 1108Hz */
	tone3_max_freq = 1108;

	/* lower 556 upper half: Ra=33k, Rb=100k, C=0.0047uF
       freq = 1.44 / ((33000+2*100000) * 0.0047e-6) = approx. 1315 Hz */
	tone4_max_freq = 1315;

	/* how to divide the V/C voltage for tone #4 */
	polybit_resistor = 20;
	opamp_resistor = 20;

	/* lower 556 lower half: Ra=200k, Rb=1k, C=0.01uF
      freq = 1.44 / ((200000+2*1000) * 0.01e-6) = approx. 713 Hz */
	noise_freq = 713;

	return common_sh_start(config, "Custom (Pop Flamer)");
}
