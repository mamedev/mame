// license:BSD-3-Clause
// copyright-holders:Derrick Renaud
/************************************************************************
 * copsnrob Sound System Analog emulation
 * Nov 2010, Derrick Renaud
 ************************************************************************/
#include "emu.h"
#include "includes/copsnrob.h"
#include "sound/discrete.h"


/* Discrete Sound Input Nodes */
#define COPSNROB_MOTOR0_INV         NODE_01
#define COPSNROB_MOTOR1_INV         NODE_02
#define COPSNROB_MOTOR2_INV         NODE_03
#define COPSNROB_MOTOR3_INV         NODE_04
#define COPSNROB_ZINGS_INV          NODE_05
#define COPSNROB_FIRES_INV          NODE_06
#define COPSNROB_CRASH_INV          NODE_07
#define COPSNROB_SCREECH_INV        NODE_08
#define COPSNROB_AUDIO_ENABLE       NODE_09

/* Discrete Sound Output Nodes */
#define COPSNROB_MOTOR0_SND         NODE_11
#define COPSNROB_MOTOR1_SND         NODE_12
#define COPSNROB_MOTOR2_SND         NODE_13
#define COPSNROB_MOTOR3_SND         NODE_14
#define COPSNROB_FZ_SND             NODE_15
#define COPSNROB_CRASH_SND          NODE_16
#define COPSNROB_SCREECH_SND        NODE_17
#define COPSNROB_NOISE_1            NODE_18
#define COPSNROB_NOISE_2            NODE_18_01

/* Parts List - Resistors */
#define COPSNROB_R16            RES_K(10)
#define COPSNROB_R18            RES_K(100)
#define COPSNROB_R19            RES_K(100)
#define COPSNROB_R20            RES_K(100)
#define COPSNROB_R21            RES_K(47)
#define COPSNROB_R25            RES_K(47)
#define COPSNROB_R26            RES_K(150)
#define COPSNROB_R27            RES_K(22)
#define COPSNROB_R28            RES_K(18)
#define COPSNROB_R33            RES_K(10)
#define COPSNROB_R35            RES_K(10)
#define COPSNROB_R36            RES_K(15)
#define COPSNROB_R37            RES_K(4.7)
#define COPSNROB_R38            RES_K(15)
#define COPSNROB_R39            RES_K(10)
#define COPSNROB_R41            RES_K(33)
#define COPSNROB_R42            RES_K(33)
#define COPSNROB_R47            RES_K(10)
#define COPSNROB_R48            RES_K(1)
#define COPSNROB_R49            RES_K(47)
#define COPSNROB_R50            RES_K(47)
#define COPSNROB_R53            RES_K(47)
#define COPSNROB_R54            RES_K(22)
#define COPSNROB_R55            RES_K(47)
#define COPSNROB_R56            RES_K(470)
#define COPSNROB_R57            RES_K(820)
#define COPSNROB_R58            RES_K(68)
#define COPSNROB_R59            RES_K(1)
#define COPSNROB_R64            RES_K(47)
#define COPSNROB_R65            RES_K(4.7)
#define COPSNROB_R66            RES_K(1.5)
#define COPSNROB_R69            RES_K(330)
#define COPSNROB_R70            RES_K(330)
#define COPSNROB_R71            RES_K(100)
#define COPSNROB_R72            RES_K(100)
#define COPSNROB_R73            RES_K(47)
#define COPSNROB_R74            RES_K(1)
#define COPSNROB_R75            RES_K(1)
#define COPSNROB_R76            RES_K(12)
#define COPSNROB_R77            RES_K(33)
#define COPSNROB_R78            RES_K(100)
#define COPSNROB_R79            RES_K(2.2)
#define COPSNROB_R80            RES_K(1)
#define COPSNROB_R81            RES_K(8.2)
#define COPSNROB_R82            RES_K(3.9)
#define COPSNROB_R83            RES_K(27)
#define COPSNROB_R84            RES_K(15)
#define COPSNROB_R85            (330)
#define COPSNROB_R86            RES_K(330)
#define COPSNROB_R87            (820)
#define COPSNROB_R88            RES_K(1)
#define COPSNROB_R89            RES_K(1)
#define COPSNROB_R92            RES_K(100)
#define COPSNROB_R93            RES_K(100)
#define COPSNROB_R94            RES_K(10)

