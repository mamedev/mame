// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Couriersud,Derrick Renaud,Frank Palazzolo
#ifndef MAME_SOUND_DISC_FLT_H
#define MAME_SOUND_DISC_FLT_H

#pragma once

/***********************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by K.Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *
 *  Coding started in November 2000
 *
 *  Additions/bugfix February 2003 - Derrick Renaud, F.Palazzolo, K.Wilkins
 *  Discrete parallel tasks 2009 - Couriersud
 *  Discrete classes 2010        - Couriersud
 *
 ***********************************************************************/

#include "discrete.h"

struct discrete_filter_coeff
{
	double x1 = 0.0, x2 = 0.0;              /* x[k-1], x[k-2], previous 2 input values */
	double y1 = 0.0, y2 = 0.0;              /* y[k-1], y[k-2], previous 2 output values */
	double a1 = 0.0, a2 = 0.0;              /* digital filter coefficients, denominator */
	double b0 = 0.0, b1 = 0.0, b2 = 0.0;    /* digital filter coefficients, numerator */
};


DISCRETE_CLASS_STEP_RESET(dst_filter1, 1,
	/* uses x1, y1, a1, b0, b1 */
	discrete_filter_coeff m_fc;
);

DISCRETE_CLASS_STEP_RESET(dst_filter2, 1,
	discrete_filter_coeff m_fc;
);

DISCRETE_CLASS_STEP_RESET(dst_sallen_key, 1,
	discrete_filter_coeff m_fc;
);

DISCRETE_CLASS_STEP_RESET(dst_crfilter, 1,
	double          m_vCap = 0.0;
	double          m_rc = 0.0;
	double          m_exponent = 0.0;
	uint8_t         m_has_rc_nodes = 0;
	//uint8_t         m_is_fast = 0;
);

DISCRETE_CLASS_STEP_RESET(dst_op_amp_filt, 1,
	int             m_type = 0;             /* What kind of filter */
	int             m_is_norton = 0;        /* 1 = Norton op-amps */
	double          m_vRef = 0.0;
	double          m_vP = 0.0;
	double          m_vN = 0.0;
	double          m_rTotal = 0.0;         /* All input resistance in parallel. */
	double          m_iFixed = 0.0;         /* Current supplied by r3 & r4 if used. */
	double          m_exponentC1 = 0.0;
	double          m_exponentC2 = 0.0;
	double          m_exponentC3 = 0.0;
	double          m_rRatio = 0.0;         /* divide ratio of resistance network */
	double          m_vC1 = 0.0;            /* Charge on C1 */
	double          m_vC1b = 0.0;           /* Charge on C1, part of C1 charge if needed */
	double          m_vC2 = 0.0;            /* Charge on C2 */
	double          m_vC3 = 0.0;            /* Charge on C2 */
	double          m_gain = 0.0;           /* Gain of the filter */
	discrete_filter_coeff m_fc;
);

DISCRETE_CLASS_STEP_RESET(dst_rc_circuit_1, 1,
	double          m_v_cap = 0.0;
	double          m_v_charge_1_2 = 0.0;
	double          m_v_drop = 0.0;
	double          m_exp_1 = 0.0;
	double          m_exp_1_2 = 0.0;
	double          m_exp_2 = 0.0;
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc, 1,
	int             m_state = 0;
	double          m_t = 0.0;              /* time */
	double          m_exponent0 = 0.0;
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc2, 1,
	int             m_state = 0;
	double          m_v_out = 0.0;
	double          m_t = 0.0;              /* time */
	double          m_exponent0 = 0.0;
	double          m_exponent1 = 0.0;
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc3, 1,
	int             m_state = 0;
	double          m_v_out = 0.0;
	double          m_t = 0.0;              /* time */
	double          m_exponent0 = 0.0;
	double          m_exponent1 = 0.0;
	double          m_v_diode = 0.0;        /* rcdisc3 */
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc4, 1,
	int             m_type = 0;
	double          m_max_out = 0.0;
	double          m_vC1 = 0.0;
	double          m_v[2]{ 0.0 };
	double          m_exp[2]{ 0.0 };
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc5, 1,
	int             m_state = 0;
	double          m_t = 0.0;              /* time */
	double          m_exponent0 = 0.0;
	double          m_v_cap = 0.0;          /* rcdisc5 */
);

DISCRETE_CLASS_STEP_RESET(dst_rcintegrate, 1,
	int             m_type = 0;
	double          m_gain_r1_r2 = 0.0;
	double          m_f = 0.0;              /* r2,r3 gain */
	double          m_vCap = 0.0;
	double          m_vCE = 0.0;
	double          m_exponent0 = 0.0;
	double          m_exponent1 = 0.0;
	double          m_exp_exponent0 = 0.0;
	double          m_exp_exponent1 = 0.0;
	double          m_c_exp0 = 0.0;
	double          m_c_exp1 = 0.0;
	double          m_EM_IC_0_7 = 0.0;
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc_mod, 1,
	double          m_v_cap = 0.0;
	double          m_exp_low[2]{ 0.0 };
	double          m_exp_high[4]{ 0.0 };
	double          m_gain[2]{ 0.0 };
	double          m_vd_gain[4]{ 0.0 };
);

DISCRETE_CLASS_STEP_RESET(dst_rcfilter, 1,
	double          m_v_out = 0.0;
	double          m_vCap = 0.0;
	double          m_rc = 0.0;
	double          m_exponent = 0.0;
	uint8_t         m_has_rc_nodes = 0;
	uint8_t         m_is_fast = 0;
);

DISCRETE_CLASS_STEP_RESET(dst_rcfilter_sw, 1,
	double          m_vCap[4]{ 0.0 };
	double          m_exp[4]{ 0.0 };
	double          m_exp0 = 0.0;           /* fast case bit 0 */
	double          m_exp1 = 0.0;           /* fast case bit 1 */
	double          m_factor = 0.0;         /* fast case */
	double          m_f1[16]{ 0.0 };
	double          m_f2[16]{ 0.0 };
);

DISCRETE_CLASS_STEP_RESET(dst_rcdiscN, 1,
	double          m_x1 = 0.0;             /* x[k-1], previous input value */
	double          m_y1 = 0.0;             /* y[k-1], previous output value */
	double          m_a1 = 0.0;             /* digital filter coefficients, denominator */
	//double          m_b[2]{ 0.0 };          /* digital filter coefficients, numerator */
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc2N, 1,
	discrete_filter_coeff m_fc0;
	discrete_filter_coeff m_fc1;
	double          m_x1 = 0.0;
	double          m_y1 = 0.0;
);


#endif // MAME_SOUND_DISC_FLT_H
