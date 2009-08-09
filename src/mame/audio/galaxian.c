
#include "driver.h"
#include "streams.h"
#include "includes/galaxian.h"

#if !GALAXIAN_USE_DISCRETE
#include "sound/samples.h"

#define VERBOSE 0

#define NEW_LFO 1

#define XTAL		18432000

#define SOUND_CLOCK (XTAL/6/2)			/* 1.536 MHz */

#define RNG_RATE	(XTAL/3)			/* RNG clock is XTAL/3 */
#define NOISE_RATE	(XTAL/3/192/2/2)	/* 2V = 8kHz */
#define NOISE_LENGTH (NOISE_RATE*4) 	/* four seconds of noise */

#define SHOOT_RATE 2672
#define SHOOT_LENGTH 13000

#define TOOTHSAW_LENGTH 16
#define TOOTHSAW_VOLUME 0.36f
#define STEPS 16
#define LFO_VOLUME 0.06f
#define SHOOT_VOLUME 0.50f
#define NOISE_VOLUME 0.50f
#define NOISE_AMPLITUDE (70*256)
#define TOOTHSAW_AMPLITUDE (64*256)

/* see comments in galaxian_sh_update() */
#define MINFREQ (139-139/3)
#define MAXFREQ (139+139/3)

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

static emu_timer *lfotimer = NULL;
static INT32 freq;

#define STEP 1

static INT32 noise_enable;
static INT32 noisevolume;
static INT16 *noisewave;
static INT16 *shootwave;
static INT32 shoot_length;
static INT32 shoot_rate;

static UINT8 last_port2=0;

static INT16 tonewave[4][TOOTHSAW_LENGTH];
static INT32 pitch,vol;

static INT32 counter, countdown;
static INT32 lfobit[4];

static const INT16 backgroundwave[32] =
{
   0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000,
   0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000, 0x4000,
  -0x4000,-0x4000,-0x4000,-0x4000,-0x4000,-0x4000,-0x4000,-0x4000,
  -0x4000,-0x4000,-0x4000,-0x4000,-0x4000,-0x4000,-0x4000,-0x4000,
};

#define CHANNEL_NOISE	0
#define CHANNEL_SHOOT	1
#define CHANNEL_LFO		2
static sound_stream * tone_stream;
static emu_timer *noisetimer;

static TIMER_CALLBACK( lfo_timer_cb );
static TIMER_CALLBACK( galaxian_sh_update );

static STREAM_UPDATE( tone_update )
{
	stream_sample_t *buffer = outputs[0];
	int i,j;
	INT16 *w = tonewave[vol];

	/* only update if we have non-zero volume and frequency */
	if( pitch != 0xff )
	{
		for (i = 0; i < samples; i++)
		{
			int mix = 0;

			for (j = 0;j < STEPS;j++)
			{
				if (countdown >= 256)
				{
					counter = (counter + 1) % TOOTHSAW_LENGTH;
					countdown = pitch;
				}
				countdown++;

				mix += w[counter];
			}
			*buffer++ = mix / STEPS;
		}
	}
	else
	{
		for( i = 0; i < samples; i++ )
			*buffer++ = 0;
	}
}

WRITE8_DEVICE_HANDLER( galaxian_sound_w )
{
	data &= 0x01;
	switch (offset & 7)
	{
		case 0:		/* FS1 (controls 555 timer at 8R) */
		case 1:		/* FS2 (controls 555 timer at 8S) */
		case 2:		/* FS3 (controls 555 timer at 8T) */
			galaxian_background_enable_w(device, offset, data);
			break;

		case 3:		/* HIT */
			galaxian_noise_enable_w(device, 0, data);
			break;

		case 4:		/* n/c */
			break;

		case 5:		/* FIRE */
			galaxian_shoot_enable_w(device, 0, data);
			break;

		case 6:		/* VOL1 */
		case 7:		/* VOL2 */
			galaxian_vol_w(device, offset & 1, data);
			break;
	}
}


WRITE8_DEVICE_HANDLER( galaxian_pitch_w )
{
	stream_update(tone_stream);

	pitch = data;
}

WRITE8_DEVICE_HANDLER( galaxian_vol_w )
{
	stream_update(tone_stream);

	/* offset 0 = bit 0, offset 1 = bit 1 */
	vol = (vol & ~(1 << offset)) | ((data & 1) << offset);
}


static TIMER_CALLBACK( noise_timer_cb )
{
	if( !noise_enable && noisevolume > 0 )
	{
		const device_config *samples = devtag_get_device(machine, GAL_AUDIO);
		noisevolume -= (noisevolume / 10) + 1;
		sample_set_volume(samples, CHANNEL_NOISE,noisevolume / 100.0);
	}
}

WRITE8_DEVICE_HANDLER( galaxian_noise_enable_w )
{
	noise_enable = data & 1;

	if( noise_enable )
	{
		noisevolume = 100;
		sample_set_volume(device, CHANNEL_NOISE,noisevolume / 100.0);
	}
}

