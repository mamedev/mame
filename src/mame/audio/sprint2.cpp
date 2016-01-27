// license:BSD-3-Clause
// copyright-holders:Hans Andersson
/*************************************************************************

    audio\sprint2.c

*************************************************************************/
#include "emu.h"
#include "includes/sprint2.h"


/************************************************************************/
/* Sprint2 / Sprint1 Sound System Analog emulation                      */
/* Rewritten by Hans Andersson. March 2005                              */
/************************************************************************/

static const discrete_lfsr_desc sprint2_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,                  /* Bit Length */
	0,                   /* Reset Value */
	0,                   /* Use Bit 0 as XOR input 0 */
	14,                  /* Use Bit 14 as XOR input 1 */
	DISC_LFSR_XNOR,      /* Feedback stage1 is XNOR */
	DISC_LFSR_OR,        /* Feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,   /* Feedback stage3 replaces the shifted register contents */
	0x000001,            /* Everything is shifted into the first bit only */
	0,                   /* Output is not inverted */
	15                   /* Output bit */
};

static const discrete_dac_r1_ladder sprint2_motor_v_dac =
{
	4,          // size of ladder
	{RES_M(2.2), RES_M(1), RES_K(470), RES_K(220)}, // R5, R6, R7, R8
	4.4,        // 5V minus diode junction (0.6V)
	RES_K(68),  // R9
	0,          // no rGnd
	CAP_U(10)   // C17
};

static const discrete_555_cc_desc sprint2_motor_vco =
{
	DISC_555_OUT_DC | DISC_555_OUT_SQW,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES,
	0.7 // VBE 2N3644 (Si)
};

static const discrete_dac_r1_ladder sprint2_motor_out_dac =
{
	4,          // size of ladder
	{RES_K(10), RES_K(10), 0, RES_K(10)}, // R42, R43, -, R44
	DEFAULT_TTL_V_LOGIC_1,
	0,          // no rBias
	0,          // no rGnd
	CAP_U(0.1)  // C36
};

static const discrete_dac_r1_ladder sprint2_crash_dac =
{
	4,      // size of ladder
	{RES_K(8.2), RES_K(3.9), RES_K(2.2), RES_K(1)}, // R27, R25, R26, R24
	0,      // no vBias
	0,      // no rBias
	0,      // no rGnd
	CAP_U(0.1) // C69
};

static const discrete_schmitt_osc_desc sprint2_screech_osc =
{
	RES_K(1),   // R19
	100,        // R16
	CAP_U(10),  // C8
	DEFAULT_7414_VALUES,
	DISC_SCHMITT_OSC_IN_IS_LOGIC | DISC_SCHMITT_OSC_ENAB_IS_AND
};

static const discrete_mixer_desc sprint2_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{RES_K(13.3333), RES_K(10.5456), RES_K(33)},    // R61 + motor, R60 + bang, R62
	{0},            // No variable resistor nodes
	{0},            // No caps
	0,              // No rI
	RES_K(5),       // R63
	0,              // No Filter
	CAP_U(0.1),     // C35
	0,              // not used in resistor network
	30000   // final gain
};

/* Nodes - Sounds */
#define SPRINT2_MOTORSND1       NODE_10
#define SPRINT2_MOTORSND2       NODE_11
#define SPRINT2_CRASHSND        NODE_12
#define SPRINT2_SKIDSND1        NODE_13
#define SPRINT2_SKIDSND2        NODE_14
#define SPRINT2_NOISE           NODE_15

#define SPRINT2_HSYNC   15750.0
#define SPRINT2_1V      SPRINT2_HSYNC/2
#define SPRINT2_2V      SPRINT2_1V/2