/* Parts List - Capacitors */
#define COPSNROB_C3             CAP_U(100)
#define COPSNROB_C12            CAP_U(0.1)
#define COPSNROB_C13            CAP_U(0.1)
#define COPSNROB_C17            CAP_U(0.001)
#define COPSNROB_C19            CAP_U(10)
#define COPSNROB_C20            CAP_U(0.01)
#define COPSNROB_C23            CAP_U(0.1)
#define COPSNROB_C24            CAP_U(0.22)
#define COPSNROB_C28            CAP_U(1)
#define COPSNROB_C30            CAP_U(0.01)
#define COPSNROB_C31            CAP_U(0.1)
#define COPSNROB_C32            CAP_U(5)
#define COPSNROB_C33            CAP_U(10)
#define COPSNROB_C36            CAP_U(100)
#define COPSNROB_C39            CAP_U(0.1)
#define COPSNROB_C37            CAP_U(0.1)
#define COPSNROB_C40            CAP_U(0.1)
#define COPSNROB_C41            CAP_U(0.03)
#define COPSNROB_C42            CAP_U(0.047)
#define COPSNROB_C43            CAP_U(0.047)
#define COPSNROB_C55            CAP_U(0.1)
#define COPSNROB_C58            CAP_U(0.1)
#define COPSNROB_C59            CAP_U(0.1)

/* Timing values */
#define COPSNROB_2V             (15750/2/2) /* has to be verified */


static const discrete_555_desc copsnrob_motor23_555_1 =
{
	DISC_555_OUT_SQW,
	5, DEFAULT_555_VALUES
};

static const discrete_555_desc copsnrob_motor23_555_2 =
{
	DISC_555_OUT_ENERGY | DISC_555_TRIGGER_IS_VOLTAGE,
	5, DEFAULT_555_VALUES
};

static const discrete_op_amp_filt_info copsnrob_motor23_filter =
{
	COPSNROB_R28, 0, COPSNROB_R87, 0, COPSNROB_R86,                 /* r1, r2, r3, r4, rF */
	COPSNROB_C42, COPSNROB_C43, 0,                                  /* c1, c2, c3 */
	5.0 * RES_VOLTAGE_DIVIDER(COPSNROB_R88, COPSNROB_R89), 5, 0     /* vRef, vP, vN */
};

static const discrete_dac_r1_ladder copsnrob_crash_dac =
{
	4,                                                      /* ladderLength */
	{COPSNROB_R81, COPSNROB_R82, COPSNROB_R79, COPSNROB_R80},
	0, 0, 0, COPSNROB_C58                                   /* vBias, rBias, rGnd, cFilter */
};

static const discrete_mixer_desc copsnrob_final_mixer01 =
{
	DISC_MIXER_IS_RESISTOR,
	{COPSNROB_R21, COPSNROB_R25, COPSNROB_R33, COPSNROB_R70, COPSNROB_R71},
	{0}, {0}, 0, COPSNROB_R35, 0, COPSNROB_C59,             /* r_node{}, c{}, rI, rF, cF, cAmp */
	0, 1                                                    /* vRef, gain */
};

static const discrete_dac_r1_ladder copsnrob_motor01_cc_dac =
{
	1,                                      /* ladderLength */
	{COPSNROB_R56},
	5.0 - 0.5, COPSNROB_R58, COPSNROB_R57, COPSNROB_C33 /* vBias; rBias; rGnd; cFilter */
};

static const discrete_dac_r1_ladder copsnrob_motor01_out_dac =
{
	4,                                      /* ladderLength */
	{COPSNROB_R20, COPSNROB_R18, 0, COPSNROB_R19},
	0, 0, 0, COPSNROB_C12                   /* vBias; rBias; rGnd; cFilter */
};

static const discrete_mixer_desc copsnrob_final_mixer23 =
{
	DISC_MIXER_IS_RESISTOR,
	{COPSNROB_R93, COPSNROB_R92, COPSNROB_R72, COPSNROB_R69, COPSNROB_R16},
	{0}, {0}, 0, COPSNROB_R94, 0, COPSNROB_C55,             /* r_node{}, c{}, rI, rF, cF, cAmp */
	0, 1                                                    /* vRef, gain */
};

