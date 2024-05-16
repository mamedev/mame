// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    Galaxian-derived sound hardware

****************************************************************************

Notes:
-----

- There is currently no way to exactly reproduce the CD4066 switch control
  mixing. This is changing impedance of the input resistor for e.g.
  following filters to >> 10M Ohm. These resistors are static values in
  the discrete core.


TODO:
----

- Check more schematics for differences.

***************************************************************************/

#include "emu.h"
#include "galaxian_a.h"
#include "galaxian.h"
#include "speaker.h"

/*************************************
 *
 *  Defines
 *
 *************************************/

#define SOUND_CLOCK             (GALAXIAN_MASTER_CLOCK/6/2)          /* 1.536 MHz */
#define RNG_RATE                (GALAXIAN_MASTER_CLOCK/3*2)          /* RNG clock is XTAL/3*2 see Aaron's note in video/galaxian.c */

/* 74LS259 */
#define GAL_INP_BG_DAC          NODE_10     /* at 9M Q4 to Q7 in schematics */

#define GAL_INP_FS1             NODE_20     /* FS1 9L Q0 */
#define GAL_INP_FS2             NODE_21     /* FS2 9L Q1 */
#define GAL_INP_FS3             NODE_22     /* FS3 9L Q2 */
#define GAL_INP_HIT             NODE_23     /* HIT 9L Q3 */
//#define GAL_9L_Q4             NODE_24
#define GAL_INP_FIRE            NODE_25     /* FIRE 9L Q5 */
#define GAL_INP_VOL1            NODE_26     /* VOL1 9L Q6 */
#define GAL_INP_VOL2            NODE_27     /* VOL2 9L Q7 */

#define GAL_INP_PITCH           NODE_28     /* at 6T in schematics */

#define SB_INP_NOISE_3          NODE_30
#define SB_INP_NOISE_2          NODE_31
#define SB_INP_NOISE_1          NODE_32


#define TTL_OUT                 (4.0)

#define GAL_R15                 RES_K(100)
#define GAL_R16                 RES_K(220)
#define GAL_R17                 RES_K(470)
#define GAL_R18                 RES_K(1000)
#define GAL_R19                 RES_K(330)

#define GAL_R20                 RES_K(15)
#define GAL_R21                 RES_K(100)
#define GAL_R22                 RES_K(100)
#define GAL_R23                 RES_K(470)
#define GAL_R24                 RES_K(10)
#define GAL_R25                 RES_K(100)
#define GAL_R26                 RES_K(330)
#define GAL_R27                 RES_K(10)
#define GAL_R28                 RES_K(100)
#define GAL_R29                 RES_K(220)

#define GAL_R30                 RES_K(10)
#define GAL_R31                 RES_K(47)
#define GAL_R32                 RES_K(47)
#define GAL_R33                 RES_K(10)
/*
 * R34 is given twice on galaxian board and both times as 5.1k. On moon cresta
 * it is only listed once and given as 15k. This is more in line with recordings
 */
#define GAL_R34                 RES_K(5.1)
#define MCRST_R34               RES_K(15)

#define GAL_R35                 RES_K(150)
#define GAL_R36                 RES_K(22)
#define GAL_R37                 RES_K(470)
#define GAL_R38                 RES_K(33)
#define GAL_R39                 RES_K(22)

/* The hit sound is too low compared with recordings
 * There may be an issue with the op-amp band filter
 */
#define GAL_R40                 (RES_K(2.2)*0.6)    /* Volume adjust */
#define GAL_R41                 RES_K(100)
#define GAL_R43                 RES_K(2.2)
#define GAL_R44                 RES_K(10)
#define GAL_R45                 RES_K(22)
#define GAL_R46                 RES_K(10)
#define GAL_R47                 RES_K(2.2)
#define GAL_R48                 RES_K(2.2)
#define GAL_R49                 RES_K(10)

#define GAL_R50                 RES_K(22)
#define GAL_R51                 RES_K(33)
#define GAL_R52                 RES_K(15)

#define GAL_R91                 RES_K(10)

#define GAL_C15                 CAP_U(1)
#define GAL_C17                 CAP_U(0.01)
#define GAL_C18                 CAP_U(0.01)
#define GAL_C19                 CAP_U(0.01)

