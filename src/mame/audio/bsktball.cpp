// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    audio\bsktball.c

*************************************************************************/
#include "emu.h"
#include "includes/bsktball.h"
#include "sound/discrete.h"

/***************************************************************************
Sound handlers
***************************************************************************/
WRITE8_MEMBER(bsktball_state::bsktball_bounce_w)
{
	m_discrete->write(space, BSKTBALL_CROWD_DATA, data & 0x0f);  // Crowd
	m_discrete->write(space, BSKTBALL_BOUNCE_EN, data & 0x10);   // Bounce
}

WRITE8_MEMBER(bsktball_state::bsktball_note_w)
{
	m_discrete->write(space, BSKTBALL_NOTE_DATA, data);  // Note
}

WRITE8_MEMBER(bsktball_state::bsktball_noise_reset_w)
{
	m_discrete->write(space, BSKTBALL_NOISE_EN, offset & 0x01);
}


/************************************************************************/
/* bsktball Sound System Analog emulation                               */
/************************************************************************/

static const discrete_lfsr_desc bsktball_lfsr =
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
	15          /* Output bit */
};

static const discrete_dac_r1_ladder bsktball_crowd_r1_ladder =
{
	4,
	{RES_K(390), RES_K(220), RES_K(100), RES_K(56)},    // r55, r54, r53, r52
	0, 0,       // no bias
	RES_K(1),   // r21
	CAP_U(0.1)  // c32
};

static const discrete_op_amp_filt_info bsktball_crowd_filt =
{
	1.0/(1.0/RES_K(390) + 1.0/RES_K(220) + 1.0/RES_K(100) + 1.0/RES_K(56) + 1.0/RES_K(1)),  // r55, r54, r53, r52, r21
	0, 0, 0,
	RES_K(330),     // r58
	CAP_U(.01),     // c55
	CAP_U(.022),    // c56
	0,
	5, 12, 0
};

static const discrete_mixer_desc bsktball_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{RES_K(47), RES_K(47), RES_K(220)},     // r56, r57, r60
	{0},            // no rNodes
	{CAP_U(.01), CAP_U(.01), CAP_U(.01)},   // c53, c54, c57
	0,
	RES_K(47),      // r61
	CAP_U(.001),    // c58
	CAP_U(1),
	5,
	7500
};

#define BSKTBALL_32H            12096000.0/4/32
#define BSKTBALL_256H           12096000.0/768

/* Nodes - Sounds */
#define BSKTBALL_NOISE          NODE_10
#define BSKTBALL_BOUNCE_SND     BSKTBALL_BOUNCE_EN
#define BSKTBALL_NOTE_SND       NODE_12
#define BSKTBALL_CROWD_SND      NODE_13

DISCRETE_SOUND_START(bsktball)
	/************************************************/
	/* Input register mapping for bsktball          */
	/************************************************/
	/*                    NODE                                 GAIN     OFFSET  INIT */
	DISCRETE_INPUT_DATA  (BSKTBALL_NOTE_DATA)
	DISCRETE_INPUT_DATA  (BSKTBALL_CROWD_DATA)
	/* Bounce is a trigger fed directly to the amp  */
	DISCRETE_INPUTX_LOGIC(BSKTBALL_BOUNCE_EN,    DEFAULT_TTL_V_LOGIC_1,  0,      0.0)
	DISCRETE_INPUT_NOT   (BSKTBALL_NOISE_EN)

	/************************************************/
	/* Crowd effect is variable amplitude, filtered */
	/* random noise.                                */
	/* LFSR clk = 256H = 15750.0Hz                  */
	/************************************************/
	DISCRETE_LFSR_NOISE(BSKTBALL_NOISE, BSKTBALL_NOISE_EN, BSKTBALL_NOISE_EN, BSKTBALL_256H, 1, 0, .5, &bsktball_lfsr)
	DISCRETE_SWITCH(NODE_20, 1, BSKTBALL_NOISE, 0, BSKTBALL_CROWD_DATA) // enable data, gate D11
	DISCRETE_DAC_R1(NODE_21, NODE_20, DEFAULT_TTL_V_LOGIC_1, &bsktball_crowd_r1_ladder)
	DISCRETE_OP_AMP_FILTER(BSKTBALL_CROWD_SND, 1, NODE_21, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &bsktball_crowd_filt)

	/************************************************/
	/* Note sound is created by a divider circuit.  */
	/* The master clock is the 32H signal, which is */
	/* 12.096MHz/128.  This is then sent to a       */
	/* preloadable 8 bit counter, which loads the   */
	/* value from OUT30 when overflowing from 0xFF  */
	/* to 0x00.  Therefore it divides by 2 (OUT30   */
	/* = FE) to 256 (OUT30 = 00).                   */
	/* There is also a final /2 stage.              */
	/* Note that there is no music disable line.    */
	/* When there is no music, the game sets the    */
	/* oscillator to 0Hz.  (OUT30 = FF)             */
	/************************************************/
	DISCRETE_NOTE(NODE_30, 1, BSKTBALL_32H, BSKTBALL_NOTE_DATA, 255, 1, DISC_CLK_IS_FREQ | DISC_OUT_IS_ENERGY)
	DISCRETE_GAIN(BSKTBALL_NOTE_SND, NODE_30, DEFAULT_TTL_V_LOGIC_1)


	/************************************************/
	/* Mixing stage - B11                           */
	/************************************************/
	DISCRETE_MIXER3(NODE_90, 1, BSKTBALL_NOTE_SND, BSKTBALL_BOUNCE_SND, BSKTBALL_CROWD_SND, &bsktball_mixer)
	DISCRETE_OUTPUT(NODE_90, 1)
DISCRETE_SOUND_END