static const discrete_dac_r1_ladder copsnrob_motor23_cv_dac =
{
	1,                                      /* ladderLength */
	{COPSNROB_R65},
	5, RES_K(5), RES_K(10), COPSNROB_C36    /* vBias; rBias; rGnd; cFilter */
};

static const discrete_555_cc_desc copsnrob_motor01_555cc =
{
	DISC_555_OUT_COUNT_R | DISCRETE_555_CC_TO_DISCHARGE_PIN,
	5,                          /* v_pos */
	DEFAULT_555_CC_SOURCE,
	1,                          /* v_out_high - ignored */
	0.6                         /* v_cc_junction */
};


#define COPSNROB_MOTOR01_BASE_NODE      NODE_20
#define COPSNROB_MOTOR23_BASE_NODE      NODE_30
#define COPSNROB_NODE(_base, _num, _offset)     NODE_RELATIVE(_base, _num * 100 + _offset)
#define COPSNROB_MOTOR01_NODE(_num, _offset)    COPSNROB_NODE(COPSNROB_MOTOR01_BASE_NODE, _num, _offset)
#define COPSNROB_MOTOR23_NODE(_num, _offset)    COPSNROB_NODE(COPSNROB_MOTOR23_BASE_NODE, _num, _offset)


/************************************************
 * MOTOR0/1 Definition Start
 ************************************************/
#define COPSNROB_MOTOR01(_output, _input, _num)                                                 \
		/* simulate the RC connected to the transistor with a DAC */                                \
	DISCRETE_DAC_R1(COPSNROB_MOTOR01_NODE(_num, 0),                                             \
		_input, 4.2, &copsnrob_motor01_cc_dac)                      /* DATA; VDATA - TTL with light load */     \
	DISCRETE_555_CC(COPSNROB_MOTOR01_NODE(_num, 1),                 /* IC F2, pin 10 from IC F3, pin 9 */       \
		1,                                                          /* RESET - IC F3, pin 10 */     \
		COPSNROB_MOTOR01_NODE(_num, 0),                             /* VIN */                       \
		COPSNROB_R53, COPSNROB_C20, 0, 0, COPSNROB_R39,             /* R; C; RBIAS; RGND; RDIS */   \
		&copsnrob_motor01_555cc)                                                                \
	/* IC D2, pin 12 and IC E3, pin 5 make a /4 counter */                                      \
	DISCRETE_COUNTER(COPSNROB_MOTOR01_NODE(_num, 2),                /* IC E3, pin 5 */          \
		1, 0,                                                       /* ENAB; RESET */           \
		COPSNROB_MOTOR01_NODE(_num, 1),                             /* IC D2, pin 1 */          \
		0, 3, DISC_COUNT_UP, 0, DISC_CLK_BY_COUNT)                  /* MIN; MAX; DIR; INIT0; CLKTYPE */ \
	DISCRETE_COUNTER_7492(COPSNROB_MOTOR01_NODE(_num, 3),           /* IC E3, pins 11, 9, & 8 */\
		1, 0,                                                       /* ENAB; RESET */           \
		COPSNROB_MOTOR01_NODE(_num, 1),                             /* IC E3, pin 14 */         \
		DISC_CLK_BY_COUNT)                                          /* CLKTYPE */               \
	DISCRETE_TRANSFORM3(COPSNROB_MOTOR01_NODE(_num, 4),                                         \
		COPSNROB_MOTOR01_NODE(_num, 2),                             /* INP0 */                  \
		COPSNROB_MOTOR01_NODE(_num, 3), 2,                          /* INP1, INP2 */            \
		"02&2/12*+")                                                /* get bits ready for DAC */\
	DISCRETE_DAC_R1(_output,                                                                    \
		COPSNROB_MOTOR01_NODE(_num, 4), 4.2, &copsnrob_motor01_out_dac)     /* DATA; VDATA - TTL with light load */

/************************************************
 * MOTOR0/1 Definition Start
 ************************************************/


/************************************************
 * MOTOR2/3 Definition Start
 ************************************************/
