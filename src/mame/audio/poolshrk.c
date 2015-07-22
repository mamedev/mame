// license:???
// copyright-holders:Derrick Renaud
/*************************************************************************

    audio\poolshrk.c

*************************************************************************/
#include "emu.h"
#include "includes/poolshrk.h"
#include "sound/discrete.h"

/************************************************************************/
/* poolshrk Sound System Analog emulation                               */
/* Jan 2004, Derrick Renaud                                             */
/************************************************************************/
static const discrete_dac_r1_ladder poolshrk_score_v_dac =
{
	4,      // size of ladder
	{220000, 470000, 1000000, 2200000, 0,0,0,0},    // R57 - R60
	5,      // vBias
	100000,     // R61
	1500000,    // R100
	1.e-5       // C13
};

static const discrete_555_cc_desc poolshrk_score_vco =
{
	DISC_555_OUT_SQW,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES,
	0.7     // Q3 junction voltage
};

static const discrete_mixer_desc poolshrk_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{330000, 330000, 330000, 82000 + 470},  // R77, R75, R74, R76 + R53
	{0},                // No variable resistor nodes
	{0},                // No caps
	0,                  // No rI
	1000,               // R78
	0,                  // No filtering
	1e-7,               // C21
	0,                  // vBias not used for resistor network
	1000000
};

/* Nodes - Inputs */
#define POOLSHRK_BUMP_EN    NODE_01
#define POOLSHRK_CLICK_EN   NODE_02
#define POOLSHRK_SCORE_EN   NODE_03
/* Nodes - Sounds */
#define POOLSHRK_BUMP_SND       NODE_10
#define POOLSHRK_SCRATCH_SND    NODE_11
#define POOLSHRK_CLICK_SND      NODE_12
#define POOLSHRK_SCORE_SND      NODE_13

