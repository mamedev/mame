// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*************************************************************************

    audio\firetrk.c

*************************************************************************/

#include "emu.h"
#include "includes/firetrk.h"
#include "sound/discrete.h"


WRITE8_MEMBER(firetrk_state::firetrk_skid_reset_w)
{
	m_skid[0] = 0;
	m_skid[1] = 0;

	// also SUPERBUG_SKID_EN
	m_discrete->write(space, FIRETRUCK_SKID_EN, 1);
}


WRITE8_MEMBER(firetrk_state::montecar_skid_reset_w)
{
	m_discrete->write(space, MONTECAR_SKID_EN, 1);
}


WRITE8_MEMBER(firetrk_state::firetrk_crash_snd_w)
{
	// also SUPERBUG_CRASH_DATA and MONTECAR_CRASH_DATA
	m_discrete->write(space, FIRETRUCK_CRASH_DATA, data >> 4);
}


WRITE8_MEMBER(firetrk_state::firetrk_skid_snd_w)
{
	// also SUPERBUG_SKID_EN and MONTECAR_SKID_EN
	m_discrete->write(space, FIRETRUCK_SKID_EN, 0);
}


WRITE8_MEMBER(firetrk_state::firetrk_motor_snd_w)
{
	// also MONTECAR_DRONE_MOTOR_DATA
	m_discrete->write(space, FIRETRUCK_SIREN_DATA, data >> 4);

	// also MONTECAR_MOTOR_DATA
	m_discrete->write(space, FIRETRUCK_MOTOR_DATA, data & 0x0f);
}


WRITE8_MEMBER(firetrk_state::superbug_motor_snd_w)
{
	m_discrete->write(space, SUPERBUG_SPEED_DATA, data & 0x0f);
}


WRITE8_MEMBER(firetrk_state::firetrk_xtndply_w)
{
	// also SUPERBUG_ASR_EN (extended play)
	m_discrete->write(space, FIRETRUCK_XTNDPLY_EN, data);
}


#define FIRETRUCK_HSYNC 15750.0     /* not checked */
#define FIRETRUCK_1V    FIRETRUCK_HSYNC/2
#define FIRETRUCK_2V    FIRETRUCK_1V/2
#define FIRETRUCK_8V    FIRETRUCK_1V/8
#define FIRETRUCK_64V   FIRETRUCK_1V/64

/************************************************************************/
/* firetrk Sound System Analog emulation by K.Wilkins Feb 2001          */
/* Questions/Suggestions to mame@esplexo.co.uk                          */
/* Modified and added superbug/montecar sounds.  Jan 2003 D.R.          */
/* Complete re-write Feb 2004, D. Renaud                                */
/************************************************************************/

static const discrete_lfsr_desc firetrk_lfsr={
	DISC_CLK_IS_FREQ,
	16,         /* Bit Length */
	0,          /* Reset Value */
	0,          /* Use Bit 0 as XOR input 0 */
	14,         /* Use Bit 14 as XOR input 1 */
	DISC_LFSR_XNOR,     /* Feedback stage1 is XNOR */
	DISC_LFSR_OR,       /* Feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,  /* Feedback stage3 replaces the shifted register contents */
	0x000001,       /* Everything is shifted into the first bit only */
	0,          /* Output is not inverted */
	15          /* Output bit */
};

static const discrete_dac_r1_ladder firetrk_motor_v_dac =
{
	4,          // size of ladder
	{RES_M(2.2), RES_M(1), RES_K(470), RES_K(220), 0,0,0,0},    // R24, R23, R22, R21
	4.4,        // 5V - diode junction (0.6V)
	RES_K(68),  // R25
	0,          // no rGnd
	CAP_U(10)   // C24
};

static const discrete_555_cc_desc firetrk_motor_vco =
{
	DISC_555_OUT_DC | DISC_555_OUT_SQW,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES,
	0.7     // Q2 junction voltage
};

static const discrete_dac_r1_ladder firetrk_motor_out_dac =
{
	4,          // size of ladder
	{RES_K(10), 0,0, RES_K(10)},    // R74, -, -, R73
	DEFAULT_TTL_V_LOGIC_1,
	0,          // no rBias
	0,          // no rGnd
	CAP_U(0.1)  // C43
};

static const discrete_dac_r1_ladder firetrk_siren_cv_dac =
{
	4,              // size of ladder
	{RES_M(2.2), RES_M(1), RES_K(470), RES_K(220), 0,0,0,0},    // R46, R47, R45, R48
	5,              // 5V
	RES_K(100),     // R49
	RES_K(390),     // R44
	CAP_U(10)       // C30
};

