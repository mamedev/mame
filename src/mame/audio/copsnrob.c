/************************************************************************
 * copsnrob Sound System Analog emulation
 * Nov 2010, Derrick Renaud
 ************************************************************************/
#include "emu.h"
#include "includes/copsnrob.h"
#include "sound/discrete.h"


/* Discrete Sound Input Nodes */
#define COPSNROB_MOTOR0_INV			NODE_01
#define COPSNROB_MOTOR1_INV			NODE_02
#define COPSNROB_MOTOR2_INV			NODE_03
#define COPSNROB_MOTOR3_INV			NODE_04
#define COPSNROB_ZINGS_INV			NODE_05
#define COPSNROB_FIRES_INV			NODE_06
#define COPSNROB_CRASH_INV			NODE_07
#define COPSNROB_SCREECH_INV		NODE_08
#define COPSNROB_AUDIO_ENABLE		NODE_09

/* Discrete Sound Output Nodes */
#define COPSNROB_MOTOR0_SND			NODE_11
#define COPSNROB_MOTOR1_SND			NODE_12
#define COPSNROB_MOTOR2_SND			NODE_13
#define COPSNROB_MOTOR3_SND			NODE_14
#define COPSNROB_FZ_SND				NODE_15
#define COPSNROB_CRASH_SND			NODE_16
#define COPSNROB_SCREECH_SND		NODE_17
#define COPSNROB_NOISE_1			NODE_18
#define COPSNROB_NOISE_2			NODE_18_01

/* Parts List - Resistors */
#define COPSNROB_R16			RES_K(10)
#define COPSNROB_R18			RES_K(100)
#define COPSNROB_R19			RES_K(100)
#define COPSNROB_R20			RES_K(100)
#define COPSNROB_R21			RES_K(47)
#define COPSNROB_R25			RES_K(47)
#define COPSNROB_R26			RES_K(150)
#define COPSNROB_R27			RES_K(22)
#define COPSNROB_R28			RES_K(18)
#define COPSNROB_R33			RES_K(10)
#define COPSNROB_R35			RES_K(10)
#define COPSNROB_R36			RES_K(15)
#define COPSNROB_R37			RES_K(4.7)
#define COPSNROB_R38			RES_K(15)
#define COPSNROB_R39			RES_K(10)
#define COPSNROB_R41			RES_K(33)
#define COPSNROB_R42			RES_K(33)
#define COPSNROB_R47			RES_K(10)
#define COPSNROB_R48			RES_K(1)
#define COPSNROB_R49			RES_K(47)
#define COPSNROB_R50			RES_K(47)
#define COPSNROB_R53			RES_K(47)
#define COPSNROB_R54			RES_K(22)
#define COPSNROB_R55			RES_K(47)
#define COPSNROB_R56			RES_K(470)
#define COPSNROB_R57			RES_K(820)
#define COPSNROB_R58			RES_K(68)
#define COPSNROB_R59			RES_K(1)
#define COPSNROB_R64			RES_K(47)
#define COPSNROB_R65			RES_K(4.7)
#define COPSNROB_R66			RES_K(1.5)
#define COPSNROB_R69			RES_K(330)
#define COPSNROB_R70			RES_K(330)
#define COPSNROB_R71			RES_K(100)
#define COPSNROB_R72			RES_K(100)
#define COPSNROB_R73			RES_K(47)
#define COPSNROB_R74			RES_K(1)
#define COPSNROB_R75			RES_K(1)
#define COPSNROB_R76			RES_K(12)
#define COPSNROB_R77			RES_K(33)
#define COPSNROB_R78			RES_K(100)
#define COPSNROB_R79			RES_K(2.2)
#define COPSNROB_R80			RES_K(1)
#define COPSNROB_R81			RES_K(8.2)
#define COPSNROB_R82			RES_K(3.9)
#define COPSNROB_R83			RES_K(27)
#define COPSNROB_R84			RES_K(15)
#define COPSNROB_R85			(330)
#define COPSNROB_R86			RES_K(330)
#define COPSNROB_R87			(820)
#define COPSNROB_R88			RES_K(1)
#define COPSNROB_R89			RES_K(1)
#define COPSNROB_R92			RES_K(100)
#define COPSNROB_R93			RES_K(100)
#define COPSNROB_R94			RES_K(10)

