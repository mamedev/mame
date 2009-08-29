/*

Battlezone sound info, courtesy of Al Kossow:

D7  motor enable            this enables the engine sound
D6  start LED
D5  sound enable            this enables ALL sound outputs
                            including the POKEY output
D4  engine rev en           this controls the engine speed
                            the engine sound is an integrated square
                            wave (saw tooth) that is frequency modulated
                            by engine rev.
D3  shell loud, soft/       explosion volume
D2  shell enable
D1  explosion loud, soft/   explosion volume
D0  explosion enable        gates a noise generator

*/

#include <math.h>
#include "driver.h"
#include "streams.h"
#include "bzone.h"

#if !BZONE_DISCRETE
#define OUTPUT_RATE (6000*4)


/* Statics */
static INT16 *discharge = NULL;
#define EXP(charge,n) (charge ? 0x7fff - discharge[0x7fff-n] : discharge[n])

static sound_stream *channel;
static int latch;
static int poly_counter;
static int poly_shift;

static int explosion_clock;
static int explosion_out;
static int explosion_amp;
static int explosion_amp_counter;

static int shell_clock;
static int shell_out;
static int shell_amp;
static int shell_amp_counter;

static int motor_counter;
static int motor_counter_a;
static int motor_counter_b;
static int motor_rate;
static int motor_rate_new;
static int motor_rate_counter;
static int motor_amp;
static int motor_amp_new;
static int motor_amp_step;
static int motor_amp_counter;

WRITE8_HANDLER( bzone_sounds_w )
{
	if( data == latch )
		return;

	stream_update(channel);
    latch = data;

    sound_global_enable(latch & 0x20);
}