#define COPSNROB_MOTOR23(_output, _input, _num)                                                 \
		/* simulate the RC connected to the 555 CV pin with a DAC */                                \
	DISCRETE_DAC_R1(COPSNROB_MOTOR23_NODE(_num, 0),                                             \
		_input, 4.2, &copsnrob_motor23_cv_dac)                      /* DATA; VDATA - TTL with light load */     \
	DISCRETE_555_ASTABLE_CV(COPSNROB_MOTOR23_NODE(_num, 1),         /* IC J2, pin 5 */          \
		1,                                                          /* RESET */                 \
		COPSNROB_R64, COPSNROB_R42, COPSNROB_C24,                                               \
		COPSNROB_MOTOR23_NODE(_num, 0),                             /* CTRLV - IC J2, pin 3 */  \
		&copsnrob_motor23_555_1)                                                                \
	DISCRETE_CRFILTER_VREF(COPSNROB_MOTOR23_NODE(_num, 2),                                      \
		COPSNROB_MOTOR23_NODE(_num, 1),                                 /* IN0 */               \
		RES_3_PARALLEL(COPSNROB_R27, RES_K(10), RES_K(5)), COPSNROB_C17,    /* R is in parallel with 555 internal R */  \
		5.0 * RES_VOLTAGE_DIVIDER(RES_2_PARALLEL(COPSNROB_R27, RES_K(10)), RES_K(5)))   /* VREF */  \
	DISCRETE_555_MSTABLE(COPSNROB_MOTOR23_NODE(_num, 3),            /* IC J3, pin 9 */          \
		1,                                                          /* RESET */                 \
		COPSNROB_MOTOR23_NODE(_num, 2),                             /* IC J3, pin 8 */          \
		COPSNROB_R41, COPSNROB_C23,                                                             \
		&copsnrob_motor23_555_2)                                                                \
	DISCRETE_OP_AMP_FILTER(_output,                                 /* IC L4, pin 7 */          \
		1,                                                          /* ENAB */                  \
		COPSNROB_MOTOR23_NODE(_num, 3), 0,                          /* INP0; INP1 */            \
		DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &copsnrob_motor23_filter)
/************************************************
 * MOTOR2/3 Definition End
 ************************************************/


/************************************************
 * CUSTOM_NOISE Definition Start
 * - output is energy
 ************************************************/
#define COPSNROB_CUSTOM_NOISE__FREQ     DISCRETE_INPUT(0)

DISCRETE_CLASS_STEP_RESET(copsnrob_custom_noise, 2,
	int     m_flip_flop;
	int     m_noise1_had_xtime;
	int     m_noise2_had_xtime;
	UINT8   m_high_byte;
	UINT8   m_low_byte;
	double  m_t_used;
	double  m_t1;
);

#define COPSNROB_CUSTOM_NOISE_HIGH  4.2

DISCRETE_STEP(copsnrob_custom_noise)
{
	double  t_used = m_t_used;
	double  t1 = m_t1;
	double  x_time = 0;
	UINT8   low_byte = m_low_byte;
	UINT8   high_byte = m_high_byte;
	UINT8   xnor_out;                           /* IC F2, pin 2 */
	int     last_noise1_bit = (low_byte >> 4) & 0x01;
	int     last_noise2_bit = (low_byte >> 5) & 0x01;

	t_used += this->sample_time();

	/* This clock will never run faster then the sample rate,
	 * so we do not bother to check.
	 */
	if (t_used > t1)
	{
		/* calculate the overshoot time */
		t_used -= t1;
		m_flip_flop ^= 1;
		/* clocks on low to high */
		if (m_flip_flop)
		{
			int new_noise_bit;

			/* shift */
			xnor_out = (((low_byte >> 6) & 0x01) ^ (high_byte & 0x01)) ^ 0x01;
			low_byte = (low_byte << 1) | ((high_byte >> 7) & 0x01);
			high_byte = (high_byte << 1) | xnor_out;
			if (high_byte == 0xff)      /* IC H1, pin 8 */
				high_byte = 0;
			m_low_byte = low_byte;
			m_high_byte = high_byte;

			/* Convert last switch time to a ratio */
			x_time = t_used / this->sample_time();
			/* use x_time if bit changed */
			new_noise_bit = (low_byte >> 4) & 0x01;
			if (last_noise1_bit != new_noise_bit)
			{
				set_output(0,  COPSNROB_CUSTOM_NOISE_HIGH * (new_noise_bit ? x_time : (1.0 - x_time)));
				m_noise1_had_xtime = 1;
			}
			new_noise_bit = (low_byte >> 5) & 0x01;
			if (last_noise2_bit != new_noise_bit)
			{
				set_output(1, COPSNROB_CUSTOM_NOISE_HIGH * (new_noise_bit ? x_time : (1.0 - x_time)));
				m_noise2_had_xtime = 1;
			}
		}
	}
	else
	{
		/* see if we need to move from x_time state to full state */
		if (m_noise1_had_xtime)
		{
			set_output(0, COPSNROB_CUSTOM_NOISE_HIGH * last_noise1_bit);
			m_noise1_had_xtime = 0;
		}
		if (m_noise2_had_xtime)
		{
			set_output(1, COPSNROB_CUSTOM_NOISE_HIGH * last_noise2_bit);
			m_noise2_had_xtime = 0;
		}
	}

	m_t_used = t_used;
}

