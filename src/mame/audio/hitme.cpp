// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    audio\hitme.c

*************************************************************************/
#include "emu.h"
#include "includes/hitme.h"
#include "sound/discrete.h"


static const discrete_555_desc desc_hitme_555 =
{
	DISC_555_OUT_ENERGY | DISC_555_OUT_DC,
	5,              // B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_comp_adder_table desc_hitme_adder =
{
	DISC_COMP_P_CAPACITOR, 0, 5,
	{
		0.100e-6,   // C19
		0.022e-6,   // C18
		0.033e-6,   // C17
		0.010e-6,   // C16
		0.005e-6    // C15
	}
};

/* Nodes - Adjustment */
#define HITME_GAME_SPEED        NODE_05
/* Nodes - Sounds */
#define HITME_FINAL_SND         NODE_90


DISCRETE_SOUND_START(hitme)

	/* These are the inputs; PULSE-type inputs are used for oneshot latching signals */
	DISCRETE_INPUT_DATA (HITME_DOWNCOUNT_VAL)
	DISCRETE_INPUT_PULSE(HITME_OUT0, 0)
	DISCRETE_INPUT_DATA (HITME_ENABLE_VAL)
	DISCRETE_INPUT_PULSE(HITME_OUT1, 0)

	/* This represents the resistor at R3, which controls the speed of the sound effects */
	DISCRETE_ADJUSTMENT(HITME_GAME_SPEED,0.0,25000.0,DISC_LINADJ,"R3")

	/* The clock for the main downcounter is a "404", or LS123 retriggerable multivibrator.
	 * It is clocked by IPH2 (8.945MHz/16 = 559kHz), then triggers a pulse which is adjustable
	 * via the resistor R3. When the pulse is finished, it immediately retriggers itself to
	 * form a clock. The length of the clock pulse is 0.45*R*C, where R is the variable R3
	 * resistor value, and C is 6.8uF. Thus the frequency of the resulting wave is
	 * 1.0/(0.45*R*C). We compute that frequency and use a standard 50% duty cycle square wave.
	 * This is because the "off time" of the clock is very small (559kHz), and we will miss
	 * edges if we model it perfectly accurately. */
	DISCRETE_TRANSFORM3(NODE_16,1,0.45*6.8e-6,HITME_GAME_SPEED,"012*/")
	DISCRETE_SQUAREWAVE(NODE_17,1,NODE_16,1,50,0.5,0)

	/* There are 2 cascaded 4-bit downcounters (2R = low, 2P = high), effectively
	 * making an 8-bit downcounter, clocked by the clock from the 404 chip.
	 * The initial count is latched by writing OUT0. */
	DISCRETE_COUNTER(NODE_20,1,HITME_OUT0,NODE_17,0,255,0,HITME_DOWNCOUNT_VAL,DISC_CLK_ON_F_EDGE)
	/* When the counter rolls over from 0->255, we clock a D-type flipflop at 2N. */
	DISCRETE_TRANSFORM2(NODE_21,NODE_20,255,"01=!")

	/* This flipflop represents the latch at 1L. It is clocked when OUT1 is written and latches
	 * the value from the processor. When the downcounter above rolls over, it clears the latch. */
	DISCRETE_LOGIC_DFLIPFLOP(NODE_22,NODE_21,1,HITME_OUT1,HITME_ENABLE_VAL)

	/* The output of the latch goes through a series of various capacitors in parallel. */
	DISCRETE_COMP_ADDER(NODE_23,NODE_22,&desc_hitme_adder)

	/* The combined capacitance is input to a 555 timer in astable mode. */
	DISCRETE_555_ASTABLE(NODE_24,1,22e3,39e3,NODE_23,&desc_hitme_555)

	/* The output of the 555 timer is fed through a simple CR filter in the amp stage. */
	DISCRETE_CRFILTER(HITME_FINAL_SND,NODE_24,1e3,50e-6)

	/* We scale the final output of 3.8 to 16-bit range and output it at full volume */
	DISCRETE_OUTPUT(HITME_FINAL_SND,32000.0/3.8)
DISCRETE_SOUND_END