WRITE8_DEVICE_HANDLER( galaxian_shoot_enable_w )
{
	if( data & 1 && !(last_port2 & 1) )
	{
		sample_start_raw(device, CHANNEL_SHOOT, shootwave, shoot_length, shoot_rate, 0);
		sample_set_volume(device, CHANNEL_SHOOT,SHOOT_VOLUME);
	}
	last_port2=data;
}


static SAMPLES_START( galaxian_sh_start )
{
	running_machine *machine = device->machine;
	int i, j, sweep, charge, countdown, generator, bit1, bit2;

	freq = MAXFREQ;

	sample_set_volume(device, CHANNEL_NOISE, NOISE_VOLUME);
	sample_set_volume(device, CHANNEL_SHOOT, SHOOT_VOLUME);
	sample_set_volume(device, CHANNEL_LFO+0, LFO_VOLUME);
	sample_set_volume(device, CHANNEL_LFO+1, LFO_VOLUME);
	sample_set_volume(device, CHANNEL_LFO+2, LFO_VOLUME);

	noisewave = auto_alloc_array(machine, INT16, NOISE_LENGTH);

#define SHOOT_SEC 2
	shoot_rate = machine->sample_rate;
	shoot_length = SHOOT_SEC * shoot_rate;
	shootwave = auto_alloc_array(machine, INT16, shoot_length);

	/*
     * The RNG shifter is clocked with RNG_RATE, bit 17 is
     * latched every 2V cycles (every 2nd scanline).
     * This signal is used as a noise source.
     */
	generator = 0;
	countdown = NOISE_RATE / 2;
	for( i = 0; i < NOISE_LENGTH; i++ )
	{
		countdown -= RNG_RATE;
		while( countdown < 0 )
		{
			generator <<= 1;
			bit1 = (~generator >> 17) & 1;
			bit2 = (generator >> 5) & 1;
			if (bit1 ^ bit2) generator |= 1;
			countdown += NOISE_RATE;
		}
		noisewave[i] = ((generator >> 17) & 1) ? NOISE_AMPLITUDE : -NOISE_AMPLITUDE;
	}

	/* dummy */
	sweep = 100;
	charge = +2;
	j=0;
	{
#define R41__ 100000
#define R44__ 10000
#define R45__ 22000
#define R46__ 10000
#define R47__ 2200
#define R48__ 2200
#define C25__ 0.000001
#define C27__ 0.00000001
#define C28__ 0.000047
#define C29__ 0.00000001
#define IC8L3_L 0.2   /* 7400 L level */
#define IC8L3_H 4.5   /* 7400 H level */
#define NOISE_L 0.2   /* 7474 L level */
#define NOISE_H 4.5   /* 7474 H level */
/*
    key on/off time is programmable
    Therefore,  it is necessary to make separate sample with key on/off.
    And,  calculate the playback point according to the voltage of c28.
*/
#define SHOOT_KEYON_TIME 0.1  /* second */
/*
    NE555-FM input calculation is wrong.
    The frequency is not proportional to the voltage of FM input.
    And,  duty will be changed,too.
*/
#define NE555_FM_ADJUST_RATE 0.80
		/* discharge : 100K * 1uF */
		double v  = 5.0;
		double vK = (shoot_rate) ? exp(-1 / (R41__*C25__) / shoot_rate) : 0;
		/* -- SHOOT KEY port -- */
		double IC8L3 = IC8L3_L; /* key on */
		int IC8Lcnt = SHOOT_KEYON_TIME * shoot_rate; /* count for key off */
		/* C28 : KEY port capacity */
		/*       connection : 8L-3 - R47(2.2K) - C28(47u) - R48(2.2K) - C29 */
		double c28v = IC8L3_H - (IC8L3_H-(NOISE_H+NOISE_L)/2)/(R46__+R47__+R48__)*R47__;
		double c28K = (shoot_rate) ? exp(-1 / (22000 * 0.000047 ) / shoot_rate) : 0;
		/* C29 : NOISE capacity */
		/*       connection : NOISE - R46(10K) - C29(0.1u) - R48(2.2K) - C28 */
		double c29v  = IC8L3_H - (IC8L3_H-(NOISE_H+NOISE_L)/2)/(R46__+R47__+R48__)*(R47__+R48__);
		double c29K1 = (shoot_rate) ? exp(-1 / (22000  * 0.00000001 ) / shoot_rate) : 0; /* form C28   */
		double c29K2 = (shoot_rate) ? exp(-1 / (100000 * 0.00000001 ) / shoot_rate) : 0; /* from noise */
		/* NE555 timer */
		/* RA = 10K , RB = 22K , C=.01u ,FM = C29 */
		double ne555cnt = 0;
		double ne555step = (shoot_rate) ? ((1.44/((R44__+R45__*2)*C27__)) / shoot_rate) : 0;
		double ne555duty = (double)(R44__+R45__)/(R44__+R45__*2); /* t1 duty */
		double ne555sr;		/* threshold (FM) rate */
		/* NOISE source */
		double ncnt  = 0.0;
		double nstep = (shoot_rate) ? ((double)NOISE_RATE / shoot_rate) : 0;
		double noise_sh2; /* voltage level */

		for( i = 0; i < shoot_length; i++ )
		{
			/* noise port */
			noise_sh2 = noisewave[(int)ncnt % NOISE_LENGTH] == NOISE_AMPLITUDE ? NOISE_H : NOISE_L;
			ncnt+=nstep;
			/* calculate NE555 threshold level by FM input */
			ne555sr = c29v*NE555_FM_ADJUST_RATE / (5.0*2/3);
			/* calc output */
			ne555cnt += ne555step;
			if( ne555cnt >= ne555sr) ne555cnt -= ne555sr;
			if( ne555cnt < ne555sr*ne555duty )
			{
				 /* t1 time */
				shootwave[i] = v/5*0x7fff;
				/* discharge output level */
				if(IC8L3==IC8L3_H)
					v *= vK;
			}
			else
				shootwave[i] = 0;
			/* C28 charge/discharge */
			c28v += (IC8L3-c28v) - (IC8L3-c28v)*c28K;	/* from R47 */
			c28v += (c29v-c28v) - (c29v-c28v)*c28K;		/* from R48 */
			/* C29 charge/discharge */
			c29v += (c28v-c29v) - (c28v-c29v)*c29K1;	/* from R48 */
			c29v += (noise_sh2-c29v) - (noise_sh2-c29v)*c29K2;	/* from R46 */
			/* key off */
			if(IC8L3==IC8L3_L && --IC8Lcnt==0)
				IC8L3=IC8L3_H;
		}
	}

	memset(tonewave, 0, sizeof(tonewave));

	for( i = 0; i < TOOTHSAW_LENGTH; i++ )
	{
		#define V(r0,r1) 2*TOOTHSAW_AMPLITUDE*(r0)/(r0+r1)-TOOTHSAW_AMPLITUDE
		double r0a = 1.0/1e12, r1a = 1.0/1e12;
		double r0b = 1.0/1e12, r1b = 1.0/1e12;

		/* #0: VOL1=0 and VOL2=0
         * only the 33k and the 22k resistors R51 and R50
         */
		if( i & 1 )
		{
			r1a += 1.0/33000;
			r1b += 1.0/33000;
		}
		else
		{
			r0a += 1.0/33000;
			r0b += 1.0/33000;
		}
		if( i & 4 )
		{
			r1a += 1.0/22000;
			r1b += 1.0/22000;
		}
		else
		{
			r0a += 1.0/22000;
			r0b += 1.0/22000;
		}
		tonewave[0][i] = V(1.0/r0a, 1.0/r1a);

		/* #1: VOL1=1 and VOL2=0
         * add the 10k resistor R49 for bit QC
         */
		if( i & 4 )
			r1a += 1.0/10000;
		else
			r0a += 1.0/10000;
		tonewave[1][i] = V(1.0/r0a, 1.0/r1a);

		/* #2: VOL1=0 and VOL2=1
         * add the 15k resistor R52 for bit QD
         */
		if( i & 8 )
			r1b += 1.0/15000;
		else
			r0b += 1.0/15000;
		tonewave[2][i] = V(1.0/r0b, 1.0/r1b);

		/* #3: VOL1=1 and VOL2=1
         * add the 10k resistor R49 for QC
         */
		if( i & 4 )
			r0b += 1.0/10000;
		else
			r1b += 1.0/10000;
		tonewave[3][i] = V(1.0/r0b, 1.0/r1b);
		LOG(("tone[%2d]: $%02x $%02x $%02x $%02x\n", i, tonewave[0][i], tonewave[1][i], tonewave[2][i], tonewave[3][i]));
	}

	pitch = 0xff;
	vol = 0;

	tone_stream = stream_create(device,0,1,SOUND_CLOCK/STEPS,NULL,tone_update);
	stream_set_output_gain(tone_stream, 0, TOOTHSAW_VOLUME);

	sample_set_volume(device, CHANNEL_NOISE,0);
	sample_start_raw(device, CHANNEL_NOISE,noisewave,NOISE_LENGTH,NOISE_RATE,1);

	sample_set_volume(device, CHANNEL_SHOOT,0);
	sample_start_raw(device, CHANNEL_SHOOT,shootwave,SHOOT_LENGTH,SHOOT_RATE,1);

	sample_set_volume(device, CHANNEL_LFO+0,0);
	sample_start_raw(device, CHANNEL_LFO+0,backgroundwave,ARRAY_LENGTH(backgroundwave),1000,1);
	sample_set_volume(device, CHANNEL_LFO+1,0);
	sample_start_raw(device, CHANNEL_LFO+1,backgroundwave,ARRAY_LENGTH(backgroundwave),1000,1);
	sample_set_volume(device, CHANNEL_LFO+2,0);
	sample_start_raw(device, CHANNEL_LFO+2,backgroundwave,ARRAY_LENGTH(backgroundwave),1000,1);

	noisetimer = timer_alloc(machine, noise_timer_cb, NULL);
	timer_adjust_periodic(noisetimer, ATTOTIME_IN_NSEC((155000+22000)/100*693*22), 0, ATTOTIME_IN_NSEC((155000+22000)/100*693*22));

	lfotimer = timer_alloc(machine, lfo_timer_cb, NULL);

	timer_pulse(machine, video_screen_get_frame_period(machine->primary_screen), NULL, 0, galaxian_sh_update);

	state_save_register_global(machine, freq);
	state_save_register_global(machine, noise_enable);
	state_save_register_global(machine, noisevolume);
	state_save_register_global(machine, last_port2);
	state_save_register_global(machine, pitch);
	state_save_register_global(machine, vol);
	state_save_register_global(machine, counter);
	state_save_register_global(machine, countdown);
	state_save_register_global_array(machine, lfobit);
}



