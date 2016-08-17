// license:BSD-3-Clause
// copyright-holders:Derrick Renaud
/************************************************************************
 * starshp1 Sound System Analog emulation
 * Aug 2008, Derrick Renaud
 ************************************************************************/

#include "emu.h"
#include "includes/starshp1.h"
#include "sound/discrete.h"


/* voltage out for each 1/256 step of MC1408 circuit */
#define MC1408_DAC(v_ref, r, rf)        ((double)v_ref / (double)r * (double)rf / 256.0)

#define OP_AMP_NON_INVERT_GAIN(ri, rf)  ((double)rf / (double)ri + 1)

/* Discrete Sound Input Nodes */
/* see "starshp1.h" */

/* Nodes - Sounds */
#define STARSHP1_PHASOR_SND     NODE_13
#define STARSHP1_NOISE_SND      NODE_14
#define STARSHP1_MOTOR_SND      NODE_15
#define STARSHP1_SL1_SND        NODE_16
#define STARSHP1_SL2_SND        NODE_17

/* Timing signals */
#define STARSHP1_HSYNC  15750   /* per manual */
#define STARSHP1_S1V    STARSHP1_HSYNC / 2
#define STARSHP1_S128V  STARSHP1_S1V / 128

/* R/C values */
#define STARSHP1_R4     RES_K(1)
#define STARSHP1_R54    RES_K(47)
#define STARSHP1_R55    RES_K(22)
#define STARSHP1_R58    RES_K(1)
#define STARSHP1_R59    RES_K(1)
#define STARSHP1_R60    RES_K(6.8)
#define STARSHP1_R61    RES_K(10)
#define STARSHP1_R62    750
#define STARSHP1_R63    RES_K(22)
#define STARSHP1_R64    RES_K(10)
#define STARSHP1_R65    RES_K(3.3)
#define STARSHP1_R66    RES_K(33)
#define STARSHP1_R67    RES_K(1)
#define STARSHP1_R68    RES_K(3.3)
#define STARSHP1_R69    RES_K(330)
#define STARSHP1_R70    RES_K(10)
#define STARSHP1_R71    RES_K(6.8)
#define STARSHP1_R72    RES_K(47)
#define STARSHP1_R74    RES_K(100)
#define STARSHP1_R75    RES_K(47)
#define STARSHP1_R76    RES_K(5)
#define STARSHP1_R123   RES_K(1)
#define STARSHP1_R124   RES_K(1)
#define STARSHP1_R126   RES_K(1)
#define STARSHP1_R151   RES_K(10)
#define STARSHP1_R152   RES_K(1.5)
#define STARSHP1_C34    CAP_U(0.1)
#define STARSHP1_C36    CAP_U(2.2)
#define STARSHP1_C45    CAP_U(0.1)
#define STARSHP1_C46    CAP_U(0.1)
#define STARSHP1_C47    CAP_U(0.015)
#define STARSHP1_C59    CAP_U(0.22)
#define STARSHP1_C61    CAP_U(0.01)
#define STARSHP1_C63    CAP_U(0.1)
#define STARSHP1_C78    CAP_U(0.1)

/* Convert DAC value to voltage out at IC E10, pin 4 */
#define STARSHP1_MC1408_GAIN        MC1408_DAC(5.0, STARSHP1_R4, STARSHP1_R62)
/* Gain at IC E10, pin 12 */
#define STARSHP1_TONE_PITCH_GAIN    OP_AMP_NON_INVERT_GAIN(STARSHP1_R63, STARSHP1_R64)

#define STARSHP1_NOISE_AMPLITUDE_GAIN   RES_VOLTAGE_DIVIDER(STARSHP1_R124, STARSHP1_R123)

/* tweak to apply compensation filter at pin 6 */
#define STARSHP1_FAKE_MC3340_COMPENSATION_R     RES_K(6.2)

/* Discrete info structures */

static const discrete_lfsr_desc starshp1_lfsr={
	DISC_CLK_IS_FREQ,
	16,                 /* Bit Length */
	0,                  /* Reset Value */
	13,                 /* Use Bit 0 as XOR input 0 */
	14,                 /* Use Bit 14 as XOR input 1 */
	DISC_LFSR_XNOR,     /* Feedback stage1 is XNOR */
	DISC_LFSR_OR,       /* Feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,  /* Feedback stage3 replaces the shifted register contents */
	0x000001,           /* Everything is shifted into the first bit only */
	0,                  /* Output is not inverted */
	15                  /* Output bit */
};

static const discrete_555_desc starshp1_556_c10 =
{
	DISC_555_OUT_ENERGY,
	5,      /* B+ voltage of 555 */
	DEFAULT_555_VALUES
};