#define GAL_C20                 CAP_U(0.1)
#define GAL_C21                 CAP_U(2.2)
#define GAL_C22                 CAP_U(0.01)
#define GAL_C23                 CAP_U(0.01)
#define GAL_C25                 CAP_U(1)
#define GAL_C26                 CAP_U(0.01)
#define GAL_C27                 CAP_U(0.01)
#define GAL_C28                 CAP_U(47)

#define GAL_C46                 CAP_U(0.1)

/*
 * Hoei Space Battle values
 *
*/
#define SB_VOL_MIX_R1           RES_K(120)
#define SB_VOL_MIX_R2           RES_K(39)
#define SB_VOL_MIX_R3           RES_K(82)
#define SB_VOL_MIX_R4           RES_K(62)

#define SB_R76                  RES_K(33)
#define SB_R77                  RES_K(22)

#define SB_R66                  RES_K(470)
#define SB_R62                  RES_K(150)
#define SB_R36                  RES_K(22)
#define SB_C30                  CAP_U(0.01)
#define SB_C29                  CAP_U(0.01)
#define SB_C22                  CAP_U(2.2)

#define SB_R67                  RES_K(470)
#define SB_R68                  RES_K(100)
#define SB_R61                  RES_K(22)
#define SB_C26                  CAP_U(1.0)
#define SB_C32                  CAP_U(0.01)
#define SB_C33                  CAP_U(0.01)

#define SB_R69                  RES_K(470)
#define SB_R64                  RES_K(100)
#define SB_R58                  RES_K(22)
#define SB_C19                  CAP_U(1.0)
#define SB_C35                  CAP_U(0.01)
#define SB_C34                  CAP_U(0.01)
#define SB_C40                  CAP_U(0.1)
#define SB_C41                  CAP_U(0.1)
#define SB_C43                  CAP_U(0.1)
#define SB_C44                  CAP_U(0.1)

#define SB_R80                  RES_K(4.7)
#define SB_R82                  RES_K(2.2)
#define SB_R83                  RES_K(2.2)
#define SB_R85                  RES_K(2.2)


/*************************************
 *
 *  Structures for discrete core
 *
 *************************************/

static const discrete_dac_r1_ladder galaxian_bck_dac =
{
	4,          // size of ladder
	{GAL_R18, GAL_R17, GAL_R16, GAL_R15, 0,0,0,0},
	4.4,        // 5V - diode junction (0.6V)
	GAL_R20,    // rBIAS
	GAL_R19,    // rGnd
	0           // no C
};

static const discrete_555_cc_desc galaxian_bck_vco =
{
	DISC_555_OUT_DC | DISC_555_OUT_CAP,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES,
	0.7     // Q2 junction voltage
};

static const discrete_555_desc galaxian_555_vco_desc =
{
	DISC_555_OUT_ENERGY | DISC_555_OUT_DC,
	5.0,
	DEFAULT_555_CHARGE,
	(5.0 - 0.5)         // 10k means no real load
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
	17,                     /* Bit Length */
	0,                      /* Reset Value */
	4,                      /* Use Bit 10 (QC of second LS164) as F0 input 0 */
	16,                     /* Use Bit 23 (QH of third LS164) as F0 input 1 */
	DISC_LFSR_XOR_INV_IN1,  /* F0 is XOR */
	DISC_LFSR_IN0,          /* F1 is inverted F0*/
	DISC_LFSR_REPLACE,      /* F2 replaces the shifted register contents */
	0x000001,               /* Everything is shifted into the first bit only */
	DISC_LFSR_FLAG_OUTPUT_F0, /* Output is result of F0 */
	0                       /* Output bit */
};

static const discrete_mixer_desc galaxian_mixerpre_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{GAL_R51, 0, GAL_R50, 0, GAL_R34},      /* A, C, C, D */
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
	{GAL_R34, GAL_R40, GAL_R43},        /* A, C, C, D */
	{0, 0, 0},
	{0,0,GAL_C26},
	0, GAL_R91,
	0,
	GAL_C46,
	0, 1
};

/* moon cresta has different mixing */

static const discrete_mixer_desc mooncrst_mixer_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{GAL_R51, 0, GAL_R50, 0, MCRST_R34, GAL_R40, GAL_R43},      /* A, C, C, D */
	{0, GAL_INP_VOL1, 0, GAL_INP_VOL2, 0, 0, 0},
	{0,0,0,0,0,0,GAL_C26},
	0, 0*GAL_R91,
	0,
	GAL_C46,
	0, 1
};