WRITE8_DEVICE_HANDLER( galaxian_background_enable_w )
{
	sample_set_volume(device, CHANNEL_LFO+offset,(data & 1) ? LFO_VOLUME : 0);
}

static TIMER_CALLBACK( lfo_timer_cb )
{
	if( freq > MINFREQ )
		freq--;
	else
		freq = MAXFREQ;
}

WRITE8_DEVICE_HANDLER( galaxian_lfo_freq_w )
{
#if NEW_LFO
	/* R18 1M,R17 470K,R16 220K,R15 100K */
	static const int rv[4] = { 1000000,470000,220000,100000};
	double r1,r2,Re,td;
	int i;

	if( (data & 1) == lfobit[offset] )
		return;

	/*
     * NE555 9R is setup as astable multivibrator
     * - this circuit looks LINEAR RAMP V-F converter
       I  = 1/Re * ( R1/(R1+R2)-Vbe)
       td = (2/3VCC*Re*(R1+R2)*C) / (R1*VCC-Vbe*(R1+R2))
      parts assign
       R1  : (R15* L1)|(R16* L2)|(R17* L3)|(R18* L1)
       R2  : (R15*~L1)|(R16*~L2)|(R17*~L3)|(R18*~L4)|R??(330K)
       Re  : R21(100K)
       Vbe : Q2(2SA1015)-Vbe
     * - R20(15K) and Q1 is unknown,maybe current booster.
    */

	lfobit[offset] = data & 1;

	/* R20 15K */
	r1 = 1e12;
	/* R19? 330k to gnd */
	r2 = 330000;
	//r1 = 15000;
	/* R21 100K */
	Re = 100000;
	/* register calculation */
	for(i=0;i<4;i++)
	{
		if(lfobit[i])
			r1 = (r1*rv[i])/(r1+rv[i]); /* Hi  */
		else
			r2 = (r2*rv[i])/(r2+rv[i]); /* Low */
	}

#define Vcc 5.0
#define Vbe 0.65		/* 2SA1015 */
#define Cap 0.000001	/* C15 1uF */
	td = (Vcc*2/3*Re*(r1+r2)*Cap) / (r1*Vcc - Vbe*(r1+r2) );
#undef Cap
#undef Vbe
#undef Vcc
	LOG(("lfo timer bits:%d%d%d%d r1:%d, r2:%d, re: %d, td: %9.2fsec\n", lfobit[0], lfobit[1], lfobit[2], lfobit[3], (int)r1, (int)r2, (int)Re, td));
	timer_adjust_periodic(lfotimer, attotime_make(0, ATTOSECONDS_PER_SECOND / (MAXFREQ-MINFREQ) * td), 0, attotime_make(0, ATTOSECONDS_PER_SECOND / (MAXFREQ-MINFREQ) * td));
#else
	double r0, r1, rx = 100000.0;

	if( (data & 1) == lfobit[offset] )
		return;

	/*
     * NE555 9R is setup as astable multivibrator
     * - Ra is between 100k and ??? (open?)
     * - Rb is zero here (bridge between pins 6 and 7)
     * - C is 1uF
     * charge time t1 = 0.693 * (Ra + Rb) * C
     * discharge time t2 = 0.693 * (Rb) *  C
     * period T = t1 + t2 = 0.693 * (Ra + 2 * Rb) * C
     * -> min period: 0.693 * 100 kOhm * 1uF -> 69300 us = 14.4Hz
     * -> max period: no idea, since I don't know the max. value for Ra :(
     */

	lfobit[offset] = data & 1;

	/* R?? 330k to gnd */
	r0 = 1.0/330000;
	/* open is a very high value really ;-) */
	r1 = 1.0/1e12;

	/* R18 1M */
	if( lfobit[0] )
		r1 += 1.0/1000000;
	else
		r0 += 1.0/1000000;

	/* R17 470k */
	if( lfobit[1] )
		r1 += 1.0/470000;
	else
		r0 += 1.0/470000;

	/* R16 220k */
	if( lfobit[2] )
		r1 += 1.0/220000;
	else
		r0 += 1.0/220000;

	/* R15 100k */
	if( lfobit[3] )
		r1 += 1.0/100000;
	else
		r0 += 1.0/100000;

	r0 = 1.0/r0;
	r1 = 1.0/r1;

	/* I used an arbitrary value for max. Ra of 2M */
	rx = rx + 2000000.0 * r0 / (r0+r1);

	LOG(("lfotimer bits:%d%d%d%d r0:%d, r1:%d, rx: %d, time: %9.2fus\n", lfobit[3], lfobit[2], lfobit[1], lfobit[0], (int)r0, (int)r1, (int)rx, 0.639 * rx));
	timer_adjust_periodic(lfotimer, attotime_make(0, ATTOSECONDS_PER_SECOND / 1000000000 / (MAXFREQ-MINFREQ) * 639 * rx), 0, attotime_make(0, ATTOSECONDS_PER_SECOND / 1000000000 / (MAXFREQ-MINFREQ) * 639 * rx));
#endif
}