static const discrete_555_desc firetrk_siren_555 =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_dac_r1_ladder firetrk_bang_dac =
{
	4,      // size of ladder
	{RES_K(8.2), RES_K(3.9), RES_K(2.2), RES_K(1), 0,0,0,0},    // R37, R35, R36, R34
	0,      // no vBias
	0,      // no rBias
	0,      // no rGnd
	0       // no smoothing cap
};

static const discrete_schmitt_osc_desc firetrk_screech_osc =
{
	RES_K(2.2), // R29
	330,        // R16
	CAP_U(2.2), // C8
	DEFAULT_74LS14_VALUES,
	DISC_SCHMITT_OSC_IN_IS_LOGIC | DISC_SCHMITT_OSC_ENAB_IS_NOR
};

static const discrete_mixer_desc firetrk_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{RES_K(4.7), RES_K(22), RES_K(31.333), RES_K(33), RES_K(10.5456), RES_K(32), RES_K(150)}, // R54, R55, R72||(R70+R71), R53, R56 + R37||R35||R36||R34, R58 + R73||R74, R52
	{0},            // No variable resistor nodes
	{CAP_U(0.22), CAP_U(0.22), CAP_U(0.01), CAP_U(0.22), CAP_U(0.22), 0, CAP_U(0.22)}, // C34, C32, C44, C35, C33, NA, C31
	0,                  // No rI
	RES_K(22),          // R43
	0,                  // No Filter
	CAP_U(10),          // C12
	5.0 * 820 / (270 + 820),    // vBias = 5V * R51/(R41+R51)
	3400    // final gain
};

#define FIRETRUCK_ATTRACT_INV   NODE_09
/* Nodes - Sounds */
#define FIRETRUCK_NOISE         NODE_11
#define FIRETRUCK_MOTORSND      NODE_12
#define FIRETRUCK_HORNSND       NODE_13
#define FIRETRUCK_SIRENSND      NODE_14
#define FIRETRUCK_BANGSND       NODE_15
#define FIRETRUCK_SCREECHSND    NODE_16
#define FIRETRUCK_BELLSND       NODE_17
#define FIRETRUCK_XTNDPLYSND    NODE_18