static const discrete_op_amp_filt_info galaxian_bandpass_desc =
{
	GAL_R35, GAL_R36, 0, 0,
	GAL_R37,
	GAL_C22, GAL_C23, 0,
	5.0*GAL_R39/(GAL_R38+GAL_R39),
	5, 0
};

/* sbhoei mixing */

static const discrete_mixer_desc sbhoei_mixer_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{SB_VOL_MIX_R1, 0, SB_VOL_MIX_R3, 0,SB_R82, SB_R83, SB_R85, SB_R80},        /* A, C, C, D */
	{0, GAL_INP_VOL1, 0, GAL_INP_VOL2,0, 0, 0, 0, },
	{0, 0, 0, 0, SB_C41,SB_C43,SB_C44,SB_C40},
	0, GAL_R91,
	0,
	GAL_C46,
	0, 1
};

static const discrete_op_amp_filt_info sbhoei_bandpass_desc1 =
{
	SB_R62, SB_R36, 0, 0,
	SB_R66,
	SB_C30, SB_C29, 0,
	5.0*SB_R76/(SB_R77+SB_R76),
	5, 0
};

static const discrete_op_amp_filt_info sbhoei_bandpass_desc2 =
{
	SB_R68, SB_R61, 0, 0,
	SB_R67,
	SB_C32, SB_C33, 0,
	5.0*SB_R76/(SB_R77+SB_R76),
	5, 0
};

static const discrete_op_amp_filt_info sbhoei_bandpass_desc3 =
{
	SB_R64, SB_R58, 0, 0,
	SB_R69,
	SB_C35, SB_C34, 0,
	5.0*SB_R76/(SB_R77+SB_R76),
	5, 0
};



/*************************************
 *
 *  Discrete Sound Blocks
 *
 *************************************/