static TIMER_CALLBACK( galaxian_sh_update )
{
	const device_config *samples = devtag_get_device(machine, GAL_AUDIO);

	/*
     * NE555 8R, 8S and 8T are used as pulse position modulators
     * FS1 Ra=100k, Rb=470k and C=0.01uF
     *  -> 0.693 * 1040k * 0.01uF -> 7207.2us = 139Hz
     * FS2 Ra=100k, Rb=330k and C=0.01uF
     *  -> 0.693 * 760k * 0.01uF -> 5266.8us = 190Hz
     * FS2 Ra=100k, Rb=220k and C=0.01uF
     *  -> 0.693 * 540k * 0.01uF -> 3742.2us = 267Hz
     */

	sample_set_freq(samples, CHANNEL_LFO+0, sizeof(backgroundwave)*freq*(100+2*470)/(100+2*470) );
	sample_set_freq(samples, CHANNEL_LFO+1, sizeof(backgroundwave)*freq*(100+2*330)/(100+2*470) );
	sample_set_freq(samples, CHANNEL_LFO+2, sizeof(backgroundwave)*freq*(100+2*220)/(100+2*470) );
}


static const samples_interface galaxian_custom_interface =
{
	5,
	NULL,
	galaxian_sh_start
};

MACHINE_DRIVER_START( galaxian_audio )

	/* sound hardware */

	MDRV_SOUND_ADD(GAL_AUDIO, SAMPLES, 0)
	MDRV_SOUND_CONFIG(galaxian_custom_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

#else

#define XTAL					18432000

#define SOUND_CLOCK 			(XTAL/6/2)			/* 1.536 MHz */
#define RNG_RATE				(XTAL/3*2)			/* RNG clock is XTAL/3*2 see Aaron's note in video/galaxian.c */

#define DISCRETE_BITSET(_N, _N1, _B, _OV) DISCRETE_TRANSFORM4(_N, _N1, (1 << ((_B)-1)), 0, _OV, "01&2>3*")

/* 74LS259 */
#define GAL_INP_BG_DAC			NODE_10		/* at 9M Q4 to Q7 in schematics */

#define GAL_INP_FS1				NODE_20		/* FS1 9L Q0 */
#define GAL_INP_FS2				NODE_21		/* FS2 9L Q1 */
#define GAL_INP_FS3				NODE_22		/* FS3 9L Q2 */
#define GAL_INP_HIT				NODE_23		/* HIT 9L Q3 */
//#define GAL_9L_Q4				NODE_24		
#define GAL_INP_FIRE			NODE_25		/* FIRE 9L Q5 */
#define GAL_INP_VOL1			NODE_26		/* VOL1 9L Q6 */
#define GAL_INP_VOL2			NODE_27		/* VOL2 9L Q7 */

#define GAL_INP_PITCH_HIGH		NODE_28		/* at 6T in schematics */

#define TTL_OUT					(4.0)

#define GAL_R15					RES_K(100)
#define GAL_R16					RES_K(220)
#define GAL_R17					RES_K(470)
#define GAL_R18					RES_K(1000)
#define GAL_R19					RES_K(330)

#define GAL_R20					RES_K(15)
#define GAL_R21					RES_K(100)
#define GAL_R22					RES_K(100)
#define GAL_R23					RES_K(470)
#define GAL_R24					RES_K(10)
#define GAL_R25					RES_K(100)
#define GAL_R26					RES_K(330)
#define GAL_R27					RES_K(10)
#define GAL_R28					RES_K(100)
#define GAL_R29					RES_K(220)

#define GAL_R30					RES_K(10)
#define GAL_R31					RES_K(47)
#define GAL_R32					RES_K(47)
#define GAL_R33					RES_K(10)
/*
 * R34 is given twice on galaxian board and both times as 5.1k. On moon cresta
 * it is only listed once and given as 15k. This is more in line with recordings
 */
#if HARDWARE == HARDWARE_GALAXIAN
#define GAL_R34					RES_K(5.1)			/* val from mcresta galaxian : RES_K(5.1) */
#else
#define GAL_R34					RES_K(15)			/* val from mcresta galaxian : RES_K(5.1) */
#endif
#define GAL_R35					RES_K(150)
#define GAL_R36					RES_K(22)
#define GAL_R37					RES_K(470)
#define GAL_R38					RES_K(33)
#define GAL_R39					RES_K(22)

/* The hit sound is too low compared with recordings
 * There may be an issue with the op-amp band filter
 */
#define GAL_R40					(RES_K(2.2)*0.6)	/* Volume adjust */		
#define GAL_R41					RES_K(100)
#define GAL_R43					RES_K(2.2)
#define GAL_R44					RES_K(10)
#define GAL_R45					RES_K(22)
#define GAL_R46					RES_K(10)
#define GAL_R47					RES_K(2.2)
#define GAL_R48					RES_K(2.2)
#define GAL_R49					RES_K(10)

#define GAL_R50					RES_K(22)
#define GAL_R51					RES_K(33)
#define GAL_R52					RES_K(15)

#define GAL_R91					RES_K(10)

#define GAL_C15					CAP_U(1)
#define GAL_C17					CAP_U(0.01)
#define GAL_C18					CAP_U(0.01)
#define GAL_C19					CAP_U(0.01)

#define GAL_C20					CAP_U(0.1)
#define GAL_C21					CAP_U(2.2)
#define GAL_C22					CAP_U(0.01)
#define GAL_C23					CAP_U(0.01)
#define GAL_C25					CAP_U(1)
#define GAL_C26					CAP_U(0.01)
#define GAL_C27					CAP_U(0.01)
#define GAL_C28					CAP_U(47)

#define GAL_C46					CAP_U(0.1)




static const discrete_dac_r1_ladder galaxian_bck_dac =
{
	4,			// size of ladder
	{GAL_R18, GAL_R17, GAL_R16, GAL_R15, 0,0,0,0},	
	4.4,		// 5V - diode junction (0.6V)
	GAL_R20,	// rBIAS
	GAL_R19,	// rGnd
	0			// no C
};

static const discrete_555_cc_desc galaxian_bck_vco =
{
	DISC_555_OUT_DC | DISC_555_OUT_CAP,
	5,		// B+ voltage of 555
	DEFAULT_555_VALUES,
	0.7		// Q2 junction voltage
};

static const discrete_555_desc galaxian_555_vco_desc =
{
	DISC_555_OUT_ENERGY | DISC_555_OUT_DC,
	5.0,
	DEFAULT_555_CHARGE,
	(5.0 - 0.5)			// 10k means no real load
};

static const discrete_555_desc galaxian_555_fire_vco_desc =
{
	DISC_555_OUT_DC,
	5.0,
	DEFAULT_555_CHARGE,
	1.0 // Logic output			
};

static const discrete_mixer_desc galaxian_bck_mixer_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{GAL_R24, GAL_R27, GAL_R30},
	{0,0,0},
	{0,0,0,0},  /* no node capacitors */
	0, 0,
	GAL_C20,
	0,
	0, 1
};