DISCRETE_SOUND_START(firetrk)
	/************************************************/
	/* Input register mapping for firetruck         */
	/************************************************/
	DISCRETE_INPUT_DATA (FIRETRUCK_MOTOR_DATA)
	DISCRETE_INPUT_LOGIC(FIRETRUCK_HORN_EN)
	DISCRETE_INPUT_DATA (FIRETRUCK_SIREN_DATA)
	DISCRETE_INPUTX_DATA(FIRETRUCK_CRASH_DATA , -1, 0x0f, 0)
	DISCRETE_INPUT_LOGIC(FIRETRUCK_SKID_EN)
	DISCRETE_INPUT_LOGIC(FIRETRUCK_BELL_EN)
	DISCRETE_INPUT_LOGIC(FIRETRUCK_ATTRACT_EN)
	DISCRETE_INPUT_NOT  (FIRETRUCK_XTNDPLY_EN)

	DISCRETE_LOGIC_INVERT(FIRETRUCK_ATTRACT_INV, FIRETRUCK_ATTRACT_EN)

	/************************************************/
	/* Motor sound circuit is based on a 556 VCO    */
	/* with the input frequency set by the MotorSND */
	/* latch (4 bit). This freqency is then used to */
	/* drive a modulo 12 counter, with div6 & div12 */
	/* summed as the output of the circuit.         */
	/************************************************/
	DISCRETE_ADJUSTMENT(NODE_20,
				RES_K(260), // R26 + R27 @ max
				RES_K(10),  // R26 + R27 @ min
				DISC_LOGADJ, "R27")
	DISCRETE_DAC_R1(NODE_21,        // base of Q1
			FIRETRUCK_MOTOR_DATA,   // IC F8, pins 2,5,6,9
			DEFAULT_TTL_V_LOGIC_1,
			&firetrk_motor_v_dac)
	DISCRETE_555_CC(NODE_22, 1, // IC B9 pin 3, always enabled
			NODE_21,            // vIn
			NODE_20,            // current adjust
			CAP_U(0.01),        // C25
			RES_M(1), 0, 0,     // R28, no rGnd, no rDis
			&firetrk_motor_vco)
	DISCRETE_COUNTER_7492(NODE_23, 1, FIRETRUCK_ATTRACT_EN, // IC A9, QB-QD
			NODE_22, DISC_CLK_ON_F_EDGE)                    // from IC B9, pin 3
	DISCRETE_TRANSFORM2(NODE_24, NODE_23, 0x04, "01&")  // IC A9, pin 8
	DISCRETE_COUNTER(NODE_25, 1, FIRETRUCK_ATTRACT_EN,  // IC A9, pin 12
			NODE_24,                                    // from IC A9, pin 8
			0, 1, 1, 0, DISC_CLK_ON_F_EDGE)
	DISCRETE_TRANSFORM3(NODE_26, NODE_23, 2, NODE_25, "01*2+")  // Mix QA and QB-D together
	DISCRETE_DAC_R1(FIRETRUCK_MOTORSND, NODE_26,
			DEFAULT_TTL_V_LOGIC_1,
			&firetrk_motor_out_dac)

	/************************************************/
	/* Horn, this is taken from the 64V signal.     */
	/*  64V = HSYNC/128                             */
	/*      = 15750/128                             */
	/************************************************/
	DISCRETE_SQUAREWFIX(NODE_30, FIRETRUCK_ATTRACT_INV, FIRETRUCK_64V, DEFAULT_TTL_V_LOGIC_1, 50.0, DEFAULT_TTL_V_LOGIC_1/2, 0)
	DISCRETE_ONOFF(FIRETRUCK_HORNSND, FIRETRUCK_HORN_EN, NODE_30)

	/************************************************/
	/* Siren is built around a 556 based VCO, the   */
	/* 4 bit input value is smoothed between trans- */
	/* itions by a 10u capacitor.                   */
	/* 0000 = 666Hz with 35% duty cycle             */
	/* 1111 = 526Hz with 63% duty cycle             */
	/************************************************/
	DISCRETE_DAC_R1(NODE_40,        // IC E9, pin 7
			FIRETRUCK_SIREN_DATA,   // IC F8, pins 15,16,12,19
			DEFAULT_TTL_V_LOGIC_1,
			&firetrk_siren_cv_dac)
	DISCRETE_555_ASTABLE_CV(FIRETRUCK_SIRENSND, FIRETRUCK_ATTRACT_INV,
				RES_K(2.2), // R60
				RES_K(10),  // R59
				CAP_U(0.1), // C37
				NODE_40,    // CV is straight from DAC because op-amp E9 is just x1 buffer
				&firetrk_siren_555)

	/************************************************/
	/* Crash circuit is built around a noise        */
	/* generator built from 2 shift registers that  */
	/* are clocked by the 2V signal.                */
	/* 2V = HSYNC/4                                 */
	/*    = 15750/4                                 */
	/* Output is binary weighted with 4 bits of     */
	/* crash volume.                                */
	/************************************************/
	DISCRETE_LFSR_NOISE(FIRETRUCK_NOISE, FIRETRUCK_ATTRACT_INV, FIRETRUCK_ATTRACT_INV, FIRETRUCK_2V, 1.0, 0, 0.5, &firetrk_lfsr)

	DISCRETE_SWITCH(NODE_50, 1, FIRETRUCK_NOISE, 0, // Enable gate K9
			FIRETRUCK_CRASH_DATA)       // IC K9, pins 3,6,11,8
	DISCRETE_DAC_R1(FIRETRUCK_BANGSND,  // Bang
			NODE_50,        // from enable gates K9
			DEFAULT_TTL_V_LOGIC_1,
			&firetrk_bang_dac)

	/************************************************/
	/* Screech is just the noise modulating a       */
	/* Schmitt VCO.                                 */
	/************************************************/
	DISCRETE_SCHMITT_OSCILLATOR(FIRETRUCK_SCREECHSND, FIRETRUCK_SKID_EN, FIRETRUCK_NOISE, DEFAULT_TTL_V_LOGIC_1, &firetrk_screech_osc)

	/************************************************/
	/* Bell circuit -                               */
	/* The Hsync signal is put into a div 16        */
	/* counter.                                     */
	/************************************************/
	DISCRETE_RCDISC2(NODE_70, FIRETRUCK_BELL_EN,
				4.4, 10,        // Q3 instantally charges C42
				0, RES_K(33),   // discharges through R66
				CAP_U(10))      // C42
	DISCRETE_TRANSFORM2(NODE_71, NODE_70,
				5.0 * 680 / (RES_K(10) + 680),  // vRef = 5V * R64 / (R65 + R64)
				"01<")              // Output is low until vIn drops below vRef
	DISCRETE_COUNTER(NODE_72, 1, NODE_71, FIRETRUCK_HSYNC, 0, 15, 1, 0, DISC_CLK_IS_FREQ)   // IC B10
	DISCRETE_TRANSFORM4(FIRETRUCK_BELLSND, NODE_72,
				8,  // count 0-7 allow cap voltage to output.  8-15 ground output.
				NODE_70,    // scale logic to cap voltage
				RES_K(47) / (RES_K(47) + RES_K(47) + RES_K(47)),    // to drop voltage per divider R72 / (R70+R71+R72)
				"01<2*3*")

	/************************************************/
	/* Extended play circuit is just the 8V signal  */
	/* 8V = HSYNC/16                                */
	/*    = 15750/16                                */
	/************************************************/
	DISCRETE_SQUAREWFIX(FIRETRUCK_XTNDPLYSND, FIRETRUCK_XTNDPLY_EN, FIRETRUCK_8V, DEFAULT_TTL_V_LOGIC_1, 50.0, DEFAULT_TTL_V_LOGIC_1, 0.0)

	/************************************************/
	/* Combine all 7 sound sources.                 */
	/************************************************/
	DISCRETE_MIXER7(NODE_90, 1, FIRETRUCK_XTNDPLYSND, FIRETRUCK_HORNSND, FIRETRUCK_BELLSND, FIRETRUCK_SCREECHSND, FIRETRUCK_BANGSND, FIRETRUCK_MOTORSND, FIRETRUCK_SIRENSND, &firetrk_mixer)

	DISCRETE_OUTPUT(NODE_90, 1)
