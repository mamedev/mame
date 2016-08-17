// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    audio\canyon.c

*************************************************************************/
#include "emu.h"
#include "includes/canyon.h"
#include "sound/discrete.h"


/*************************************
 *
 *  Write handlers
 *
 *************************************/

WRITE8_MEMBER(canyon_state::canyon_motor_w)
{
	m_discrete->write(space, NODE_RELATIVE(CANYON_MOTOR1_DATA, (offset & 0x01)), data & 0x0f);
}


WRITE8_MEMBER(canyon_state::canyon_explode_w)
{
	m_discrete->write(space, CANYON_EXPLODE_DATA, data >> 4);
}


WRITE8_MEMBER(canyon_state::canyon_attract_w)
{
	m_discrete->write(space, NODE_RELATIVE(CANYON_ATTRACT1_EN, (offset & 0x01)), offset & 0x02);
}


WRITE8_MEMBER(canyon_state::canyon_whistle_w)
{
	m_discrete->write(space, NODE_RELATIVE(CANYON_WHISTLE1_EN, (offset & 0x01)), offset & 0x02);
}


/************************************************************************/
/* canyon Sound System Analog emulation                                 */
/************************************************************************/

static const discrete_555_desc canyonWhistl555 =
{
	DISC_555_OUT_CAP | DISC_555_OUT_AC,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_lfsr_desc canyon_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,                 /* Bit Length */
	0,                  /* Reset Value */
	6,                  /* Use Bit 6 as XOR input 0 */
	8,                  /* Use Bit 8 as XOR input 1 */
	DISC_LFSR_XNOR,     /* Feedback stage1 is XNOR */
	DISC_LFSR_OR,       /* Feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,  /* Feedback stage3 replaces the shifted register contents */
	0x000001,           /* Everything is shifted into the first bit only */
	0,                  /* Output is not inverted, Active Low Reset */
	15                  /* Output bit */
};

/* Nodes - Sounds */
#define CANYON_MOTORSND1        NODE_10
#define CANYON_MOTORSND2        NODE_11
#define CANYON_EXPLODESND       NODE_12
#define CANYON_WHISTLESND1      NODE_13
#define CANYON_WHISTLESND2      NODE_14
#define CANYON_NOISE            NODE_15