static STREAM_UPDATE( bzone_sound_update )
{
	stream_sample_t *buffer = outputs[0];
	while( samples-- )
	{
		static int last_val = 0;
		int sum = 0;

		/* polynome shifter H5 and H4 (LS164) clocked with 6kHz */
		poly_counter -= 6000;
		while( poly_counter <= 0 )
		{
			int clock;

			poly_counter += OUTPUT_RATE;
			if( ((poly_shift & 0x0008) == 0) == ((poly_shift & 0x4000) == 0) )
				poly_shift = (poly_shift << 1) | 1;
			else
				poly_shift <<= 1;

			/* NAND gate J4 */
			clock = ((poly_shift & 0x7000) == 0x7000) ? 0 : 1;

			/* raising edge on pin 3 of J5 (LS74)? */
			if( clock && !explosion_clock )
				explosion_out ^= 1;

			/* save explo clock level */
			explosion_clock = clock;

			/* input 11 of J5 (LS74) */
			clock = (poly_shift >> 15) & 1;

			/* raising edge on pin 11 of J5 (LS74)? */
			if( clock && !shell_clock )
				shell_out ^= 1;

			/* save shell clock level */
			shell_clock = clock;
		}

		/* explosion enable: charge C14 */
		if( latch & 0x01 )
			explosion_amp = 32767;

		/* explosion output? */
		if( explosion_out )
		{
			if( explosion_amp > 0 )
			{
				/*
                 * discharge C14 through R17 + R16
                 * time constant is 10e-6 * 23000 = 0.23 seconds
                 * (samples were decaying much slower: 1/4th rate? )
                 */
				explosion_amp_counter -= (int)(32767 / (0.23*4));
				if( explosion_amp_counter < 0 )
				{
					int n = (-explosion_amp_counter / OUTPUT_RATE) + 1;
					explosion_amp_counter += n * OUTPUT_RATE;
					if( (explosion_amp -= n) < 0 )
						explosion_amp = 0;
				}
			}
			/*
             * I don't know the amplification of the op-amp
             * and feedback, so the loud/soft values are arbitrary
             */
			if( latch & 0x02 )	/* explosion loud ? */
				sum += EXP(0,explosion_amp)/3;
			else
				sum += EXP(0,explosion_amp)/4;
		}

		/* shell enable: charge C9 */
		if( latch & 0x04 )
			shell_amp = 32767;

		/* shell output? */
		if( shell_out )
		{
			if( shell_amp > 0 )
			{
				/*
                 * discharge C9 through R14 + R15
                 * time constant is 4.7e-6 * 23000 = 0.1081 seconds
                 * (samples were decaying much slower: 1/4th rate? )
                 */
				shell_amp_counter -= (int)(32767 / (0.1081*4));
				if( shell_amp_counter < 0 )
				{
					int n = (-shell_amp_counter / OUTPUT_RATE) + 1;
					shell_amp_counter += n * OUTPUT_RATE;
					if( (shell_amp -= n) < 0 )
						shell_amp = 0;
				}
			}
			/*
             * I don't know the amplification of the op-amp
             * and feedback, so the loud/soft values are arbitrary
             */
			if( latch & 0x08 )	/* shell loud ? */
				sum += EXP(0,shell_amp)/3;
			else
				sum += EXP(0,shell_amp)/4;
		}

		if( latch & 0x80 )
		{
			/* NE5555 timer
             * C = 0.018u, Ra = 100k, Rb = 125k
             * charge time = 0.693 * (Ra + Rb) * C = 3870us
             * discharge time = 0.693 * Rb * C = 1559.25us
             * freq approx. 184 Hz
             * I have no idea what frequencies are coming from the NE555
             * with "MOTOR REV EN" being high or low. I took 240Hz as
             * higher rate and sweep up or down to the new rate in 0.25s
             */
			motor_rate_new = (latch & 0x10) ? 240 : 184;
			if( motor_rate != motor_rate_new )
			{
				/* sweep rate to new rate */
				motor_rate_counter -= (int)((240 - 184) / 0.25);
				while( motor_rate_counter <= 0 )
				{
					motor_rate_counter += OUTPUT_RATE;
					motor_rate += (motor_rate < motor_rate_new) ? +1 : -1;
				}
			}
			motor_counter -= motor_rate;
			while( motor_counter <= 0 )
			{
				double r0, r1;

				motor_counter += OUTPUT_RATE;

				r0 = 1.0/1e12;
				r1 = 1.0/1e12;

				if( ++motor_counter_a == 16 )
					motor_counter_a = 6;
				if( ++motor_counter_b == 16 )
					motor_counter_b = 4;

				if( motor_counter_a & 8 )	/* bit 3 */
					r1 += 1.0/33000;
				else
					r0 += 1.0/33000;
				if( motor_counter_a == 15 ) /* ripple carry */
					r1 += 1.0/33000;
				else
					r0 += 1.0/33000;

				if( motor_counter_b & 8 )	/* bit 3 */
					r1 += 1.0/33000;
				else
					r0 += 1.0/33000;
				if( motor_counter_b == 15 ) /* ripple carry */
					r1 += 1.0/33000;
				else
					r0 += 1.0/33000;

				/* new voltage at C29 */
				r0 = 1.0/r0;
				r1 = 1.0/r1;
				motor_amp_new = (int)(32767 * r0 / (r0 + r1));

				/* charge/discharge C29 (0.47uF) */
				if( motor_amp_new > motor_amp )
					motor_amp_step = (int)((motor_amp_new - motor_amp) / (r1*0.47e-6));
				else
					motor_amp_step = (int)((motor_amp - motor_amp_new) / (r0*0.47e-6));
			}
			if( motor_amp != motor_amp_new )
			{
				motor_amp_counter -= motor_amp_step;
				if( motor_amp_counter < 0 )
				{
					int n = (-motor_amp_counter / OUTPUT_RATE) + 1;
					motor_amp_counter += n * OUTPUT_RATE;
					if( motor_amp > motor_amp_new )
					{
						motor_amp -= n;
						if( motor_amp < motor_amp_new )
							motor_amp = motor_amp_new;
					}
					else
					{
						motor_amp += n;
						if( motor_amp > motor_amp_new )
							motor_amp = motor_amp_new;
					}
				}
			}
			sum += EXP((motor_amp<motor_amp_new),motor_amp)/3;
		}

		*buffer++ = (sum + last_val) / 2;

		/* crude 75% low pass filter */
		last_val = (sum + last_val * 3) / 4;
	}
}

