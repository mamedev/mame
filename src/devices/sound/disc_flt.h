// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Couriersud,Derrick Renaud,Frank Palazzolo
#pragma once

#ifndef __DISC_FLTH__
#define __DISC_FLT_H__

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
	double x1, x2;      /* x[k-1], x[k-2], previous 2 input values */
	double y1, y2;      /* y[k-1], y[k-2], previous 2 output values */
	double a1, a2;      /* digital filter coefficients, denominator */
	double b0, b1, b2;  /* digital filter coefficients, numerator */
};


DISCRETE_CLASS_STEP_RESET(dst_filter1, 1,
	/* uses x1, y1, a1, b0, b1 */
	struct discrete_filter_coeff m_fc;
);

DISCRETE_CLASS_STEP_RESET(dst_filter2, 1,
	struct discrete_filter_coeff m_fc;
);

DISCRETE_CLASS_STEP_RESET(dst_sallen_key, 1,
	struct discrete_filter_coeff m_fc;
);

DISCRETE_CLASS_STEP_RESET(dst_crfilter, 1,
	double          m_vCap;
	double          m_rc;
	double          m_exponent;
	UINT8           m_has_rc_nodes;
	//UINT8           m_is_fast;
);

DISCRETE_CLASS_STEP_RESET(dst_op_amp_filt, 1,
	int             m_type;                 /* What kind of filter */
	int             m_is_norton;            /* 1 = Norton op-amps */
	double          m_vRef;
	double          m_vP;
	double          m_vN;
	double          m_rTotal;               /* All input resistance in parallel. */
	double          m_iFixed;               /* Current supplied by r3 & r4 if used. */
	double          m_exponentC1;
	double          m_exponentC2;
	double          m_exponentC3;
	double          m_rRatio;               /* divide ratio of resistance network */
	double          m_vC1;                  /* Charge on C1 */
	double          m_vC1b;                 /* Charge on C1, part of C1 charge if needed */
	double          m_vC2;                  /* Charge on C2 */
	double          m_vC3;                  /* Charge on C2 */
	double          m_gain;                 /* Gain of the filter */
	struct discrete_filter_coeff m_fc;
);

DISCRETE_CLASS_STEP_RESET(dst_rc_circuit_1, 1,
	double          m_v_cap;
	double          m_v_charge_1_2;
	double          m_v_drop;
	double          m_exp_1;
	double          m_exp_1_2;
	double          m_exp_2;
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc, 1,
	int             m_state;
	double          m_t;                    /* time */
	double          m_exponent0;
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc2, 1,
	int             m_state;
	double          m_v_out;
	double          m_t;                    /* time */
	double          m_exponent0;
	double          m_exponent1;
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc3, 1,
	int             m_state;
	double          m_v_out;
	double          m_t;                    /* time */
	double          m_exponent0;
	double          m_exponent1;
	double          m_v_diode;              /* rcdisc3 */
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc4, 1,
	int             m_type;
	double          m_max_out;
	double          m_vC1;
	double          m_v[2];
	double          m_exp[2];
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc5, 1,
	int             m_state;
	double          m_t;                    /* time */
	double          m_exponent0;
	double          m_v_cap;                /* rcdisc5 */
);

DISCRETE_CLASS_STEP_RESET(dst_rcintegrate, 1,
	int             m_type;
	double          m_gain_r1_r2;
	double          m_f;                    /* r2,r3 gain */
	double          m_vCap;
	double          m_vCE;
	double          m_exponent0;
	double          m_exponent1;
	double          m_exp_exponent0;
	double          m_exp_exponent1;
	double          m_c_exp0;
	double          m_c_exp1;
	double          m_EM_IC_0_7;
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc_mod, 1,
	double          m_v_cap;
	double          m_exp_low[2];
	double          m_exp_high[4];
	double          m_gain[2];
	double          m_vd_gain[4];
);

DISCRETE_CLASS_STEP_RESET(dst_rcfilter, 1,
	double          m_v_out;
	double          m_vCap;
	double          m_rc;
	double          m_exponent;
	UINT8           m_has_rc_nodes;
	UINT8           m_is_fast;
);

DISCRETE_CLASS_STEP_RESET(dst_rcfilter_sw, 1,
	double          m_vCap[4];
	double          m_exp[4];
	double          m_exp0;                 /* fast case bit 0 */
	double          m_exp1;                 /* fast case bit 1 */
	double          m_factor;               /* fast case */
	double          m_f1[16];
	double          m_f2[16];
);

DISCRETE_CLASS_STEP_RESET(dst_rcdiscN, 1,
	double          m_x1;                   /* x[k-1], previous input value */
	double          m_y1;                   /* y[k-1], previous output value */
	double          m_a1;                   /* digital filter coefficients, denominator */
	//double          m_b[2];                 /* digital filter coefficients, numerator */
);

DISCRETE_CLASS_STEP_RESET(dst_rcdisc2N, 1,
	struct discrete_filter_coeff m_fc0;
	struct discrete_filter_coeff m_fc1;
	double          m_x1;
	double          m_y1;
);


#endif /* __DISC_FLT_H__ */