static const discrete_lfsr_desc galaxian_lfsr =
{
	DISC_CLK_IS_FREQ,
	17,			          	/* Bit Length */
	0,			          	/* Reset Value */
	4,			          	/* Use Bit 10 (QC of second LS164) as F0 input 0 */
	16,			          	/* Use Bit 23 (QH of third LS164) as F0 input 1 */
	DISC_LFSR_XOR_INV_IN1,	/* F0 is XOR */
	DISC_LFSR_IN0,	  		/* F1 is inverted F0*/
	DISC_LFSR_REPLACE,	  	/* F2 replaces the shifted register contents */
	0x000001,		      	/* Everything is shifted into the first bit only */
	DISC_LFSR_FLAG_OUTPUT_F0, /* Output is result of F0 */
	0			          	/* Output bit */
};

#if HARDWARE == HARDWARE_GALAXIAN
static const discrete_mixer_desc galaxian_mixerpre_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{GAL_R51, 0, GAL_R50, 0, GAL_R34},		/* A, C, C, D */
	{0, GAL_INP_VOL1, 0, GAL_INP_VOL2, 0},
	{0,0,0,0,0},  
	0, 0,
	0,
	0,
	0, 1
};
static const discrete_mixer_desc galaxian_mixer_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{GAL_R34, GAL_R40, GAL_R43},		/* A, C, C, D */
	{0, 0, 0},
	{0,0,GAL_C26},  
	0, GAL_R91,
	0,
	GAL_C46,
	0, 1
};
#else
static const discrete_mixer_desc galaxian_mixer_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{GAL_R51, 0, GAL_R50, 0, GAL_R34, GAL_R40, GAL_R43},		/* A, C, C, D */
	{0, GAL_INP_VOL1, 0, GAL_INP_VOL2, 0, 0, 0},
	{0,0,0,0,0,0,GAL_C26},  
	0, 0*GAL_R91,
	0,
	GAL_C46,
	0, 1
};
#endif
static const discrete_op_amp_filt_info galaxian_bandpass_desc =
{
	GAL_R35, GAL_R36, 0, 0,
	GAL_R37, 
	GAL_C22, GAL_C23, 0, 
	5.0*GAL_R39/(GAL_R38+GAL_R39), 
	5, 0
};