/* Parts List - Capacitors */
#define COPSNROB_C3				CAP_U(100)
#define COPSNROB_C12			CAP_U(0.1)
#define COPSNROB_C13			CAP_U(0.1)
#define COPSNROB_C17			CAP_U(0.001)
#define COPSNROB_C19			CAP_U(10)
#define COPSNROB_C20			CAP_U(0.01)
#define COPSNROB_C23			CAP_U(0.1)
#define COPSNROB_C24			CAP_U(0.22)
#define COPSNROB_C28			CAP_U(1)
#define COPSNROB_C30			CAP_U(0.01)
#define COPSNROB_C31			CAP_U(0.1)
#define COPSNROB_C32			CAP_U(5)
#define COPSNROB_C33			CAP_U(10)
#define COPSNROB_C36			CAP_U(100)
#define COPSNROB_C39			CAP_U(0.1)
#define COPSNROB_C37			CAP_U(0.1)
#define COPSNROB_C40			CAP_U(0.1)
#define COPSNROB_C41			CAP_U(0.03)
#define COPSNROB_C42			CAP_U(0.047)
#define COPSNROB_C43			CAP_U(0.047)
#define COPSNROB_C55			CAP_U(0.1)
#define COPSNROB_C58			CAP_U(0.1)
#define COPSNROB_C59			CAP_U(0.1)

/* Timing values */
#define COPSNROB_2V				(15750/2/2)	/* has to be verified */


static const discrete_555_desc copsnrob_motor23_555_1 =
{
	DISC_555_OUT_SQW,
	5, DEFAULT_555_VALUES
};

static const discrete_555_desc copsnrob_motor23_555_2 =
{
	DISC_555_OUT_ENERGY | DISC_555_TRIGGER_IS_LOGIC,
	5, DEFAULT_555_VALUES
};

static const discrete_op_amp_filt_info copsnrob_motor23_filter =
{
	COPSNROB_R28, 0, COPSNROB_R87, 0, COPSNROB_R86,					/* r1, r2, r3, r4, rF */
	COPSNROB_C42, COPSNROB_C43, 0,									/* c1, c2, c3 */
	5.0 * RES_VOLTAGE_DIVIDER(COPSNROB_R88, COPSNROB_R89), 5, 0		/* vRef, vP, vN */
};

static const discrete_dac_r1_ladder copsnrob_crash_dac =
{
	4,														/* ladderLength */
	{COPSNROB_R81, COPSNROB_R82, COPSNROB_R79, COPSNROB_R80},
	0, 0, 0, COPSNROB_C58									/* vBias, rBias, rGnd, cFilter */
};

static const discrete_mixer_desc copsnrob_final_mixer01 =
{
	DISC_MIXER_IS_RESISTOR,
	{COPSNROB_R21, COPSNROB_R25, COPSNROB_R33, COPSNROB_R70, COPSNROB_R71},
	{0}, {0}, 0, COPSNROB_R35, 0, COPSNROB_C59,				/* r_node{}, c{}, rI, rF, cF, cAmp */
	0, 1													/* vRef, gain */
};

static const discrete_mixer_desc copsnrob_final_mixer23 =
{
	DISC_MIXER_IS_RESISTOR,
	{COPSNROB_R93, COPSNROB_R92, COPSNROB_R72, COPSNROB_R69, COPSNROB_R16},
	{0}, {0}, 0, COPSNROB_R94, 0, COPSNROB_C55,				/* r_node{}, c{}, rI, rF, cF, cAmp */
	0, 1													/* vRef, gain */
};

static const discrete_dac_r1_ladder copsnrob_motor23_cv_dac =
{
	1,										/* ladderLength */
	{COPSNROB_R65},
	5, RES_K(5), RES_K(10), COPSNROB_C36	/* vBias; rBias; rGnd; cFilter */
};

