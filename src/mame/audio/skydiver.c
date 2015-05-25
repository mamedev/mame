// license:BSD-3-Clause
// copyright-holders:Derrick Renaud
/*************************************************************************

    audio\skydiver.c

*************************************************************************/
#include "emu.h"
#include "includes/skydiver.h"
#include "sound/discrete.h"


/************************************************************************/
/* skydiver Sound System Analog emulation                               */
/* Jan 2004, Derrick Renaud                                             */
/************************************************************************/

static const discrete_555_desc skydiverWhistl555 =
{
	DISC_555_OUT_SQW | DISC_555_OUT_AC,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_lfsr_desc skydiver_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,                     /* Bit Length */
	0,                      /* Reset Value */
	0,                      /* Use Bit 0 as XOR input 0 */
	14,                     /* Use Bit 14 as XOR input 1 */
	DISC_LFSR_XNOR,         /* Feedback stage1 is XNOR */
	DISC_LFSR_OR,           /* Feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,      /* Feedback stage3 replaces the shifted register contents */
	0x000001,               /* Everything is shifted into the first bit only */
	DISC_LFSR_FLAG_OUT_INVERT,  /* Output is inverted, Active Low Reset */
	15                      /* Output bit */
};

/* Nodes - Sounds */
#define SKYDIVER_NOTE_SND       NODE_11
#define SKYDIVER_NOISE_SND      NODE_12
#define SKYDIVER_WHISTLE1_SND   NODE_13
#define SKYDIVER_WHISTLE2_SND   NODE_14

