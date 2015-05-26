// license:BSD-3-Clause
// copyright-holders:Derrick Renaud
/***************************************************************************
    galaga.c
    Sound handler
****************************************************************************/

#include "emu.h"
#include "namco52.h"
#include "namco54.h"
#include "includes/galaga.h"


/*************************************
 *
 *  Bosconian
 *
 *  Discrete sound emulation: Feb 2007, D.R.
 *
 *************************************/

/* nodes - sounds */
#define BOSCO_CHANL1_SND        NODE_11
#define BOSCO_CHANL2_SND        NODE_12
#define BOSCO_CHANL3_SND        NODE_13
#define BOSCO_CHANL4_SND        NODE_14

#define BOSCO_54XX_DAC_R (1.0 / (1.0 / RES_K(47) + 1.0 / RES_K(22) + 1.0 / RES_K(10) + 1.0 / RES_K(4.7)))
static const discrete_dac_r1_ladder bosco_54xx_dac =
{
	4,              /* number of DAC bits */
					/* 54XX_0   54XX_1  54XX_2 */
	{ RES_K(47),    /* R28,     R32,    R46 */
		RES_K(22),  /* R27,     R31,    R45 */
		RES_K(10),  /* R26,     R30,    R44 */
		RES_K(4.7)},    /* R25,     R29,    R43 */
	0, 0, 0, 0      /* nothing extra */
};

#define BOSCO_52XX_DAC_R (1.0 / (1.0 / RES_K(100) + 1.0 / RES_K(47) + 1.0 / RES_K(22) + 1.0 / RES_K(10)))
static const discrete_dac_r1_ladder bosco_52xx_dac =
{
	4,              /* number of DAC bits */
	{ RES_K(100),   /* R14 */
		RES_K(47),  /* R13 */
		RES_K(22),  /* R12 */
		RES_K(10)}, /* R11 */
	0, 0, 0, 0      /* nothing extra */
};

/*                         R39           R38          R39 */
#define BOSCO_VREF (5.0 * (RES_K(2.2) / (RES_K(3.3) + RES_K(2.2))))

static const discrete_op_amp_filt_info bosco_chanl1_filt =
{
	BOSCO_54XX_DAC_R + RES_K(100),  /* R24 */
	0,                  /* no second input */
	RES_K(22),          /* R23 */
	0,                  /* not used */
	RES_K(220),         /* R22 */
	CAP_U(0.001),       /* C31 */
	CAP_U(0.001),       /* C30 */
	0,                  /* not used */
	BOSCO_VREF,         /* vRef */
	5,                  /* vP */
	0                   /* vN */
};

static const discrete_op_amp_filt_info bosco_chanl2_filt =
{
	BOSCO_54XX_DAC_R + RES_K(47),   /* R33 */
	0,                  /* no second input */
	RES_K(10),          /* R34 */
	0,                  /* not used */
	RES_K(150),         /* R35 */
	CAP_U(0.01),        /* C29 */
	CAP_U(0.01),        /* C28 */
	0,                  /* not used */
	BOSCO_VREF,         /* vRef */
	5,                  /* vP */
	0                   /* vN */
};

static const discrete_op_amp_filt_info bosco_chanl3_filt =
{
	BOSCO_54XX_DAC_R + RES_K(150),  /* R42 */
	0,                  /* no second input */
	RES_K(22),          /* R41 */
	0,                  /* not used */
	RES_K(470),         /* R40 */
	CAP_U(0.01),        /* C27 */
	CAP_U(0.01),        /* C26 */
	0,                  /* not used */
	BOSCO_VREF,         /* vRef */
	5,                  /* vP */
	0                   /* vN */
};

static const discrete_mixer_desc bosco_filter_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{ RES_K(33),    /* R21 */
		RES_K(33),  /* R36 */
		RES_K(10)}, /* R37 */
	{0},            /* no rNode{} */
	{0},            /* no c{} */
	0,              /* no rI */
	RES_K(3.3),     /* R20 */
	0,              /* no cF */
	0,              /* no cAmp */
	BOSCO_VREF,     /* vRef */
	1,              /* gain */
};

