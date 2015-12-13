// license:???
// copyright-holders:Derrick Renaud
/************************************************************************
 * skyraid Sound System Analog emulation
 * Sept 2009, Derrick Renaud
 ************************************************************************/

#include "emu.h"
#include "includes/skyraid.h"


/* Discrete Sound Input Nodes */
#define SKYRAID_PLANE_SWEEP_EN      NODE_01
#define SKYRAID_MISSILE_EN          NODE_02
#define SKYRAID_EXPLOSION_EN        NODE_03
#define SKYRAID_PLANE_ON_EN         NODE_04
#define SKYRAID_PLANE_ON__IC_E8     SKYRAID_PLANE_ON_EN
#define SKYRAID_ATTRACT_EN          NODE_05

/* Discrete Sound Output Nodes */
#define SKYRAID_NOISE               NODE_10
#define SKYRAID_EXPLOSION_SND       NODE_11
#define SKYRAID_JET_A_SND           NODE_12
#define SKYRAID_JET_B_SND           NODE_13
#define SKYRAID_MSL_A_SND           NODE_14
#define SKYRAID_MSL_B_SND           NODE_15
#define SKYRAID_PLANE_SND           NODE_16

/* Parts List - Resistors */
#define SKYRAID_R12     RES_K(1)
#define SKYRAID_R13     RES_K(10)
#define SKYRAID_R14     RES_K(100)
#define SKYRAID_R16     RES_K(10)
#define SKYRAID_R18     RES_K(10)
#define SKYRAID_R19     RES_K(10)
#define SKYRAID_R20     RES_K(47)
#define SKYRAID_R24     RES_M(10)
#define SKYRAID_R25     RES_M(3.9)
#define SKYRAID_R27     RES_M(1.5)
#define SKYRAID_R28     RES_M(1.5)
#define SKYRAID_R29     RES_M(4.7)
#define SKYRAID_R30     RES_M(1)
#define SKYRAID_R31     RES_M(1)
#define SKYRAID_R32     RES_M(4.7)
#define SKYRAID_R84     RES_K(22)
#define SKYRAID_R85     RES_K(100)
#define SKYRAID_R86     820
#define SKYRAID_R110    RES_K(330)
#define SKYRAID_R118    RES_K(470)
#define SKYRAID_R119    RES_K(10)
#define SKYRAID_R120    RES_K(220)
#define SKYRAID_R121    RES_K(1)
#define SKYRAID_R122    RES_K(150)

/* Parts List - Capacitors */
#define SKYRAID_C44     CAP_U(4.7)
#define SKYRAID_C45     CAP_U(.0047)
#define SKYRAID_C46     CAP_U(.001)
#define SKYRAID_C48     CAP_U(.0047)
#define SKYRAID_C49     CAP_U(.1)
#define SKYRAID_C50     CAP_U(.001)
#define SKYRAID_C51     CAP_U(.01)
#define SKYRAID_C68     CAP_U(10)
#define SKYRAID_C85     CAP_U(.068)
#define SKYRAID_C86     CAP_U(.1)
#define SKYRAID_C93     CAP_U(.1)


static const discrete_lfsr_desc skyraid_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,                 /* Bit Length */
	0,                  /* Reset Value */
	0,                  /* Use Bit 0 as XOR input 0 */
	14,                 /* Use Bit 14 as XOR input 1 */
	DISC_LFSR_XNOR,     /* Feedback stage1 is XNOR */
	DISC_LFSR_OR,       /* Feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,  /* Feedback stage3 replaces the shifted register contents */
	0x000001,           /* Everything is shifted into the first bit only */
	DISC_LFSR_FLAG_RESET_TYPE_H,    /* Output is not inverted */
	15                  /* Output bit */
};

static const discrete_op_amp_filt_info skyraid_explosion_filter =
{
	SKYRAID_R85, 0, SKYRAID_R86, 0, SKYRAID_R110,   /* r1, r2, r3, r4, rF*/
	SKYRAID_C85, SKYRAID_C86, 0,    /* c1, c2, c3 */
	0, 12, -5,  /* vRef, vP, vN */
};

