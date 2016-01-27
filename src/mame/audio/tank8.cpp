// license:BSD-3-Clause
// copyright-holders:Hans Andersson
/*************************************************************************

    audio\tank8.c

*************************************************************************/
#include "emu.h"
#include "includes/tank8.h"
#include "sound/discrete.h"


/************************************************************************/
/* tank8 Sound System Analog emulation                                  */
/* Written by Hans Andersson. Feb 2005                                  */
/************************************************************************/

static const discrete_lfsr_desc tank8_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,         /* Bit Length */
	0,          /* Reset Value */
	10,         /* Use Bit 10 as F0 input 0 */
	15,         /* Use Bit 15 as F0 input 1 */
	DISC_LFSR_XOR,      /* F0 is XOR */
	DISC_LFSR_XOR,      /* F1 is F0 XOR with external feed. External feed is (address line) A2 */
	DISC_LFSR_REPLACE,  /* F2 replaces the shifted register contents */
	0x000001,       /* Everything is shifted into the first bit only */
	1,          /* Output is inverted by Q20 */
	12          /* Output bit */
};

static const discrete_integrate_info tank8_op1_integrate_info =
{
	DISC_INTEGRATE_OP_AMP_1,
	RES_K(27),      /* R98 */
	RES_K(2.2),     /* R97 */
	RES_K(47),      /* R96 */
	CAP_U(0.22),    /* C60 */
	5,
	12,             // B+ is 12V, not 15V shown in the schematic.
	0,
	0,
	0,
};

static const discrete_integrate_info tank8_op2_integrate_info =
{
	DISC_INTEGRATE_OP_AMP_1,
	RES_K(27),      /* R99 */
	RES_K(2.2),     /* R97 */
	RES_K(47),      /* R96 */
	CAP_U(0.1),     /* C59 */
	5,
	12,             // B+ is 12V, not 15V shown in the schematic.
	0,
	0,
	0,
};

static const discrete_555_desc tank8_555_a =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_555_desc tank8_555_m =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC | DISC_555_TRIGGER_IS_VOLTAGE,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES
};


static const discrete_op_amp_filt_info tank8_filt =
{
	RES_K(18),      // R56
	0,
	RES_K(0.820),   //R57
	0,
	RES_K(330),     //R58
	CAP_U(.047),    //C42
	CAP_U(.047),    //C43
	0,
	2.5,
	5, /* VCC */
	0  /* VEE */
};

static const discrete_dac_r1_ladder tank8_dac =
{
	1,
	{RES_K(4.7)},   // R89
	5,              // 555 Vcc
	RES_K(5),       // 555 internal
	RES_K(10),      // 555 internal
	CAP_U(100)      // C26
};

/* Nodes - Sounds */
#define TANK8_MOTOR1        NODE_15
#define TANK8_MOTOR2        NODE_16
#define TANK8_MOTOR3        NODE_17
#define TANK8_MOTOR4        NODE_18
#define TANK8_MOTOR5        NODE_19
#define TANK8_MOTOR6        NODE_20
#define TANK8_MOTOR7        NODE_21
#define TANK8_MOTOR8        NODE_22
#define TANK8_CRASHEXPL     NODE_23
#define TANK8_BUGLE         NODE_24
#define TANK8_FINAL_MIX     NODE_25
#define TANK8_A2_LINE       NODE_26


