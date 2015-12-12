// license:???
// copyright-holders:Derrick Renaud 
/*************************************************************************

    audio\atarifb.c

*************************************************************************/

#include "emu.h"
#include "includes/atarifb.h"
#include "sound/discrete.h"


/************************************************************************/
/* atarifb Sound System Analog emulation                                */
/*                                                                      */
/* Complete re-write Dec 2004, D. Renaud                                */
/************************************************************************/

static const discrete_555_desc atarifbWhistl555 =
{
	DISC_555_OUT_CAP | DISC_555_OUT_DC,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_lfsr_desc atarifb_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,         /* Bit Length */
	0,          /* Reset Value */
	0,          /* Use Bit 0 as XOR input 0 */
	14,         /* Use Bit 14 as XOR input 1 */
	DISC_LFSR_XNOR,     /* Feedback stage1 is XNOR */
	DISC_LFSR_OR,       /* Feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,  /* Feedback stage3 replaces the shifted register contents */
	0x000001,       /* Everything is shifted into the first bit only */
	0,          /* Output is already inverted by XNOR */
	16          /* Output bit is feedback bit */
};

static const discrete_dac_r1_ladder atarifb_crowd_r1_ladder =
{
	4,
	{RES_K(390), RES_K(220), RES_K(100), RES_K(56)},    // r17, r16, r14, r15
	0, 0,       // no bias
	0,          // no rGnd
	CAP_U(0.1)  // c32
};

static const discrete_op_amp_filt_info atarifb_crowd_filt =
{
	1.0/(1.0/RES_K(390) + 1.0/RES_K(220) + 1.0/RES_K(100) + 1.0/RES_K(56)), // r17, r16, r14, r15
	0, 0, 0,
	RES_K(330), // r35
	CAP_U(.01), // c33
	CAP_U(.01), // c44
	0,
	5, 12, 0
};

static const discrete_mixer_desc atarifb_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{RES_K(47), RES_K(47), RES_K(220)}, // r71, r70, r73
	{0},            // no rNodes
	{CAP_U(.01), CAP_U(.01), CAP_U(.01)},   // c53, c52, c51
	0,
	RES_K(47),      // r74
	CAP_U(.001),    // c54
	CAP_U(0.1),     // c57
	5,
	40000
};

/* Nodes - Sounds */
#define ATARIFB_NOISE           NODE_10
#define ATARIFB_HIT_SND         ATARIFB_HIT_EN
#define ATARIFB_WHISTLE_SND     NODE_12
#define ATARIFB_CROWD_SND       NODE_13

DISCRETE_SOUND_START(atarifb)
	/************************************************/
	/* Input register mapping for atarifb           */
	/************************************************/
	/*                    NODE                            GAIN      OFFSET INIT */
	DISCRETE_INPUT_LOGIC (ATARIFB_WHISTLE_EN)
	DISCRETE_INPUT_DATA  (ATARIFB_CROWD_DATA)
	/* Hit is a trigger fed directly to the amp     */
	DISCRETE_INPUTX_LOGIC(ATARIFB_HIT_EN,    DEFAULT_TTL_V_LOGIC_1, 0,      0.0)
	DISCRETE_INPUT_NOT   (ATARIFB_ATTRACT_EN)
	DISCRETE_INPUT_LOGIC (ATARIFB_NOISE_EN)

	/************************************************/
	/* Crowd effect is variable amplitude, filtered */
	/* random noise.                                */
	/* LFSR clk = 256H = 15750.0Hz                  */
	/************************************************/
	DISCRETE_LFSR_NOISE(NODE_20, ATARIFB_NOISE_EN, ATARIFB_NOISE_EN, 15750.0, 1, 0, .5, &atarifb_lfsr)
	DISCRETE_SWITCH(NODE_21, 1, NODE_20, 0, ATARIFB_CROWD_DATA) // enable data, gate D8
	DISCRETE_DAC_R1(NODE_22, NODE_21, DEFAULT_TTL_V_LOGIC_1, &atarifb_crowd_r1_ladder)
	DISCRETE_OP_AMP_FILTER(ATARIFB_CROWD_SND, 1, NODE_22, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &atarifb_crowd_filt)

	/************************************************/
	/* Whistle effect is a triggered 555 cap charge */
	/* Q10 is a buffer to make sure there is no     */
	/* load on the cap.  It shifts the voltage by   */
	/* -0.5V                                        */
	/************************************************/
	DISCRETE_555_ASTABLE(NODE_30, ATARIFB_WHISTLE_EN,
							RES_K(2.2), RES_K(2.2), CAP_U(0.1), // r34, r33, c44
							&atarifbWhistl555)
	DISCRETE_TRANSFORM3(ATARIFB_WHISTLE_SND, NODE_30, .5, 0, "01-P2>*") // Q10 drop

	/************************************************/
	/* Mixing stage - A9                            */
	/* We will diliberately drive the amp into      */
	/* clipping.                                    */
	/* This is what the original game did.          */
	/************************************************/
	DISCRETE_MIXER3(NODE_90, ATARIFB_ATTRACT_EN, ATARIFB_HIT_SND, ATARIFB_WHISTLE_SND, ATARIFB_CROWD_SND, &atarifb_mixer)
	DISCRETE_OUTPUT(NODE_90, 1)