static const discrete_dac_r1_ladder skyraid_plane_dac =
{
	2, {SKYRAID_R28, SKYRAID_R27},
	0, 0, 0, 0              /* no vBias, rBias, rGnd, cFilter */
};

static const discrete_mixer_desc skyraid_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{SKYRAID_R120, SKYRAID_R32, SKYRAID_R29, SKYRAID_R30, SKYRAID_R31, RES_2_PARALLEL(SKYRAID_R28, SKYRAID_R27)},
	{0, 0, 0, 0, 0, SKYRAID_PLANE_ON__IC_E8},   /* r_nodes */
	{0}, 0, 0, 0, 0, 0, 1       /* no c, rI, rF, cF, cAmp, vRef, gain */
};


/************************************************************************
 *
 * Custom skyraid missle charge
 *
 * input[0]    - In1 (Logic)
 * input[1]    - R1
 * input[2]    - R2
 * input[3]    - R3
 * input[4]    - C
 *
 *              12V            5V
 *               v              v
 *               |              |
 *               Z             ---
 *               Z R1           ^   1N914
 *               Z             / \  Diode
 *               |            -----
 *               |              |
 *               +--------------+
 *               |              |
 *              ---             Z
 *              --- C           Z R2
 *               |              Z
 *               |              |
 *               +--------------+----> Node Output
 *               |
 *               Z
 *               Z R3
 *         O.C.  Z
 *          |\   |
 *   In1 >--| o--+
 *          |/
 *
 ************************************************************************/
#define SKYRAID_MISSLE_CUSTOM_IN1       DISCRETE_INPUT(0)
#define SKYRAID_MISSLE_CUSTOM_R1        DISCRETE_INPUT(1)
#define SKYRAID_MISSLE_CUSTOM_R2        DISCRETE_INPUT(2)
#define SKYRAID_MISSLE_CUSTOM_R3        DISCRETE_INPUT(3)
#define SKYRAID_MISSLE_CUSTOM_C         DISCRETE_INPUT(4)

DISCRETE_CLASS_STEP_RESET(skyraid_missle_custom_charge, 2,
		double m_v_charge[2];
		double m_v_cap;
		double m_exp[2];
);

/* the high charge is clamped by the diode to 0.7V above the 5V line */
#define SKYRAID_MISSLE_CHARGE_PLUS  (5.0 + 0.7)

DISCRETE_STEP( skyraid_missle_custom_charge )
{
	int in_1 = (SKYRAID_MISSLE_CUSTOM_IN1 == 0) ? 0 : 1;

	/* charge/discharge cap */
	m_v_cap += (m_v_charge[in_1] - m_v_cap) * m_exp[in_1];

	set_output(0,  SKYRAID_MISSLE_CHARGE_PLUS - m_v_cap);
}

DISCRETE_RESET( skyraid_missle_custom_charge )
{
	/* everything is based on the input to the O.C. inverter */

	/* the charging voltage across the cap */
	m_v_charge[0] = 0;
	m_v_charge[1] = SKYRAID_MISSLE_CHARGE_PLUS * RES_VOLTAGE_DIVIDER(SKYRAID_MISSLE_CUSTOM_R1 + SKYRAID_MISSLE_CUSTOM_R2, SKYRAID_MISSLE_CUSTOM_R3);
	m_v_charge[1] = SKYRAID_MISSLE_CHARGE_PLUS - m_v_charge[1];
	m_v_cap = 0;

	/* precalculate charging exponents */
	/* discharge cap */
	m_exp[0] = RC_CHARGE_EXP(SKYRAID_MISSLE_CUSTOM_R2 * SKYRAID_MISSLE_CUSTOM_C);
	/* charge cap */
	m_exp[1] = RC_CHARGE_EXP(RES_2_PARALLEL(SKYRAID_MISSLE_CUSTOM_R1 + SKYRAID_MISSLE_CUSTOM_R2, SKYRAID_MISSLE_CUSTOM_R3) * SKYRAID_MISSLE_CUSTOM_C);

	/* starts at full voltage until cap starts charging */
	set_output(0,  SKYRAID_MISSLE_CHARGE_PLUS);
}