DISCRETE_SOUND_START(sprint2)

	/************************************************/
	/* Input register mapping for sprint2           */
	/************************************************/
	DISCRETE_INPUT_LOGIC(SPRINT2_ATTRACT_EN)
	DISCRETE_INPUT_LOGIC(SPRINT2_SKIDSND1_EN)
	DISCRETE_INPUT_LOGIC(SPRINT2_SKIDSND2_EN)
	DISCRETE_INPUTX_DATA(SPRINT2_MOTORSND1_DATA, -1, 0x0f, 0)
	DISCRETE_INPUTX_DATA(SPRINT2_MOTORSND2_DATA, -1, 0x0f, 0)
	DISCRETE_INPUT_DATA (SPRINT2_CRASHSND_DATA)
	DISCRETE_INPUT_NOT  (SPRINT2_NOISE_RESET)

	/************************************************/
	/* Motor sound circuit is based on a 556 VCO    */
	/* with the input frequency set by the MotorSND */
	/* latch (4 bit). This freqency is then used to */
	/* drive three counters, that are summed up     */
	/* and are output through a DAC                 */
	/************************************************/

	DISCRETE_ADJUSTMENT(NODE_20,
				RES_K(260), // R21 + R23 @ max
				RES_K(10),  // R21 + R23 @ min
				DISC_LOGADJ, "MOTOR1")

	DISCRETE_DAC_R1(NODE_21,
			SPRINT2_MOTORSND1_DATA,
			DEFAULT_TTL_V_LOGIC_1,
			&sprint2_motor_v_dac)

	DISCRETE_555_CC(NODE_22, 1,
			NODE_21,
			NODE_20,
			CAP_U(0.01),        // C22
			RES_M(3.3), 0, 0,   // R22
			&sprint2_motor_vco)

	/* QB-D of 7492 */
	DISCRETE_COUNTER_7492(NODE_23, 1, SPRINT2_ATTRACT_EN, NODE_22, DISC_CLK_ON_F_EDGE)

	/* Mask the bits and XOR for clock input */
	DISCRETE_TRANSFORM2(NODE_24, NODE_23, 1, "01&")
	DISCRETE_TRANSFORM2(NODE_25, NODE_23, 4, "01&")
	DISCRETE_LOGIC_XOR(NODE_26, NODE_24, NODE_25)

	/* QA of 7492 */
	DISCRETE_COUNTER(NODE_27, 1, SPRINT2_ATTRACT_EN, NODE_26, 0, 1, 1, 0, DISC_CLK_ON_F_EDGE)

	/* Mix QA and QB-D together */
	DISCRETE_TRANSFORM3(NODE_28, NODE_23, 2, NODE_27, "01*2+")

	DISCRETE_DAC_R1(SPRINT2_MOTORSND1, NODE_28,
			DEFAULT_TTL_V_LOGIC_1,
			&sprint2_motor_out_dac)

	/************************************************/
	/* Car2 motor sound                             */
	/************************************************/
	DISCRETE_ADJUSTMENT(NODE_40,
				RES_K(260), // R21 + R23 @ max
				RES_K(10),  // R21 + R23 @ min
				DISC_LOGADJ, "MOTOR2")

	DISCRETE_DAC_R1(NODE_41,
			SPRINT2_MOTORSND2_DATA,
			DEFAULT_TTL_V_LOGIC_1,
			&sprint2_motor_v_dac)

	DISCRETE_555_CC(NODE_42, 1,
			NODE_41,
			NODE_40,
			CAP_U(0.01),
			RES_M(3.3), 0, 0,
			&sprint2_motor_vco)

	/* QB-D of 7492 */
	DISCRETE_COUNTER_7492(NODE_43, 1, SPRINT2_ATTRACT_EN, NODE_42, DISC_CLK_ON_F_EDGE)

	/* Mask the bits and XOR for clock input */
	DISCRETE_TRANSFORM2(NODE_44, NODE_43, 1, "01&")
	DISCRETE_TRANSFORM2(NODE_45, NODE_43, 4, "01&")
	DISCRETE_LOGIC_XOR(NODE_46, NODE_44, NODE_45)

	/* QA of 7492 */
	DISCRETE_COUNTER(NODE_47, 1, SPRINT2_ATTRACT_EN, NODE_46, 0, 1, 1, 0, DISC_CLK_ON_F_EDGE)

	/* Mix QA and QB-D together */
	DISCRETE_TRANSFORM3(NODE_48, NODE_43, 2, NODE_47, "01*2+")

	DISCRETE_DAC_R1(SPRINT2_MOTORSND2, NODE_48,
			DEFAULT_TTL_V_LOGIC_1,
			&sprint2_motor_out_dac)

	/************************************************/
	/* Crash circuit is built around a noise        */
	/* generator built from 2 shift registers that  */
	/* are clocked by the 2V signal.                */
	/* Output is binary weighted with 4 bits of     */
	/* crash volume.                                */
	/************************************************/
	DISCRETE_LFSR_NOISE(SPRINT2_NOISE, SPRINT2_NOISE_RESET, SPRINT2_NOISE_RESET, SPRINT2_2V, 1.0, 0, 0.5, &sprint2_lfsr)

	DISCRETE_SWITCH(NODE_60, 1, SPRINT2_NOISE, 0, SPRINT2_CRASHSND_DATA)

	DISCRETE_DAC_R1(SPRINT2_CRASHSND, NODE_60, DEFAULT_TTL_V_LOGIC_1, &sprint2_crash_dac)

	/************************************************/
	/* Screech is noise modulating a  Schmitt VCO.  */
	/************************************************/
	DISCRETE_SCHMITT_OSCILLATOR(SPRINT2_SKIDSND1, SPRINT2_SKIDSND1_EN, SPRINT2_NOISE, DEFAULT_TTL_V_LOGIC_1, &sprint2_screech_osc)

	DISCRETE_SCHMITT_OSCILLATOR(SPRINT2_SKIDSND2, SPRINT2_SKIDSND2_EN, SPRINT2_NOISE, DEFAULT_TTL_V_LOGIC_1, &sprint2_screech_osc)

	/************************************************/
	/* Combine all 5 sound sources.                 */
	/************************************************/
	DISCRETE_MIXER3(NODE_90, 1, SPRINT2_MOTORSND1, SPRINT2_CRASHSND, SPRINT2_SKIDSND1, &sprint2_mixer)
	DISCRETE_MIXER3(NODE_91, 1, SPRINT2_MOTORSND2, SPRINT2_CRASHSND, SPRINT2_SKIDSND2, &sprint2_mixer)

	DISCRETE_OUTPUT(NODE_90, 1)
	DISCRETE_OUTPUT(NODE_91, 1)
