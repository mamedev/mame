// license:BSD-3-Clause
// copyright-holders:Couriersud
/*

Battlezone sound info, courtesy of Al Kossow:

D7  motor enable            this enables the engine sound
D6  start LED
D5  sound enable            this enables ALL sound outputs
                            including the POKEY output
D4  engine rev en           this controls the engine speed
                            the engine sound is an integrated square
                            wave (saw tooth) that is frequency modulated
                            by engine rev.
D3  shell loud, soft/       explosion volume
D2  shell enable
D1  explosion loud, soft/   explosion volume
D0  explosion enable        gates a noise generator

*/

#include "emu.h"
#include "includes/bzone.h"

#include "sound/discrete.h"
#include "sound/pokey.h"

/* This sets an amount of gain boost to apply to the final signal
 * that will drive it into clipping.  The slider is ajusted by the
 * reverse factor, so that the final result is not clipped.
 * This allows for the user to easily adjust the sound into the clipping
 * range so it sounds more like a real cabinet.
 */
#define BZ_FINAL_GAIN   2

#define BZ_NOISE_CLOCK      12000

#define TTL_OUT 3.4

/*************************************
 *
 *  Discrete Sound Defines
 *
 *************************************/

/* Discrete Sound Input Nodes */
#define BZ_INPUT            NODE_01     /* at M2 LS273 */
#define BZ_INP_EXPLO        NODE_10_00
#define BZ_INP_EXPLOLS      NODE_10_01
#define BZ_INP_SHELL        NODE_10_02
#define BZ_INP_SHELLLS      NODE_10_03
#define BZ_INP_ENGREV       NODE_10_04
#define BZ_INP_SOUNDEN      NODE_10_05
#define BZ_INP_STARTLED     NODE_10_06
#define BZ_INP_MOTEN        NODE_10_07

/* Adjusters */
#define BZ_R11_POT          NODE_11

/* Discrete Sound Output Nodes */
#define BZ_NOISE            NODE_20
#define BZ_SHELL_SND        NODE_21
#define BZ_EXPLOSION_SND    NODE_22
#define BZ_ENGINE_SND       NODE_23
#define BZ_POKEY_SND        NODE_24

/* Parts List - Resistors */
#define BZ_R5           RES_K(1)
#define BZ_R6           RES_K(4.7)
#define BZ_R7           RES_K(1)
#define BZ_R8           RES_K(100)
#define BZ_R9           RES_K(22)
#define BZ_R10          RES_K(100)
#define BZ_R11          RES_K(250)
#define BZ_R12          RES_K(33)
#define BZ_R13          RES_K(10)
#define BZ_R14          RES_K(22)
#define BZ_R15          RES_K(1)
#define BZ_R16          RES_K(1)
#define BZ_R17          RES_K(22)
#define BZ_R18          RES_K(10)
#define BZ_R19          RES_K(33)
#define BZ_R20          RES_K(33)
#define BZ_R21          RES_K(33)
#define BZ_R25          RES_K(100)
#define BZ_R26          RES_K(33)
#define BZ_R27          RES_K(330)
#define BZ_R28          RES_K(100)
#define BZ_R29          RES_K(22)
#define BZ_R30          RES_K(10)
#define BZ_R31          RES_K(100)
#define BZ_R32          RES_K(330)
#define BZ_R33          RES_K(330)
#define BZ_R34          RES_K(33)
#define BZ_R35          RES_K(33)

/* Parts List - Capacitors */
#define BZ_C9           CAP_U(4.7)
#define BZ_C11          CAP_U(0.015)
#define BZ_C13          CAP_U(10)
#define BZ_C14          CAP_U(10)
#define BZ_C20          CAP_U(0.1)
#define BZ_C21          CAP_U(0.0047)
#define BZ_C22          CAP_U(0.0047)
#define BZ_C29          CAP_U(0.47)

/*************************************
 *
 *  Discrete Sound static structs
 *
 *************************************/


static const discrete_lfsr_desc bzone_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,                     /* Bit Length */
	0,                      /* Reset Value */
	3,                      /* Use Bit 10 (QC of second LS164) as F0 input 0 */
	14,                     /* Use Bit 23 (QH of third LS164) as F0 input 1 */
	DISC_LFSR_XOR,          /* F0 is XOR */
	DISC_LFSR_NOT_IN0,      /* F1 is inverted F0*/
	DISC_LFSR_REPLACE,      /* F2 replaces the shifted register contents */
	0x000001,               /* Everything is shifted into the first bit only */
	DISC_LFSR_FLAG_OUTPUT_SR_SN1, /* output the complete shift register to sub node 1*/
	15                  /* Output bit */
};