DISCRETE_SOUND_END


/************************************************************************/
/* superbug Sound System Analog emulation                               */
/* Complete re-write Feb 2004, D. Renaud                                */
/************************************************************************/

static const discrete_dac_r1_ladder superbug_motor_v_dac =
{
	4,          // size of ladder
	{RES_M(2.2), RES_M(1), RES_K(470), RES_K(220), 0,0,0,0},    // R8, R9, R6, R7
	4.4,        // 5V - diode junction (0.6V)
	RES_K(68),  // R10
	0,          // no rGnd
	CAP_U(10)   // C20
};

static const discrete_555_cc_desc superbug_motor_vco =
{
	DISC_555_OUT_DC | DISC_555_OUT_SQW,
	5,              // B+ voltage of 555
	DEFAULT_555_VALUES,
	0.7             // Q1 junction voltage
};

static const discrete_dac_r1_ladder superbug_motor_out_dac =
{
	4,          // size of ladder
	{RES_K(10), 0,0, RES_K(10)},    // R34, -, -, R32
	DEFAULT_TTL_V_LOGIC_1,
	0,          // no rBias
	0,          // no rGnd
	CAP_U(0.1)  // C26
};

static const discrete_dac_r1_ladder superbug_bang_dac =
{
	4,          // size of ladder
	{RES_K(8.2), RES_K(3.9), RES_K(2.2), RES_K(1), 0,0,0,0},    // R28, R28, R27, R26
	0,          // no vBias
	0,          // no rBias
	0,          // no rGnd
	CAP_U(0.1)  // C25
};

static const discrete_schmitt_osc_desc superbug_screech_osc =
{
	RES_K(2.2),     // R30
	330,            // R52
	CAP_U(2.2), // C27
	DEFAULT_7414_VALUES,
	DISC_SCHMITT_OSC_IN_IS_LOGIC | DISC_SCHMITT_OSC_ENAB_IS_NOR
};

static const discrete_mixer_desc superbug_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{RES_K(15), RES_K(10.5456), RES_K(33), RES_K(4.7)}, // R54, R55, R72||(R70+R71), R53, R56 + R37||R35||R36||R34, R58 + R73||R74, R52
	{0},            // No variable resistor nodes
	{0},            // No caps
	0,              // No rI
	RES_K(5),       // R63
	0,              // No Filter
	CAP_U(0.1),     // C35
	0,              // not used in resistor network
	33000   // final gain
};

/* Nodes - Inputs */
#define SUPERBUG_ATTRACT_INV    NODE_09
/* Nodes - Sounds */
#define SUPERBUG_NOISE          NODE_10
#define SUPERBUG_MOTORSND       NODE_11
#define SUPERBUG_BANGSND        NODE_12
#define SUPERBUG_SCREECHSND     NODE_13
#define SUPERBUG_ASRSND         NODE_14