DISCRETE_RESET(copsnrob_custom_noise)
{
	m_t1 = 0.5 / COPSNROB_CUSTOM_NOISE__FREQ ;
	m_flip_flop = 0;
	m_low_byte = 0;
	m_high_byte = 0;
	m_noise1_had_xtime = 0;
	m_noise2_had_xtime = 0;
	m_t_used = 0;
}

/************************************************
 * CUSTOM_NOISE Definition End
 ************************************************/


/************************************************
 * CUSTOM_ZINGS_555_MONOSTABLE Definition Start
 * - output is energy
 ************************************************/
#define COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__TRIG  DISCRETE_INPUT(0)
#define COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__R     DISCRETE_INPUT(1)
#define COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__C     DISCRETE_INPUT(2)

DISCRETE_CLASS_STEP_RESET(copsnrob_zings_555_monostable, 1,
	double  m_rc;
	double  m_exponent;
	double  m_v_cap;
	int     m_flip_flop;
);

DISCRETE_STEP(copsnrob_zings_555_monostable)
{
	const double v_threshold = 5.0 * 2 / 3;
	const double v_out_high = 5.0 - 0.5;    /* light load */

	int     ff_set = COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__TRIG < (5.0 / 3) ? 1 : 0;
	int     flip_flop = m_flip_flop;
	double  v_cap = m_v_cap;
	double  x_time = 0;

	/* From testing a real IC */
	/* Trigger going low overides everything.  It forces the FF/Output high.
	 * If Threshold is high, the output will still go high as long as trigger is low.
	 * The output will then go low when trigger rises above it's 1/3VCC value.
	 * If threshold is below it's 2/3VCC value, the output will remain high.
	 */
	if (ff_set)
	{
		flip_flop = 1;
		m_flip_flop = flip_flop;
	}

	if (flip_flop)
	{
		double  v_diff = v_out_high - v_cap;

		/* charge */
		v_cap += v_diff * m_exponent;
		/* no state change if trigger is low */
		if (!ff_set && (v_cap > v_threshold))
		{
			double rc = m_rc;

			flip_flop = 0;
			m_flip_flop = flip_flop;
			/* calculate overshoot */
			x_time = rc * log(1.0 / (1.0 - ((v_cap - v_threshold) / v_diff)));
			/* discharge the overshoot */
			v_cap = v_threshold;
			v_cap -= v_cap * RC_CHARGE_EXP_DT(rc, x_time);
			x_time /= this->sample_time();
		}
	}
	else
	{
		/* Optimization - already discharged */
		if (v_cap == 0)
			return;
		/* discharge */
		v_cap -= v_cap * m_exponent;
		/* Optimization - close enough to 0 to be 0 */
		if (v_cap < 0.000001)
			v_cap = 0;
	}
	m_v_cap = v_cap;

	if (x_time > 0)
		set_output(0, v_out_high * x_time);
	else if (flip_flop)
		set_output(0, v_out_high);
	else
		set_output(0, 0.0);
}

DISCRETE_RESET(copsnrob_zings_555_monostable)
{
	m_rc = COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__R * COPSNROB_CUSTOM_ZINGS_555_MONOSTABLE__C;
	m_exponent = RC_CHARGE_EXP(m_rc);
	m_v_cap = 0;
	m_flip_flop = 0;
	set_output(0, 0.0);
}

/************************************************
 * CUSTOM_ZINGS_555_MONOSTABLE Definition End
 ************************************************/


/************************************************
 * CUSTOM_ZINGS_555_ASTABLE Definition Start
 * - output is energy
 ************************************************/
