/************************************************************************
 * sprint8 Sound System Analog emulation
 * Sept 2009, Derrick Renaud
 ************************************************************************/

#include "driver.h"
#include "sprint8.h"


/* Discrete Sound Input Nodes */
#define SPRINT8_CRASH_EN			NODE_01
#define SPRINT8_SCREECH_EN			NODE_02
#define SPRINT8_ATTRACT_EN			NODE_03
#define SPRINT8_MOTOR1_EN			NODE_04
#define SPRINT8_MOTOR2_EN			NODE_05
#define SPRINT8_MOTOR3_EN			NODE_06
#define SPRINT8_MOTOR4_EN			NODE_07
#define SPRINT8_MOTOR5_EN			NODE_08
#define SPRINT8_MOTOR6_EN			NODE_09
#define SPRINT8_MOTOR7_EN			NODE_10
#define SPRINT8_MOTOR8_EN			NODE_11

/* Discrete Sound Output Nodes */
#define SPRINT8_NOISE				NODE_12
#define SPRINT8_MOTOR1_SND			NODE_13
#define SPRINT8_MOTOR2_SND			NODE_14
#define SPRINT8_MOTOR3_SND			NODE_15
#define SPRINT8_MOTOR4_SND			NODE_16
#define SPRINT8_MOTOR5_SND			NODE_17
#define SPRINT8_MOTOR6_SND			NODE_18
#define SPRINT8_MOTOR7_SND			NODE_19
#define SPRINT8_MOTOR8_SND			NODE_20
#define SPRINT8_CRASH_SCREECH_SND	NODE_21
#define SPRINT8_AUDIO_1_2			NODE_22
#define SPRINT8_AUDIO_3_7			NODE_23
#define SPRINT8_AUDIO_5_6			NODE_24
#define SPRINT8_AUDIO_4_8			NODE_25


/* Parts List - Resistors */
#define SPRINT8_R1		RES_K(47)
#define SPRINT8_R3		RES_K(47)
#define SPRINT8_R4		RES_K(47)
#define SPRINT8_R19		RES_K(1)
#define SPRINT8_R20		RES_K(1)
#define SPRINT8_R27		RES_K(18)
#define SPRINT8_R28		820
#define SPRINT8_R29		RES_K(330)
#define SPRINT8_R39		RES_K(120)
#define SPRINT8_R40		RES_K(22)
#define SPRINT8_R41		RES_K(150)
#define SPRINT8_R89		RES_K(22)
#define SPRINT8_R91		RES_K(47)
#define SPRINT8_R93		RES_K(2.2)
#define SPRINT8_R96		RES_K(47)
#define SPRINT8_R97		RES_K(2.2)
#define SPRINT8_R99		RES_K(27)
#define SPRINT8_R100	RES_K(1)
#define SPRINT8_R145	RES_K(3.3)
#define SPRINT8_R146	RES_K(7.5)
#define SPRINT8_R147	100
#define SPRINT8_R148	RES_K(1)

/* Parts List - Capacitors */
#define SPRINT8_C17		CAP_U(.001)
#define SPRINT8_C18		CAP_U(.047)
#define SPRINT8_C19		CAP_U(.047)
#define SPRINT8_C26		CAP_U(100)
#define SPRINT8_C27		CAP_U(.22)
#define SPRINT8_C28		CAP_U(.1)
#define SPRINT8_C59		CAP_U(.1)
#define SPRINT8_C64		CAP_U(.1)
#define SPRINT8_C89		CAP_U(.1)
#define SPRINT8_C90		CAP_U(.1)

#define SPRINT8_HSYNC	15750.0		/* not checked */
#define SPRINT8_1V	SPRINT8_HSYNC/2
#define SPRINT8_2V	SPRINT8_1V/2


static const discrete_lfsr_desc sprint8_lfsr =
{
	DISC_CLK_IS_FREQ,
	16,					/* Bit Length */
	0,					/* Reset Value */
	10,					/* Use Bit 10 as XOR input 0 */
	15,					/* Use Bit 15 as XOR input 1 */
	DISC_LFSR_XOR,		/* Feedback stage1 is XOR */
	DISC_LFSR_OR,		/* Feedback stage2 is just stage 1 output OR with external feed */
	DISC_LFSR_REPLACE,	/* Feedback stage3 replaces the shifted register contents */
	0x000001,			/* Everything is shifted into the first bit only */
	DISC_LFSR_FLAG_RESET_TYPE_H,	/* Output is not inverted */
	12					/* Output bit */
};

static const discrete_555_desc sprint8_crash_555a_desc =
{
	DISC_555_OUT_ENERGY,
	5, DEFAULT_555_VALUES
};

static const discrete_integrate_info sprint8_crash_integrate =
{
	DISC_INTEGRATE_OP_AMP_1,
	SPRINT8_R99, SPRINT8_R97, SPRINT8_R96, SPRINT8_C59,		/*r1, r2, r3, c, */
	5, 5,							/* v1, vP*/
	0, 0, 0							/* no functions */
};