static DEVICE_START( bzone_sound )
{
	int i;

	discharge = auto_alloc_array(device->machine, INT16, 32768);
	for( i = 0; i < 0x8000; i++ )
		discharge[0x7fff-i] = (INT16) (0x7fff/exp(1.0*i/4096));

	channel = stream_create(device, 0, 1, OUTPUT_RATE, 0, bzone_sound_update);
}


DEVICE_GET_INFO( bzone_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(bzone_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Battlezone Engine");			break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}

#else

#include "sound/discrete.h"
#include "sound/pokey.h"

#define BZ_NOISE_CLOCK		12000		/* FIXME */

/*************************************
 *
 *  Discrete Sound Defines
 *
 *************************************/

#define BZ_INPUT			NODE_01		/* at M2 LS273 */
#define BZ_INP_EXPLO		NODE_SUB(10, 0)
#define BZ_INP_EXPLOLS		NODE_SUB(10, 1)
#define BZ_INP_SHELL		NODE_SUB(10, 2)
#define BZ_INP_SHELLLS		NODE_SUB(10, 3)
#define BZ_INP_ENGREV		NODE_SUB(10, 4)
#define BZ_INP_SOUNDEN		NODE_SUB(10, 5)
#define BZ_INP_STARTLED		NODE_SUB(10, 6)
#define BZ_INP_MOTEN		NODE_SUB(10, 7)

#define TTL_OUT 5

#define BZ_R5			RES_K(1)
#define BZ_R6			RES_K(4.7)
#define BZ_R7			RES_K(1)
#define BZ_R8			RES_K(100)
#define BZ_R9			RES_K(22)

//#define RT            (1.0/BZ_R5 + 1.0/BZ_R6 * 1.0/BZ_R7)

#define BZ_R10			RES_K(100)
#define BZ_R11			RES_K(250)
#define BZ_R12			RES_K(33)
#define BZ_R13			RES_K(10)
#define BZ_R14			RES_K(22)
#define BZ_R15			RES_K(1)
#define BZ_R16			RES_K(1)
#define BZ_R17			RES_K(22)
#define BZ_R18			RES_K(10)
#define BZ_R19			RES_K(33)

#define BZ_R20			RES_K(33)
#define BZ_R21			RES_K(33)
#define BZ_R25			RES_K(100)
#define BZ_R26			RES_K(33)
#define BZ_R27			RES_K(330)
#define BZ_R28			RES_K(100)
#define BZ_R29			RES_K(22)

#define BZ_R32			RES_K(330)
#define BZ_R33			RES_K(330)
#define BZ_R34			RES_K(33)
#define BZ_R35			RES_K(33)

#define BZ_C9			CAP_U(4.7)

#define BZ_C11			CAP_U(0.015)
#define BZ_C13			CAP_U(10)
#define BZ_C14			CAP_U(10)

#define BZ_C21			CAP_U(0.0047)
#define BZ_C22			CAP_U(0.0047)
#define BZ_C29			CAP_U(0.47)

/*************************************
 *
 *  Discrete Sound static structs
 *
 *************************************/


static const discrete_lfsr_desc bzone_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,			          	/* Bit Length */
	0,			          	/* Reset Value */
	3,			          	/* Use Bit 10 (QC of second LS164) as F0 input 0 */
	14,			          	/* Use Bit 23 (QH of third LS164) as F0 input 1 */
	DISC_LFSR_XOR,			/* F0 is XOR */
	DISC_LFSR_NOT_IN0, 		/* F1 is inverted F0*/
	DISC_LFSR_REPLACE,	  	/* F2 replaces the shifted register contents */
	0x000001,		      	/* Everything is shifted into the first bit only */
	DISC_LFSR_FLAG_OUTPUT_SR_SN1, /* output the complete shift register to sub node 1*/
	15		          	/* Output bit */
};

static const discrete_op_amp_filt_info bzone_explo_0 =
{
		BZ_R18 + BZ_R19, 0, 0, 0, 		/* r1, r2, r3, r4 */
		BZ_R33,							/* rF */
		BZ_C22, 0, 0,					/* c1, c2, c3 */
		0,								/* vRef - not used */
		22, 0							/* vP, vN */
};

static const discrete_op_amp_filt_info bzone_explo_1 =
{
		BZ_R18, 0, 0, 0, 				/* r1, r2, r3, r4 */
		BZ_R33,							/* rF */
		BZ_C22, 0, 0,					/* c1, c2, c3 */
		0,								/* vRef - not used */
		22, 0							/* vP, vN */
};

static const discrete_op_amp_filt_info bzone_shell_0 =
{
		BZ_R13 + BZ_R12, 0, 0, 0, 		/* r1, r2, r3, r4 */
		BZ_R32,							/* rF */
		BZ_C21, 0, 0,					/* c1, c2, c3 */
		0,								/* vRef - not used */
		22, 0							/* vP, vN */
};

static const discrete_op_amp_filt_info bzone_shell_1 =
{
		BZ_R13, 0, 0, 0, 				/* r1, r2, r3, r4 */
		BZ_R32,							/* rF */
		BZ_C21, 0, 0,					/* c1, c2, c3 */
		0,								/* vRef - not used */
		22, 0							/* vP, vN */
};

static const discrete_555_desc bzone_vco_desc =
{
	DISC_555_OUT_DC,
	5.0,
	DEFAULT_555_CHARGE,
	1.0 // Logic output
};

static const discrete_mixer_desc bzone_eng_mixer_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{BZ_R20, BZ_R21, BZ_R34, BZ_R35},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	0, 0,
	BZ_C29,
	0, /* no out cap */
	0, TTL_OUT		/* inputs are logic */
};