DISCRETE_SOUND_START( skyraid )
	/************************************************
	 * Input register mapping
	 ************************************************/
	/* convert the PLANE_SWEEP line to voltage*/
	DISCRETE_INPUTX_LOGIC(SKYRAID_PLANE_SWEEP_EN, DEFAULT_TTL_V_LOGIC_1 * RES_VOLTAGE_DIVIDER(SKYRAID_R25, SKYRAID_R24), 0, 0)
	DISCRETE_INPUT_LOGIC(SKYRAID_MISSILE_EN)
	DISCRETE_INPUT_LOGIC(SKYRAID_EXPLOSION_EN)
	/* convert PLANE_ON into 4066 Ron value of 270 ohms @ 5V */
	DISCRETE_INPUTX_LOGIC(SKYRAID_PLANE_ON__IC_E8, 270, 0, 0)
	DISCRETE_INPUT_LOGIC(SKYRAID_ATTRACT_EN)

	/************************************************
	 * Noise Generator, Explosion sound
	 ************************************************/
	/* Note: the noise reset is not emulated 100% accurate */
	/* According to the schematics, Attract only clears the lower 8 bits. */
	/* I may modify the noise module in the future to properly emulate this, */
	/* but you will never hear the difference. */
	/* It only wrong for 8 cycles of the 555 while no sound is being generated. */
	DISCRETE_LFSR_NOISE(SKYRAID_NOISE,          /* IC F7, pin 13 */
		1,                                      /* ENAB */
		SKYRAID_ATTRACT_EN,                     /* RESET */
		1.49 / ((SKYRAID_R20 + 2 * SKYRAID_R19) * SKYRAID_C51),     /* CLK - 555 astable source */
		1, 0, 0.5, &skyraid_lfsr)               /* AMPL, FEED, BIAS */

	DISCRETE_LOGIC_NOR(NODE_20, SKYRAID_EXPLOSION_EN, SKYRAID_NOISE)
	DISCRETE_RC_CIRCUIT_1(NODE_21,
		SKYRAID_EXPLOSION_EN, NODE_20,          /* INP0, INP1 */
		RES_2_PARALLEL(SKYRAID_R84, SKYRAID_R85 + SKYRAID_R86), SKYRAID_C68)
	DISCRETE_OP_AMP_FILTER(NODE_22,
		1,                                      /* ENAB */
		NODE_21, 0,                             /* INP0, INP1 */
		DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &skyraid_explosion_filter)  /* TYPE,INFO */
	/* IC E10, pin 14 gain and clipping */
	DISCRETE_GAIN(NODE_23, NODE_22, SKYRAID_R119 / SKYRAID_R121 + 1)
	DISCRETE_CLAMP(SKYRAID_EXPLOSION_SND, NODE_23, -5, 12.0 - 1.5)

	/************************************************
	 * Jet, Plane sound
	 ************************************************/
	DISCRETE_RCFILTER(NODE_30,              /* IC J6, pin 5 */
		SKYRAID_PLANE_SWEEP_EN,             /* IN0 */
		RES_2_PARALLEL(SKYRAID_R25, SKYRAID_R24), SKYRAID_C49)
	DISCRETE_566(NODE_31,                   /* IC J6, pin 3 */
		NODE_30,                            /* VMOD */
		SKYRAID_R18, SKYRAID_C48,
		5, -5, 5,                           /* VPOS,VNEG,VCHARGE */
		DISC_566_OUT_COUNT_R)
	DISCRETE_LOGIC_SHIFT(NODE_32,           /* IC H7, J7 output */
		SKYRAID_NOISE,                      /* IC H7, J7 pins 1 & 2 */
		1, NODE_31, 16,                     /* RESET, CLK, SIZE */
		DISC_LOGIC_SHIFT__RESET_L | DISC_LOGIC_SHIFT__LEFT | DISC_CLK_BY_COUNT)
	/* move bits together for ease of use */
	DISCRETE_TRANSFORM4(NODE_33, NODE_32, 1, 1 << 14, 2, "01&02/3&|")
	DISCRETE_DAC_R1(SKYRAID_PLANE_SND,
		NODE_33,                            /* DATA */
		DEFAULT_TTL_V_LOGIC_1, &skyraid_plane_dac)
	DISCRETE_BIT_DECODE(SKYRAID_JET_A_SND, NODE_32, 0, DEFAULT_TTL_V_LOGIC_1)   /* IC H7, pin 3 */
	DISCRETE_BIT_DECODE(SKYRAID_JET_B_SND, NODE_32, 15, DEFAULT_TTL_V_LOGIC_1)  /* IC J7, pin 13 */

	/************************************************
	 * Missle sound
	 ************************************************/
	DISCRETE_CUSTOM5(NODE_40, skyraid_missle_custom_charge, SKYRAID_MISSILE_EN, SKYRAID_R12, SKYRAID_R14, SKYRAID_R13, SKYRAID_C44, nullptr)
	DISCRETE_566(NODE_41,                   /* IC K6, pin 3 */
		NODE_40,                            /* VMOD */
		SKYRAID_R16, SKYRAID_C45,
		5, -5, SKYRAID_MISSLE_CHARGE_PLUS,  /* VPOS,VNEG,VCHARGE */
		DISC_566_OUT_COUNT_R)
	DISCRETE_LOGIC_SHIFT(NODE_42,           /* IC K7, L7 output */
		SKYRAID_NOISE,                      /* IC K7, L7 pins 1 & 2 */
		1, NODE_41, 16,                     /* RESET, CLK, SIZE */
		DISC_LOGIC_SHIFT__RESET_L | DISC_LOGIC_SHIFT__LEFT | DISC_CLK_BY_COUNT)
	DISCRETE_BIT_DECODE(SKYRAID_MSL_A_SND, NODE_42, 0, DEFAULT_TTL_V_LOGIC_1)   /* IC K7, pin 3 */
	DISCRETE_BIT_DECODE(SKYRAID_MSL_B_SND, NODE_42, 15, DEFAULT_TTL_V_LOGIC_1)  /* IC L7, pin 13 */

	/************************************************
	 * Final Mix
	 ************************************************/
	DISCRETE_LOGIC_INVERT(NODE_91, SKYRAID_ATTRACT_EN)
	DISCRETE_MIXER6(NODE_92,
		NODE_91,                /* ENAB */
		SKYRAID_EXPLOSION_SND,
		SKYRAID_JET_A_SND,
		SKYRAID_JET_B_SND,
		SKYRAID_MSL_A_SND,
		SKYRAID_MSL_B_SND,
		SKYRAID_PLANE_SND,
		&skyraid_mixer)
	DISCRETE_CRFILTER(NODE_93,
		NODE_92,                /* IN0 */
		SKYRAID_R122 + RES_6_PARALLEL(SKYRAID_R120, SKYRAID_R32, SKYRAID_R29, SKYRAID_R30, SKYRAID_R31, RES_2_PARALLEL(SKYRAID_R27, SKYRAID_R28)),
		SKYRAID_C93)
	/* IC E10, pin 1 gain and clipping */
	DISCRETE_GAIN(NODE_94, NODE_93, - SKYRAID_R118 / (SKYRAID_R122 + RES_5_PARALLEL(SKYRAID_R120, SKYRAID_R32, SKYRAID_R29, SKYRAID_R30, SKYRAID_R31)))
	DISCRETE_CLAMP(NODE_95, NODE_94, -5, 12.0 - 1.5)
	DISCRETE_OUTPUT(NODE_95, 32700.0/8)
DISCRETE_SOUND_END


WRITE8_MEMBER(skyraid_state::skyraid_sound_w)
{
	/* BIT0 => PLANE SWEEP */
	/* BIT1 => MISSILE     */
	/* BIT2 => EXPLOSION   */
	/* BIT3 => START LAMP  */
	/* BIT4 => PLANE ON    */
	/* BIT5 => ATTRACT     */

	m_discrete->write(space, SKYRAID_PLANE_SWEEP_EN, data & 0x01);
	m_discrete->write(space, SKYRAID_MISSILE_EN, data & 0x02);
	m_discrete->write(space, SKYRAID_EXPLOSION_EN, data & 0x04);
	set_led_status(machine(), 0, !(data & 0x08));
	m_discrete->write(space, SKYRAID_PLANE_ON_EN, data & 0x10);
	m_discrete->write(space, SKYRAID_ATTRACT_EN, data & 0x20);
}