DISCRETE_SOUND_START(skydiver)
	/************************************************/
	/* skydiver  Effects Relative Gain Table        */
	/*                                              */
	/* Effect  V-ampIn  Gain ratio        Relative  */
	/* Note     3.8     3.8/260.5          1000.0   */
	/* Noise    3.8     3.8/680             383.1   */
	/* Whistle  5.0     5.0/1500            228.5   */
	/************************************************/

	/************************************************/
	/* Input register mapping for skydiver          */
	/************************************************/
	/*                    NODE                  GAIN        OFFSET  INIT */
	DISCRETE_INPUT_DATA  (SKYDIVER_RANGE_DATA)
	DISCRETE_INPUT_LOGIC (SKYDIVER_RANGE3_EN)
	DISCRETE_INPUT_DATA  (SKYDIVER_NOTE_DATA)
	DISCRETE_INPUTX_DATA (SKYDIVER_NOISE_DATA,  383.1/15.0, 0.0,    0.0)
	DISCRETE_INPUT_NOT   (SKYDIVER_NOISE_RST)
	DISCRETE_INPUT_LOGIC (SKYDIVER_WHISTLE1_EN)
	DISCRETE_INPUT_LOGIC (SKYDIVER_WHISTLE2_EN)
	DISCRETE_INPUT_LOGIC (SKYDIVER_OCT1_EN)
	DISCRETE_INPUT_LOGIC (SKYDIVER_OCT2_EN)
	DISCRETE_INPUT_LOGIC (SKYDIVER_SOUND_EN)
	/************************************************/

	/************************************************/
	/* The note generator has a selectable range    */
	/* and selectable frequency.                    */
	/* The base frequency is                        */
	/* 12.096MHz / 2 / range / note freq / 2        */
	/* The final /2 is just to give a 50% duty,     */
	/* so we can just start by 12.096MHz/4/range    */
	/* The octave is selected by 3 bits selecting   */
	/* 000 64H  = 12096MHz / 2 / 128                */
	/* 001 32H  = 12096MHz / 2 / 64                 */
	/* 010 16H  = 12096MHz / 2 / 32                 */
	/* 011  8H  = 12096MHz / 2 / 16                 */
	/* 100  4H  = 12096MHz / 2 / 8                  */
	/* 101  2H  = 12096MHz / 2 / 4                  */
	/* 110  1H  = 12096MHz / 2 / 2                  */
	/* 111 6MHz = 12096MHz / 2 / 1                  */
	/* We will convert the 3 range bits to a        */
	/* divide value in the driver before sending    */
	/* to the sound interface.                      */
	/*                                              */
	/* note data: 0xff = off,                       */
	/*            0xfe = /2,                        */
	/*            0x00 = /256                       */
	/* We will send the note data bit inverted to   */
	/* sound interface so it is easier to work with.*/
	/*                                              */
	/* The note generator is disabled by a low on   */
	/* RANGE3.                                      */
	/************************************************/
	// We will disable the divide if SKYDIVER_RANGE_DATA = 0
	DISCRETE_DIVIDE(NODE_20, SKYDIVER_RANGE_DATA, 12096000.0 /2.0 / 2.0, SKYDIVER_RANGE_DATA)
	DISCRETE_ADDER2(NODE_21, 1, SKYDIVER_NOTE_DATA, 1)
	// We will disable the divide if SKYDIVER_NOTE_DATA = 0
	DISCRETE_DIVIDE(NODE_22, SKYDIVER_NOTE_DATA, NODE_20, NODE_21)  // freq
	DISCRETE_SQUAREWAVE(SKYDIVER_NOTE_SND, SKYDIVER_RANGE3_EN, NODE_22, 1000.0, 50.0, 0, 0.0)

	/************************************************/
	/* Noise circuit is built around a noise        */
	/* generator built from 2 shift registers that  */
	/* are clocked by the 1V signal.                */
	/* 1V = HSYNC/2                                 */
	/*    = 15750/2                                 */
	/* Output is binary weighted with 4 bits of     */
	/* volume.                                      */
	/************************************************/
	DISCRETE_LFSR_NOISE(SKYDIVER_NOISE_SND, SKYDIVER_NOISE_RST, SKYDIVER_NOISE_RST, 15750.0/2.0, SKYDIVER_NOISE_DATA, 0, 0, &skydiver_lfsr)

	/************************************************/
	/* Whistle circuit is a 555 capacitor charge    */
	/* waveform.  The original game pot varies from */
	/* 0-250k, but we are going to limit it because */
	/* below 50k the frequency is too high.         */
	/* When triggered it starts at it's highest     */
	/* frequency, then decays at the rate set by    */
	/* a 68k resistor and 22uf capacitor.           */
	/************************************************/
	DISCRETE_ADJUSTMENT(NODE_30, 250000, 50000, DISC_LINADJ, "WHISTLE1")    /* R66 */
	DISCRETE_MULTADD(NODE_31, SKYDIVER_WHISTLE1_EN, 3.05-0.33, 0.33)
	DISCRETE_RCDISC2(NODE_32, SKYDIVER_WHISTLE1_EN, NODE_31, 1.0, NODE_31, 68000.0, 2.2e-5) /* CV */
	DISCRETE_SWITCH(NODE_33, 1, SKYDIVER_OCT1_EN, 1e-8, 1e-8 + 3.3e-9)  /* Cap C73 & C58 */
	DISCRETE_555_ASTABLE_CV(NODE_34, SKYDIVER_WHISTLE1_EN, 100000, NODE_30, NODE_33, NODE_32, &skydiverWhistl555)
	DISCRETE_MULTIPLY(SKYDIVER_WHISTLE1_SND, NODE_34, 228.5/3.3)

	DISCRETE_ADJUSTMENT(NODE_35, 250000, 50000, DISC_LINADJ, "WHISTLE2")    /* R65 */
	DISCRETE_MULTADD(NODE_36, SKYDIVER_WHISTLE2_EN, 3.05-0.33, 0.33)
	DISCRETE_RCDISC2(NODE_37, SKYDIVER_WHISTLE2_EN, NODE_36, 1.0, NODE_36, 68000.0, 2.2e-5) /* CV */
	DISCRETE_SWITCH(NODE_38, 1, SKYDIVER_OCT2_EN, 1e-8, 1e-8 + 3.3e-9)  /* Cap C72 & C59 */
	DISCRETE_555_ASTABLE_CV(NODE_39, SKYDIVER_WHISTLE2_EN, 100000, NODE_35, NODE_38, NODE_37, &skydiverWhistl555)
	DISCRETE_MULTIPLY(SKYDIVER_WHISTLE2_SND, NODE_39, 228.5/3.3)

	/************************************************/
	/* Final mix and output.                        */
	/************************************************/
	DISCRETE_ADDER4(NODE_90, SKYDIVER_SOUND_EN, SKYDIVER_NOTE_SND, SKYDIVER_NOISE_SND, SKYDIVER_WHISTLE1_SND, SKYDIVER_WHISTLE2_SND)

	DISCRETE_OUTPUT(NODE_90, 65534.0/(1000.0 + 383.1 + 228.5 + 228.5))
DISCRETE_SOUND_END