DISCRETE_SOUND_START(tank8)
	/************************************************/
	/* Tank8 sound system: 10 Sound Sources         */
	/*    Motor 1-8                                 */
	/*    Crash                                     */
	/*    Explosion                                 */
	/*    Bugle                                     */
	/************************************************/

	/************************************************/
	/* Input register mapping for tank8           */
	/************************************************/
	/*                   NODE        */
	DISCRETE_INPUT_LOGIC(TANK8_CRASH_EN)
	DISCRETE_INPUT_LOGIC(TANK8_EXPLOSION_EN)
	DISCRETE_INPUT_LOGIC(TANK8_MOTOR1_EN)
	DISCRETE_INPUT_LOGIC(TANK8_MOTOR2_EN)
	DISCRETE_INPUT_LOGIC(TANK8_MOTOR3_EN)
	DISCRETE_INPUT_LOGIC(TANK8_MOTOR4_EN)
	DISCRETE_INPUT_LOGIC(TANK8_MOTOR5_EN)
	DISCRETE_INPUT_LOGIC(TANK8_MOTOR6_EN)
	DISCRETE_INPUT_LOGIC(TANK8_MOTOR7_EN)
	DISCRETE_INPUT_LOGIC(TANK8_MOTOR8_EN)
	DISCRETE_INPUT_NOT(TANK8_ATTRACT_EN)
	DISCRETE_INPUT_LOGIC(TANK8_BUGLE_EN)
	DISCRETE_INPUT_DATA(TANK8_BUGLE_DATA1)
	DISCRETE_INPUT_DATA(TANK8_BUGLE_DATA2)

	/************************************************/
	/* Motor sound circuit is based on a 556 VCO    */
	/* with the input frequency set by a charging   */
	/* Capacitor. The DAC emulates the 555's        */
	/* internal CV resistance.                      */
	/* A 9334 has typical 3.6V output.              */
	/************************************************/

	DISCRETE_LOGIC_INVERT(NODE_30, TANK8_MOTOR1_EN)
	DISCRETE_DAC_R1(NODE_32, TANK8_MOTOR1_EN, 3.6, &tank8_dac)
	DISCRETE_555_ASTABLE_CV(NODE_33, NODE_30, RES_K(220), RES_K(39), CAP_U(0.22), NODE_32, &tank8_555_a)
	DISCRETE_CRFILTER_VREF(NODE_34, NODE_33, RES_K(22), CAP_U(0.001), 5.0)
	DISCRETE_555_MSTABLE(NODE_35, 1, NODE_34, RES_K(56), CAP_U(0.1), &tank8_555_m)
	DISCRETE_OP_AMP_FILTER(TANK8_MOTOR1, 1, NODE_35, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &tank8_filt)

	DISCRETE_LOGIC_INVERT(NODE_40, TANK8_MOTOR2_EN)
	DISCRETE_DAC_R1(NODE_42, TANK8_MOTOR2_EN, 3.6, &tank8_dac)
	DISCRETE_555_ASTABLE_CV(NODE_43, NODE_40, RES_K(220), RES_K(39), CAP_U(0.22), NODE_42, &tank8_555_a)
	DISCRETE_CRFILTER_VREF(NODE_44, NODE_43, RES_K(22), CAP_U(0.001), 5.0)
	DISCRETE_555_MSTABLE(NODE_45, 1, NODE_44, RES_K(56), CAP_U(0.1), &tank8_555_m)
	DISCRETE_OP_AMP_FILTER(TANK8_MOTOR2, 1, NODE_45, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &tank8_filt)

	DISCRETE_LOGIC_INVERT(NODE_50, TANK8_MOTOR3_EN)
	DISCRETE_DAC_R1(NODE_52, TANK8_MOTOR3_EN, 3.6, &tank8_dac)
	DISCRETE_555_ASTABLE_CV(NODE_53, NODE_50, RES_K(220), RES_K(39), CAP_U(0.22), NODE_52, &tank8_555_a)
	DISCRETE_CRFILTER_VREF(NODE_54, NODE_53, RES_K(22), CAP_U(0.001), 5.0)
	DISCRETE_555_MSTABLE(NODE_55, 1, NODE_54, RES_K(56), CAP_U(0.1), &tank8_555_m)
	DISCRETE_OP_AMP_FILTER(TANK8_MOTOR3, 1, NODE_55, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &tank8_filt)

	DISCRETE_LOGIC_INVERT(NODE_60, TANK8_MOTOR4_EN)
	DISCRETE_DAC_R1(NODE_62, TANK8_MOTOR4_EN, 3.6, &tank8_dac)
	DISCRETE_555_ASTABLE_CV(NODE_63, NODE_60, RES_K(220), RES_K(39), CAP_U(0.22), NODE_62, &tank8_555_a)
	DISCRETE_CRFILTER_VREF(NODE_64, NODE_63, RES_K(22), CAP_U(0.001), 5.0)
	DISCRETE_555_MSTABLE(NODE_65, 1, NODE_64, RES_K(56), CAP_U(0.1), &tank8_555_m)
	DISCRETE_OP_AMP_FILTER(TANK8_MOTOR4, 1, NODE_65, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &tank8_filt)

	DISCRETE_LOGIC_INVERT(NODE_70, TANK8_MOTOR5_EN)
	DISCRETE_DAC_R1(NODE_72, TANK8_MOTOR5_EN, 3.6, &tank8_dac)
	DISCRETE_555_ASTABLE_CV(NODE_73, NODE_70, RES_K(220), RES_K(39), CAP_U(0.22), NODE_72, &tank8_555_a)
	DISCRETE_CRFILTER_VREF(NODE_74, NODE_73, RES_K(22), CAP_U(0.001), 5.0)
	DISCRETE_555_MSTABLE(NODE_75, 1, NODE_74, RES_K(56), CAP_U(0.1), &tank8_555_m)
	DISCRETE_OP_AMP_FILTER(TANK8_MOTOR5, 1, NODE_75, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &tank8_filt)

	DISCRETE_LOGIC_INVERT(NODE_80, TANK8_MOTOR6_EN)
	DISCRETE_DAC_R1(NODE_82, TANK8_MOTOR6_EN, 3.6, &tank8_dac)
	DISCRETE_555_ASTABLE_CV(NODE_83, NODE_80, RES_K(220), RES_K(39), CAP_U(0.22), NODE_82, &tank8_555_a)
	DISCRETE_CRFILTER_VREF(NODE_84, NODE_83, RES_K(22), CAP_U(0.001), 5.0)
	DISCRETE_555_MSTABLE(NODE_85, 1, NODE_84, RES_K(56), CAP_U(0.1), &tank8_555_m)
	DISCRETE_OP_AMP_FILTER(TANK8_MOTOR6, 1, NODE_85, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &tank8_filt)

	DISCRETE_LOGIC_INVERT(NODE_90, TANK8_MOTOR7_EN)
	DISCRETE_DAC_R1(NODE_92, TANK8_MOTOR7_EN, 3.6, &tank8_dac)
	DISCRETE_555_ASTABLE_CV(NODE_93, NODE_90, RES_K(220), RES_K(39), CAP_U(0.22), NODE_92, &tank8_555_a)
	DISCRETE_CRFILTER_VREF(NODE_94, NODE_93, RES_K(22), CAP_U(0.001), 5.0)
	DISCRETE_555_MSTABLE(NODE_95, 1, NODE_94, RES_K(56), CAP_U(0.1), &tank8_555_m)
	DISCRETE_OP_AMP_FILTER(TANK8_MOTOR7, 1, NODE_95, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &tank8_filt)

	DISCRETE_LOGIC_INVERT(NODE_100, TANK8_MOTOR8_EN)
	DISCRETE_DAC_R1(NODE_102, TANK8_MOTOR8_EN, 3.6, &tank8_dac)
	DISCRETE_555_ASTABLE_CV(NODE_103, NODE_100, RES_K(220), RES_K(39), CAP_U(0.22), NODE_102, &tank8_555_a)
	DISCRETE_CRFILTER_VREF(NODE_104, NODE_103, RES_K(22), CAP_U(0.001), 5.0)
	DISCRETE_555_MSTABLE(NODE_105, 1, NODE_104, RES_K(56), CAP_U(0.1), &tank8_555_m)
	DISCRETE_OP_AMP_FILTER(TANK8_MOTOR8, 1, NODE_105, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &tank8_filt)

	/************************************************/
	/* Bugle call is built from 9316 counter        */
	/* counter is  clocked by the 1V signal.        */
	/* 1V = HSYNC/2                                 */
	/*    = 15750/2                                 */
	/************************************************/
	DISCRETE_DIVIDE(NODE_110,1,TANK8_BUGLE_DATA1,7875.0)
	DISCRETE_DIVIDE(NODE_111,1,TANK8_BUGLE_DATA2,7875.0)
	DISCRETE_SQUAREWAVE2(NODE_112,TANK8_BUGLE_EN,3.4, NODE_110, NODE_111,1.7,0)
	DISCRETE_RCDISC3(NODE_113,1, NODE_112, RES_K(10),RES_K(1),CAP_U(0.1), 0.5)
	DISCRETE_CRFILTER(TANK8_BUGLE, NODE_113, RES_K(47), CAP_U(.01))

	/************************************************/
	/* Noise generator is built from 2 shift        */
	/* registers that are clocked by the 2V signal. */
	/* 2V = HSYNC/4                                 */
	/*    = 15750/4                                 */
	/************************************************/
	/* The A2 address line is also mixed in to the noise generator,
	 * but that is currently not implemented. */
	DISCRETE_CONSTANT(TANK8_A2_LINE,1)
	DISCRETE_LFSR_NOISE(NODE_120, TANK8_ATTRACT_EN, TANK8_ATTRACT_EN, 15750.0/4, 1, TANK8_A2_LINE, 0, &tank8_lfsr)

	/************************************************/
	/* Explosion envelope is created by integrating */
	/* a constant voltage                           */
	/************************************************/
	DISCRETE_INTEGRATE(NODE_121, TANK8_EXPLOSION_EN, 0, &tank8_op1_integrate_info)
	DISCRETE_GAIN(NODE_122, NODE_121, RES_K(2.2)/(RES_K(1.0)+RES_K(2.2)))

	/************************************************/
	/* Crash envelope is created by integrating     */
	/* a constant voltage                           */
	/************************************************/
	DISCRETE_INTEGRATE(NODE_123, TANK8_CRASH_EN, 0, &tank8_op2_integrate_info)
	DISCRETE_GAIN(NODE_124, NODE_123, RES_K(1)/(RES_K(1.0)+RES_K(2.2)))

	/************************************************/
	/* Add the envelopes and apply it to the noise  */
	/* generator                                    */
	/************************************************/
	DISCRETE_ADDER2(NODE_125, 1 , NODE_122, NODE_124 )
	DISCRETE_MULTIPLY(NODE_126, NODE_120, NODE_125 )
	DISCRETE_RCFILTER(NODE_127, NODE_126, RES_K(47), CAP_U(0.1))
	DISCRETE_ADJUSTMENT(NODE_128,
				1.0,                // min gain of E5
				1.0 + 100.0/22,     // max gain of E5 = 1 + r132/r101
				DISC_LINADJ, "CRASH")
	DISCRETE_MULTIPLY(NODE_129, NODE_127, NODE_128 )
	DISCRETE_CLAMP(TANK8_CRASHEXPL, NODE_129, -(12.0 - OP_AMP_VP_RAIL_OFFSET)/2, (12.0 - OP_AMP_VP_RAIL_OFFSET)/2)

	/************************************************/
	/* Combine all 10 sound sources.                */
	/************************************************/
	DISCRETE_ADDER4(NODE_130, 1, TANK8_MOTOR1, TANK8_MOTOR2, TANK8_MOTOR3, TANK8_MOTOR4)
	DISCRETE_MULTADD(NODE_131, NODE_130, 1.0, -10)
	DISCRETE_ADDER4(NODE_132, 1, TANK8_MOTOR5, TANK8_MOTOR6, TANK8_MOTOR7, TANK8_MOTOR8)
	DISCRETE_MULTADD(NODE_133, NODE_132, 1.0, -10)
	DISCRETE_ADDER4(TANK8_FINAL_MIX, TANK8_ATTRACT_EN, TANK8_CRASHEXPL, TANK8_BUGLE, NODE_131, NODE_133)

	DISCRETE_OUTPUT(TANK8_FINAL_MIX, 5000)
DISCRETE_SOUND_END