#define COPSNROB_CUSTOM_ZINGS_555_ASTABLE__RESET    DISCRETE_INPUT(0)
#define COPSNROB_CUSTOM_ZINGS_555_ASTABLE__R1       DISCRETE_INPUT(1)
#define COPSNROB_CUSTOM_ZINGS_555_ASTABLE__R2       DISCRETE_INPUT(2)
#define COPSNROB_CUSTOM_ZINGS_555_ASTABLE__C1       DISCRETE_INPUT(3)
#define COPSNROB_CUSTOM_ZINGS_555_ASTABLE__C2       DISCRETE_INPUT(4)

#define COPSNROB_CUSTOM_ZINGS_555_ASTABLE__HIGH     4.5

DISCRETE_CLASS_STEP_RESET(copsnrob_zings_555_astable, 1,
	double  m_r2c2;
	double  m_r_total_cv;
	double  m_exponent1;
	double  m_exponent2;
	double  m_v_cap1;
	double  m_v_cap2;
	int     m_flip_flop;
);

DISCRETE_STEP(copsnrob_zings_555_astable)
{
	double  v_trigger, v_threshold;
	double  v1 = COPSNROB_CUSTOM_ZINGS_555_ASTABLE__RESET;
	double  v_cap1 = m_v_cap1;
	double  v_cap2 = m_v_cap2;
	double  dt = 0;
	int     reset_active = (v1 < 0.7) ? 1 : 0;
	int     flip_flop = m_flip_flop;

	/* calculate voltage at CV pin */
	/* start by adding currents */
	double v_cv = 5.0 / RES_K(5);
	v_cv += v1 / COPSNROB_CUSTOM_ZINGS_555_ASTABLE__R1;
	/* convert to voltage */
	v_cv *= m_r_total_cv;

	/* The reset voltage also charges the CV cap */
	double v_diff1 = v_cv - v_cap1;
	/* optimization - if charged close enough to voltage */
	if (fabs(v_diff1) < 0.000001)
		v_cap1 = v_cv;
	else
		v_cap1 += v_diff1 * m_exponent1;
	m_v_cap1 = v_cap1;

	if (reset_active)
	{
		if (flip_flop)
			m_flip_flop = 0;
		/* we still need to discharge C2 */
		/* Optimization - only discharge if needed */
		if (v_cap2 != 0)
		{
			/* discharge */
			v_cap2 -= v_cap2 * m_exponent2;
			/* Optimization - close enough to 0 to be 0 */
			if (v_cap2 < 0.000001)
				set_output(0, 0.0);
			else
				set_output(0, v_cap2);
		}
		return;
	}

	v_threshold = v_cap1 * 2 / 3;
	v_trigger = v_cap1 / 3;

	/* This oscillator will never create a frequency greater then 1/2 the sample rate,
	 * so we won't worry about missing samples */
	/* No need to optimize the charge circuit.  It always charges/discharges to a voltage
	 * greater then it will ever reach. */
	if (flip_flop)
	{
		/* charge */
		double v_diff2 = COPSNROB_CUSTOM_ZINGS_555_ASTABLE__HIGH - v_cap2;
		v_cap2 += v_diff2 * m_exponent2;
		if (v_cap2 > v_threshold)
		{
			double r2c2 = m_r2c2;

			m_flip_flop = 0;
			/* calculate overshoot */
			dt = r2c2 * log(1.0 / (1.0 - ((v_cap2 - v_threshold) / v_diff2)));
			/* discharge the overshoot */
			v_cap2 = v_threshold;
			v_cap2 -= v_cap2 * RC_CHARGE_EXP_DT(r2c2, dt);
		}
	}
	else
	{
		/* discharge */
		double v_diff2 = v_cap2;
		v_cap2 -= v_diff2 * m_exponent2;
		if (v_cap2 < v_trigger)
		{
			double r2c2 = m_r2c2;

			m_flip_flop = 1;
			/* calculate overshoot */
			dt = r2c2 * log(1.0 / (1.0 - ((v_trigger - v_cap2) / v_diff2)));
			/* charge the overshoot */
			v_cap2 = v_trigger;
			v_cap2 += (COPSNROB_CUSTOM_ZINGS_555_ASTABLE__HIGH - v_cap2) * RC_CHARGE_EXP_DT(r2c2, dt);
		}
	}
	if (v_cap2 > 0)
		m_v_cap2 = v_cap2;
	else
		m_v_cap2 = 0.0;
	set_output(0, m_v_cap2);
}

