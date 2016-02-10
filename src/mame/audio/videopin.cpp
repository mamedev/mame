// license:BSD-3-Clause
// copyright-holders:Derrick Renaud
/*************************************************************************

    audio\videopin.c

*************************************************************************/
#include "emu.h"
#include "includes/videopin.h"
#include "sound/discrete.h"


/************************************************************************/
/* videopin Sound System Analog emulation                               */
/* Jan 2004, Derrick Renaud                                             */
/************************************************************************/

/* Nodes - Sounds */
#define VIDEOPIN_VOL_SND    NODE_10
#define VIDEOPIN_BELL_SND   NODE_11
#define VIDEOPIN_BONG_SND   NODE_12

DISCRETE_SOUND_START(videopin)
/************************************************/
	/* videopin  Effects Relataive Gain Table       */
	/*                                              */
	/* Effect  V-ampIn  Gain ratio        Relative  */
	/* Vol0     3.8     10/(10+50)         1000.0   */
	/* Bell     4.4     (47+10)/(47+10+50)  740.2   */
	/* Bong     3.8     10/(10+50)         1000.0   */
	/************************************************/

	/************************************************/
	/* Input register mapping for videopin          */
	/************************************************/
	/*              NODE                   GAIN        OFFSET  INIT */
	DISCRETE_INPUT_DATA (VIDEOPIN_OCTAVE_DATA)
	DISCRETE_INPUT_DATA (VIDEOPIN_NOTE_DATA)
	DISCRETE_INPUT_NOT  (VIDEOPIN_BELL_EN)
	DISCRETE_INPUT_LOGIC(VIDEOPIN_BONG_EN)
	DISCRETE_INPUT_NOT  (VIDEOPIN_ATTRACT_EN)
	DISCRETE_INPUTX_DATA(VIDEOPIN_VOL_DATA,     1000.0/7.0, 0.0,    0.0)
	/************************************************/

	/************************************************/
	/* Vol0,1,2 are 3 different amplitudes of the   */
	/* same note.  It has a selectable octave and   */
	/* selectable frequency.                        */
	/* The base frequency is                        */
	/* 12.096MHz / octave / 3 / note freq / 2       */
	/* The final /2 is just to give a 50% duty,     */
	/* so we can just start by 12.096MHz/6/octave   */
	/* The octave is selected by 3 bits selecting   */
	/* 000 32H  = 12096MHz / 2 / 2 / 32             */
	/* 001 16H  = 12096MHz / 2 / 2 / 16             */
	/* 010  8H  = 12096MHz / 2 / 2 / 8              */
	/* 011  4H  = 12096MHz / 2 / 2 / 4              */
	/* 100  2H  = 12096MHz / 2 / 2 / 2              */
	/* 101  1H  = 12096MHz / 2 / 2 / 1              */
	/* 110 6MHz = 12096MHz / 2                      */
	/* 111  0V  = Disable                           */
	/* We will convert the 3 octave bits to a       */
	/* divide value in the driver before sending    */
	/* to the sound interface.                      */
	/*                                              */
	/* note data: 0xff = off,                       */
	/*            0xfe = /2,                        */
	/*            0x00 = /256                       */
	/* We will send the note data bit inverted to   */
	/* sound interface so it is easier to work with */
	/************************************************/
	// We will disable the divide if VIDEOPIN_OCTAVE_DATA = 0
	DISCRETE_DIVIDE(NODE_20, VIDEOPIN_OCTAVE_DATA, 12096000.0 /3.0 / 2.0, VIDEOPIN_OCTAVE_DATA)
	DISCRETE_ADDER2(NODE_21, 1, VIDEOPIN_NOTE_DATA, 1)
	// We will disable the divide if VIDEOPIN_NOTE_DATA = 0
	DISCRETE_DIVIDE(NODE_22, VIDEOPIN_NOTE_DATA, NODE_20, NODE_21)  // freq
	DISCRETE_SQUAREWAVE(VIDEOPIN_VOL_SND, VIDEOPIN_OCTAVE_DATA, NODE_22, VIDEOPIN_VOL_DATA, 50.0, 0, 0.0)

	/************************************************/
	/* Bong is just a triggered 32V signal          */
	/************************************************/
	DISCRETE_SQUAREWFIX(VIDEOPIN_BONG_SND, VIDEOPIN_BONG_EN, 15750.0/64.0, 1000.0, 50.0, 0, 0.0)

	/************************************************/
	/* Bell is Hsync/16 with an R/C decay amplitude */
	/* the 1uF cap is rapidally charged when BELL   */
	/* is enabled, then dischaged through the 1M    */
	/* resistor when disabled.                      */
	/* We use 180 phase because of inverter Q17,    */
	/* but it rally has no effect on sound.         */
	/************************************************/
	DISCRETE_RCDISC2(NODE_30, VIDEOPIN_BELL_EN, 740.2, 1, 0, 1e6, 1e-6)
	DISCRETE_SQUAREWFIX(VIDEOPIN_BELL_SND, VIDEOPIN_BELL_EN, 15750.0/16.0, NODE_30, 50.0, 0, 180.0)

	/************************************************/
	/* Final mix and output.                        */
	/************************************************/
	DISCRETE_ADDER3(NODE_90, VIDEOPIN_ATTRACT_EN, VIDEOPIN_VOL_SND, VIDEOPIN_BELL_SND, VIDEOPIN_BONG_SND)

	DISCRETE_OUTPUT(NODE_90, 65534.0/(1000.0 + 740.2 + 1000.0))
DISCRETE_SOUND_END
