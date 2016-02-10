// license:BSD-3-Clause
// copyright-holders:Derrick Renaud
/*************************************************************************

    audio\nitedrvr.c

*************************************************************************/
#include "emu.h"
#include "includes/nitedrvr.h"
#include "sound/discrete.h"


/* Discrete Sound Emulation */

static const discrete_lfsr_desc nitedrvr_lfsr =
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
#define NITEDRVR_NOISE          NODE_10
#define NITEDRVR_BANG_SND       NODE_11
#define NITEDRVR_MOTOR_SND      NODE_12
#define NITEDRVR_SCREECH1_SND   NODE_13
#define NITEDRVR_SCREECH2_SND   NODE_14

DISCRETE_SOUND_START(nitedrvr)
	/************************************************/
	/* nitedrvr  Effects Relataive Gain Table       */
	/*                                              */
	/* Effect    V-ampIn  Gain ratio      Relative  */
	/* Bang       3.8     5/(5+6.8)        1000.0   */
	/* Motor      3.8     5/(5+10)          786.7   */
	/* Screech1   3.8     5/(5+47)          226.9   */
	/* Screech2   3.8     5/(5+47)          226.9   */
	/************************************************/

	/************************************************/
	/* Input register mapping for nitedrvr          */
	/************************************************/
	/*                    NODE                GAIN      OFFSET  INIT */
	DISCRETE_INPUTX_DATA (NITEDRVR_BANG_DATA, 1000.0/15, 0.0,   0.0)
	DISCRETE_INPUT_LOGIC (NITEDRVR_SKID1_EN)
	DISCRETE_INPUT_LOGIC (NITEDRVR_SKID2_EN)
	DISCRETE_INPUTX_DATA (NITEDRVR_MOTOR_DATA, -1, 0x0f, 0)
	DISCRETE_INPUT_LOGIC (NITEDRVR_CRASH_EN)
	DISCRETE_INPUT_NOT   (NITEDRVR_ATTRACT_EN)

	/************************************************/
	/* NOTE: Motor circuit is a rip from Sprint for */
	/* now. This will have to be converted to 566.  */
	/*                                              */
	/* Motor sound circuit is based on a 556 VCO    */
	/* with the input frequency set by the MotorSND */
	/* latch (4 bit). This freqency is then used to */
	/* driver a modulo 12 counter, with div6, 4 & 3 */
	/* summed as the output of the circuit.         */
	/* VCO Output is Sq wave = 27-382Hz             */
	/*  F1 freq - (Div6)                            */
	/*  F2 freq = (Div4)                            */
	/*  F3 freq = (Div3) 33.3% duty, 33.3 deg phase */
	/* To generate the frequency we take the freq.  */
	/* diff. and /15 to get all the steps between   */
	/* 0 - 15.  Then add the low frequency and send */
	/* that value to a squarewave generator.        */
	/* Also as the frequency changes, it ramps due  */
	/* to a 2.2uf capacitor on the R-ladder.        */
	/* Note the VCO freq. is controlled by a 250k   */
	/* pot.  The freq. used here is for the pot set */
	/* to 125k.  The low freq is allways the same.  */
	/* This adjusts the high end.                   */
	/* 0k = 214Hz.   250k = 4416Hz                  */
	/************************************************/
	DISCRETE_RCFILTER(NODE_20, NITEDRVR_MOTOR_DATA, 123037, 2.2e-6)
	DISCRETE_ADJUSTMENT(NODE_21, (214.0-27.0)/12/31, (4416.0-27.0)/12/31, DISC_LOGADJ, "MOTOR")
	DISCRETE_MULTIPLY(NODE_22, NODE_20, NODE_21)

	DISCRETE_MULTADD(NODE_23, NODE_22, 2, 27.0/6)   /* F1 = /12*2 = /6 */
	DISCRETE_SQUAREWAVE(NODE_24, 1, NODE_23, (786.7/3), 50.0, 0, 0)
	DISCRETE_RCFILTER(NODE_25, NODE_24, 10000, 1e-7)

	DISCRETE_MULTADD(NODE_26, NODE_22, 3, 27.0/4)   /* F2 = /12*3 = /4 */
	DISCRETE_SQUAREWAVE(NODE_27, 1, NODE_26, (786.7/3), 50.0, 0, 0)
	DISCRETE_RCFILTER(NODE_28, NODE_27, 10000, 1e-7)

	DISCRETE_MULTADD(NODE_29, NODE_22, 4, 27.0/3)   /* F3 = /12*4 = /3 */
	DISCRETE_SQUAREWAVE(NODE_30, 1, NODE_29, (786.7/3), 100.0/3, 0, 360.0/3)
	DISCRETE_RCFILTER(NODE_31, NODE_30, 10000, 1e-7)

	DISCRETE_ADDER3(NITEDRVR_MOTOR_SND, NITEDRVR_ATTRACT_EN, NODE_25, NODE_28, NODE_31)

	/************************************************/
	/* Bang circuit is built around a noise         */
	/* generator built from 2 shift registers that  */
	/* are clocked by the 4V signal.                */
	/* 4V = HSYNC/2/4                               */
	/*    = 15750/2/4                               */
	/* Output is integrated to apply decay.         */
	/************************************************/
	DISCRETE_LFSR_NOISE(NITEDRVR_NOISE, NITEDRVR_CRASH_EN, NITEDRVR_CRASH_EN, 15750.0/2/4, 1.0, 0, 0, &nitedrvr_lfsr)

	DISCRETE_MULTIPLY(NODE_62, NITEDRVR_NOISE, NITEDRVR_BANG_DATA)
	DISCRETE_RCFILTER(NITEDRVR_BANG_SND, NODE_62, 545.6, 3.3e-7)

	/************************************************/
	/* Skid circuits ripped from other drivers      */
	/* for now.                                     */
	/*                                              */
	/* Skid circuits takes the noise output from    */
	/* the crash circuit and applies +ve feedback   */
	/* to cause oscillation. There is also an RC    */
	/* filter on the input to the feedback cct.     */
	/* RC is 1K & 10uF                              */
	/* Feedback cct is modelled by using the RC out */
	/* as the frequency input on a VCO,             */
	/* breadboarded freq range as:                  */
	/*  0 = 940Hz, 34% duty                         */
	/*  1 = 630Hz, 29% duty                         */
	/*  the duty variance is so small we ignore it  */
	/************************************************/
	DISCRETE_INVERT(NODE_70, NITEDRVR_NOISE)
	DISCRETE_RCFILTER(NODE_71, NODE_70, 1000, 1e-5)
	DISCRETE_MULTADD(NODE_72, NODE_71, 940.0-630.0, ((940.0-630.0)/2)+630.0)
	DISCRETE_SQUAREWAVE(NITEDRVR_SCREECH1_SND, NITEDRVR_SKID1_EN, NODE_72, 226.9, 31.5, 0, 0.0)

	DISCRETE_MULTADD(NODE_75, NODE_71, 1380.0-626.0, 626.0+((1380.0-626.0)/2))  /* Frequency */
	DISCRETE_MULTADD(NODE_76, NODE_71, 32.0-13.0, 13.0+((32.0-13.0)/2))     /* Duty */
	DISCRETE_SQUAREWAVE(NITEDRVR_SCREECH2_SND, NITEDRVR_SKID2_EN, NODE_75, 226.9, NODE_76, 0, 0.0)

	/************************************************/
	/* Combine all sound sources.                   */
	/* Add some final gain to get to a good sound   */
	/* level.                                       */
	/************************************************/
	DISCRETE_ADDER4(NODE_90, 1, NITEDRVR_MOTOR_SND, NITEDRVR_BANG_SND, NITEDRVR_SCREECH1_SND, NITEDRVR_SCREECH2_SND)

	DISCRETE_OUTPUT(NODE_90, 65534.0/(786.7+1000.0+226.9))
DISCRETE_SOUND_END