#define COPSNROB_MOTOR01_BASE_NODE		NODE_20
#define COPSNROB_MOTOR23_BASE_NODE		NODE_30
#define COPSNROB_NODE(_base, _num, _offset)		NODE_RELATIVE(_base, _num * 100 + _offset)
#define COPSNROB_MOTOR01_NODE(_num, _offset)	COPSNROB_NODE(COPSNROB_MOTOR01_BASE_NODE, _num, _offset)
#define COPSNROB_MOTOR23_NODE(_num, _offset)	COPSNROB_NODE(COPSNROB_MOTOR23_BASE_NODE, _num, _offset)


/************************************************
 * MOTOR2/3 Definition Start
 ************************************************/
#define COPSNROB_MOTOR23(_output, _input, _num)													\
	 /* simulate the RC connected to the 555 CV pin with a DAC */								\
	DISCRETE_DAC_R1(COPSNROB_MOTOR23_NODE(_num, 1),												\
	 	_input, 4.2, &copsnrob_motor23_cv_dac)						/* DATA; VDATA - TTL with light load */		\
	DISCRETE_555_ASTABLE_CV(COPSNROB_MOTOR23_NODE(_num, 2),			/* IC J2, pin 5 */			\
		1,															/* RESET */					\
		COPSNROB_R64, COPSNROB_R42, COPSNROB_C24,												\
		COPSNROB_MOTOR23_NODE(_num, 1),								/* CTRLV - IC J2, pin 3 */	\
		&copsnrob_motor23_555_1)																\
	/* R27 and C17 can be ignored, we will just trigger on the logic */							\
	DISCRETE_555_MSTABLE(COPSNROB_MOTOR23_NODE(_num, 3),			/* IC J3, pin 9 */			\
		1,															/* RESET */					\
		COPSNROB_MOTOR23_NODE(_num, 2),								/* IC J3, pin 8 */			\
		COPSNROB_R41, COPSNROB_C23,																\
		&copsnrob_motor23_555_2)																\
	DISCRETE_OP_AMP_FILTER(_output,									/* IC L4, pin 7 */			\
		1,															/* ENAB */					\
		COPSNROB_MOTOR23_NODE(_num, 3), 0,							/* INP0; INP1 */			\
		DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &copsnrob_motor23_filter)
/************************************************
 * MOTOR2/3 Definition End
 ************************************************/


/************************************************
 * CUSTOM_NOISE Definition Start
 * - output is energy
 ************************************************/
#define COPSNROB_CUSTOM_NOISE__FREQ		DISCRETE_INPUT(0)

struct copsnrob_custom_noise_context
{
	int		flip_flop;
	int		noise1_had_xtime;
	int		noise2_had_xtime;
	UINT8	high_byte;
	UINT8	low_byte;
	double	t_used;
	double	t1;
};

#define COPSNROB_CUSTOM_NOISE_HIGH	4.2