DISCRETE_RESET(copsnrob_zings_555_astable)
{
	m_r_total_cv = RES_3_PARALLEL(COPSNROB_CUSTOM_ZINGS_555_ASTABLE__R1, RES_K(10), RES_K(5));
	m_r2c2 = COPSNROB_CUSTOM_ZINGS_555_ASTABLE__R2 * COPSNROB_CUSTOM_ZINGS_555_ASTABLE__C2;
	m_exponent1 = RC_CHARGE_EXP(COPSNROB_CUSTOM_ZINGS_555_ASTABLE__R1 * COPSNROB_CUSTOM_ZINGS_555_ASTABLE__C1);
	m_exponent2 = RC_CHARGE_EXP(m_r2c2);
	m_v_cap1 = 0;
	m_flip_flop = 0;
	m_v_cap2 = 0.0;     /* charge on C2 */
}


/************************************************
 * CUSTOM_ZINGS_555_ASTABLE Definition End
 ************************************************/


DISCRETE_SOUND_START(copsnrob)

	/************************************************
	 * Input register mapping
	 ************************************************/
	DISCRETE_INPUT_LOGIC(COPSNROB_MOTOR0_INV)
	DISCRETE_INPUT_LOGIC(COPSNROB_MOTOR1_INV)
	DISCRETE_INPUT_LOGIC(COPSNROB_MOTOR2_INV)
	DISCRETE_INPUT_LOGIC(COPSNROB_MOTOR3_INV)
/* !! DISABLED UNTIL ADDRESS IS FOUND !! */
//  DISCRETE_INPUTX_LOGIC(COPSNROB_ZINGS_INV, 4, 0, 0)
//  DISCRETE_INPUT_LOGIC(COPSNROB_FIRES_INV)
	DISCRETE_INPUT_NOT(COPSNROB_CRASH_INV)                  /* inverted for counter use */
	DISCRETE_INPUT_LOGIC(COPSNROB_SCREECH_INV)
	DISCRETE_INPUT_NOT(COPSNROB_AUDIO_ENABLE)               /* IC A1, pins 2 & 12 */