/* effect of 556 internal CV resistors at pin 3 */
static const discrete_mixer_desc starshp1_556_c10_cv =
{
	DISC_MIXER_IS_RESISTOR,
	{STARSHP1_R61, RES_K(5)},
	{0}, {0}, 0, RES_K(10), 0, 0, 0, 1
};

static const discrete_mixer_desc starshp1_566_a9_mix_r =
{
	DISC_MIXER_IS_RESISTOR,
	{STARSHP1_R54, STARSHP1_R55},
	{0}, {0}, 0, 0, 0, 0, 0, 1
};

static const discrete_mixer_desc starshp1_555_b10_mix_r =
{
	DISC_MIXER_IS_RESISTOR,
	{STARSHP1_R65, STARSHP1_R68},
	{0}, {0}, 0, 0, 0, 0, 0, 1
};

static const discrete_555_desc starshp1_555_b10 =
{
	DISC_555_OUT_ENERGY,
	5,                      /* B+ voltage of 555 */
	NODE_50,                /* charge voltage */
	DEFAULT_TTL_V_LOGIC_1   /* cheat and use and anti-alias IC A10 level */
};

static const discrete_mixer_desc starshp1_final_mix =
{
	DISC_MIXER_IS_RESISTOR,
	{STARSHP1_R66, STARSHP1_R126, STARSHP1_R59 + STARSHP1_R60, STARSHP1_R74, STARSHP1_R75},
	{0},    /* no resistor nodes */
	{0, STARSHP1_C78, 0, 0, 0},
	0,
	STARSHP1_R76,
	0,      /* no cF */
	STARSHP1_C63,
	0,      /* vRef */
	1       /* gain */
};


DISCRETE_SOUND_START( starshp1 )
	/************************************************
	 * Input register mapping
	 ************************************************/