static DISCRETE_SOUND_START(galaxian_discrete)

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
	DISCRETE_INPUT_DATA(GAL_INP_PITCH)

	DISCRETE_TASK_START(0)

		/************************************************/
		/* NOISE                                        */
		/************************************************/

		/* since only a sample of the LFSR is latched @V2 we let the lfsr
		 * run at a lower speed
		 */
		DISCRETE_LFSR_NOISE(NODE_150, 1, 1, RNG_RATE.dvalue()/100, 1.0, 0, 0.5, &galaxian_lfsr)
		DISCRETE_SQUAREWFIX(NODE_151,1,60*264/2,1.0,50,0.5,0)  /* 2V signal */
		DISCRETE_LOGIC_DFLIPFLOP(NODE_152,1,1,NODE_151,NODE_150)
	DISCRETE_TASK_END()

	/* Group Background and pitch */
	DISCRETE_TASK_START(1)

		/************************************************/
		/* Background                                   */
		/************************************************/

		DISCRETE_DAC_R1(NODE_100, GAL_INP_BG_DAC, TTL_OUT, &galaxian_bck_dac)
		DISCRETE_555_CC(NODE_105, 1, NODE_100, GAL_R21, GAL_C15, 0, 0, 0, &galaxian_bck_vco)
		// Next is mult/add opamp circuit
		DISCRETE_MULTADD(NODE_110, NODE_105, GAL_R33/RES_3_PARALLEL(GAL_R31,GAL_R32,GAL_R33),
				-5.0*GAL_R33/GAL_R31)
		DISCRETE_CLAMP(NODE_111,NODE_110,0.0,5.0)
		// The three 555
		DISCRETE_555_ASTABLE_CV(NODE_115, GAL_INP_FS1, GAL_R22, GAL_R23, GAL_C17, NODE_111, &galaxian_555_vco_desc)
		DISCRETE_555_ASTABLE_CV(NODE_116, GAL_INP_FS2, GAL_R25, GAL_R26, GAL_C18, NODE_111, &galaxian_555_vco_desc)
		DISCRETE_555_ASTABLE_CV(NODE_117, GAL_INP_FS3, GAL_R28, GAL_R29, GAL_C19, NODE_111, &galaxian_555_vco_desc)

		DISCRETE_MIXER3(NODE_120, 1, NODE_115, NODE_116, NODE_117, &galaxian_bck_mixer_desc)

		/************************************************/
		/* PITCH                                        */
		/************************************************/

		/* two cascaded LS164 which are reset to pitch latch value,
		 * thus generating SOUND_CLOCK / (256 - pitch_clock) signal
		 *
		 * One possibility to implement this is
		 * DISCRETE_TRANSFORM3(NODE_130, SOUND_CLOCK, 256, GAL_INP_PITCH, "012-/")
		 * DISCRETE_COUNTER(NODE_132, 1, 0, NODE_130, 0, 15, DISC_COUNT_UP, 0, DISC_CLK_IS_FREQ)
		 * but there is a native choice:
		 */
		DISCRETE_NOTE(NODE_132, 1, SOUND_CLOCK.dvalue(), GAL_INP_PITCH, 255, 15,  DISC_CLK_IS_FREQ)

		/* from the 74393 (counter 2 above) only QA, QC, QD are used.
		 * We decode three here and use SUB_NODE(133,x) below to access.
		 */
		DISCRETE_BITS_DECODE(NODE_133, NODE_132, 0, 3, TTL_OUT)     /* QA-QD 74393 */

	/* End of this task */
	DISCRETE_TASK_END()

	DISCRETE_TASK_START(1)

		/************************************************/
		/* HIT                                          */
		/************************************************/

		/* Not 100% correct - switching causes high impedance input for node_157
		 * this is not emulated */
		DISCRETE_RCDISC5(NODE_155, NODE_152, GAL_INP_HIT, (GAL_R35 + GAL_R36), GAL_C21)
		DISCRETE_OP_AMP_FILTER(NODE_157, 1, NODE_155, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &galaxian_bandpass_desc)
	DISCRETE_TASK_END()

	DISCRETE_TASK_START(1)
		/************************************************/
		/* FIRE                                         */
		/************************************************/

		DISCRETE_LOGIC_INVERT(NODE_170, GAL_INP_FIRE)
		DISCRETE_MULTIPLY(NODE_171, TTL_OUT, GAL_INP_FIRE)
		DISCRETE_MULTIPLY(NODE_172, TTL_OUT, NODE_170) // inverted
		DISCRETE_RCFILTER(NODE_173, NODE_172, GAL_R47, GAL_C28)
		/* Mix noise and 163 */
		DISCRETE_TRANSFORM5(NODE_177, NODE_152, TTL_OUT, 1.0/GAL_R46, NODE_173, 1.0/GAL_R48,
				"01*2*34*+" )
		//DISCRETE_MULTIPLY(NODE_174, 1, TTL_OUT, NODE_152)
		//DISCRETE_MULTIPLY(NODE_175, 1, 1.0/GAL_R46, NODE_174)
		//DISCRETE_MULTIPLY(NODE_176, 1, 1.0/GAL_R48, NODE_173)
		//DISCRETE_ADDER2(NODE_177, 1, NODE_175, NODE_176)
		DISCRETE_MULTIPLY(NODE_178, RES_2_PARALLEL(GAL_R46, GAL_R48), NODE_177)

		DISCRETE_555_ASTABLE_CV(NODE_181, 1, GAL_R44, GAL_R45, GAL_C27, NODE_178, &galaxian_555_fire_vco_desc)

		/* 555 toggles discharge on rc discharge module */
		DISCRETE_RCDISC5(NODE_182, NODE_181, NODE_171, (GAL_R41), GAL_C25)

	/* End of task */
	DISCRETE_TASK_END()

	/************************************************/
	/* FINAL MIX                                    */
	/************************************************/

	DISCRETE_TASK_START(2)
		DISCRETE_MIXER5(NODE_279, 1, NODE_133_00, NODE_133_02, NODE_133_02, NODE_133_03, NODE_120, &galaxian_mixerpre_desc)
		DISCRETE_MIXER3(NODE_280, 1, NODE_279, NODE_157, NODE_182, &galaxian_mixer_desc)
		DISCRETE_OUTPUT(NODE_280, 32767.0/5.0*5)
	DISCRETE_TASK_END()

DISCRETE_SOUND_END