#if 0
static const discrete_op_amp_filt_info bzone_explo_0 =
{
		BZ_R18 + BZ_R19, 0, 0, 0,       /* r1, r2, r3, r4 */
		BZ_R33,                         /* rF */
		BZ_C22, 0, 0,                   /* c1, c2, c3 */
		0,                              /* vRef - not used */
		22, 0                           /* vP, vN */
};

static const discrete_op_amp_filt_info bzone_explo_1 =
{
		BZ_R18, 0, 0, 0,                /* r1, r2, r3, r4 */
		BZ_R33,                         /* rF */
		BZ_C22, 0, 0,                   /* c1, c2, c3 */
		0,                              /* vRef - not used */
		22, 0                           /* vP, vN */
};

static const discrete_op_amp_filt_info bzone_shell_0 =
{
		BZ_R13 + BZ_R12, 0, 0, 0,       /* r1, r2, r3, r4 */
		BZ_R32,                         /* rF */
		BZ_C21, 0, 0,                   /* c1, c2, c3 */
		0,                              /* vRef - not used */
		22, 0                           /* vP, vN */
};

static const discrete_op_amp_filt_info bzone_shell_1 =
{
		BZ_R13, 0, 0, 0,                /* r1, r2, r3, r4 */
		BZ_R32,                         /* rF */
		BZ_C21, 0, 0,                   /* c1, c2, c3 */
		0,                              /* vRef - not used */
		22, 0                           /* vP, vN */
};
#endif

static const discrete_555_desc bzone_vco_desc =
{
	DISC_555_OUT_DC,
	5.0,
	DEFAULT_555_CHARGE,
	1.0 // Logic output
};

static const discrete_mixer_desc bzone_eng_mixer_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{BZ_R20, BZ_R21, BZ_R34, BZ_R35},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	0, 0,
	BZ_C29,
	0, /* no out cap */
	0, TTL_OUT      /* inputs are logic */
};

static const discrete_mixer_desc bzone_final_mixer_desc =
{
	DISC_MIXER_IS_RESISTOR,
	{BZ_R25, BZ_R28, BZ_R26 + BZ_R20 / 4, BZ_R27},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	0, BZ_R29,
	0,
	BZ_C20, /* The speakers are driven by a +/- signal, just using the cap is good enough */
	0, 1
};


/************************************************************************
 *
 * Custom Battlezone filter
 *
 *         .------.         r2           c
 *         |     O|-----+--ZZZZ--+-------||---------.
 *         | 4066 |     |        |                  |
 *  IN0 >--|c    I|-.   Z r1     |       r5         |
 *         '------' |   Z        +------ZZZZ--------+
 *                  |   Z        |                  |
 *                 gnd  |        |           |\     |
 *                     gnd       |           | \    |
 *                               '-----------|- \   |
 *            r3                             |   >--+----> Netlist Node
 *  IN1 >----ZZZZ----------------+-----------|+ /
 *                               |           | /
 *                               Z r4        |/
 *                               Z
 *                               Z
 *                               |           VP = B+
 *                              gnd
 *
 ************************************************************************/
#define BZONE_CUSTOM_FILTER__IN0    DISCRETE_INPUT(0)
#define BZONE_CUSTOM_FILTER__IN1    DISCRETE_INPUT(1)
#define BZONE_CUSTOM_FILTER__R1     DISCRETE_INPUT(2)
#define BZONE_CUSTOM_FILTER__R2     DISCRETE_INPUT(3)
#define BZONE_CUSTOM_FILTER__R3     DISCRETE_INPUT(4)
#define BZONE_CUSTOM_FILTER__R4     DISCRETE_INPUT(5)
#define BZONE_CUSTOM_FILTER__R5     DISCRETE_INPUT(6)
#define BZONE_CUSTOM_FILTER__C      DISCRETE_INPUT(7)
#define BZONE_CUSTOM_FILTER__VP     DISCRETE_INPUT(8)

#define CD4066_R_ON     270

DISCRETE_CLASS_STEP_RESET(bzone_custom_filter, 1,
	double  m_v_in1_gain;
	double  m_v_p;
	double  m_exponent;
	double  m_gain[2];
	double  m_out_v;
);