DISCRETE_SOUND_END


DISCRETE_SOUND_START(sprint1)

	/************************************************/
	/* Input register mapping for sprint1           */
	/************************************************/
	DISCRETE_INPUT_LOGIC(SPRINT2_ATTRACT_EN)
	DISCRETE_INPUT_LOGIC(SPRINT2_SKIDSND1_EN)
	DISCRETE_INPUTX_DATA(SPRINT2_MOTORSND1_DATA, -1, 0x0f, 0)
	DISCRETE_INPUT_DATA (SPRINT2_CRASHSND_DATA)
	DISCRETE_INPUT_NOT  (SPRINT2_NOISE_RESET)

	/************************************************/
	/* Motor sound circuit is based on a 556 VCO    */
	/* with the input frequency set by the MotorSND */
	/* latch (4 bit). This freqency is then used to */
	/* drive three counters, that are summed up     */
	/* and are output through a DAC                 */
	/************************************************/

	DISCRETE_ADJUSTMENT(NODE_20,
				RES_K(260), // R21 + R23 @ max
				RES_K(10),  // R21 + R23 @ min
				DISC_LOGADJ, "MOTOR")

	DISCRETE_DAC_R1(NODE_21,
			SPRINT2_MOTORSND1_DATA,
			DEFAULT_TTL_V_LOGIC_1,
			&sprint2_motor_v_dac)

	DISCRETE_555_CC(NODE_22, 1,
			NODE_21,
			NODE_20,
			CAP_U(0.01),        // C22
			RES_M(3.3), 0, 0,   // R22
			&sprint2_motor_vco)

	/* QB-D of 7492 */
	DISCRETE_COUNTER_7492(NODE_23, 1, SPRINT2_ATTRACT_EN, NODE_22, DISC_CLK_ON_F_EDGE)

	/* Mask the bits and XOR for clock input */
	DISCRETE_TRANSFORM2(NODE_24, NODE_23, 1, "01&")
	DISCRETE_TRANSFORM2(NODE_25, NODE_23, 4, "01&")
	DISCRETE_LOGIC_XOR(NODE_26, NODE_24, NODE_25)

	/* QA of 7492 */
	DISCRETE_COUNTER(NODE_27, 1, SPRINT2_ATTRACT_EN, NODE_26, 0, 1, 1, 0, DISC_CLK_ON_F_EDGE)

	/* Mix QA and QB-D together */
	DISCRETE_TRANSFORM3(NODE_28, NODE_23, 2, NODE_27, "01*2+")

	DISCRETE_DAC_R1(SPRINT2_MOTORSND1, NODE_28,
			DEFAULT_TTL_V_LOGIC_1,
			&sprint2_motor_out_dac)

	/************************************************/
	/* Crash circuit is built around a noise        */
	/* generator built from 2 shift registers that  */
	/* are clocked by the 2V signal.                */
	/* Output is binary weighted with 4 bits of     */
	/* crash volume.                                */
	/************************************************/
	DISCRETE_LFSR_NOISE(SPRINT2_NOISE, SPRINT2_NOISE_RESET, SPRINT2_NOISE_RESET, SPRINT2_2V, 1.0, 0, 0.5, &sprint2_lfsr)

	DISCRETE_SWITCH(NODE_60, 1, SPRINT2_NOISE, 0, SPRINT2_CRASHSND_DATA)

	DISCRETE_DAC_R1(SPRINT2_CRASHSND, NODE_60, DEFAULT_TTL_V_LOGIC_1, &sprint2_crash_dac)

	/************************************************/
	/* Screech is noise modulating a  Schmitt VCO.  */
	/************************************************/
	DISCRETE_SCHMITT_OSCILLATOR(SPRINT2_SKIDSND1, SPRINT2_SKIDSND1_EN, SPRINT2_NOISE, DEFAULT_TTL_V_LOGIC_1, &sprint2_screech_osc)

	/************************************************/
	/* Combine all 3 sound sources.                 */
	/* Add some final gain to get to a good sound   */
	/* level.                                       */
	/************************************************/
	DISCRETE_MIXER3(NODE_90, 1, SPRINT2_MOTORSND1, SPRINT2_CRASHSND, SPRINT2_SKIDSND1, &sprint2_mixer)

	DISCRETE_OUTPUT(NODE_90, 1)