DISCRETE_SOUND_START(poolshrk)
	/************************************************/
	/* Input register mapping for poolshrk          */
	/************************************************/
	/*                    NODE                   GAIN    OFFSET  INIT */
	DISCRETE_INPUT_LOGIC (POOLSHRK_BUMP_EN)
	DISCRETE_INPUTX_LOGIC(POOLSHRK_SCRATCH_SND,  3.4,    0,      0.0)
	DISCRETE_INPUT_PULSE (POOLSHRK_CLICK_EN, 0)
	DISCRETE_INPUT_PULSE (POOLSHRK_SCORE_EN, 0)
	/************************************************/

	/************************************************/
	/* Scratch is just the trigger sent directly    */
	/* to the output.  We take care of it's         */
	/* amplitude right in it's DISCRETE_INPUTX.     */
	/************************************************/

	/************************************************/
	/* Bump is just a triggered 128V signal         */
	/************************************************/
	DISCRETE_SQUAREWFIX(NODE_20, POOLSHRK_BUMP_EN, 15750.0/2.0/128.0, 3.4, 50.0, 3.4/2, 0.0)    // 128V signal 3.4V
	DISCRETE_RCFILTER(POOLSHRK_BUMP_SND, NODE_20, 470, 4.7e-6)  // Filtered by R53/C14

	/************************************************/
	/* Score is a triggered 0-15 count of the       */
	/* 64V signal.  This then sets the frequency of */
	/* the 555 timer (C9).  The final signal is /2  */
	/* to set a 50% duty cycle.                     */
	/* NOTE: when first powered up the counter is   */
	/* not at TC, so the score is counted once.     */
	/* But because C13 is not charged, it limits    */
	/* C16 voltage, causing the 555 timer (C9) to   */
	/* not oscillate.                               */
	/* This should also happen on the original PCB. */
	/************************************************/
	DISCRETE_COUNTER(NODE_30,               // Counter E8 (9316 is a 74161)
						NODE_31,                // Clock enabled by F8, pin 13
						POOLSHRK_SCORE_EN,      // Reset/triggered by score
						15750.0/2.0/64.0,       // 64V signal
						0, 15, 1,               // 4 bit binary up counter
						0, DISC_CLK_IS_FREQ)    // Cleared to 0
	DISCRETE_TRANSFORM2(NODE_31, NODE_30, 15, "01=!")   // TC output of E8, pin 15. (inverted)

	DISCRETE_DAC_R1(NODE_32,    // Base of Q3
			NODE_30,    // IC E8, Q0-Q3
			3.4,        // TTL ON level = 3.4V
			&poolshrk_score_v_dac)
	DISCRETE_555_CC(NODE_33,    // IC C9, pin 3
			NODE_31,    // reset by IC C9, pin 4
			NODE_32,    // vIn from R-ladder
			10000,      // R73
			1.e-8,      // C16
			0, 0, 0,    // No rBias, rGnd or rDischarge
			&poolshrk_score_vco)
	DISCRETE_COUNTER(NODE_34, 1, 0, // IC D9, pin 9
			NODE_33,                // from IC C9, pin 3
			0, 1, 1, 0, DISC_CLK_ON_R_EDGE) // /2 counter on rising edge
	DISCRETE_GAIN(POOLSHRK_SCORE_SND, NODE_34, 3.4)


	/*
	 * The TC output of E8 is sent to a one shot made up by
	 * C12/R62.  Clamped by CR16. Shaped to square by L9.
	 * This causes click to be triggered at the end of score.
	 */
	DISCRETE_ONESHOT(NODE_39,   // buffer L9 pin 12
				NODE_31,    // from TC pin 15 of E8
				1, 0,       // output 0/1 for the minimum sample period
				DISC_ONESHOT_FEDGE | DISC_ONESHOT_NORETRIG | DISC_OUT_ACTIVE_HIGH)  // Real circuit is rising edge but we will take into account that we are using an inverted signal in the code

	/************************************************/
	/* Click is a triggered 0-15 count of the       */
	/* 2V signal.  It is also triggered at the end  */
	/* of the score sound.                          */
	/* NOTE: when first powered up the counter is   */
	/* not at TC, so the click is counted once.     */
	/* This should also happen on the original PCB. */
	/************************************************/
	DISCRETE_LOGIC_OR(NODE_40, POOLSHRK_CLICK_EN , NODE_39) // gate K9, pin 11
	DISCRETE_COUNTER(NODE_41,               // Counter J9 (9316 is a 74161)
						NODE_42,                // Clock enabled by F8, pin 1
						NODE_40,                // Reset/triggered by K9, pin 11
						15750.0/2.0/2.0,        // 2V signal
						0, 15, 1,               // 4 bit binary up counter
						0, DISC_CLK_IS_FREQ)    // Cleared to 0
	DISCRETE_TRANSFORM2(NODE_42, NODE_41, 15, "01=!")   // TC output of J9, pin 15. Modified to function as F8 clock enable
	DISCRETE_TRANSFORM3(POOLSHRK_CLICK_SND, NODE_41, 1, 3.4, "01&2*")   // Q0 output of J9, pin 14.  Set to proper amplitude

	/************************************************/
	/* Final mix and output.                        */
	/************************************************/
	DISCRETE_MIXER4(NODE_90, 1, POOLSHRK_SCRATCH_SND, POOLSHRK_CLICK_SND, POOLSHRK_SCORE_SND, POOLSHRK_BUMP_SND, &poolshrk_mixer)
	DISCRETE_OUTPUT(NODE_90, 1)
DISCRETE_SOUND_END


/*************************************
 *
 *  Write handlers
 *
 *************************************/

WRITE8_MEMBER(poolshrk_state::scratch_sound_w)
{
	m_discrete->write(space, POOLSHRK_SCRATCH_SND, offset & 1);
}

WRITE8_MEMBER(poolshrk_state::score_sound_w)
{
	m_discrete->write(space, POOLSHRK_SCORE_EN, 1); /* this will trigger the sound code for 1 sample */
}

WRITE8_MEMBER(poolshrk_state::click_sound_w)
{
	m_discrete->write(space, POOLSHRK_CLICK_EN, 1); /* this will trigger the sound code for 1 sample */
}

WRITE8_MEMBER(poolshrk_state::bump_sound_w)
{
	m_discrete->write(space, POOLSHRK_BUMP_EN, offset & 1);
}
