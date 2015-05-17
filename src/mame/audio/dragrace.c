// license:???
// copyright-holders:Stefan Jokisch
/*************************************************************************

    audio\dragrace.c

*************************************************************************/
#include "emu.h"
#include "includes/dragrace.h"
#include "sound/discrete.h"

/************************************************************************/
/* dragrace Sound System Analog emulation                                  */
/************************************************************************/

static const discrete_lfsr_desc dragrace_lfsr =
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
#define DRAGRACE_NOISE          NODE_10
#define DRAGRACE_SCREECH1_SND   NODE_11
#define DRAGRACE_SCREECH2_SND   NODE_12
#define DRAGRACE_LOTONE_SND     NODE_13
#define DRAGRACE_HITONE_SND     NODE_14
#define DRAGRACE_TONE_SND       NODE_15
#define DRAGRACE_EXPLODE1_SND   NODE_16
#define DRAGRACE_EXPLODE2_SND   NODE_17
#define DRAGRACE_MOTOR1_SND     NODE_18
#define DRAGRACE_MOTOR2_SND     NODE_19

DISCRETE_SOUND_START(dragrace)
	/************************************************/
	/* dragrace  Effects Relataive Gain Table       */
	/*                                              */
	/* Effect  V-ampIn  Gain ratio        Relative  */
	/* LoTone   3.8     10/32               593.8   */
	/* HiTone   3.8     10/32               593.8   */
	/* Screech  3.8     10/330               57.6   */
	/* Motor    3.8     10/32.67            581.6   */
	/* Explode  5.0     10/25              1000.0   */
	/************************************************/


	/************************************************/
	/* Input register mapping for dragrace          */
	/************************************************/
	DISCRETE_INPUT_LOGIC(DRAGRACE_SCREECH1_EN)
	DISCRETE_INPUT_LOGIC(DRAGRACE_SCREECH2_EN)
	DISCRETE_INPUT_LOGIC(DRAGRACE_LOTONE_EN)
	DISCRETE_INPUT_LOGIC(DRAGRACE_HITONE_EN)
	DISCRETE_INPUT_LOGIC(DRAGRACE_EXPLODE1_EN)
	DISCRETE_INPUT_LOGIC(DRAGRACE_EXPLODE2_EN)
	DISCRETE_INPUT_DATA (DRAGRACE_MOTOR1_DATA)
	DISCRETE_INPUT_DATA (DRAGRACE_MOTOR2_DATA)
	DISCRETE_INPUT_LOGIC(DRAGRACE_MOTOR1_EN)
	DISCRETE_INPUT_LOGIC(DRAGRACE_MOTOR2_EN)
	DISCRETE_INPUT_LOGIC(DRAGRACE_KLEXPL1_EN)
	DISCRETE_INPUT_LOGIC(DRAGRACE_KLEXPL2_EN)
	DISCRETE_INPUT_LOGIC(DRAGRACE_ATTRACT_EN)

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
	/* to a 2.2uf capacitor on the R-ladder.        */
	/* Note the VCO freq. is controlled by a 250k   */
	/* pot.  The freq. used here is for the pot set */
	/* to 125k.  The low freq is allways the same.  */
	/* This adjusts the high end.                   */
	/* 0k = 214Hz.   250k = 4416Hz                  */
	/* NOTE: freqs are ripped from Sprint for now.  */
	/************************************************/
	DISCRETE_RCFILTER(NODE_20, DRAGRACE_MOTOR1_DATA, 119898, 2.2e-6)
	DISCRETE_ADJUSTMENT(NODE_21, (214.0-27.0)/12/31, (4416.0-27.0)/12/31, DISC_LOGADJ, "MOTOR1")
	DISCRETE_MULTIPLY(NODE_22, NODE_20, NODE_21)

	DISCRETE_MULTADD(NODE_23, NODE_22, 2, 27.0/6)   /* F1 = /12*2 = /6 */
	DISCRETE_SQUAREWAVE(NODE_24, 1, NODE_23, (581.6/3), 50.0, 0, 0)
	DISCRETE_RCFILTER(NODE_25, NODE_24, 10000, 1e-7)

	DISCRETE_MULTADD(NODE_26, NODE_22, 3, 27.0/4)   /* F2 = /12*3 = /4 */
	DISCRETE_SQUAREWAVE(NODE_27, 1, NODE_26, (581.6/3), 50.0, 0, 0)
	DISCRETE_RCFILTER(NODE_28, NODE_27, 10000, 1e-7)

	DISCRETE_MULTADD(NODE_29, NODE_22, 4, 27.0/3)   /* F3 = /12*4 = /3 */
	DISCRETE_SQUAREWAVE(NODE_30, 1, NODE_29, (581.6/3), 100.0/3, 0, 360.0/3)
	DISCRETE_RCFILTER(NODE_31, NODE_30, 10000, 1e-7)

	DISCRETE_ADDER3(DRAGRACE_MOTOR1_SND, DRAGRACE_MOTOR1_EN, NODE_25, NODE_28, NODE_31)

	/************************************************/
	/* Car2 motor sound is basically the same as    */
	/* Car1.  But I shifted the frequencies up for  */
	/* it to sound different from car1.             */
	/************************************************/
	DISCRETE_RCFILTER(NODE_40, DRAGRACE_MOTOR2_DATA, 119898, 2.2e-6)
	DISCRETE_ADJUSTMENT(NODE_41, (214.0-27.0)/12/31, (4416.0-27.0)/12/31, DISC_LOGADJ, "MOTOR2")
	DISCRETE_MULTIPLY(NODE_42, NODE_40, NODE_41)

	DISCRETE_MULTADD(NODE_43, NODE_42, 2, 27.0/6)   /* F1 = /12*2 = /6 */
	DISCRETE_SQUAREWAVE(NODE_44, 1, NODE_43, (581.6/3), 50.0, 0, 0)
	DISCRETE_RCFILTER(NODE_45, NODE_44, 10000, 1e-7)

	DISCRETE_MULTADD(NODE_46, NODE_42, 3, 27.0/4)   /* F2 = /12*3 = /4 */
	DISCRETE_SQUAREWAVE(NODE_47, 1, NODE_46, (581.6/3), 50.0, 0, 0)
	DISCRETE_RCFILTER(NODE_48, NODE_47, 10000, 1e-7)

	DISCRETE_MULTADD(NODE_49, NODE_42, 4, 27.0/3)   /* F3 = /12*4 = /3 */
	DISCRETE_SQUAREWAVE(NODE_50, 1, NODE_49, (581.6/3), 100.0/3, 0, 360.0/3)
	DISCRETE_RCFILTER(NODE_51, NODE_50, 10000, 1e-7)

	DISCRETE_ADDER3(DRAGRACE_MOTOR2_SND, DRAGRACE_MOTOR2_EN, NODE_45, NODE_48, NODE_51)

	/************************************************/
	/* Explosion circuit is built around a noise    */
	/* generator built from 2 shift registers that  */
	/* are clocked by the 1V signal.                */
	/* 1V = HSYNC/2                                 */
	/*    = 15750/2                                 */
	/* Output is integrated to apply decay.         */
	/************************************************/
	DISCRETE_LFSR_NOISE(DRAGRACE_NOISE, 1, DRAGRACE_ATTRACT_EN, 15750.0/2, 1.0, 0, 0, &dragrace_lfsr)

	DISCRETE_RAMP(NODE_61, DRAGRACE_EXPLODE1_EN, DRAGRACE_EXPLODE1_EN, (1000.0-0.0)/1, 1000.0, 0.0, 1000.0)
	DISCRETE_MULTIPLY(NODE_62, DRAGRACE_NOISE, NODE_61)
	DISCRETE_RCFILTER(NODE_63, NODE_62, 1500, 2.2e-7)
	DISCRETE_MULTIPLY(DRAGRACE_EXPLODE1_SND, NODE_63, DRAGRACE_KLEXPL1_EN)

	DISCRETE_RAMP(NODE_66, DRAGRACE_EXPLODE2_EN, DRAGRACE_EXPLODE2_EN, (1000.0-0.0)/1, 1000.0, 0.0, 1000.0)
	DISCRETE_MULTIPLY(NODE_67, DRAGRACE_NOISE, NODE_66)
	DISCRETE_RCFILTER(NODE_68, NODE_67, 1500, 2.2e-7)
	DISCRETE_MULTIPLY(DRAGRACE_EXPLODE2_SND, NODE_68, DRAGRACE_KLEXPL2_EN)

	/************************************************/
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
	DISCRETE_INVERT(NODE_70, DRAGRACE_NOISE)
	DISCRETE_MULTADD(NODE_71, NODE_70, 940.0-630.0, ((940.0-630.0)/2)+630.0)
	DISCRETE_RCFILTER(NODE_72, NODE_71, 1000, 1e-5)
	DISCRETE_SQUAREWAVE(NODE_73, 1, NODE_72, 407.8, 31.5, 0, 0.0)
	DISCRETE_ONOFF(DRAGRACE_SCREECH1_SND, DRAGRACE_SCREECH1_EN, NODE_73)
	DISCRETE_ONOFF(DRAGRACE_SCREECH2_SND, DRAGRACE_SCREECH2_EN, NODE_73)

	/************************************************/
	/* LoTone = 32V = 15750Hz/2/32                  */
	/* HiTone =  4V = 15750Hz/2/4                   */
	/************************************************/
	DISCRETE_SQUAREWFIX(DRAGRACE_LOTONE_SND, DRAGRACE_LOTONE_EN, 15750.0/2/32,  593.8, 50.0, 0, 0.0)
	DISCRETE_SQUAREWFIX(DRAGRACE_HITONE_SND, DRAGRACE_HITONE_EN, 15750.0/2/4,   593.8, 50.0, 0, 0.0)
	DISCRETE_ADDER2(DRAGRACE_TONE_SND, 1, DRAGRACE_LOTONE_SND, DRAGRACE_HITONE_SND)

	/************************************************/
	/* Combine all 5 sound sources.                 */
	/* Add some final gain to get to a good sound   */
	/* level.                                       */
	/************************************************/
	DISCRETE_ADDER4(NODE_90, DRAGRACE_ATTRACT_EN, DRAGRACE_TONE_SND, DRAGRACE_MOTOR1_SND, DRAGRACE_EXPLODE1_SND, DRAGRACE_SCREECH1_SND)
	DISCRETE_ADDER4(NODE_91, DRAGRACE_ATTRACT_EN, DRAGRACE_TONE_SND, DRAGRACE_MOTOR2_SND, DRAGRACE_EXPLODE2_SND, DRAGRACE_SCREECH2_SND)

	DISCRETE_OUTPUT(NODE_90, 65534.0/(593.8+581.6+1000.0+57.6))
	DISCRETE_OUTPUT(NODE_91, 65534.0/(593.8+581.6+1000.0+57.6))
DISCRETE_SOUND_END