static const discrete_mixer_desc bzone_final_mixer_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{BZ_R28, BZ_R25, BZ_R26, BZ_R27},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	0, BZ_R29,
	0,
	0, /* no out cap */
	0, 1
};

/*************************************
 *
 *  Discrete Sound Blocks
 *
 *************************************/

static DISCRETE_SOUND_START(bzone)

	/************************************************/
	/* Input register mapping for galaxian          */
	/************************************************/
	DISCRETE_INPUT_DATA(BZ_INPUT)

	/* decode the bits */
	DISCRETE_BITS_DECODE(NODE_10, BZ_INPUT, 0, 7, 5.7)// TTL_OUT)       /* QA-QD 74393 */
	DISCRETE_ADJUSTMENT_TAG(NODE_11, 0, RES_K(250), DISC_LINADJ, "R11")


	/************************************************/
	/* NOISE                                        */
	/************************************************/

	/* 12Khz clock is divided by two by B4 74LS109 */
	DISCRETE_LFSR_NOISE(NODE_30, 1, 1, BZ_NOISE_CLOCK / 2, 1.0, 0, 0.5, &bzone_lfsr)

	/* divide by 2 */
	DISCRETE_COUNTER(NODE_31, 1, 0, NODE_30, 1, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE)

	DISCRETE_BITS_DECODE(NODE_32, NODE_SUB(30,1), 11, 14, 1)		/* to NAND LS20, J4 */
	/* 11-14 */
	DISCRETE_LOGIC_NAND4(NODE_33,1,NODE_SUB(32,0),NODE_SUB(32,1),NODE_SUB(32,2),NODE_SUB(32,3))
	/* divide by 2 */
	DISCRETE_COUNTER(NODE_34, 1, 0, NODE_33, 1, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE)

	/************************************************/
	/* Explosion                                    */
	/************************************************/

	/* FIXME: +0.7 for diode */
	DISCRETE_RCDISC5(NODE_40, NODE_34, BZ_INP_EXPLO, BZ_R17 + BZ_R16, BZ_C14)
	DISCRETE_MULTIPLY(NODE_41, 1, BZ_R16 / (BZ_R17 + BZ_R16), NODE_40)

	/* one of two filter configurations active */
	DISCRETE_LOGIC_INVERT(NODE_42, 1, BZ_INP_EXPLOLS)
	DISCRETE_OP_AMP_FILTER(NODE_43, BZ_INP_EXPLOLS,  0, NODE_41,
			DISC_OP_AMP_FILTER_IS_LOW_PASS_1M, &bzone_explo_1)
	DISCRETE_OP_AMP_FILTER(NODE_44, NODE_42,  0, NODE_41,
		DISC_OP_AMP_FILTER_IS_LOW_PASS_1M, &bzone_explo_0)
	DISCRETE_ADDER2(NODE_45, 1, NODE_43, NODE_44)

	/************************************************/
	/* Shell                                        */
	/************************************************/
	/* FIXME: +0.7 for diode */
	DISCRETE_RCDISC5(NODE_50, NODE_31, BZ_INP_SHELL, BZ_R14 + BZ_R15, BZ_C9)
	DISCRETE_MULTIPLY(NODE_51, 1, BZ_R15 / (BZ_R14 + BZ_R15), NODE_50)

	/* one of two filter configurations active */
	DISCRETE_LOGIC_INVERT(NODE_52, 1, BZ_INP_SHELLLS)
	DISCRETE_OP_AMP_FILTER(NODE_53, BZ_INP_SHELLLS,  0, NODE_51,
			DISC_OP_AMP_FILTER_IS_LOW_PASS_1M, &bzone_shell_1)
	DISCRETE_OP_AMP_FILTER(NODE_54, NODE_52,  0, NODE_51,
		DISC_OP_AMP_FILTER_IS_LOW_PASS_1M, &bzone_shell_0)
	DISCRETE_ADDER2(NODE_55, 1, NODE_53, NODE_54)

	/************************************************/
	/* Engine                                       */
	/************************************************/


	DISCRETE_TRANSFORM2(NODE_60, BZ_INP_ENGREV, 0.0, "01=")
	// FIXME: from R5 .. R7
	DISCRETE_MULTIPLEX2(NODE_61, 1, NODE_60, 2.5, 4.2)
	DISCRETE_RCDISC3(NODE_62, 1, NODE_61, BZ_R8, BZ_R9, BZ_C13, -0.5)

	/* R11 taken from adjuster port */
	DISCRETE_555_ASTABLE_CV(NODE_63, 1, BZ_R10, NODE_11, BZ_C11, NODE_62, &bzone_vco_desc)

	/* two LS161, reset to 4 resp 6 counting up to 15, QD and ripple carry mixed */
	DISCRETE_COUNTER(NODE_65, BZ_INP_MOTEN, 0, NODE_63, 11, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE)
	DISCRETE_TRANSFORM2(NODE_66, NODE_65, 3, "01>") /* QD */
	DISCRETE_TRANSFORM2(NODE_67, NODE_65, 11, "01=") /* Ripple */

	DISCRETE_COUNTER(NODE_68, BZ_INP_MOTEN, 0, NODE_63, 9, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE)
	DISCRETE_TRANSFORM2(NODE_69, NODE_68, 1, "01>") /* QD */
	DISCRETE_TRANSFORM2(NODE_70, NODE_68, 9, "01=") /* Ripple */

	DISCRETE_MIXER4(NODE_75, 1, NODE_66, NODE_67, NODE_69, NODE_70, &bzone_eng_mixer_desc)

	/************************************************/
	/* FINAL MIX                                    */
	/************************************************/

	/* not sure about pokey output levels - bleow is just a estimate */
	DISCRETE_INPUTX_STREAM(NODE_85, 0, 5.0/32767.0 * 4, 0)

	DISCRETE_MIXER4(NODE_280, 1, NODE_45, NODE_55, NODE_75, NODE_85, &bzone_final_mixer_desc)
	DISCRETE_OUTPUT(NODE_280, 32767.0/5.0 * 2)
	//DISCRETE_WAVELOG1(NODE_55, 32767.0/22)
	//DISCRETE_WAVELOG2(NODE_30, 32767.0/5.0, NODE_31, 32767.0/5.0)

DISCRETE_SOUND_END

static const pokey_interface bzone_pokey_interface =
{
	{ DEVCB_NULL },
	DEVCB_INPUT_PORT("IN3")
};

WRITE8_DEVICE_HANDLER( bzone_sounds_w )
{
	discrete_sound_w(device, BZ_INPUT, data);

	output_set_value("startled", (data >> 6) & 1);
    sound_global_enable(data & 0x20);
}


MACHINE_DRIVER_START( bzone_audio )

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("pokey",  POKEY, BZONE_MASTER_CLOCK / 8)
	MDRV_SOUND_CONFIG(bzone_pokey_interface)
	MDRV_SOUND_ROUTE_EX(0, "discrete", 1.0, 0)

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(bzone)

	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

#endif