DISCRETE_SOUND_START(superbug)
	/************************************************/
	/* Input register mapping for superbug          */
	/************************************************/
	DISCRETE_INPUTX_DATA(SUPERBUG_SPEED_DATA, -1, 0x0f, 0)
	DISCRETE_INPUTX_DATA(SUPERBUG_CRASH_DATA, -1, 0x0f, 0)
	DISCRETE_INPUT_LOGIC(SUPERBUG_SKID_EN)
	DISCRETE_INPUT_LOGIC(SUPERBUG_ATTRACT_EN)
	DISCRETE_INPUT_LOGIC(SUPERBUG_ASR_EN)

	DISCRETE_LOGIC_INVERT(SUPERBUG_ATTRACT_INV, SUPERBUG_ATTRACT_EN)

	/************************************************/
	/* Motor sound circuit is based on a 556 VCO    */
	/* with the input frequency set by the MotorSND */
	/* latch (4 bit). This freqency is then used to */
	/* drive a modulo 12 counter.                   */
	/************************************************/
	DISCRETE_ADJUSTMENT(NODE_20,
				RES_K(260), // R12 + R62 @ max
				RES_K(10),  // R12 + R62 @ min
				DISC_LOGADJ, "R62")
	DISCRETE_DAC_R1(NODE_21,        // base of Q1
			SUPERBUG_SPEED_DATA,    // IC B5, pins 3, 14, 6, 11
			DEFAULT_TTL_V_LOGIC_1,
			&superbug_motor_v_dac)
	DISCRETE_555_CC(NODE_22, 1, // IC A6 pin 3, always enabled
			NODE_21,            // vIn
			NODE_20,            // current adjust
			CAP_U(0.01),        // C21
			RES_M(3.3), 0, 0,   // R11, no rGnd, no rDis
			&superbug_motor_vco)
	DISCRETE_COUNTER_7492(NODE_23, 1, SUPERBUG_ATTRACT_EN,  // IC A7, QB-QD
			NODE_22, DISC_CLK_ON_F_EDGE)                    // from IC A6, pin 3
	DISCRETE_TRANSFORM2(NODE_24, NODE_23, 0x04, "01&")  // IC A7, pin 8-QD
	DISCRETE_TRANSFORM2(NODE_25, NODE_23, 0x01, "01&")  // IC A7, pin 11-QB
	DISCRETE_LOGIC_XOR(NODE_26, NODE_24, NODE_25)   // Gate A9, pin 8
	DISCRETE_COUNTER(NODE_27, 1, SUPERBUG_ATTRACT_EN,   // IC A7, pin 12-QA
			NODE_26,                                    // from IC A9, pin 8
			0, 1, 1, 0, DISC_CLK_ON_F_EDGE)
	DISCRETE_TRANSFORM3(NODE_28, NODE_23, 2, NODE_27, "01*2+")  // Mix QA and QB-D together
	DISCRETE_DAC_R1(SUPERBUG_MOTORSND, NODE_28,
			DEFAULT_TTL_V_LOGIC_1,
			&superbug_motor_out_dac)

	/************************************************/
	/* Crash circuit is built around a noise        */
	/* generator built from 2 shift registers that  */
	/* are clocked by the 2V signal.                */
	/* 2V = HSYNC/4                                 */
	/*    = 15750/4                                 */
	/* Output is binary weighted with 4 bits of     */
	/* crash volume.                                */
	/************************************************/
	DISCRETE_LFSR_NOISE(SUPERBUG_NOISE, SUPERBUG_ATTRACT_INV, SUPERBUG_ATTRACT_INV, FIRETRUCK_2V, 1.0, 0, 0.5, &firetrk_lfsr)   // Same as firetrk

	DISCRETE_SWITCH(NODE_40, 1, SUPERBUG_NOISE, 0,  // Enable gate C8
			SUPERBUG_CRASH_DATA)        // IC D8, pins 3,14,6,11
	DISCRETE_DAC_R1(SUPERBUG_BANGSND,   // Bang
			NODE_40,        // from enable gates C8
			DEFAULT_TTL_V_LOGIC_1,
			&superbug_bang_dac)

	/************************************************/
	/* Screech is just the noise modulating a       */
	/* Schmitt VCO.                                 */
	/************************************************/
	DISCRETE_SCHMITT_OSCILLATOR(SUPERBUG_SCREECHSND, SUPERBUG_SKID_EN, SUPERBUG_NOISE, DEFAULT_TTL_V_LOGIC_1, &superbug_screech_osc)

	/************************************************/
	/* ASR circuit is just the 8V signal            */
	/* 8V = HSYNC/16                                */
	/*    = 15750/16                                */
	/************************************************/
	DISCRETE_SQUAREWFIX(SUPERBUG_ASRSND, SUPERBUG_ASR_EN, FIRETRUCK_8V, DEFAULT_TTL_V_LOGIC_1, 50.0, DEFAULT_TTL_V_LOGIC_1/2, 0.0)

	/************************************************/
	/* Combine all 4 sound sources.                 */
	/************************************************/
	DISCRETE_MIXER4(NODE_90, 1, SUPERBUG_MOTORSND, SUPERBUG_BANGSND, SUPERBUG_SCREECHSND, SUPERBUG_ASRSND, &superbug_mixer)
	DISCRETE_OUTPUT(NODE_90, 1)