DISCRETE_STEP(bzone_custom_filter)
{
	int     in0 = (BZONE_CUSTOM_FILTER__IN0 == 0) ? 0 : 1;
	double  v;

	if (BZONE_CUSTOM_FILTER__IN1 > 0)
		v = 0;

	v = BZONE_CUSTOM_FILTER__IN1 * m_v_in1_gain * m_gain[in0];
	if (v > m_v_p) v = m_v_p;
	if (v < 0) v = 0;

	m_out_v += (v - m_out_v) * m_exponent;
	set_output(0, m_out_v);
}

DISCRETE_RESET(bzone_custom_filter)
{
	m_gain[0] = BZONE_CUSTOM_FILTER__R1 + BZONE_CUSTOM_FILTER__R2;
	m_gain[0] = BZONE_CUSTOM_FILTER__R5 / m_gain[0] + 1;
	m_gain[1] = RES_2_PARALLEL(CD4066_R_ON, BZONE_CUSTOM_FILTER__R1) + BZONE_CUSTOM_FILTER__R2;
	m_gain[1] = BZONE_CUSTOM_FILTER__R5 / m_gain[1] + 1;
	m_v_in1_gain = RES_VOLTAGE_DIVIDER(BZONE_CUSTOM_FILTER__R3, BZONE_CUSTOM_FILTER__R4);
	m_v_p = BZONE_CUSTOM_FILTER__VP - OP_AMP_VP_RAIL_OFFSET;
	m_exponent = RC_CHARGE_EXP(BZONE_CUSTOM_FILTER__R5 * BZONE_CUSTOM_FILTER__C);;
	m_out_v = 0.0;
}

/*************************************
 *
 *  Discrete Sound Blocks
 *
 *************************************/