static DISCRETE_SOUND_START(mooncrst_discrete)
	DISCRETE_IMPORT(galaxian_discrete)

	/************************************************/
	/* Moon Cresta mixing stage                     */
	/************************************************/
	DISCRETE_DELETE(NODE_279, NODE_279)
	DISCRETE_REPLACE
	DISCRETE_MIXER7(NODE_280, 1, NODE_133_00, NODE_133_02, NODE_133_02,NODE_133_03, NODE_120, NODE_157, NODE_182, &mooncrst_mixer_desc)
DISCRETE_SOUND_END


static DISCRETE_SOUND_START(bongo_discrete)
	DISCRETE_IMPORT(mooncrst_discrete)

	DISCRETE_REPLACE
	DISCRETE_NOTE(NODE_132, 1, (SOUND_CLOCK/8).dvalue(), GAL_INP_PITCH, 255, 15,  DISC_CLK_IS_FREQ)
DISCRETE_SOUND_END


static DISCRETE_SOUND_START(sbhoei_discrete)

	/************************************************/
	/* Input register mapping for hoei space battle */
	/************************************************/

	/* HIT */
	DISCRETE_INPUTX_DATA(SB_INP_NOISE_3, TTL_OUT, 0, 0)
	DISCRETE_INPUTX_DATA(SB_INP_NOISE_2, TTL_OUT, 0, 0)
	DISCRETE_INPUTX_DATA(SB_INP_NOISE_1, TTL_OUT, 0, 0)

	/* FIRE */
	DISCRETE_INPUT_LOGIC(GAL_INP_FIRE)

	/* PITCH */
	DISCRETE_INPUT_DATA(GAL_INP_PITCH)

	/* Turns on / off resistors in mixer */
	DISCRETE_INPUTX_DATA(GAL_INP_VOL1, SB_VOL_MIX_R2, 0, 0)
	DISCRETE_INPUTX_DATA(GAL_INP_VOL2, SB_VOL_MIX_R4, 0, 0)

	DISCRETE_TASK_START(0)

		/************************************************/
		/* NOISE                                        */
		/************************************************/

		/* since only a sample of the LFSR is latched @V2 we let the lfsr
		 * run at a lower speed
		 */
		DISCRETE_LFSR_NOISE(NODE_150, 1, 1, RNG_RATE.dvalue()/100, 1.0, 0, 0.5, &galaxian_lfsr)
		DISCRETE_SQUAREWFIX(NODE_151,1,(60*264)/2,1.0,50,0.5,0)  /* 2V signal */
		DISCRETE_LOGIC_DFLIPFLOP(NODE_152,1,1,NODE_151,NODE_150)

		DISCRETE_SQUAREWFIX(NODE_200,1,(60*264*384)/128,1.0,50,0.5,0)  /* 128H signal */
		DISCRETE_LOGIC_DFLIPFLOP(NODE_201,1,1,NODE_200,NODE_150)

		DISCRETE_SQUAREWFIX(NODE_206,1,(60*264)/8,1.0,50,0.5,0)  /* 4v+8v signal */
		DISCRETE_LOGIC_DFLIPFLOP(NODE_205,1,1,NODE_206,NODE_150)
	DISCRETE_TASK_END()

	/* pitch */
	DISCRETE_TASK_START(1)

		/************************************************/
		/* PITCH                                        */
		/************************************************/

		/* two cascaded LS164 which are reset to pitch latch value,
		 * thus generating SOUND_CLOCK / (256 - pitch_clock) signal
		 *
		 * One possibility to implement this is
		 * DISCRETE_TRANSFORM3(NODE_130, SOUND_CLOCK, 256, GAL_INP_PITCH, "012-/")
		 * DISCRETE_COUNTER(NODE_132, 1, 0, NODE_130, 0, 15, DISC_COUNT_UP, 0, DISC_CLK_IS_FREQ)
		 * but there is a native choice:
		 */
		DISCRETE_NOTE(NODE_132, 1, SOUND_CLOCK.dvalue(), GAL_INP_PITCH, 255, 15,  DISC_CLK_IS_FREQ)

		/* from the 74393 (counter 2 above) only QA, QC, QD are used.
		 * We decode three here and use SUB_NODE(133,x) below to access.
		 */
		DISCRETE_BITS_DECODE(NODE_133, NODE_132, 0, 3, TTL_OUT)     /* QA-QD 74393 */

	/* End of this task */
	DISCRETE_TASK_END()

	DISCRETE_TASK_START(1)

		/* NOISE 3
		 * Not 100% correct - switching causes high impedance input for node_157
		 * this is not emulated */
		DISCRETE_RCDISC5(NODE_155, NODE_152, SB_INP_NOISE_3, (SB_R62 + SB_R36), SB_C22)
		DISCRETE_OP_AMP_FILTER(NODE_157, 1, NODE_155, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &sbhoei_bandpass_desc1)

		/* NOISE 2
		 * Not 100% correct - switching causes high impedance input for node_157
		 * this is not emulated */
		DISCRETE_RCDISC5(NODE_202, NODE_205, SB_INP_NOISE_2, (SB_R64 + SB_R58), SB_C19)
		DISCRETE_OP_AMP_FILTER(NODE_204, 1, NODE_202, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &sbhoei_bandpass_desc3)

		/* NOISE 1
		 * Not 100% correct - switching causes high impedance input for node_157
		 * this is not emulated */
		DISCRETE_RCDISC5(NODE_207, NODE_201, SB_INP_NOISE_1, (SB_R68 + SB_R61), SB_C26)
		DISCRETE_OP_AMP_FILTER(NODE_203, 1, NODE_207, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &sbhoei_bandpass_desc2)

	DISCRETE_TASK_END()

	DISCRETE_TASK_START(1)
		/************************************************/
		/* FIRE                                         */
		/************************************************/

		DISCRETE_LOGIC_INVERT(NODE_170, GAL_INP_FIRE)
		DISCRETE_MULTIPLY(NODE_171, TTL_OUT, GAL_INP_FIRE)
		DISCRETE_MULTIPLY(NODE_172, TTL_OUT, NODE_170) // inverted
		DISCRETE_RCFILTER(NODE_173, NODE_172, GAL_R47, GAL_C28)
		/* Mix noise and 163 */
		DISCRETE_TRANSFORM5(NODE_177, NODE_152, TTL_OUT, 1.0/GAL_R46, NODE_173, 1.0/GAL_R48,
				"01*2*34*+" )
		//DISCRETE_MULTIPLY(NODE_174, 1, TTL_OUT, NODE_152)
		//DISCRETE_MULTIPLY(NODE_175, 1, 1.0/GAL_R46, NODE_174)
		//DISCRETE_MULTIPLY(NODE_176, 1, 1.0/GAL_R48, NODE_173)
		//DISCRETE_ADDER2(NODE_177, 1, NODE_175, NODE_176)
		DISCRETE_MULTIPLY(NODE_178, RES_2_PARALLEL(GAL_R46, GAL_R48), NODE_177)

		DISCRETE_555_ASTABLE_CV(NODE_181, 1, GAL_R44, GAL_R45, GAL_C27, NODE_178, &galaxian_555_fire_vco_desc)

		/* 555 toggles discharge on rc discharge module */
		DISCRETE_RCDISC5(NODE_182, NODE_181, NODE_171, (GAL_R41), GAL_C25)

	/* End of task */
	DISCRETE_TASK_END()

	/************************************************/
	/* FINAL MIX                                    */
	/************************************************/

	DISCRETE_TASK_START(2)
		DISCRETE_MIXER8(NODE_280, 1, NODE_133_00, NODE_133_02, NODE_133_02, NODE_133_03,NODE_157, NODE_203, NODE_204, NODE_182, &sbhoei_mixer_desc)
		DISCRETE_OUTPUT(NODE_280, 32767.0/5.0*5)
	DISCRETE_TASK_END()