//  DISCRETE_INPUTX_DATA(STARSHP1_NOISE_AMPLITUDE, STARSHP1_MC1408_GAIN * STARSHP1_NOISE_AMPLITUDE_GAIN, 0, 0)
	/* Fake the MC1408, no where near correct */
	DISCRETE_INPUTX_DATA(STARSHP1_NOISE_AMPLITUDE, -4.0/255, 4, 0)
	DISCRETE_INPUTX_DATA(STARSHP1_TONE_PITCH,      STARSHP1_MC1408_GAIN * STARSHP1_TONE_PITCH_GAIN, 0, 0)
	DISCRETE_INPUTX_DATA(STARSHP1_MOTOR_SPEED,     STARSHP1_MC1408_GAIN, 0, 0)
	DISCRETE_INPUT_LOGIC(STARSHP1_NOISE_FREQ)
	DISCRETE_INPUT_LOGIC(STARSHP1_SL2)
	DISCRETE_INPUT_LOGIC(STARSHP1_SL1)
	DISCRETE_INPUT_LOGIC(STARSHP1_KICKER)
	DISCRETE_INPUT_LOGIC(STARSHP1_PHASOR_ON)
	DISCRETE_INPUT_NOT  (STARSHP1_MOLVL)
	DISCRETE_INPUT_NOT  (STARSHP1_ATTRACT)      /* Inverted by Q9 */

	/************************************************
	 * Noise
	 ************************************************/
	DISCRETE_LFSR_NOISE(NODE_20,                    /* IC B9, pin 13 */
						1,                          /* ENAB */
						STARSHP1_KICKER,            /* IC B9 & C9, pin 9 */
						STARSHP1_S1V,               /* IC B9 & C9, pin 8 */
						1,                          /* 1 p/p */
						0,                          /* no feed */
						1.0 / 2,                    /* shift to 0/1 */
						&starshp1_lfsr)
	/* calculate noise level */
	DISCRETE_SWITCH(    NODE_21,                    /* voltage at R151 & R152 junction */
						NODE_20,
						STARSHP1_NOISE_FREQ,
						DEFAULT_TTL_V_LOGIC_1 * RES_VOLTAGE_DIVIDER(STARSHP1_R151, STARSHP1_R152),
						DEFAULT_TTL_V_LOGIC_1)
	/* fake MC3340 */
	DISCRETE_MULTIPLY(  NODE_22,
						NODE_21,
						STARSHP1_NOISE_AMPLITUDE)
	DISCRETE_RCFILTER(  NODE_23,
						NODE_22,
						STARSHP1_FAKE_MC3340_COMPENSATION_R,
						STARSHP1_C61)
	/* when NOISE_FREQ is high (NOISE_FREQ_INV is low), C59 is in circuit */
	DISCRETE_RCFILTER(  NODE_24,
						NODE_22,
						STARSHP1_FAKE_MC3340_COMPENSATION_R,
						STARSHP1_C61 + STARSHP1_C59)
	DISCRETE_SWITCH(    STARSHP1_NOISE_SND,
						1,                          /* ENAB */
						STARSHP1_NOISE_FREQ,
						NODE_23,
						NODE_24)

	/************************************************
	 * Phasor
	 ************************************************/
	DISCRETE_555_ASTABLE(NODE_30,                   /* IC C10, pin 9 */
						STARSHP1_PHASOR_ON,         /* RESET */
						STARSHP1_R70,
						STARSHP1_R69,
						STARSHP1_C46,
						&starshp1_556_c10)
	/* compensate for internal 555 CV resistors */
	DISCRETE_MIXER2(    NODE_31,
						1,                          /* ENAB */
						NODE_30,                    /* IC C10, pin 9 */
						5,                          /* 555 B+ */
						&starshp1_556_c10_cv)
	DISCRETE_555_ASTABLE_CV(STARSHP1_PHASOR_SND,    /* IC C10, pin 5 */
						STARSHP1_PHASOR_ON,         /* RESET */
						STARSHP1_R68,
						STARSHP1_R67,
						STARSHP1_C45,
						NODE_31,                    /* IC C10, pin 3 */
						&starshp1_556_c10)

	/************************************************
	 * Motor
	 ************************************************/
	DISCRETE_SQUAREWFIX(NODE_40,                    /* S128V signal */
						1,                          /* ENAB */
						STARSHP1_S128V,
						DEFAULT_TTL_V_LOGIC_1,
						50,                         /* DUTY */
						DEFAULT_TTL_V_LOGIC_1 / 2,  /* BIAS */
						0)                          /* PHASE */
	DISCRETE_MIXER2(    NODE_41,                    /* voltage at R54 & R55 junction */
						1,                          /* ENAB */
						NODE_40,                    /* IC E10, pin 12 */
						5,                          /* 5V to R55 */
						&starshp1_566_a9_mix_r)
	DISCRETE_566(       NODE_42,                    /* IC A9, pin 4 */
						STARSHP1_MOTOR_SPEED,       /* IC A9, pin 5 */
						RES_2_PARALLEL(STARSHP1_R54, STARSHP1_R55),
						STARSHP1_C34,
						5, -10, NODE_41,            /* VPOS,VNEG,VCHARGE */
						DISC_566_OUT_DC | DISC_566_OUT_TRIANGLE)
	DISCRETE_SWITCH(NODE_43,
						1,                          /* ENAB */
						STARSHP1_MOLVL,             /* SWITCH */
						STARSHP1_R59 + RES_2_PARALLEL(STARSHP1_R58, STARSHP1_R60 + STARSHP1_R76),
						STARSHP1_R59 + STARSHP1_R60 + STARSHP1_R76)
	DISCRETE_CRFILTER(NODE_44,
						NODE_42,                    /* IN0 */
						NODE_43,                    /* RVAL*/
						STARSHP1_C36)
	DISCRETE_SWITCH(NODE_45,                        /* gain affect of voltage divider */
						1,                          /* ENAB */
						STARSHP1_MOLVL,             /* SWITCH */
						RES_VOLTAGE_DIVIDER(STARSHP1_R59, STARSHP1_R58), 1)
	DISCRETE_MULTIPLY(  STARSHP1_MOTOR_SND,
						NODE_44,
						NODE_45)

	/************************************************
	 * SL1, SL2
	 ************************************************/
	DISCRETE_MIXER2(    NODE_50,                    /* voltage at R65 & R71 junction */
						1,                          /* ENAB */
						STARSHP1_TONE_PITCH,        /* IC E10, pin 12 */
						10,                         /* 10V to R71 */
						&starshp1_555_b10_mix_r)
	DISCRETE_555_ASTABLE(NODE_51,                   /* IC B10, pin 3 */
						1,                          /* RESET */
						RES_2_PARALLEL(STARSHP1_R65, STARSHP1_R71),
						STARSHP1_R72,
						STARSHP1_C47,
						&starshp1_555_b10)
	/* use switch instead of logic AND, so we can switch between
	 * 0V and the anti-aliased TTL level out that we set NODE_51 to. */
	DISCRETE_ONOFF(     STARSHP1_SL1_SND,           /* IC A10, pin 11 */
						STARSHP1_SL1, NODE_51)
	DISCRETE_ONOFF(     STARSHP1_SL2_SND,           /* IC A10, pin 8 */
						STARSHP1_SL2, NODE_51)

	/************************************************
	 * Mixing and output
	 ************************************************/
	DISCRETE_MIXER5(    NODE_90,
						STARSHP1_ATTRACT,           /* ENAB */
						STARSHP1_PHASOR_SND,
						STARSHP1_NOISE_SND,
						STARSHP1_MOTOR_SND,
						STARSHP1_SL1_SND,
						STARSHP1_SL2_SND,
						&starshp1_final_mix)

	/* set a gain that clips the noise some, just like the real board */
	DISCRETE_OUTPUT(NODE_90, 110000)

DISCRETE_SOUND_END