DISCRETE_STEP(copsnrob_custom_noise)
{
	DISCRETE_DECLARE_CONTEXT(copsnrob_custom_noise)

	double	t_used = context->t_used;
	double	t1 = context->t1;
	double	x_time = 0;
	UINT8	low_byte = context->low_byte;
	UINT8	high_byte = context->high_byte;
	UINT8	xnor_out;							/* IC F2, pin 2 */
	int		last_noise1_bit = (low_byte >> 4) & 0x01;
	int		last_noise2_bit = (low_byte >> 5) & 0x01;

	t_used += node->info->sample_time;

	/* This clock will never run faster then the sample rate,
	 * so we do not bother to check.
	 */
	if (t_used > t1)
	{
		/* calculate the overshoot time */
		t_used -= t1;
		context->flip_flop ^= 1;
		/* clocks on low to high */
		if (context->flip_flop)
		{
			int new_noise_bit;

			/* shift */
			xnor_out = (((low_byte >> 6) & 0x01) ^ (high_byte & 0x01)) ^ 0x01;
			low_byte = (low_byte << 1) | ((high_byte >> 7) & 0x01);
			high_byte = (high_byte << 1) | xnor_out;
			if (high_byte == 0xff)		/* IC H1, pin 8 */
				high_byte = 0;
			context->low_byte = low_byte;
			context->high_byte = high_byte;

			/* Convert last switch time to a ratio */
			x_time = t_used / node->info->sample_time;
			/* use x_time if bit changed */
			new_noise_bit = (low_byte >> 4) & 0x01;
			if (last_noise1_bit != new_noise_bit)
			{
				node->output[0] = COPSNROB_CUSTOM_NOISE_HIGH * (new_noise_bit ? x_time : (1.0 - x_time));
				context->noise1_had_xtime = 1;
			}
			new_noise_bit = (low_byte >> 5) & 0x01;
			if (last_noise2_bit != new_noise_bit)
			{
				node->output[1] = COPSNROB_CUSTOM_NOISE_HIGH * (new_noise_bit ? x_time : (1.0 - x_time));
				context->noise2_had_xtime = 1;
			}
		}
	}
	else
	{
		/* see if we need to move from x_time state to full state */
		if (context->noise1_had_xtime)
		{
			node->output[0] = COPSNROB_CUSTOM_NOISE_HIGH * last_noise1_bit;
			context->noise1_had_xtime = 0;
		}
		if (context->noise2_had_xtime)
		{
			node->output[1] = COPSNROB_CUSTOM_NOISE_HIGH * last_noise2_bit;
			context->noise2_had_xtime = 0;
		}
	}

	context->t_used = t_used;
}

DISCRETE_RESET(copsnrob_custom_noise)
{
	DISCRETE_DECLARE_CONTEXT(copsnrob_custom_noise)

	context->t1 = 0.5 / COPSNROB_CUSTOM_NOISE__FREQ ;
	context->flip_flop = 0;
	context->low_byte = 0;
	context->high_byte = 0;
	context->t_used = 0;
	context->noise1_had_xtime = 0;
	context->noise2_had_xtime = 0;
}

static const discrete_custom_info copsnrob_custom_noise =
{
	DISCRETE_CUSTOM_MODULE( copsnrob_custom_noise, struct copsnrob_custom_noise_context),
	NULL
};
/************************************************
 * CUSTOM_NOISE Definition End
 ************************************************/


#if (0)
/************************************************
 * CUSTOM_ZINGS_555_MONOSTABLE Definition Start
 * - output is energy
 ************************************************/
#define COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__TRIG	DISCRETE_INPUT(0)
#define COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__R		DISCRETE_INPUT(1)
#define COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__C		DISCRETE_INPUT(2)

struct copsnrob_zings_555_monostable_context
{
	double	rc;
	double	exponent;
	double	v_cap;
	int		flip_flop;
};

DISCRETE_STEP(copsnrob_zings_555_monostable)
{
	DISCRETE_DECLARE_CONTEXT(copsnrob_zings_555_monostable)

	const double v_threshold = 5.0 * 2 / 3;
	const double v_out_high = 5.0 - 0.5;	/* light load */

	/* int ff_reset = (COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__TRIG > v_threshold ? 1 : 0; */
	int		ff_set = COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__TRIG < (5.0 / 3) ? 1 : 0;
	int 	flip_flop = context->flip_flop;
	double	v_cap = context->v_cap;
	double	x_time = 0;

	/* From testing a real IC */
	/* Trigger going low overides everything.  It forces the FF/Output high.
	 * If Threshold is high, the output will still go high as long as trigger is low.
	 * The output will then go low when trigger rises above it's 1/3VCC value.
	 * If threshold is below it's 2/3VCC value, the output will remain high.
	 */
	if (ff_set)
	{
		flip_flop = 1;
		context->flip_flop = flip_flop;
	}

	if (flip_flop)
	{
		double	v_diff = v_out_high - v_cap;

		/* charge */
		v_cap += v_diff * context->exponent;
		/* no state change if trigger is low */
		if (!ff_set && (v_cap > v_threshold))
		{
			double rc = context->rc;

			flip_flop = 0;
			context->flip_flop = flip_flop;
			/* calculate overshoot */
			x_time = rc * log(1.0 / (1.0 - ((v_cap - v_threshold) / v_diff)));
			/* discharge the overshoot */
			v_cap = v_threshold;
			v_cap -= v_cap * RC_CHARGE_EXP_DT(rc, x_time);
			x_time /= node->info->sample_time;
		}
	}
	else
	{
		/* Optimization - already discharged */
		if (v_cap == 0)
			return;
		/* discharge */
		v_cap -= v_cap * context->exponent;
		/* Optimization - close enough to 0 to be 0 */
		if (v_cap < 0.0001)
			v_cap = 0;
	}
	context->v_cap = v_cap;

	if (x_time > 0)
		node->output[0] = v_out_high * x_time;
	else if (flip_flop)
		node->output[0] = v_out_high;
	else
		node->output[0] = 0;
}

