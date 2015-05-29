// license:???
// copyright-holders:Derrick Renaud
/*************************************************************************

    audio\orbit.c

*************************************************************************/
#include "emu.h"
#include "includes/orbit.h"
#include "sound/discrete.h"

/*************************************
 *
 *  Write handlers
 *
 *************************************/

WRITE8_MEMBER(orbit_state::orbit_note_w)
{
	m_discrete->write(space, ORBIT_NOTE_FREQ, (~data) & 0xff);
}

WRITE8_MEMBER(orbit_state::orbit_note_amp_w)
{
	m_discrete->write(space, ORBIT_ANOTE1_AMP, data & 0x0f);
	m_discrete->write(space, ORBIT_ANOTE2_AMP, data >> 4);
}

WRITE8_MEMBER(orbit_state::orbit_noise_amp_w)
{
	m_discrete->write(space, ORBIT_NOISE1_AMP, data & 0x0f);
	m_discrete->write(space, ORBIT_NOISE2_AMP, data >> 4);
}

WRITE8_MEMBER(orbit_state::orbit_noise_rst_w)
{
	m_discrete->write(space, ORBIT_NOISE_EN, 0);
}


/************************************************************************/
/* orbit Sound System Analog emulation                                  */
/************************************************************************/

static const discrete_lfsr_desc orbit_lfsr =
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

/* Nodes - Sounds */
#define ORBIT_NOISE         NODE_10
#define ORBIT_NOISE1_SND    NODE_11
#define ORBIT_NOISE2_SND    NODE_12
#define ORBIT_ANOTE1_SND    NODE_13
#define ORBIT_ANOTE2_SND    NODE_14
#define ORBIT_WARNING_SND   NODE_15

DISCRETE_SOUND_START(orbit)
	/************************************************/
	/* orbit  Effects Relataive Gain Table          */
	/*                                              */
	/* Effect  V-ampIn  Gain ratio        Relative  */
	/* Note     3.8     10/(20.1+.47+.55)   962.1   */
	/* Warning  3.8     10/(20.1+6.8)       755.4   */
	/* Noise    3.8     10/(20.1+.22)      1000.0   */
	/************************************************/

	/************************************************/
	/* Input register mapping for orbit             */
	/************************************************/
	/*                   NODE              GAIN      OFFSET  INIT */
	DISCRETE_INPUT_DATA (ORBIT_NOTE_FREQ)
	DISCRETE_INPUTX_DATA(ORBIT_ANOTE1_AMP, 962.1/15,  0,     0.0)
	DISCRETE_INPUTX_DATA(ORBIT_ANOTE2_AMP, 962.1/15,  0,     0.0)
	DISCRETE_INPUTX_DATA(ORBIT_NOISE1_AMP, 1000.0/15*10, 0,     0.0)
	DISCRETE_INPUTX_DATA(ORBIT_NOISE2_AMP, 1000.0/15*10, 0,     0.0)
	DISCRETE_INPUT_LOGIC(ORBIT_WARNING_EN)
	DISCRETE_INPUT_PULSE(ORBIT_NOISE_EN, 1)

	/************************************************/
	/* Warning is just a triggered 2V signal        */
	/************************************************/
	DISCRETE_SQUAREWFIX(ORBIT_WARNING_SND, ORBIT_WARNING_EN, 15750.0/4, 755.4, 50.0, 0, 0.0)

	/************************************************/
	/* Noise effect is variable amplitude, filtered */
	/* random noise.                                */
	/* LFSR clk = 32V= 15750.0Hz/2/32               */
	/************************************************/
	DISCRETE_LFSR_NOISE(ORBIT_NOISE, ORBIT_NOISE_EN, ORBIT_NOISE_EN, 15750.0/2/32, 1, 0, 0, &orbit_lfsr)
	DISCRETE_MULTIPLY(NODE_20, ORBIT_NOISE, ORBIT_NOISE1_AMP)
	DISCRETE_MULTIPLY(NODE_21, ORBIT_NOISE, ORBIT_NOISE2_AMP)
	DISCRETE_FILTER2(ORBIT_NOISE1_SND, 1, NODE_20, 160.0, (1.0 / 10.0), DISC_FILTER_BANDPASS)
	DISCRETE_FILTER2(ORBIT_NOISE2_SND, 1, NODE_21, 160.0, (1.0 / 10.0), DISC_FILTER_BANDPASS)

	/************************************************/
	/* Note sound is created by a divider circuit.  */
	/* The master clock is the 64H signal, which is */
	/* 12.096MHz/256.  This is then sent to a       */
	/* preloadable 8 bit counter, which loads the   */
	/* value from NOTELD when overflowing from 0xFF */
	/* to 0x00.  Therefore it divides by 2 (NOTELD  */
	/* = FE) to 256 (NOTELD = 00).                  */
	/* There is also a final /2 stage.              */
	/* Note that there is no music disable line.    */
	/* When there is no music, the game sets the    */
	/* oscillator to 0Hz.  (NOTELD = FF)            */
	/************************************************/
	DISCRETE_ADDER2(NODE_30, 1, ORBIT_NOTE_FREQ, 1) /* To get values of 1 - 256 */
	DISCRETE_DIVIDE(NODE_31, 1, 12096000.0/256/2, NODE_30)
	DISCRETE_SQUAREWAVE(ORBIT_ANOTE1_SND, ORBIT_NOTE_FREQ, NODE_31, ORBIT_ANOTE1_AMP, 50.0, 0, 0.0) /* NOTE=FF Disables audio */
	DISCRETE_SQUAREWAVE(ORBIT_ANOTE2_SND, ORBIT_NOTE_FREQ, NODE_31, ORBIT_ANOTE2_AMP, 50.0, 0, 0.0) /* NOTE=FF Disables audio */

	/************************************************/
	/* Final mix and output.                        */
	/************************************************/
	DISCRETE_ADDER3(NODE_90, 1, ORBIT_WARNING_SND, ORBIT_NOISE1_SND, ORBIT_ANOTE1_SND)
	DISCRETE_ADDER3(NODE_91, 1, ORBIT_WARNING_SND, ORBIT_NOISE2_SND, ORBIT_ANOTE2_SND)

	DISCRETE_OUTPUT(NODE_90, 65534.0/(962.1+755.4+1000.0))
	DISCRETE_OUTPUT(NODE_91, 65534.0/(962.1+755.4+1000.0))
DISCRETE_SOUND_END