static DISCRETE_SOUND_START(galaxian)

	/************************************************/
	/* Input register mapping for galaxian          */
	/************************************************/
	DISCRETE_INPUT_DATA(GAL_INP_BG_DAC)
	
	/* FS1 to FS3 */
	DISCRETE_INPUT_LOGIC(GAL_INP_FS1)
	DISCRETE_INPUT_LOGIC(GAL_INP_FS2)
	DISCRETE_INPUT_LOGIC(GAL_INP_FS3)

	/* HIT */
	DISCRETE_INPUTX_DATA(GAL_INP_HIT, TTL_OUT, 0, 0)
	
	/* FIRE */
	DISCRETE_INPUT_LOGIC(GAL_INP_FIRE)
	
	/* Turns on / off resistors in mixer */
	DISCRETE_INPUTX_DATA(GAL_INP_VOL1, GAL_R49, 0, 0)
	DISCRETE_INPUTX_DATA(GAL_INP_VOL2, GAL_R52, 0, 0)

	/* Pitch */
	DISCRETE_INPUT_DATA(GAL_INP_PITCH_HIGH)
	
	/************************************************/
	/* Background                                   */
	/************************************************/
	
	DISCRETE_DAC_R1(NODE_100, 1, GAL_INP_BG_DAC, TTL_OUT, &galaxian_bck_dac)
	DISCRETE_555_CC(NODE_105, 1, NODE_100, GAL_R21, GAL_C15, 0, 0, 0, &galaxian_bck_vco)
	// Next is mult/add opamp circuit
	DISCRETE_MULTADD(NODE_110, 1, NODE_105, GAL_R33/RES_3_PARALLEL(GAL_R31,GAL_R32,GAL_R33),
			-5.0*GAL_R33/GAL_R31)
	DISCRETE_CLAMP(NODE_111,1,NODE_110,0.0,5.0,0.0)
	// The three 555
	DISCRETE_555_ASTABLE_CV(NODE_115, GAL_INP_FS1, GAL_R22, GAL_R23, GAL_C17, NODE_111, &galaxian_555_vco_desc)
	DISCRETE_555_ASTABLE_CV(NODE_116, GAL_INP_FS2, GAL_R25, GAL_R26, GAL_C18, NODE_111, &galaxian_555_vco_desc)
	DISCRETE_555_ASTABLE_CV(NODE_117, GAL_INP_FS3, GAL_R28, GAL_R29, GAL_C19, NODE_111, &galaxian_555_vco_desc)

	DISCRETE_MIXER3(NODE_120, 1, NODE_115, NODE_116, NODE_117, &galaxian_bck_mixer_desc)

	/************************************************/
	/* PITCH                                        */
	/************************************************/

	/* Needs to be replaced by timer ... */
    DISCRETE_BITSET(NODE_133, GAL_INP_PITCH_HIGH, 1, TTL_OUT)		/* QA 74393 */	
    DISCRETE_BITSET(NODE_134, GAL_INP_PITCH_HIGH, 3, TTL_OUT)		/* QC 74393 */	
    DISCRETE_BITSET(NODE_135, GAL_INP_PITCH_HIGH, 4, TTL_OUT)		/* QD 74393 */	

	/************************************************/
	/* HIT                                        */
	/************************************************/

    /* NOISE */
    /* since only a sample of the LFSR is latched @V2 we let the lfsr
     * run at a lower speed
     */
	DISCRETE_LFSR_NOISE(NODE_150, 1, 1, RNG_RATE/100, 1.0, 0, 0.5, &galaxian_lfsr)
	DISCRETE_SQUAREWFIX(NODE_151,1,60*264/2,1.0,50,0.5,0)  /* 2V signal */
	DISCRETE_LOGIC_DFLIPFLOP(NODE_152,1,1,1,NODE_151,NODE_150)


	/* Not 100% correct - Factor 2 to account for being always enabled */
	DISCRETE_RCDISC5(NODE_155, 1, GAL_INP_HIT, (GAL_R35 + GAL_R36)*2, GAL_C21)
	DISCRETE_MULTIPLY(NODE_156, 1, NODE_152, NODE_155)
	DISCRETE_OP_AMP_FILTER(NODE_157, 1, NODE_156, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &galaxian_bandpass_desc)

	/************************************************/ 	
	/* FIRE                                         */
	/************************************************/
	DISCRETE_LOGIC_INVERT(NODE_160, 1, GAL_INP_FIRE)
	DISCRETE_MULTIPLY(NODE_161, 1, TTL_OUT, GAL_INP_FIRE) 
	DISCRETE_MULTIPLY(NODE_162, 1, TTL_OUT, NODE_160) // inverted
	DISCRETE_RCFILTER(NODE_163, 1, NODE_162, GAL_R47, GAL_C28)
	/* Mix noise and 163 */
	DISCRETE_TRANSFORM5(NODE_167, NODE_152, TTL_OUT, 1.0/GAL_R46, NODE_163, 1.0/GAL_R48,
			"01*2*34*+" )
	//DISCRETE_MULTIPLY(NODE_164, 1, TTL_OUT, NODE_152) 
	//DISCRETE_MULTIPLY(NODE_165, 1, 1.0/GAL_R46, NODE_164) 
	//DISCRETE_MULTIPLY(NODE_166, 1, 1.0/GAL_R48, NODE_163) 
	//DISCRETE_ADDER2(NODE_167, 1, NODE_165, NODE_166) 
	DISCRETE_MULTIPLY(NODE_168, 1, RES_2_PARALLEL(GAL_R46, GAL_R48), NODE_167)
	
	DISCRETE_RCDISC5(NODE_170, 1, NODE_161, (GAL_R41)*2, GAL_C25)

	DISCRETE_555_ASTABLE_CV(NODE_171, 1, GAL_R44, GAL_R45, GAL_C27, NODE_168, &galaxian_555_fire_vco_desc)
	DISCRETE_MULTIPLY(NODE_172, 1, NODE_171, NODE_170)

	/************************************************/
	/* FINAL MIX                                    */
	/************************************************/