DISCRETE_RESET(copsnrob_zings_555_monostable)
{
	DISCRETE_DECLARE_CONTEXT(copsnrob_zings_555_monostable)

	context->rc = COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__R * COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__C;
	context->exponent = RC_CHARGE_EXP(context->rc);
	context->v_cap = 0;
	context->flip_flop = 0;
	node->output[0] = 0;
}

static const discrete_custom_info copsnrob_zings_555_monostable =
{
	DISCRETE_CUSTOM_MODULE( copsnrob_zings_555_monostable, struct copsnrob_zings_555_monostable_context),
	NULL
};
/************************************************
 * CUSTOM_ZINGS_555_MONOSTABLE Definition End
 ************************************************/


/************************************************
 * CUSTOM_ZINGS_555_ASTABLE Definition Start
 * - output is energy
 ************************************************/
#define COPSNROB_CUSTOM_ZINGS_555_ASTABLE__RESET	DISCRETE_INPUT(0)
#define COPSNROB_CUSTOM_ZINGS_555_ASTABLE__R1		DISCRETE_INPUT(1)
#define COPSNROB_CUSTOM_ZINGS_555_ASTABLE__R2		DISCRETE_INPUT(2)
#define COPSNROB_CUSTOM_ZINGS_555_ASTABLE__C1		DISCRETE_INPUT(3)
#define COPSNROB_CUSTOM_ZINGS_555_ASTABLE__C2		DISCRETE_INPUT(4)

struct copsnrob_zings_555_astable_context
{
	double	r1c1;
	double	r2c2;
	double	exponent1;
	double	exponent2;
	double	v_cap1;
	double	v_cap2;
	int		flip_flop;
};

DISCRETE_STEP(copsnrob_zings_555_astable)
{
	DISCRETE_DECLARE_CONTEXT(copsnrob_zings_555_astable)

	int		reset_active = COPSNROB_CUSTOM_ZINGS_555_ASTABLE__RESET < 0.7;
	int 	flip_flop = context->flip_flop;
	double	v_cap1 = context->v_cap1;
	double	v_cap2 = context->v_cap2;
	double	x_time = 0;

	if (reset_active)
	{
		if (flip_flop)
		{
			flip_flop = 0;
			context->flip_flop = 0;
		}
		/* Optimization - only discharge if needed */
		if (v_cap1 != 0)
		{
			/* discharge */
			v_cap1 -= v_cap1 * context->exponent;
			/* Optimization - close enough to 0 to be 0 */
			if (v_cap1 < 0.0001)
				v_cap1 = 0;
		}
	}
	else
	{
		/* this oscillator will never create a frequency greater then 1/2 the sample rate,
		 * so we won't worry about missing samples */
		if (flip_flop)
		{
			/* The reset voltage also charges the CV cap */
			double	v_diff1 = COPSNROB_CUSTOM_ZINGS_555_ASTABLE__RESET - v_cap1;

			/* charge */
			v_cap1 += v_diff1 * context->exponent1;

			if (v_cap1 > v_threshold)
			{
				double r1c1 = context->r1c1;

				flip_flop = 0;
				context->flip_flop = flip_flop;
				/* calculate overshoot */
				x_time = r1c1 * log(1.0 / (1.0 - ((v_cap1 - v_threshold) / v_diff1)));
				/* discharge the overshoot */
				v_cap1 = v_threshold;
				v_cap1 -= v_cap1 * RC_CHARGE_EXP_DT(r1c1, x_time);
				x_time /= node->info->sample_time;
			}
		}

	}

}