DISCRETE_SOUND_END


/************************************************************************/
/* montecar Sound System Analog emulation                               */
/* Complete re-write Mar 2004, D. Renaud                                */
/************************************************************************/

/* Both cars use same parts for v_dac, vco and out_dac */
static const discrete_dac_r1_ladder montecar_motor_v_dac =
{
	4,              // size of ladder
	{RES_M(2.2), RES_M(1), RES_K(470), RES_K(220), 0,0,0,0},    // R44, R43, R46, R45
	4.4,            // 5V - diode junction (0.6V)
	RES_K(68),      // R80
	0,              // no rGnd
	CAP_U(4.7)      // C77
};

static const discrete_555_cc_desc montecar_motor_vco =
{
	DISC_555_OUT_DC | DISC_555_OUT_SQW,
	5,      // B+ voltage of 555
	DEFAULT_555_VALUES,
	0.7     // Q1 junction voltage
};

static const discrete_dac_r1_ladder montecar_motor_out_dac =
{
	4,          // size of ladder
	{RES_K(10), RES_K(10), 0, RES_K(10)},   // R31, R30, -, R29
	DEFAULT_TTL_V_LOGIC_1,
	0,          // no rBias
	0,          // no rGnd
	CAP_U(0.1)  // C53
};

static const discrete_comp_adder_table montecar_drone_vol_res =
{
	DISC_COMP_P_RESISTOR,
	0,                  // no default
	4,                  // # of resistors
	{RES_K(100.25), RES_K(47.25), RES_K(22.25), RES_K(10.25), 0,0,0,0}  // R105, R104, R103, R102 (250 added to all for 4066 resistance)
};

static const discrete_dac_r1_ladder montecar_bang_dac =
{
	4,      // size of ladder
	{RES_K(8.2), RES_K(3.9), RES_K(2.2), RES_K(1), 0,0,0,0},    // R39, R42, R41, R40
	0,      // no vBias
	0,      // no rBias
	0,      // no rGnd
	0       // no cSmoothing
};

static const discrete_op_amp_filt_info montecar_bang_filt =
{
	1.0/(1.0/RES_K(8.2) + 1.0/RES_K(3.9) + 1.0/RES_K(2.2) + 1.0/RES_K(1)) + RES_K(3.3), // R39, R42, R41, R40, r82
	0,
	680,            // R83
	0,
	RES_K(330),     // r84
	CAP_U(.1),      // c79
	CAP_U(.1),      // c78
	0,
	5, 12, 0
};

static const discrete_schmitt_osc_desc montecar_screech_osc =
{
	RES_K(2.2), // R54
	330,    // R53
	CAP_U(2.2), // C57
	DEFAULT_74LS14_VALUES,
	DISC_SCHMITT_OSC_IN_IS_LOGIC | DISC_SCHMITT_OSC_ENAB_IS_NOR
};

static const discrete_mixer_desc montecar_mixer =
{
	DISC_MIXER_IS_OP_AMP,
	{RES_K(15), RES_K(33), RES_K(10), RES_K(10), RES_K(13.3333)}, // R93, R97, R96, variable , R95 + R31||R30||R29
	{0,0,0,NODE_30,0},          // Only drone has variable node
	{CAP_U(0.22), CAP_U(0.22), CAP_U(0.22), CAP_U(1), CAP_U(1)},    // C83, C84, C85, C88, C47
	RES_K(27),      // R92
	RES_K(82),      // R98
	0,              // No Filter
	CAP_U(0.22),    // C6
	5,              // vRef
	5000    // final gain
};