static const discrete_555_desc sprint8_motor_555a_desc =
{
	DISC_555_OUT_COUNT_F_X,
	5, DEFAULT_555_VALUES
};

static const discrete_555_desc sprint8_motor_555m_desc =
{
	DISC_555_OUT_ENERGY | DISC_555_TRIGGER_IS_COUNT,
	5, DEFAULT_555_VALUES
};

static const discrete_op_amp_filt_info sprint8_motor_filter =
{
	SPRINT8_R27, 0, SPRINT8_R28 + RES_2_PARALLEL(SPRINT8_R19, SPRINT8_R20), 0, SPRINT8_R29,	/* r1, r2, r3, r4, rF, */
	SPRINT8_C18, SPRINT8_C19, 0, 									/* c1, c2, c3, */
	5.0 * RES_VOLTAGE_DIVIDER(SPRINT8_R19, SPRINT8_R20), 5, 0		/* vRef, vP, vN */
};

static const discrete_mixer_desc sprint8_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{SPRINT8_R1 + SPRINT8_R100, SPRINT8_R3, SPRINT8_R4},
	{0}, {0}, 0, 0, 0, 0, 0, 1		/* no r_nodes, c, rI, rF, cF, cAmp, vRef, gain */
};


/************************************************
 * Car Motor
 ************************************************/
/* The first (astable) 555 generates a quick falling pulse that triggers the second (monostable) 555.
 * This pulse is passed through C17 and pulled up with R40.  This pulse is too fast to emulate so
 * we will just tell the monostable it was triggered once and ignore C17/R40.
 */
#define SPRINT8_MOTOR_CIRCUIT(_car)                                                         	\
DISCRETE_RCFILTER(NODE_RELATIVE(NODE_30, _car - 1), 1, NODE_RELATIVE(SPRINT8_MOTOR1_EN, _car - 1), SPRINT8_R89, SPRINT8_C26) 	\
DISCRETE_ADDER2(NODE_RELATIVE(NODE_40, _car - 1), 1, NODE_RELATIVE(NODE_30, _car - 1), 0.7)	/* add Q21 shift */ 				\
DISCRETE_555_ASTABLE_CV(NODE_RELATIVE(NODE_50, _car - 1), 1, SPRINT8_R39, 0, SPRINT8_C27, NODE_RELATIVE(NODE_40, _car - 1), &sprint8_motor_555a_desc) \
DISCRETE_555_MSTABLE(NODE_RELATIVE(NODE_60, _car - 1), 1, NODE_RELATIVE(NODE_50, _car - 1), SPRINT8_R41, SPRINT8_C28, &sprint8_motor_555m_desc)       \
DISCRETE_OP_AMP_FILTER(NODE_RELATIVE(SPRINT8_MOTOR1_SND, _car - 1), 1, NODE_RELATIVE(NODE_60, _car - 1), 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &sprint8_motor_filter)


DISCRETE_SOUND_START( sprint8 )
	/************************************************
     * Input register mapping
     ************************************************/
	DISCRETE_INPUT_NOT  (SPRINT8_CRASH_EN)
	DISCRETE_INPUT_NOT  (SPRINT8_SCREECH_EN)
	DISCRETE_INPUT_NOT  (SPRINT8_ATTRACT_EN)
	DISCRETE_INPUTX_LOGIC(SPRINT8_MOTOR1_EN, DEFAULT_TTL_V_LOGIC_1, 0, 0)
	DISCRETE_INPUTX_LOGIC(SPRINT8_MOTOR2_EN, DEFAULT_TTL_V_LOGIC_1, 0, 0)
	DISCRETE_INPUTX_LOGIC(SPRINT8_MOTOR3_EN, DEFAULT_TTL_V_LOGIC_1, 0, 0)
	DISCRETE_INPUTX_LOGIC(SPRINT8_MOTOR4_EN, DEFAULT_TTL_V_LOGIC_1, 0, 0)
	DISCRETE_INPUTX_LOGIC(SPRINT8_MOTOR5_EN, DEFAULT_TTL_V_LOGIC_1, 0, 0)
	DISCRETE_INPUTX_LOGIC(SPRINT8_MOTOR6_EN, DEFAULT_TTL_V_LOGIC_1, 0, 0)
	DISCRETE_INPUTX_LOGIC(SPRINT8_MOTOR7_EN, DEFAULT_TTL_V_LOGIC_1, 0, 0)
	DISCRETE_INPUTX_LOGIC(SPRINT8_MOTOR8_EN, DEFAULT_TTL_V_LOGIC_1, 0, 0)

	/************************************************
     * Noise Generator, Crash, Screech
     ************************************************/
	DISCRETE_LFSR_NOISE(SPRINT8_NOISE,			/* IC F7, pin 13 */
		1,										/* ENAB */
		SPRINT8_ATTRACT_EN,						/* RESET */
		SPRINT8_2V, 1, 0, 0.5, &sprint8_lfsr)	/* CLK,AMPL,FEED,BIAS,LFSRTB */

	DISCRETE_CONSTANT(NODE_70, 0)