DISCRETE_RESET(copsnrob_zings_555_astable)
{
	DISCRETE_DECLARE_CONTEXT(copsnrob_zings_555_astable)

	context->r1c1 = COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__R1 * COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__C1;
	context->r2c2 = COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__R2 * COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__C2;
	context->exponent1 = RC_CHARGE_EXP(context->r1c1);
	context->exponent2 = RC_CHARGE_EXP(context->r2c2);
	context->v_cap1 = 0;
	context->v_cap2 = 0;
	context->flip_flop = 0;
	node->output[0] = 0;
}

static const discrete_custom_info copsnrob_zings_555_astable =
{
	DISCRETE_CUSTOM_MODULE( copsnrob_zings_555_astable, struct copsnrob_zings_555_astable_context),
	NULL
};
/************************************************
 * CUSTOM_ZINGS_555_ASTABLE Definition End
 ************************************************/
#endif


DISCRETE_SOUND_START(copsnrob)

	/************************************************
	 * Input register mapping
	 ************************************************/
	DISCRETE_INPUTX_LOGIC(COPSNROB_MOTOR0_INV, 3.8, 0, 0)	/* TTL at high load */
	DISCRETE_INPUTX_LOGIC(COPSNROB_MOTOR1_INV, 3.8, 0, 0)	/* TTL at high load */
	DISCRETE_INPUT_LOGIC(COPSNROB_MOTOR2_INV)
	DISCRETE_INPUT_LOGIC(COPSNROB_MOTOR3_INV)
	DISCRETE_INPUT_LOGIC(COPSNROB_ZINGS_INV)
	DISCRETE_INPUT_LOGIC(COPSNROB_FIRES_INV)
	DISCRETE_INPUT_NOT(COPSNROB_CRASH_INV)					/* inverted for counter use */
	DISCRETE_INPUT_LOGIC(COPSNROB_SCREECH_INV)
	DISCRETE_INPUT_NOT(COPSNROB_AUDIO_ENABLE)				/* IC A1, pins 2 & 12 */

	/************************************************
	 * MOTOR0/1
	 ************************************************/
	 DISCRETE_CONSTANT(COPSNROB_MOTOR0_SND, 0)
	 DISCRETE_CONSTANT(COPSNROB_MOTOR1_SND, 0)

	/************************************************
	 * MOTOR2/3
	 ************************************************/
	COPSNROB_MOTOR23(COPSNROB_MOTOR2_SND, COPSNROB_MOTOR2_INV, 0)
	COPSNROB_MOTOR23(COPSNROB_MOTOR3_SND, COPSNROB_MOTOR3_INV, 1)

	/************************************************
	 * CRASH
	 ************************************************/
	 DISCRETE_CUSTOM1(COPSNROB_NOISE_1,						/* IC J2, pin 10 */
		COPSNROB_2V,										/* CLK */
		&copsnrob_custom_noise)
	/* COPSNROB_NOISE_2 derived from sub out of above custom module - IC J2, pin 11 */
	/* We use the measured 555 timer frequency (IC M3) for speed */
	DISCRETE_COUNTER(NODE_40,								/* IC L2 */
		NODE_41,											/* ENAB - IC L2, pin 14 */
		COPSNROB_CRASH_INV,									/* RESET - IC L2, pin 11 */
		92,													/* IC L2, pin 4 - freq measured */
		0, 15, DISC_COUNT_DOWN, 15, DISC_CLK_IS_FREQ)		/* MIN; MAX; DIR; INIT0; CLKTYPE */
	DISCRETE_TRANSFORM2(NODE_41,							/* IC M2, pin 3 - goes high at count 0 */
		NODE_40, 0,	"01=!")									/* -we will invert it for use by the counter module */
	DISCRETE_SWITCH(NODE_42,								/* IC L3 */
		1, COPSNROB_NOISE_2, 0, NODE_40)					/* ENAB; SWITCH; INP0; INP1 */
	DISCRETE_DAC_R1(COPSNROB_CRASH_SND,
		NODE_42, 3.8,										/* DATA; VDATA */
		&copsnrob_crash_dac)