static const discrete_mixer_desc bosco_final_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{ RES_K(10),    /* R51 */
		RES_K(10)}, /* R55 */
	{0},            /* no rNode{} */
	{0},            /* no c{} */
	0,              /* no rI */
	RES_K(1),       /* VR1 */
	0,              /* no cF */
	CAP_U(0.1),     /* C18 */
	0,              /* vRef */
	462000          /* gain */
};

DISCRETE_SOUND_START(bosco)

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
					&bosco_54xx_dac)
	DISCRETE_OP_AMP_FILTER(BOSCO_CHANL1_SND,
					1,          /* ENAB */
					NODE_20,    /* INP0 */
					0,          /* INP1 - not used */
					DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &bosco_chanl1_filt)

	/************************************************
	 * CHANL2 sound
	 ************************************************/
	DISCRETE_DAC_R1(NODE_30,
					NAMCO_54XX_1_DATA(NODE_01),
					4,          /* 4V - unmeasured*/
					&bosco_54xx_dac)
	DISCRETE_OP_AMP_FILTER(BOSCO_CHANL2_SND,
					1,          /* ENAB */
					NODE_30,    /* INP0 */
					0,          /* INP1 - not used */
					DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &bosco_chanl2_filt)

	/************************************************
	 * CHANL3 sound
	 ************************************************/
	DISCRETE_DAC_R1(NODE_40,
					NAMCO_54XX_0_DATA(NODE_01),
					4,          /* 4V - unmeasured*/
					&bosco_54xx_dac)
	DISCRETE_OP_AMP_FILTER(BOSCO_CHANL3_SND,
					1,          /* ENAB */
					NODE_40,    /* INP0 */
					0,          /* INP1 - not used */
					DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &bosco_chanl3_filt)

	/************************************************
	 * CHANL4 sound
	 ************************************************/
	/* this circuit was simulated in SPICE and an equivalent filter circuit generated */
	DISCRETE_DAC_R1(NODE_50,
					NAMCO_52XX_P_DATA(NODE_04),
					4,          /* 4V - unmeasured*/
					&bosco_52xx_dac)
	DISCRETE_FILTER2(NODE_51,
					1,          /* ENAB */
					NODE_50,    /* INP0 */
					80,         /* FREQ */
					1.0 / 0.3,  /* DAMP */
					DISC_FILTER_HIGHPASS)
	DISCRETE_FILTER2(NODE_52,
					1,          /* ENAB */
					NODE_51,    /* INP0 */
					2400,       /* FREQ */
					1.0 / 0.9,  /* DAMP */
					DISC_FILTER_LOWPASS)
	DISCRETE_GAIN(BOSCO_CHANL4_SND,
					NODE_52,    /* IN0 */
					0.25        /* overall filter GAIN */)

	/************************************************
	 * Output
	 ************************************************/
	DISCRETE_MIXER3(NODE_90,
					1,                  /* ENAB */
					BOSCO_CHANL1_SND,   /* IN0 */
					BOSCO_CHANL2_SND,   /* IN1 */
					BOSCO_CHANL3_SND,   /* IN2 */
					&bosco_filter_mixer /* INFO */)
	DISCRETE_MIXER2(NODE_91,
					1,                  /* ENAB */
					NODE_90,            /* IN0 */
					BOSCO_CHANL4_SND,   /* IN1 */
					&bosco_final_mixer  /* INFO */)
	DISCRETE_OUTPUT(NODE_91, 1)
DISCRETE_SOUND_END



/*************************************
 *
 *  Galaga/Xevious
 *
 *  Discrete sound emulation: Feb 2007, D.R.
 *
 *************************************/

/* nodes - sounds */
#define GALAGA_CHANL1_SND       NODE_11
#define GALAGA_CHANL2_SND       NODE_12
#define GALAGA_CHANL3_SND       NODE_13


#define GALAGA_54XX_DAC_R (1.0 / (1.0 / RES_K(47) + 1.0 / RES_K(22) + 1.0 / RES_K(10) + 1.0 / RES_K(4.7)))
static const discrete_dac_r1_ladder galaga_54xx_dac =
{
	4,              /* number of DAC bits */
	{ RES_K(47),
		RES_K(22),
		RES_K(10),
		RES_K(4.7)},
	0, 0, 0, 0      /* nothing extra */
};