DISCRETE_SOUND_START(canyon)
	/************************************************/
	/* Canyon sound system: 5 Sound Sources         */
	/*                     Relative Volume          */
	/*    1/2) Motor           14.29%               */
	/*      3) Explode        100.00%               */
	/*    4/5) Whistle         51.94%               */
	/* Relative volumes calculated from resitor     */
	/* network in combiner circuit taking voltages  */
	/* into account                                 */
	/*                                              */
	/* Motor   3.8V * 5/(5+100) = 0.1810            */
	/* Explode 3.8V * 5/(5+10)  = 1.2667            */
	/* Whistle 5.0V * 5/(5+33)  = 0.6579            */
	/*                                              */
	/************************************************/

	/************************************************/
	/* Input register mapping for canyon            */
	/************************************************/
	DISCRETE_INPUTX_DATA (CANYON_MOTOR1_DATA , -1, 0x0f, 0)
	DISCRETE_INPUTX_DATA (CANYON_MOTOR2_DATA , -1, 0x0f, 0)
	DISCRETE_INPUT_LOGIC (CANYON_WHISTLE1_EN)
	DISCRETE_INPUT_LOGIC (CANYON_WHISTLE2_EN)
	DISCRETE_INPUT_NOT   (CANYON_ATTRACT1_EN)
	DISCRETE_INPUT_NOT   (CANYON_ATTRACT2_EN)
	DISCRETE_INPUTX_DATA (CANYON_EXPLODE_DATA, 1000.0/15.0, 0,     0.0)

	/************************************************/
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
	/* to a 1uf capacitor on the R-ladder.          */
	/* Note the VCO freq. is controlled by a 250k   */
	/* pot.  The freq. used here is for the pot set */
	/* to 125k.  The low freq is allways the same.  */
	/* This adjusts the high end.                   */
	/* 0k = 214Hz.   250k = 4416Hz                  */
	/************************************************/
	DISCRETE_RCFILTER(NODE_20, CANYON_MOTOR1_DATA, 123000, 1e-6)
	DISCRETE_ADJUSTMENT(NODE_21, (214.0-27.0)/12/15, (4416.0-27.0)/12/15, DISC_LOGADJ, "MOTOR1")
	DISCRETE_MULTIPLY(NODE_22, NODE_20, NODE_21)

	DISCRETE_MULTADD(NODE_23, NODE_22, 2, 27.0/6)   /* F1 = /12*2 = /6 */
	DISCRETE_SQUAREWAVE(NODE_24, 1, NODE_23, (142.9/3), 50.0, 0, 0)
	DISCRETE_RCFILTER(NODE_25, NODE_24, 10000, 1e-7)

	DISCRETE_MULTADD(NODE_26, NODE_22, 3, 27.0/4)   /* F2 = /12*3 = /4 */
	DISCRETE_SQUAREWAVE(NODE_27, 1, NODE_26, (142.9/3), 50.0, 0, 0)
	DISCRETE_RCFILTER(NODE_28, NODE_27, 10000, 1e-7)

	DISCRETE_MULTADD(NODE_29, NODE_22, 4, 27.0/3)   /* F3 = /12*4 = /3 */
	DISCRETE_SQUAREWAVE(NODE_30, 1, NODE_29, (142.9/3), 100.0/3, 0, 360.0/3)
	DISCRETE_RCFILTER(NODE_31, NODE_30, 10000, 1e-7)

	DISCRETE_ADDER3(CANYON_MOTORSND1, CANYON_ATTRACT1_EN, NODE_25, NODE_28, NODE_31)

	/************************************************/
	/* The motor2 sound is basically the same as    */
	/* for 1.  But I shifted the frequencies up for */
	/* it to sound different from motor 1.          */
	/************************************************/
	DISCRETE_RCFILTER(NODE_40, CANYON_MOTOR2_DATA, 123000, 1e-6)
	DISCRETE_ADJUSTMENT(NODE_41, (214.0-27.0)/12/15, (4416.0-27.0)/12/15, DISC_LOGADJ, "MOTOR2")
	DISCRETE_MULTIPLY(NODE_42, NODE_40, NODE_41)

	DISCRETE_MULTADD(NODE_43, NODE_42, 2, 27.0/6)   /* F1 = /12*2 = /6 */
	DISCRETE_SQUAREWAVE(NODE_44, 1, NODE_43, (142.9/3), 50.0, 0, 0)
	DISCRETE_RCFILTER(NODE_45, NODE_44, 10000, 1e-7)

	DISCRETE_MULTADD(NODE_46, NODE_42, 3, 27.0/4)   /* F2 = /12*3 = /4 */
	DISCRETE_SQUAREWAVE(NODE_47, 1, NODE_46, (142.9/3), 50.0, 0, 0)
	DISCRETE_RCFILTER(NODE_48, NODE_47, 10000, 1e-7)

	DISCRETE_MULTADD(NODE_49, NODE_42, 4, 27.0/3)   /* F3 = /12*4 = /3 */
	DISCRETE_SQUAREWAVE(NODE_50, 1, NODE_49, (142.9/3), 100.0/3, 0, 360.0/3)
	DISCRETE_RCFILTER(NODE_51, NODE_50, 10000, 1e-7)

	DISCRETE_ADDER3(CANYON_MOTORSND2, CANYON_ATTRACT2_EN, NODE_45, NODE_48, NODE_51)

	/************************************************/
	/* Explode circuit is built around a noise      */
	/* generator built from 2 shift registers that  */
	/* are clocked by the 2V signal.                */
	/* 2V = HSYNC/4                                 */
	/*    = 15750/4                                 */
	/* Output is binary weighted with 4 bits of     */
	/* crash volume.                                */
	/************************************************/
	DISCRETE_LOGIC_OR(NODE_60, CANYON_ATTRACT1_EN, CANYON_ATTRACT2_EN)
	DISCRETE_LFSR_NOISE(CANYON_NOISE, NODE_60, NODE_60, 15750.0/4, 1.0, 0, 0, &canyon_lfsr)

	DISCRETE_MULTIPLY(NODE_61, CANYON_NOISE, CANYON_EXPLODE_DATA)
	DISCRETE_RCFILTER(CANYON_EXPLODESND, NODE_61, 545, 5e-6)

	/************************************************/
	/* Whistle circuit is a 555 capacitor charge    */
	/* waveform.  The original game pot varies from */
	/* 0-100k, but we are going to limit it because */
	/* below 50k the frequency is too high.         */
	/* When triggered it starts at it's highest     */
	/* frequency, then decays at the rate set by    */
	/* a 68k resistor and 22uf capacitor.           */
	/************************************************/
	DISCRETE_ADJUSTMENT(NODE_70, 50000, 100000, DISC_LINADJ, "WHISTLE1")    /* R59 */
	DISCRETE_MULTADD(NODE_71, CANYON_WHISTLE1_EN, 3.05-0.33, 0.33)
	DISCRETE_RCDISC2(NODE_72, CANYON_WHISTLE1_EN, NODE_71, 1.0, NODE_71, 68000.0, 2.2e-5)   /* CV */
	DISCRETE_555_ASTABLE_CV(NODE_73, CANYON_WHISTLE1_EN, 33000, NODE_70, 1e-8, NODE_72, &canyonWhistl555)
	DISCRETE_MULTIPLY(CANYON_WHISTLESND1, NODE_73, 519.4/3.3)

	DISCRETE_ADJUSTMENT(NODE_75, 50000, 100000, DISC_LINADJ, "WHISTLE2")    /* R69 */
	DISCRETE_MULTADD(NODE_76, CANYON_WHISTLE2_EN, 3.05-0.33, 0.33)
	DISCRETE_RCDISC2(NODE_77, CANYON_WHISTLE2_EN, NODE_76, 1.0, NODE_76, 68000.0, 2.2e-5)   /* CV */
	DISCRETE_555_ASTABLE_CV(NODE_78, CANYON_WHISTLE2_EN, 33000, NODE_75, 1e-8, NODE_77, &canyonWhistl555)
	DISCRETE_MULTIPLY(CANYON_WHISTLESND2, NODE_78, 519.4/3.3)

	/************************************************/
	/* Combine all 5 sound sources.                 */
	/* Add some final gain to get to a good sound   */
	/* level.                                       */
	/************************************************/
	DISCRETE_ADDER3(NODE_90, 1, CANYON_MOTORSND1, CANYON_EXPLODESND, CANYON_WHISTLESND1)
	DISCRETE_ADDER3(NODE_91, 1, CANYON_MOTORSND2, CANYON_EXPLODESND, CANYON_WHISTLESND2)

	DISCRETE_OUTPUT(NODE_90, 77)
	DISCRETE_OUTPUT(NODE_91, 77)
DISCRETE_SOUND_END