//DISCRETE_WAVLOG2(COPSNROB_NOISE_1, 1000, COPSNROB_NOISE_2, 1000)
//DISCRETE_WAVLOG2(NODE_42, 1000, COPSNROB_CRASH_SND, 1000)

	/************************************************
	 * SCREECH
	 ************************************************/
	 DISCRETE_CONSTANT(COPSNROB_SCREECH_SND, 0)

	/************************************************
	 * FZ (Fires, Zings)
	 ************************************************/
#if (0)
	DISCRETE_CUSTOM2(NODE_60,							/* IC D3, pin 5 */
		COPSNROB_R38, COPSNROB_C19,
		&copsnrob_zings_555_monostable)
	DISCRETE_CUSTOM4(NODE_60,							/* IC D3, pin 8 & 12 */
		COPSNROB_R36, COPSNROB_R37,
		COPSNROB_C3, COPSNROB_C13,
		&copsnrob_zings_555_astable)

#else
	 DISCRETE_CONSTANT(COPSNROB_FZ_SND, 0)
#endif

	/************************************************
	 * MIXER
	 ************************************************/
	 DISCRETE_MIXER5(NODE_90,								/* IC B3, pin 3 */
		COPSNROB_AUDIO_ENABLE,								/* ENAB */
		COPSNROB_MOTOR1_SND, COPSNROB_MOTOR0_SND, COPSNROB_FZ_SND, COPSNROB_SCREECH_SND, COPSNROB_CRASH_SND,
		&copsnrob_final_mixer01)
	 DISCRETE_MIXER5(NODE_91,								/* IC P3, pin 3 */
		COPSNROB_AUDIO_ENABLE,								/* ENAB */
		COPSNROB_MOTOR3_SND, COPSNROB_MOTOR2_SND, COPSNROB_CRASH_SND, COPSNROB_SCREECH_SND, COPSNROB_FZ_SND,
		&copsnrob_final_mixer23)
	 DISCRETE_OUTPUT(NODE_90, 32767.0*3.5)
	 DISCRETE_OUTPUT(NODE_91, 32767.0*3.5)
DISCRETE_SOUND_END


WRITE8_HANDLER( copsnrob_misc_w )
{
	device_t *device = space->machine->device("discrete");
	copsnrob_state *state = space->machine->driver_data<copsnrob_state>();
	UINT8 latched_data = state->ic_h3_data;
	UINT8 special_data = data & 0x01;

	/* ignore if no change */
	if (((latched_data >> offset) & 0x01) == special_data)
		return;

	if (special_data)
		latched_data |= 1 << offset;
	else
		latched_data &= ~(1 << offset);

	switch (offset)
	{
		case 0x00:
			discrete_sound_w(device, COPSNROB_MOTOR3_INV, special_data);
			break;

		case 0x01:
			discrete_sound_w(device, COPSNROB_MOTOR2_INV, special_data);
			break;

		case 0x02:
			discrete_sound_w(device, COPSNROB_MOTOR1_INV, special_data);
			break;

		case 0x03:
			discrete_sound_w(device, COPSNROB_MOTOR0_INV, special_data);
			break;

		case 0x04:
			discrete_sound_w(device, COPSNROB_SCREECH_INV, special_data);
			break;

		case 0x05:
			discrete_sound_w(device, COPSNROB_CRASH_INV, special_data);
			break;

		case 0x06:
			/* One Start */
			set_led_status(space->machine, 0, !special_data);
			break;

		case 0x07:
			discrete_sound_w(device, COPSNROB_AUDIO_ENABLE, special_data);
			break;

	}

	state->ic_h3_data = latched_data;
}