//    DISCRETE_CUSTOM4(NODE_70, SPRINT8_NOISE, SPRINT8_R148, SPRINT8_R147, SPRINT8_C90, &sprint8_custom_screech_charge)
	DISCRETE_555_ASTABLE_CV(NODE_71,
		SPRINT8_SCREECH_EN,						/* RESET */
		SPRINT8_R145, SPRINT8_R146, SPRINT8_C89,
		NODE_70,								/* CTRLV */
		&sprint8_crash_555a_desc)
	DISCRETE_INTEGRATE(NODE_72, SPRINT8_CRASH_EN, 0, &sprint8_crash_integrate)
	DISCRETE_RCDISC2(NODE_73,
		SPRINT8_NOISE,							/* SWITCH - inverted by Q20 */
		NODE_72, SPRINT8_R93 + SPRINT8_R91,		/* INP0,RVAL0 */
		0, SPRINT8_R91,							/* INP1,RVAL1 */
		SPRINT8_C64)							/* CVAL */

	DISCRETE_CONSTANT(SPRINT8_CRASH_SCREECH_SND, 0)

	/************************************************
     * Car Motor
     ************************************************/
	DISCRETE_TASK_START()
	SPRINT8_MOTOR_CIRCUIT(1)
	SPRINT8_MOTOR_CIRCUIT(2)
	DISCRETE_MIXER3(SPRINT8_AUDIO_1_2,
		SPRINT8_ATTRACT_EN,				/* ENAB */
		SPRINT8_CRASH_SCREECH_SND,
		SPRINT8_MOTOR1_SND,
		SPRINT8_MOTOR2_SND,
		&sprint8_mixer)
	DISCRETE_TASK_END()

	DISCRETE_TASK_START()
	SPRINT8_MOTOR_CIRCUIT(3)
	SPRINT8_MOTOR_CIRCUIT(7)
	DISCRETE_MIXER3(SPRINT8_AUDIO_3_7,
		SPRINT8_ATTRACT_EN,				/* ENAB */
		SPRINT8_CRASH_SCREECH_SND,
		SPRINT8_MOTOR3_SND,
		SPRINT8_MOTOR7_SND,
		&sprint8_mixer)
	DISCRETE_TASK_END()

	DISCRETE_TASK_START()
	SPRINT8_MOTOR_CIRCUIT(5)
	SPRINT8_MOTOR_CIRCUIT(6)
	DISCRETE_MIXER3(SPRINT8_AUDIO_5_6,
		SPRINT8_ATTRACT_EN,				/* ENAB */
		SPRINT8_CRASH_SCREECH_SND,
		SPRINT8_MOTOR5_SND,
		SPRINT8_MOTOR6_SND,
		&sprint8_mixer)
	DISCRETE_TASK_END()

	DISCRETE_TASK_START()
	SPRINT8_MOTOR_CIRCUIT(4)
	SPRINT8_MOTOR_CIRCUIT(8)
	DISCRETE_MIXER3(SPRINT8_AUDIO_4_8,
		SPRINT8_ATTRACT_EN,				/* ENAB */
		SPRINT8_CRASH_SCREECH_SND,
		SPRINT8_MOTOR4_SND,
		SPRINT8_MOTOR8_SND,
		&sprint8_mixer)
	DISCRETE_TASK_END()

	/************************************************
     * Final Mix
     ************************************************/
	DISCRETE_OUTPUT(SPRINT8_AUDIO_1_2, 32700.0/8)
	DISCRETE_OUTPUT(SPRINT8_AUDIO_3_7, 32700.0/8)
	DISCRETE_OUTPUT(SPRINT8_AUDIO_5_6, 32700.0/8)
	DISCRETE_OUTPUT(SPRINT8_AUDIO_4_8, 32700.0/8)
DISCRETE_SOUND_END


WRITE8_DEVICE_HANDLER( sprint8_crash_w )
{
	discrete_sound_w(device, SPRINT8_CRASH_EN, data & 0x01);
}

WRITE8_DEVICE_HANDLER( sprint8_screech_w )
{
	discrete_sound_w(device, SPRINT8_SCREECH_EN, data & 0x01);
}

WRITE8_DEVICE_HANDLER( sprint8_attract_w )
{
	discrete_sound_w(device, SPRINT8_ATTRACT_EN, data & 0x01);
}

WRITE8_DEVICE_HANDLER( sprint8_motor_w )
{
	discrete_sound_w(device, NODE_RELATIVE(SPRINT8_MOTOR1_EN, offset & 0x07), data & 0x01);
}