DISCRETE_SOUND_END


DEFINE_DEVICE_TYPE(GALAXIAN_SOUND, galaxian_sound_device, "galaxian_sound", "Galaxian Custom Sound")
DEFINE_DEVICE_TYPE(MOONCRST_SOUND, mooncrst_sound_device, "mooncrst_sound", "Moon Cresta Custom Sound")
DEFINE_DEVICE_TYPE(BONGO_SOUND, bongo_sound_device, "bongo_sound", "Bongo Custom Sound")
DEFINE_DEVICE_TYPE(SBHOEI_SOUND, sbhoei_sound_device, "sbhoei_sound", "Space Battle Custom Sound")

galaxian_sound_device::galaxian_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: galaxian_sound_device(mconfig, GALAXIAN_SOUND, tag, owner, clock)
{
}

galaxian_sound_device::galaxian_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_discrete(*this, "discrete")
{
}

mooncrst_sound_device::mooncrst_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: galaxian_sound_device(mconfig, MOONCRST_SOUND, tag, owner, clock)
{
}

bongo_sound_device::bongo_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: galaxian_sound_device(mconfig, BONGO_SOUND, tag, owner, clock)
{
}

sbhoei_sound_device::sbhoei_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: galaxian_sound_device(mconfig, SBHOEI_SOUND, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void galaxian_sound_device::device_start()
{
	m_lfo_val = 0;

	save_item(NAME(m_lfo_val));
}

void sbhoei_sound_device::device_start()
{
}


//-------------------------------------------------
//  machine_add_config - add device configuration
//-------------------------------------------------

void galaxian_sound_device::device_add_mconfig(machine_config &config)
{
	DISCRETE(config, m_discrete).add_route(ALL_OUTPUTS, ":speaker", 1.0);
	m_discrete->set_intf(galaxian_discrete);
}

void mooncrst_sound_device::device_add_mconfig(machine_config &config)
{
	galaxian_sound_device::device_add_mconfig(config);
	m_discrete->set_intf(mooncrst_discrete);
}

void bongo_sound_device::device_add_mconfig(machine_config &config)
{
	galaxian_sound_device::device_add_mconfig(config);
	m_discrete->set_intf(bongo_discrete);
}

void sbhoei_sound_device::device_add_mconfig(machine_config &config)
{
	galaxian_sound_device::device_add_mconfig(config);
	m_discrete->set_intf(sbhoei_discrete);
}


/*************************************
 *
 *  Write handlers
 *
 *************************************/

/* IC 9J */
void galaxian_sound_device::pitch_w(uint8_t data)
{
	m_discrete->write(GAL_INP_PITCH, data);
}

void galaxian_sound_device::lfo_freq_w(offs_t offset, uint8_t data)
{
	uint8_t lfo_val_new = (m_lfo_val & ~(1<<offset)) | ((data & 0x01) << offset);

	if (m_lfo_val != lfo_val_new)
	{
		m_lfo_val = lfo_val_new;
		m_discrete->write(GAL_INP_BG_DAC, m_lfo_val);
	}
}

void galaxian_sound_device::background_enable_w(offs_t offset, uint8_t data)
{
	m_discrete->write(NODE_RELATIVE(GAL_INP_FS1, offset), data & 0x01);
}

void galaxian_sound_device::noise_enable_w(uint8_t data)
{
	m_discrete->write(GAL_INP_HIT, data & 0x01);
}

void galaxian_sound_device::vol_w(offs_t offset, uint8_t data)
{
	m_discrete->write(NODE_RELATIVE(GAL_INP_VOL1, offset), data & 0x01);
}

void galaxian_sound_device::fire_enable_w(uint8_t data)
{
	m_discrete->write(GAL_INP_FIRE, data & 0x01);
}

/* FIXME: May be replaced by one call! */
void galaxian_sound_device::sound_w(offs_t offset, uint8_t data)
{
	data &= 0x01;
	switch (offset & 7)
	{
		case 0:     /* FS1 (controls 555 timer at 8R) */
		case 1:     /* FS2 (controls 555 timer at 8S) */
		case 2:     /* FS3 (controls 555 timer at 8T) */
			background_enable_w(offset, data);
			break;

		case 3:     /* HIT */
			noise_enable_w(data);
			break;

		case 4:     /* n/c */
			break;

		case 5:     /* FIRE */
			fire_enable_w(data);
			break;

		case 6:     /* VOL1 */
		case 7:     /* VOL2 */
			vol_w(offset & 1, data);
			break;
	}
}


void sbhoei_sound_device::noise1_enable_w(offs_t offset, uint8_t data)
{
	m_discrete->write(SB_INP_NOISE_1, data & 0x01);
}

void sbhoei_sound_device::noise2_enable_w(offs_t offset, uint8_t data)
{
	m_discrete->write(SB_INP_NOISE_2, data & 0x01);
}

void sbhoei_sound_device::noise3_enable_w(offs_t offset, uint8_t data)
{
	m_discrete->write(SB_INP_NOISE_3, data & 0x01);
}