#if HARDWARE == HARDWARE_GALAXIAN
	DISCRETE_MIXER5(NODE_279, 1, NODE_133, NODE_134, NODE_134, NODE_135, NODE_120, &galaxian_mixerpre_desc)
	DISCRETE_MIXER3(NODE_280, 1, NODE_279, NODE_157, NODE_172, &galaxian_mixer_desc)
#else
	DISCRETE_MIXER7(NODE_280, 1, NODE_133, NODE_134, NODE_134, NODE_135, NODE_120, NODE_157, NODE_172, &galaxian_mixer_desc)
#endif
	DISCRETE_OUTPUT(NODE_280, 32767.0/5.0*5)

DISCRETE_SOUND_END

static UINT8 lfo_val;
static emu_timer *pitch_timer;
static UINT8 pitch_l;
static UINT8 pitch_h;

static TIMER_CALLBACK( pitch_callback )
{
	const device_config *device = devtag_get_device(machine, GAL_AUDIO);

	timer_adjust_oneshot(pitch_timer, ATTOTIME_IN_HZ(SOUND_CLOCK / (256 - pitch_l)), 0);
	pitch_h++;
	if (pitch_h > 15)
		pitch_h = 0;
	discrete_sound_w(device, GAL_INP_PITCH_HIGH, pitch_h );
}