static DISCRETE_SOUND_START(bzone)

	/************************************************/
	/* Input register mapping for Battlezone        */
	/************************************************/
	DISCRETE_INPUT_DATA(BZ_INPUT)
	/* decode the bits */
	DISCRETE_BITS_DECODE(NODE_10, BZ_INPUT, 0, 7, 1)             /* IC M2, bits 0 - 7 */

	/* the pot is 250K, but we will use a smaller range to get a better adjustment range */
	DISCRETE_ADJUSTMENT(BZ_R11_POT, RES_K(75), RES_K(10), DISC_LINADJ, "R11")


	/************************************************/
	/* NOISE                                        */
	/************************************************/

	/* 12Khz clock is divided by two by B4 74LS109 */
	DISCRETE_LFSR_NOISE(BZ_NOISE,                               /* IC H4, pin 13 */
		1, 1, BZ_NOISE_CLOCK / 2, 1.0, 0, 0.5, &bzone_lfsr)

	/* divide by 2 */
	DISCRETE_COUNTER(NODE_31,                                   /* IC J5, pin 8 */
		1, 0, BZ_NOISE, 0, 1, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE)

	DISCRETE_BITS_DECODE(NODE_32, NODE_SUB(BZ_NOISE, 1), 11, 14, 1)     /* IC H4, pins 6, 10, 11, 12 */
	DISCRETE_LOGIC_NAND4(NODE_33,                               /* IC J4, pin 8 */
		NODE_32_00, NODE_32_01, NODE_32_02, NODE_32_03)         /* LSFR bits 11-14 */
	/* divide by 2 */
	DISCRETE_COUNTER(NODE_34,                                   /* IC J5, pin 6 */
		1, 0, NODE_33, 0, 1, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE)

	/************************************************/
	/* Shell                                        */
	/************************************************/
	DISCRETE_RC_CIRCUIT_1(NODE_40,                  /* IC J3, pin 9 */
		BZ_INP_SHELL, NODE_31,                      /* INP0, INP1 */
		BZ_R14 + BZ_R15, BZ_C9)
	DISCRETE_CUSTOM9(BZ_SHELL_SND, bzone_custom_filter,                 /* IC K5, pin 1 */
		BZ_INP_EXPLOLS, NODE_40,                    /* IN0, IN1 */
		BZ_R12, BZ_R13, BZ_R14, BZ_R15, BZ_R32,
		BZ_C21,
		22,                                         /* B+ of op-amp */
		nullptr)

	/************************************************/
	/* Explosion                                    */
	/************************************************/

	DISCRETE_RC_CIRCUIT_1(NODE_50,                  /* IC J3, pin 3 */
		BZ_INP_EXPLO, NODE_34,                      /* INP0, INP1 */
		BZ_R17 + BZ_R16, BZ_C14)
	DISCRETE_CUSTOM9(BZ_EXPLOSION_SND, bzone_custom_filter,             /* IC K5, pin 1 */
		BZ_INP_EXPLOLS, NODE_50,                    /* IN0, IN1 */
		BZ_R19, BZ_R18, BZ_R17, BZ_R16, BZ_R33,
		BZ_C22,
		22,                                         /* B+ of op-amp */
		nullptr)
	/************************************************/
	/* Engine                                       */
	/************************************************/

	DISCRETE_SWITCH(NODE_61,                                /* effect of IC L4, pin 2 */
		1, BZ_INP_ENGREV,                                   /* ENAB, SWITCH */
		5.0 * RES_VOLTAGE_DIVIDER(BZ_R7, BZ_R6),            /* INP0 */
		5.0 * RES_VOLTAGE_DIVIDER(BZ_R7, RES_2_PARALLEL(CD4066_R_ON + BZ_R5, BZ_R6)))   /* INP1 */
	/* R5, R6, R7 all affect the following circuit charge discharge rates */
	/* they are not emulated as their effect is less than the 5% component tolerance */
	DISCRETE_RCDISC3(NODE_62,                               /* IC K5, pin 7 */
		1, NODE_61, BZ_R8, BZ_R9, BZ_C13, -0.5)

	DISCRETE_555_ASTABLE_CV(NODE_63,                        /* IC F3, pin 3 */
		1,                                                  /* RESET */
		BZ_R10, BZ_R11_POT, BZ_C11,
		NODE_62,                                            /* CV - IC F3, pin 5 */
		&bzone_vco_desc)

	DISCRETE_LOGIC_INVERT(NODE_64, BZ_INP_MOTEN)
	DISCRETE_COUNTER(NODE_65,                               /* IC F4 */
		1, NODE_64, NODE_63,                                /* ENAB, RESET, CLK */
		4, 15, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE)        /* MIN, MAX, DIR, INIT, CLKTYPE */
	DISCRETE_TRANSFORM2(NODE_66, NODE_65, 7, "01>")         /* QD - IC F4, pin 11 */
	DISCRETE_TRANSFORM2(NODE_67, NODE_65, 15, "01=")        /* Ripple - IC F4, pin 15 */

	DISCRETE_COUNTER(NODE_68,                               /* IC F5 */
		1, NODE_64, NODE_63,                                /* ENAB, RESET, CLK */
		6, 15, DISC_COUNT_UP, 0, DISC_CLK_ON_R_EDGE)        /* MIN, MAX, DIR, INIT, CLKTYPE */
	DISCRETE_TRANSFORM2(NODE_69, NODE_68, 7, "01>")         /* QD - IC F5, pin 11 */
	DISCRETE_TRANSFORM2(NODE_70, NODE_68, 15, "01=")        /* Ripple - IC F5, pin 15 */

	DISCRETE_MIXER4(BZ_ENGINE_SND, 1, NODE_66, NODE_67, NODE_69, NODE_70, &bzone_eng_mixer_desc)

	/************************************************/
	/* FINAL MIX                                    */
	/************************************************/
	/* We won't bother emulating the final gain of op-amp IC K5, pin 14.
	 * This signal never reaches a value where it clips, so we will
	 * just output the final 16-bit level.
	 */

	/* Convert Pokey output to 5V Signal */
	DISCRETE_INPUTX_STREAM(BZ_POKEY_SND, 0, 5.0 / 32768, 0)

	DISCRETE_MIXER4(NODE_280,
		BZ_INP_SOUNDEN,
		BZ_SHELL_SND, BZ_EXPLOSION_SND, BZ_ENGINE_SND, BZ_POKEY_SND,
		&bzone_final_mixer_desc)
	DISCRETE_OUTPUT(NODE_280, 48000)

DISCRETE_SOUND_END

WRITE8_MEMBER(bzone_state::bzone_sounds_w)
{
	m_discrete->write(space, BZ_INPUT, data);

	output().set_value("startled", (data >> 6) & 1);
	machine().sound().system_enable(data & 0x20);
}


MACHINE_CONFIG_FRAGMENT( bzone_audio )

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("pokey", POKEY, BZONE_MASTER_CLOCK / 8)
	MCFG_POKEY_ALLPOT_R_CB(IOPORT("IN3"))
	MCFG_POKEY_OUTPUT_RC(RES_K(10), CAP_U(0.015), 5.0)
	MCFG_SOUND_ROUTE_EX(0, "discrete", 1.0, 0)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(bzone)

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END