DISCRETE_SOUND_END


/************************************************************************/
/* abaseb Sound System Analog emulation                                 */
/*                                                                      */
/* Complete re-write Dec 2004, D. Renaud                                */
/************************************************************************/

static const discrete_dac_r1_ladder abaseb_crowd_r1_ladder =
{
	4,
	{RES_K(390), RES_K(220), RES_K(100), RES_K(56)},    // r17, r16, r14, r15
	0, 0,       // no bias
	RES_K(1),   // r76
	0           // no filter
};

static const discrete_op_amp_filt_info abaseb_crowd_filt =
{
	1.0/(1.0/RES_K(390) + 1.0/RES_K(220) + 1.0/RES_K(100) + 1.0/RES_K(56) + 1.0/RES_K(1)),  // r17, r16, r14, r15, r76
	0, 0, 0,
	RES_K(470),     // r35
	CAP_U(.01),     // c33
	CAP_U(.022),    // c44
	0,
	5, 12, 0
};

static const discrete_mixer_desc abaseb_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{RES_K(330), RES_K(220), RES_K(220)},   // r71, r70, r73
	{0},                // no rNodes
	{CAP_U(.01), CAP_U(.01), CAP_U(.01)},   // c53, c52, c51
	0,
	RES_K(47),      // r74
	CAP_U(.001),    // c54
	CAP_U(10),      // c69
	5,
	25000
};

/* Nodes - Sounds */
#define ABASEB_NOISE            NODE_10
#define ABASEB_HIT_SND          ATARIFB_HIT_EN
#define ABASEB_WHISTLE_SND      NODE_12
#define ABASEB_CROWD_SND        NODE_13

DISCRETE_SOUND_START(abaseb)
	/************************************************/
	/* Input register mapping for abaseb            */
	/************************************************/
	/*                    NODE                             GAIN      OFFSET  INIT */
	DISCRETE_INPUT_LOGIC (ATARIFB_WHISTLE_EN)
	DISCRETE_INPUT_DATA  (ATARIFB_CROWD_DATA)
	/* Hit is a trigger fed directly to the amp     */
	DISCRETE_INPUTX_LOGIC(ATARIFB_HIT_EN,    DEFAULT_TTL_V_LOGIC_1,  0,      0.0)
	DISCRETE_INPUT_NOT   (ATARIFB_ATTRACT_EN)
	DISCRETE_INPUT_LOGIC (ATARIFB_NOISE_EN)

	/************************************************/
	/* Crowd effect is variable amplitude, filtered */
	/* random noise.                                */
	/* LFSR clk = 256H = 15750.0Hz                  */
	/************************************************/
	DISCRETE_LFSR_NOISE(NODE_20, ATARIFB_NOISE_EN, ATARIFB_NOISE_EN, 15750.0, 1, 0, .5, &atarifb_lfsr)
	DISCRETE_SWITCH(NODE_21, 1, NODE_20, 0, ATARIFB_CROWD_DATA) // enable data, gate D8
	DISCRETE_DAC_R1(NODE_22, NODE_21, DEFAULT_TTL_V_LOGIC_1, &abaseb_crowd_r1_ladder)
	DISCRETE_OP_AMP_FILTER(ABASEB_CROWD_SND, 1, NODE_22, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &abaseb_crowd_filt)

	/************************************************/
	/* Whistle effect is a triggered 555 cap charge */
	/* Q10 is a buffer to make sure there is no     */
	/* load on the cap.  It shifts the voltage by   */
	/* -0.5V                                        */
	/************************************************/
	DISCRETE_555_ASTABLE(NODE_30, ATARIFB_WHISTLE_EN,
							RES_K(2.2), RES_K(2.2), CAP_U(0.1), // r34, r33, c44
							&atarifbWhistl555)
	DISCRETE_TRANSFORM3(ABASEB_WHISTLE_SND, NODE_30, .5, 0, "01-P2>*")  // Q10 drop

	/************************************************/
	/* Mixing stage - A9                            */
	/************************************************/
	DISCRETE_MIXER3(NODE_90, ATARIFB_ATTRACT_EN, ABASEB_HIT_SND, ABASEB_WHISTLE_SND, ABASEB_CROWD_SND, &abaseb_mixer)
	DISCRETE_OUTPUT(NODE_90, 1)
DISCRETE_SOUND_END
