// license:BSD-3-Clause
// copyright-holders:Derrick Renaud
/*************************************************************************

    audio\subs.c

*************************************************************************/
#include "emu.h"
#include "includes/subs.h"
#include "sound/discrete.h"


/***************************************************************************
sub sound functions
***************************************************************************/

WRITE8_MEMBER(subs_state::sonar1_w)
{
	m_discrete->write(space, SUBS_SONAR1_EN, offset & 0x01);
}

WRITE8_MEMBER(subs_state::sonar2_w)
{
	m_discrete->write(space, SUBS_SONAR2_EN, offset & 0x01);
}

WRITE8_MEMBER(subs_state::crash_w)
{
	m_discrete->write(space, SUBS_CRASH_EN, offset & 0x01);
}

WRITE8_MEMBER(subs_state::explode_w)
{
	m_discrete->write(space, SUBS_EXPLODE_EN, offset & 0x01);
}

WRITE8_MEMBER(subs_state::noise_reset_w)
{
	/* Pulse noise reset */
	m_discrete->write(space, SUBS_NOISE_RESET, 0);
}


/************************************************************************/
/* subs Sound System Analog emulation                                   */
/************************************************************************/

static const discrete_lfsr_desc subs_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,         /* Bit Length */
	0,          /* Reset Value */
	13,         /* Use Bit 13 as XOR input 0 */
	14,         /* Use Bit 14 as XOR input 1 */
	DISC_LFSR_XNOR,     /* Feedback stage1 is XNOR */
	DISC_LFSR_OR,       /* Feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,  /* Feedback stage3 replaces the shifted register contents */
	0x000001,       /* Everything is shifted into the first bit only */
	0,          /* Output is already inverted by XNOR */
	13          /* Output bit */
};

/* Nodes - Sounds */
#define SUBS_NOISE          NODE_10
#define SUBS_SONAR1_SND     NODE_11
#define SUBS_SONAR2_SND     NODE_12
#define SUBS_LAUNCH_SND     NODE_13
#define SUBS_CRASH_SND      NODE_14
#define SUBS_EXPLODE_SND    NODE_15

DISCRETE_SOUND_START(subs)
	/************************************************/
	/* subs  Effects Relataive Gain Table           */
	/*                                              */
	/* NOTE: The schematic does not show the amp    */
	/*  stage so I will assume a 5K volume control. */
	/*                                              */
	/* Effect       V-ampIn   Gain ratio  Relative  */
	/* Sonar         2.33     5/(5+2.7)     320.8   */
	/* Launch        3.8      5/(5+47)       77.5   */
	/* Crash         3.8      5/(5+22)      149.2   */
/* Explosion     10.0     5/(5+5.6)    1000.0   */
	/************************************************/

	/************************************************/
	/* Input register mapping for subs              */
	/************************************************/
	/*                   NODE              GAIN      OFFSET  INIT */
	DISCRETE_INPUTX_DATA(SUBS_SONAR1_EN,   400.0,     0,     0.0)
	DISCRETE_INPUTX_DATA(SUBS_SONAR2_EN,   400.0,     0,     0.0)
	DISCRETE_INPUTX_DATA(SUBS_LAUNCH_DATA,  77.5/15,  0,     0.0)
	DISCRETE_INPUTX_DATA(SUBS_CRASH_DATA,  149.2/15,  0,     0.0)
	DISCRETE_INPUT_NOT  (SUBS_CRASH_EN)
	DISCRETE_INPUT_NOT  (SUBS_EXPLODE_EN)
	DISCRETE_INPUT_PULSE(SUBS_NOISE_RESET, 1)

	/************************************************/
	/* Noise source                                 */
	/* LFSR clk = 256H = 15750.0Hz                  */
	/************************************************/
	DISCRETE_LFSR_NOISE(SUBS_NOISE, SUBS_NOISE_RESET, SUBS_NOISE_RESET, 15750.0, 1.0, 0, 0, &subs_lfsr)

	/************************************************/
	/* Launch is just amplitude contolled noise     */
	/************************************************/
	DISCRETE_MULTIPLY(SUBS_LAUNCH_SND, SUBS_NOISE, SUBS_LAUNCH_DATA)

	/************************************************/
	/* Crash resamples the noise at 8V and then     */
	/* controls the amplitude.                      */
	/* 8V = Hsync/2/8 = 15750/2/8                   */
	/************************************************/
	DISCRETE_SQUAREWFIX(NODE_20, 1, 15750.0/2/8, 1.0, 50, 1.0/2, 0) /* Resample freq. */
	DISCRETE_SAMPLHOLD(NODE_21, SUBS_NOISE, NODE_20, DISC_SAMPHOLD_REDGE)
	DISCRETE_MULTIPLY(NODE_22, NODE_21, SUBS_CRASH_DATA)
	DISCRETE_ONOFF(SUBS_CRASH_SND, SUBS_CRASH_EN, NODE_22)

	/************************************************/
	/* Explode filters the crash sound.             */
	/* I'm not sure of the exact filter freq.       */
	/************************************************/
	DISCRETE_TRANSFORM3(NODE_30, NODE_21, SUBS_CRASH_DATA, 1000.0 / 149.2*7.6, "01*2*")
	DISCRETE_FILTER2(SUBS_EXPLODE_SND, SUBS_EXPLODE_EN, NODE_30, 100.0, (1.0 / 7.6), DISC_FILTER_BANDPASS)

	/************************************************/
	/* Not sure how the sonar works yet.            */
	/************************************************/
	DISCRETE_RCDISC2(NODE_40, SUBS_SONAR1_EN, SUBS_SONAR1_EN, 680000.0, SUBS_SONAR1_EN, 1000.0, 1e-6)   /* Decay envelope */
	DISCRETE_ADDER2(NODE_41, 1, NODE_40, 800)
	DISCRETE_LOGIC_AND(NODE_42, SUBS_SONAR1_EN, SUBS_NOISE)
	DISCRETE_TRIANGLEWAVE(SUBS_SONAR1_SND, NODE_42, NODE_41, 320.8, 0.0, 0)

	DISCRETE_RCDISC2(NODE_50, SUBS_SONAR2_EN, SUBS_SONAR2_EN, 18600.0, SUBS_SONAR2_EN, 20.0, 4.7e-6)    /* Decay envelope */
	DISCRETE_ADDER2(NODE_51, 1, NODE_50, 800)
	DISCRETE_LOGIC_AND(NODE_52, SUBS_SONAR2_EN, SUBS_NOISE)
	DISCRETE_TRIANGLEWAVE(SUBS_SONAR2_SND, NODE_52, NODE_51, 320.8, 0.0, 0)

	/************************************************/
	/* Combine all sound sources.                   */
	/* Add some final gain to get to a good sound   */
	/* level.                                       */
	/************************************************/

	DISCRETE_ADDER4(NODE_90, 1, 0, SUBS_LAUNCH_SND, SUBS_CRASH_SND, SUBS_EXPLODE_SND)
	DISCRETE_ADDER4(NODE_91, 1, 0, SUBS_LAUNCH_SND, SUBS_CRASH_SND, SUBS_EXPLODE_SND)

	DISCRETE_OUTPUT(NODE_90, 65534.0/(320.8+77.5+149.2+1000.0))
	DISCRETE_OUTPUT(NODE_91, 65534.0/(320.8+77.5+149.2+1000.0))
DISCRETE_SOUND_END
