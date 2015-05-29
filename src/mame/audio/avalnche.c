// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    audio\avalnche.c

*************************************************************************/
#include "emu.h"
#include "includes/avalnche.h"
#include "sound/discrete.h"


/* Avalanche Discrete Sound Input Nodes */
#define AVALNCHE_AUD0_EN            NODE_01
#define AVALNCHE_AUD1_EN            NODE_02
#define AVALNCHE_AUD2_EN            NODE_03
#define AVALNCHE_SOUNDLVL_DATA      NODE_04
#define AVALNCHE_ATTRACT_EN         NODE_05


/***************************************************************************
  Avalanche memory audio output handlers
***************************************************************************/

WRITE8_MEMBER(avalnche_state::avalnche_noise_amplitude_w)
{
	m_discrete->write(space, AVALNCHE_SOUNDLVL_DATA, data & 0x3f);
}

WRITE8_MEMBER(avalnche_state::avalnche_attract_enable_w)
{
	m_discrete->write(space, AVALNCHE_ATTRACT_EN, data & 0x01);
}

WRITE8_MEMBER(avalnche_state::avalnche_audio_w)
{
	int bit = data & 0x01;

	switch (offset & 0x07)
	{
	case 0x00:      /* AUD0 */
		m_discrete->write(space, AVALNCHE_AUD0_EN, bit);
		break;

	case 0x01:      /* AUD1 */
		m_discrete->write(space, AVALNCHE_AUD1_EN, bit);
		break;

	case 0x02:      /* AUD2 */
	default:
		m_discrete->write(space, AVALNCHE_AUD2_EN, bit);
		break;
	}
}


/***************************************************************************
  Avalanche sound system analog emulation
***************************************************************************/

static const discrete_lfsr_desc avalnche_lfsr={
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
#define AVALNCHE_NOISE              NODE_10
#define AVALNCHE_AUD1_SND           NODE_11
#define AVALNCHE_AUD2_SND           NODE_12
#define AVALNCHE_SOUNDLVL_AUD0_SND  NODE_13

DISCRETE_SOUND_START(avalnche)
	/************************************************/
	/* avalnche  Effects Relataive Gain Table       */
	/*                                              */
	/* Effect    V-ampIn  Gain ratio      Relative  */
	/* Aud0       3.8     50/(50+33+39)     725.6   */
	/* Aud1       3.8     50/(50+68)        750.2   */
	/* Aud2       3.8     50/(50+68)        750.2   */
	/* Soundlvl   3.8     50/(50+33+.518)  1000.0   */
	/************************************************/

	/************************************************/
	/* Input register mapping for avalnche          */
	/************************************************/
	/*                    NODE                    GAIN      OFFSET  INIT */
	DISCRETE_INPUT_LOGIC (AVALNCHE_AUD0_EN)
	DISCRETE_INPUT_LOGIC (AVALNCHE_AUD1_EN)
	DISCRETE_INPUT_LOGIC (AVALNCHE_AUD2_EN)
	DISCRETE_INPUTX_DATA (AVALNCHE_SOUNDLVL_DATA, 500.0/63, 0,      0.0)
	DISCRETE_INPUT_NOT   (AVALNCHE_ATTRACT_EN)

	/************************************************/
	/* Aud0 = 2V  = HSYNC/4 = 15750/4               */
	/* Aud1 = 32V = HSYNC/64 = 15750/64             */
	/* Aud2 = 8V  = HSYNC/16 = 15750/16             */
	/************************************************/
	DISCRETE_SQUAREWFIX(NODE_20, AVALNCHE_AUD0_EN, 15750.0/4,  725.6, 50.0, 0, 0.0) // Aud0
	DISCRETE_SQUAREWFIX(AVALNCHE_AUD1_SND, AVALNCHE_AUD1_EN, 15750.0/64, 750.2, 50.0, 0, 0.0)
	DISCRETE_SQUAREWFIX(AVALNCHE_AUD2_SND, AVALNCHE_AUD2_EN, 15750.0/16, 750.2, 50.0, 0, 0.0)

	/************************************************/
	/* Soundlvl is variable amplitude, filtered     */
	/* random noise.                                */
	/* LFSR clk = 16V = 15750.0Hz/16/2              */
	/************************************************/
	DISCRETE_LFSR_NOISE(AVALNCHE_NOISE, AVALNCHE_ATTRACT_EN, AVALNCHE_ATTRACT_EN, 15750.0, AVALNCHE_SOUNDLVL_DATA, 0, 0, &avalnche_lfsr)
	DISCRETE_ADDER2(NODE_30, 1, NODE_20, AVALNCHE_NOISE)
	DISCRETE_RCFILTER(AVALNCHE_SOUNDLVL_AUD0_SND, NODE_30, 556.7, 1e-7)

	/************************************************/
	/* Final mix and output.                        */
	/************************************************/
	DISCRETE_ADDER3(NODE_90, AVALNCHE_ATTRACT_EN, AVALNCHE_AUD1_SND, AVALNCHE_AUD2_SND, AVALNCHE_SOUNDLVL_AUD0_SND)

	DISCRETE_OUTPUT(NODE_90, 65534.0/(725.6+750.2+750.2+1000.0))
DISCRETE_SOUND_END


/***************************************************************************
  Catch memory audio output handlers
***************************************************************************/

WRITE8_MEMBER(avalnche_state::catch_audio_w)
{
	/* Different from avalnche, it plays a sound (offset 0/1/2) on data bit 0 rising edge.
	There's no indication that the game silences sound, it's probably done automatically. */
}