/* These inputs are disabled until their address triggers are determined.
 * Then these constants can be removed.
 */
	DISCRETE_CONSTANT(COPSNROB_ZINGS_INV, 4)    /* data bit will be normally high, when it goes low it triggers the one-shot which has a minimum on time of 0.2s */
	DISCRETE_CONSTANT(COPSNROB_FIRES_INV, 4)    /* data bit will be normally high */

	/************************************************
	 * MOTOR0/1
	 ************************************************/
	COPSNROB_MOTOR01(COPSNROB_MOTOR0_SND, COPSNROB_MOTOR0_INV, 0)
	COPSNROB_MOTOR01(COPSNROB_MOTOR1_SND, COPSNROB_MOTOR1_INV, 1)

	/************************************************
	 * MOTOR2/3
	 ************************************************/
	COPSNROB_MOTOR23(COPSNROB_MOTOR2_SND, COPSNROB_MOTOR2_INV, 0)
	COPSNROB_MOTOR23(COPSNROB_MOTOR3_SND, COPSNROB_MOTOR3_INV, 1)

	/************************************************
	 * CRASH
	 ************************************************/
	DISCRETE_CUSTOM1(COPSNROB_NOISE_1,  copsnrob_custom_noise,                  /* IC J2, pin 10 */
		COPSNROB_2V,                                        /* CLK */
		nullptr)
	/* COPSNROB_NOISE_2 derived from sub out of above custom module - IC J2, pin 11 */
	/* We use the measured 555 timer frequency (IC M3) for speed */
	DISCRETE_COUNTER(NODE_40,                               /* IC L2 */
		NODE_41,                                            /* ENAB - IC L2, pin 14 */
		COPSNROB_CRASH_INV,                                 /* RESET - IC L2, pin 11 */
		92,                                                 /* IC L2, pin 4 - freq measured */
		0, 15, DISC_COUNT_DOWN, 15, DISC_CLK_IS_FREQ)       /* MIN; MAX; DIR; INIT0; CLKTYPE */
	DISCRETE_TRANSFORM2(NODE_41,                            /* IC M2, pin 3 - goes high at count 0 */
		NODE_40, 0, "01=!")                                 /* -we will invert it for use by the counter module */
	DISCRETE_SWITCH(NODE_42,                                /* IC L3 */
		1, COPSNROB_NOISE_2, 0, NODE_40)                    /* ENAB; SWITCH; INP0; INP1 */
	DISCRETE_DAC_R1(COPSNROB_CRASH_SND,
		NODE_42, 3.8,                                       /* DATA; VDATA */
		&copsnrob_crash_dac)

	/************************************************
	 * SCREECH
	 ************************************************/
		DISCRETE_CONSTANT(COPSNROB_SCREECH_SND, 0)

	/************************************************
	 * FZ (Fires, Zings)
	 ************************************************/
	DISCRETE_CUSTOM3(NODE_60, copsnrob_zings_555_monostable,                            /* IC D3, pin 5 */
		/* We can ignore R47 & R48 */
		COPSNROB_ZINGS_INV,                             /* IC D3, pin 6 */
		COPSNROB_R38, COPSNROB_C19,
		nullptr)
	DISCRETE_CUSTOM5(NODE_61, copsnrob_zings_555_astable,                           /* IC D3, pin 8 & 12 */
		NODE_60,                                        /* IC D3, pin 10 */
		COPSNROB_R36, COPSNROB_R37,
		COPSNROB_C3, COPSNROB_C13,
		nullptr)
	/* FIX - do a better implemetation of IC L4 */
	DISCRETE_CRFILTER_VREF(NODE_62,                     /* IC L4, pin 9 */
		NODE_61,                                        /* IN0 */
		COPSNROB_R26, COPSNROB_C39,
		5.0 * RES_VOLTAGE_DIVIDER(COPSNROB_R74, COPSNROB_R75))  /* VREF */
	DISCRETE_GAIN(COPSNROB_FZ_SND,                      /* IC L4, pin 8 */
		NODE_62,
		COPSNROB_R73 / COPSNROB_R26)

	/************************************************
	 * MIXER
	 ************************************************/
		DISCRETE_MIXER5(NODE_90,                                /* IC B3, pin 3 */
		COPSNROB_AUDIO_ENABLE,                              /* ENAB */
		COPSNROB_MOTOR1_SND, COPSNROB_MOTOR0_SND, COPSNROB_FZ_SND, COPSNROB_SCREECH_SND, COPSNROB_CRASH_SND,
		&copsnrob_final_mixer01)
		DISCRETE_MIXER5(NODE_91,                                /* IC P3, pin 3 */
		COPSNROB_AUDIO_ENABLE,                              /* ENAB */
		COPSNROB_MOTOR3_SND, COPSNROB_MOTOR2_SND, COPSNROB_CRASH_SND, COPSNROB_SCREECH_SND, COPSNROB_FZ_SND,
		&copsnrob_final_mixer23)
		DISCRETE_OUTPUT(NODE_90, 32767.0*3.5)
		DISCRETE_OUTPUT(NODE_91, 32767.0*3.5)
DISCRETE_SOUND_END


WRITE8_MEMBER(copsnrob_state::copsnrob_misc_w)
{
	UINT8 latched_data = m_ic_h3_data;
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
			m_discrete->write(space, COPSNROB_MOTOR3_INV, special_data);
			break;

		case 0x01:
			m_discrete->write(space, COPSNROB_MOTOR2_INV, special_data);
			break;

		case 0x02:
			m_discrete->write(space, COPSNROB_MOTOR1_INV, special_data);
			break;

		case 0x03:
			m_discrete->write(space, COPSNROB_MOTOR0_INV, special_data);
			break;

		case 0x04:
			m_discrete->write(space, COPSNROB_SCREECH_INV, special_data);
			break;

		case 0x05:
			m_discrete->write(space, COPSNROB_CRASH_INV, special_data);
			break;

		case 0x06:
			/* One Start */
			set_led_status(machine(), 0, !special_data);
			break;

		case 0x07:
			m_discrete->write(space, COPSNROB_AUDIO_ENABLE, special_data);
			//machine().sound().system_mute(special_data);
			break;

	}

	m_ic_h3_data = latched_data;
}
