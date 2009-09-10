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
	DISCRETE_LOGIC_NAND4(NODE_33,NODE_SUB(32,0),NODE_SUB(32,1),NODE_SUB(32,2),NODE_SUB(32,3))
	/* divide by 2 */
	DISCRETE_COUNTER(NODE_34, 1, 0, NODE_33, 1, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE)

	/************************************************/
	/* Explosion                                    */
	/************************************************/

	/* FIXME: +0.7 for diode */
	DISCRETE_RCDISC5(NODE_40, NODE_34, BZ_INP_EXPLO, BZ_R17 + BZ_R16, BZ_C14)
	DISCRETE_MULTIPLY(NODE_41, BZ_R16 / (BZ_R17 + BZ_R16), NODE_40)

	/* one of two filter configurations active */
	DISCRETE_LOGIC_INVERT(NODE_42, BZ_INP_EXPLOLS)
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
	DISCRETE_MULTIPLY(NODE_51, BZ_R15 / (BZ_R14 + BZ_R15), NODE_50)

	/* one of two filter configurations active */
	DISCRETE_LOGIC_INVERT(NODE_52, BZ_INP_SHELLLS)
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
	DISCRETE_MULTIPLEX2(NODE_61, NODE_60, 2.5, 4.2)
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
    sound_global_enable(device->machine, data & 0x20);
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