DISCRETE_SOUND_END


/************************************************************************/
/* dominos Sound System Analog emulation                                */
/* updated to new code March 2005, DR.                                  */
/************************************************************************/

static const discrete_dac_r1_ladder dominos_tone_vco_dac =
{
	4,
	{RES_M(2.2), RES_M(1), RES_K(470), RES_K(220)}, // R5, R6, R7, R8
	4.4,        // 5V - diode junction (0.6V)
	RES_K(68),  // R9
	RES_K(470), // R87
	CAP_U(.1)   // C17
};

static const discrete_555_cc_desc dominos_tone_vco =
{
	DISC_555_OUT_DC | DISC_555_OUT_SQW,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES,
	0.7     // Q1 junction voltage
};

static const discrete_dac_r1_ladder dominos_tone_dac =
{
	4,
	{RES_K(8.2), RES_K(3.9), RES_K(2.2), RES_K(1)}, // R27, R24, R25, R26
	0, 0, 0,
	CAP_U(.1)   // C69
};

static const discrete_mixer_desc dominos_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{RES_K(33), RES_K(10)+ 1.0/(1.0/RES_K(8.2) + 1.0/RES_K(3.9) + 1.0/RES_K(2.2) + 1.0/RES_K(1))}, // R62, R60 + R27||R24||R25||R26
	{0},            // No variable resistor nodes
	{0},            // No caps
	0,              // No rI
	RES_K(5),       // R66
	CAP_U(0.1),     // C71
	CAP_U(0.1),     // C54
	0,              // not used in resistor network
	40000   // final gain
};

/* Nodes - Sounds */
#define DOMINOS_TONE_SND        NODE_10
#define DOMINOS_TOPPLE_SND      NODE_11
/* Nodes - Adjusters */
#define DOMINOS_R23             NODE_15

DISCRETE_SOUND_START(dominos)
	/************************************************/
	/* Input register mapping for dominos           */
	/************************************************/
	DISCRETE_INPUT_LOGIC(DOMINOS_TUMBLE_EN)
	DISCRETE_INPUTX_DATA(DOMINOS_FREQ_DATA, -1, 0x0f, 0)    // IC D4
	DISCRETE_INPUT_DATA (DOMINOS_AMP_DATA)                  // IC C4
	DISCRETE_INPUT_LOGIC(DOMINOS_ATTRACT_EN)

	DISCRETE_ADJUSTMENT(DOMINOS_R23,
				RES_K(60),  // R21 + R23 @ max
				RES_K(10),  // R21 + R23 @ min
				DISC_LINADJ, "R23")

	/************************************************/
	/* Tone Sound                                   */
	/************************************************/
	DISCRETE_DAC_R1(NODE_20, DOMINOS_FREQ_DATA, DEFAULT_TTL_V_LOGIC_1, &dominos_tone_vco_dac)
	DISCRETE_555_CC(NODE_21, 1, NODE_20, DOMINOS_R23, CAP_U(.01), 0, 0, 0, &dominos_tone_vco)
	DISCRETE_COUNTER_7492(NODE_22, 1, DOMINOS_ATTRACT_EN,   // IC D8, QB-QD
			NODE_21, DISC_CLK_ON_F_EDGE)                    // from IC D7/8, pin 3
	DISCRETE_TRANSFORM2(NODE_23, NODE_22, 0x01, "01&")  // IC D8, pin 11-QB
	DISCRETE_SWITCH(NODE_24, 1, NODE_23, 0, // Enable gate C5
			DOMINOS_AMP_DATA)       // IC C4
	DISCRETE_DAC_R1(DOMINOS_TONE_SND, NODE_24, DEFAULT_TTL_V_LOGIC_1, &dominos_tone_dac)

	/************************************************/
	/* Topple sound is just the 4V source           */
	/* 4V = HSYNC/8                                 */
	/*    = 15750/8                                 */
	/************************************************/
	DISCRETE_SQUAREWFIX(DOMINOS_TOPPLE_SND, DOMINOS_TUMBLE_EN, 15750.0/8, DEFAULT_TTL_V_LOGIC_1, 50.0, DEFAULT_TTL_V_LOGIC_1/2, 0)

	/************************************************/
	/* Combine both sound sources.                  */
	/* Add some final gain to get to a good sound   */
	/* level.                                       */
	/************************************************/
	DISCRETE_MIXER2(NODE_90, 1, DOMINOS_TOPPLE_SND, DOMINOS_TONE_SND, &dominos_mixer)

	DISCRETE_OUTPUT(NODE_90, 1)
DISCRETE_SOUND_END