static SOUND_START(galaxian)
{
	lfo_val = 0;
	pitch_l = 0;
	pitch_h = 0;
	pitch_timer = timer_alloc(machine, pitch_callback, NULL);
	timer_adjust_oneshot(pitch_timer, ATTOTIME_IN_HZ(SOUND_CLOCK/256), 0);
}


WRITE8_DEVICE_HANDLER( galaxian_lfo_freq_w )
{
	UINT8 lfo_val_new = (lfo_val & ~(1<<offset)) | ((data & 0x01) << offset);
	
	if (lfo_val != lfo_val_new)
	{
		lfo_val = lfo_val_new;
		discrete_sound_w(device, GAL_INP_BG_DAC, lfo_val);
	}
}

WRITE8_DEVICE_HANDLER( galaxian_background_enable_w )
{
	discrete_sound_w(device, NODE_RELATIVE(GAL_INP_FS1, offset), data & 0x01);
}

WRITE8_DEVICE_HANDLER( galaxian_noise_enable_w )
{
	discrete_sound_w(device, GAL_INP_HIT, data & 0x01);
}

WRITE8_DEVICE_HANDLER( galaxian_vol_w )
{
	discrete_sound_w(device, NODE_RELATIVE(GAL_INP_VOL1,offset), data & 0x01);
}

/* FIXME: rename to fire to be consistent */
WRITE8_DEVICE_HANDLER( galaxian_shoot_enable_w )
{
	discrete_sound_w(device, GAL_INP_FIRE, data & 0x01);
}

/* FIXME: use timer ! -- IC 9J */
WRITE8_DEVICE_HANDLER( galaxian_pitch_w )
{
	pitch_l = data;
}

/* FIXME: May be replaced by one call! */
WRITE8_DEVICE_HANDLER( galaxian_sound_w )
{
	data &= 0x01;
	switch (offset & 7)
	{
		case 0:		/* FS1 (controls 555 timer at 8R) */
		case 1:		/* FS2 (controls 555 timer at 8S) */
		case 2:		/* FS3 (controls 555 timer at 8T) */
			galaxian_background_enable_w(device, offset, data);
			break;

		case 3:		/* HIT */
			galaxian_noise_enable_w(device, 0, data);
			break;

		case 4:		/* n/c */
			break;

		case 5:		/* FIRE */
			galaxian_shoot_enable_w(device, 0, data);
			break;

		case 6:		/* VOL1 */
		case 7:		/* VOL2 */
			galaxian_vol_w(device, offset & 1, data);
			break;
	}
}

MACHINE_DRIVER_START( galaxian_audio )

	MDRV_SOUND_START(galaxian)
	
	MDRV_SOUND_ADD(GAL_AUDIO, DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(galaxian)
	
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

#endif