#define GALAGA_VREF (5.0 * (RES_K(2.2) / (RES_K(3.3) + RES_K(2.2))))

static const discrete_op_amp_filt_info galaga_chanl1_filt =
{
	GALAGA_54XX_DAC_R + RES_K(100),
	0,                  /* no second input */
	RES_K(22),
	0,                  /* not used */
	RES_K(220),
	CAP_U(0.001),
	CAP_U(0.001),
	0,                  /* not used */
	GALAGA_VREF,        /* vRef */
	5,                  /* vP */
	0,                  /* vN */
};

static const discrete_op_amp_filt_info galaga_chanl2_filt =
{
	GALAGA_54XX_DAC_R + RES_K(47),
	0,                  /* no second input */
	RES_K(10),
	0,                  /* not used */
	RES_K(150),
	CAP_U(0.01),
	CAP_U(0.01),
	0,                  /* not used */
	GALAGA_VREF,        /* vRef */
	5,                  /* vP */
	0,                  /* vN */
};

static const discrete_op_amp_filt_info galaga_chanl3_filt =
{
	GALAGA_54XX_DAC_R + RES_K(150),
	0,                  /* no second input */
	RES_K(22),
	0,                  /* not used */
	RES_K(470),
	CAP_U(0.01),
	CAP_U(0.01),
	0,                  /* not used */
	GALAGA_VREF,        /* vRef */
	5,                  /* vP */
	0,                  /* vN */
};

static const discrete_mixer_desc galaga_final_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{ RES_K(33),
		RES_K(33),
		RES_K(10)},
	{0},            /* no rNode{} */
	{0},            /* no c{} */
	0,              /* no rI */
	RES_K(3.3),
	0,              /* no cF */
	CAP_U(0.1),
	GALAGA_VREF,    /* vRef */
	40800,          /* gain */
};

DISCRETE_SOUND_START(galaga)

	/************************************************
	 * Input register mapping
	 ************************************************/
	DISCRETE_INPUT_DATA(NAMCO_54XX_0_DATA(NODE_01))
	DISCRETE_INPUT_DATA(NAMCO_54XX_1_DATA(NODE_01))
	DISCRETE_INPUT_DATA(NAMCO_54XX_2_DATA(NODE_01))

	/************************************************
	 * CHANL1 sound
	 ************************************************/
	DISCRETE_DAC_R1(NODE_20,
					NAMCO_54XX_2_DATA(NODE_01),
					4,          /* 4V - unmeasured*/
					&galaga_54xx_dac)
	DISCRETE_OP_AMP_FILTER(GALAGA_CHANL1_SND,
					1,          /* ENAB */
					NODE_20,    /* INP0 */
					0,          /* INP1 - not used */
					DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &galaga_chanl1_filt)

	/************************************************
	 * CHANL2 sound
	 ************************************************/
	DISCRETE_DAC_R1(NODE_30,
					NAMCO_54XX_1_DATA(NODE_01),
					4,          /* 4V - unmeasured*/
					&galaga_54xx_dac)
	DISCRETE_OP_AMP_FILTER(GALAGA_CHANL2_SND,
					1,          /* ENAB */
					NODE_30,    /* INP0 */
					0,          /* INP1 - not used */
					DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &galaga_chanl2_filt)

	/************************************************
	 * CHANL3 sound
	 ************************************************/
	DISCRETE_DAC_R1(NODE_40,
					NAMCO_54XX_0_DATA(NODE_01),
					4,          /* 4V - unmeasured*/
					&galaga_54xx_dac)
	DISCRETE_OP_AMP_FILTER(GALAGA_CHANL3_SND,
					1,          /* ENAB */
					NODE_40,    /* INP0 */
					0,          /* INP1 - not used */
					DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &galaga_chanl3_filt)

	/************************************************
	 * Output
	 ************************************************/
	DISCRETE_MIXER3(NODE_90,
					1,                  /* ENAB */
					GALAGA_CHANL1_SND,  /* IN0 */
					GALAGA_CHANL2_SND,  /* IN1 */
					GALAGA_CHANL3_SND,  /* IN2 */
					&galaga_final_mixer /* INFO */)
	DISCRETE_OUTPUT(NODE_90, 1)
DISCRETE_SOUND_END