#define MONTECAR_ATTRACT_EN     NODE_09
/* Nodes - Sounds */
#define MONTECAR_NOISE              NODE_15
#define MONTECAR_MOTORSND           NODE_10 // MotorAud1
#define MONTECAR_BEEPSND            NODE_11
#define MONTECAR_DRONE_MOTORSND     NODE_12 // MotorAud2
#define MONTECAR_BANGSND            NODE_13
#define MONTECAR_SCREECHSND         NODE_14

DISCRETE_SOUND_START(montecar)
	/************************************************/
	/* Input register mapping for montecar          */
	/************************************************/
	DISCRETE_INPUT_DATA (MONTECAR_MOTOR_DATA)       // Motor 1-4
	DISCRETE_INPUT_DATA (MONTECAR_DRONE_MOTOR_DATA) // Motor 5-8
	DISCRETE_INPUTX_DATA(MONTECAR_CRASH_DATA, -1, 0x0f, 0)
	DISCRETE_INPUT_LOGIC(MONTECAR_SKID_EN )
	DISCRETE_INPUT_DATA (MONTECAR_DRONE_LOUD_DATA)
	DISCRETE_INPUT_LOGIC(MONTECAR_ATTRACT_INV)
	DISCRETE_INPUT_NOT  (MONTECAR_BEEPER_EN)

	DISCRETE_LOGIC_INVERT(MONTECAR_ATTRACT_EN, MONTECAR_ATTRACT_INV)

	/************************************************/
	/* Motor sound circuit is based on a 556 VCO    */
	/* with the input frequency set by the MotorSND */
	/* latch (4 bit). This freqency is then used to */
	/* driver a modulo 12 counter, with div6, 4 & 3 */
	/* summed as the output of the circuit.         */
	/************************************************/
	DISCRETE_ADJUSTMENT(NODE_20,
				RES_K(260), // R87 + R89 @ max
				RES_K(10),  // R87 + R89 @ min
				DISC_LOGADJ, "R89")
	DISCRETE_DAC_R1(NODE_21,        // base of Q7
			MONTECAR_MOTOR_DATA,    // IC H8, pins 5, 2, 9, 6
			DEFAULT_TTL_V_LOGIC_1,
			&montecar_motor_v_dac)
	DISCRETE_555_CC(NODE_22, 1, // IC C9 pin 9
			NODE_21,            // vIn
			NODE_20,            // current adjust
			CAP_U(0.01),        // C81
			RES_M(1), 0, 0,     // R86, no rGnd, no rDis
			&montecar_motor_vco)
	DISCRETE_COUNTER_7492(NODE_23, 1, MONTECAR_ATTRACT_EN,  // IC B/C9, QB-QD
			NODE_22, DISC_CLK_ON_F_EDGE)                        // from IC C9, pin 9
	DISCRETE_TRANSFORM2(NODE_24, NODE_23, 0x04, "01&")  // IC B/C9, pin 8-QD
	DISCRETE_TRANSFORM2(NODE_25, NODE_23, 0x01, "01&")  // IC B/C9, pin 11-QB
	DISCRETE_LOGIC_XOR(NODE_26, NODE_24, NODE_25)   // Gate A9, pin 11
	DISCRETE_COUNTER(NODE_27, 1, MONTECAR_ATTRACT_EN,   // IC B/C9, pin 12-QA
			NODE_26,                                    // from IC A9, pin 11
			0, 1, 1, 0, DISC_CLK_ON_F_EDGE)
	DISCRETE_TRANSFORM3(NODE_28, NODE_23, 2, NODE_27, "01*2+")  // Mix QA and QB-D together
	DISCRETE_DAC_R1(MONTECAR_MOTORSND, NODE_28,
			DEFAULT_TTL_V_LOGIC_1,
			&montecar_motor_out_dac)

	/************************************************/
	/* Drone motor sound is basically the same as   */
	/* the regular car but with a volume control.   */
	/* Also I shifted the frequencies up for it to  */
	/* sound different from the player's car.       */
	/************************************************/
	DISCRETE_COMP_ADDER(NODE_30, MONTECAR_DRONE_LOUD_DATA, &montecar_drone_vol_res) // make sure to change the node value in the mixer table if you change this node number

	DISCRETE_ADJUSTMENT(NODE_40,
				RES_K(260), // R85 + R88 @ max
				RES_K(10),  // R85 + R88 @ min
				DISC_LOGADJ, "R88")
	DISCRETE_DAC_R1(NODE_41,            // base of Q7
			MONTECAR_DRONE_MOTOR_DATA,  // IC H8, pins 19, 16, 12, 15
			DEFAULT_TTL_V_LOGIC_1,
			&montecar_motor_v_dac)
	DISCRETE_555_CC(NODE_42, 1, // IC C9 pin 5
			NODE_41,            // vIn
			NODE_40,            // current adjust
			CAP_U(0.01),        // C80
			RES_M(1), 0, 0,     // R81, no rGnd, no rDis
			&montecar_motor_vco)
	DISCRETE_COUNTER_7492(NODE_43, 1, MONTECAR_ATTRACT_EN,  // IC A/B9, QB-QD
			NODE_42, DISC_CLK_ON_F_EDGE)                    // from IC C9, pin 5
	DISCRETE_TRANSFORM2(NODE_44, NODE_43, 0x04, "01&")  // IC A/B9, pin 8-QD
	DISCRETE_TRANSFORM2(NODE_45, NODE_43, 0x01, "01&")  // IC A/B9, pin 11-QB
	DISCRETE_LOGIC_XOR(NODE_46, NODE_44, NODE_45)   // Gate A9, pin 6
	DISCRETE_COUNTER(NODE_47, 1, MONTECAR_ATTRACT_EN,   // IC A/B9, pin 12-QA
			NODE_46,                                    // from IC A9, pin 6
			0, 1, 1, 0, DISC_CLK_ON_F_EDGE)
	DISCRETE_TRANSFORM3(NODE_48, NODE_43, 2, NODE_47, "01*2+")  // Mix QA and QB-D together
	DISCRETE_DAC_R1(MONTECAR_DRONE_MOTORSND, NODE_48,
			DEFAULT_TTL_V_LOGIC_1,
			&montecar_motor_out_dac)

	/************************************************/
	/* Crash circuit is built around a noise        */
	/* generator built from 2 shift registers that  */
	/* are clocked by the 2V signal.                */
	/* 2V = HSYNC/4                                 */
	/*    = 15750/4                                 */
	/* Output is binary weighted with 4 bits of     */
	/* crash volume.                                */
	/************************************************/
	DISCRETE_LFSR_NOISE(MONTECAR_NOISE, MONTECAR_ATTRACT_INV, MONTECAR_ATTRACT_INV,FIRETRUCK_2V , 1.0, 0, 0.5, &firetrk_lfsr)   // Same as firetrk

	DISCRETE_SWITCH(NODE_50, 1, MONTECAR_NOISE, 0,  // Enable gate A9
			MONTECAR_CRASH_DATA)        // IC J8, pins 3,6,11,14
	DISCRETE_DAC_R1(NODE_51,    // Bang
			NODE_50,    // from enable gates A9
			DEFAULT_TTL_V_LOGIC_1,
			&montecar_bang_dac)
	DISCRETE_OP_AMP_FILTER(MONTECAR_BANGSND, 1, NODE_51, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &montecar_bang_filt)

	/************************************************/
	/* Screech is just the noise modulating a       */
	/* Schmitt VCO.                                 */
	/************************************************/
	DISCRETE_SCHMITT_OSCILLATOR(NODE_60, MONTECAR_SKID_EN, MONTECAR_NOISE, DEFAULT_TTL_V_LOGIC_1, &montecar_screech_osc)
	DISCRETE_SWITCH(MONTECAR_SCREECHSND, 1, MONTECAR_ATTRACT_INV, 0, NODE_60)

	/************************************************/
	/* Beep circuit is just the 8V signal           */
	/* 8V = HSYNC/16                                */
	/*    = 15750/16                                */
	/************************************************/
	DISCRETE_SQUAREWFIX(MONTECAR_BEEPSND, MONTECAR_BEEPER_EN, FIRETRUCK_8V, DEFAULT_TTL_V_LOGIC_1, 50.0, DEFAULT_TTL_V_LOGIC_1/2, 0.0)

	/************************************************/
	/* Combine all 5 sound sources.                 */
	/************************************************/
	DISCRETE_MIXER5(NODE_90, 1, MONTECAR_BEEPSND, MONTECAR_SCREECHSND, MONTECAR_BANGSND, MONTECAR_DRONE_MOTORSND, MONTECAR_MOTORSND, &montecar_mixer)
	DISCRETE_OUTPUT(NODE_90, 1)
DISCRETE_SOUND_END
