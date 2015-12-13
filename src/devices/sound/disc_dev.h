// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Couriersud
// thanks-to:Derrick Renaud, F.Palazzolo
#pragma once

#ifndef __DISC_DEV_H__
#define __DISC_DEV_H__

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

DISCRETE_CLASS_STEP_RESET(dsd_555_astbl, 1,
	int             m_use_ctrlv;
	int             m_output_type;
	int             m_output_is_ac;
	double          m_ac_shift;                 /* DC shift needed to make waveform ac */
	int             m_flip_flop;                /* 555 flip/flop output state */
	double          m_cap_voltage;              /* voltage on cap */
	double          m_threshold;
	double          m_trigger;
	double          m_v_out_high;               /* Logic 1 voltage level */
	double          m_v_charge;
	const double *  m_v_charge_node;            /* point to output of node */
	int             m_has_rc_nodes;
	double          m_exp_bleed;
	double          m_exp_charge;
	double          m_exp_discharge;
	double          m_t_rc_bleed;
	double          m_t_rc_charge;
	double          m_t_rc_discharge;
	double          m_last_r1;
	double          m_last_r2;
	double          m_last_c;
);

DISCRETE_CLASS_STEP_RESET(dsd_555_mstbl, 1,
	int             m_trig_is_logic;
	int             m_trig_discharges_cap;
	int             m_output_type;
	double          m_ac_shift;                 /* DC shift needed to make waveform ac */
	int             m_flip_flop;                /* 555 flip/flop output state */
	int             m_has_rc_nodes;
	double          m_exp_charge;
	double          m_cap_voltage;              /* voltage on cap */
	double          m_threshold;
	double          m_trigger;
	double          m_v_out_high;               /* Logic 1 voltage level */
	double          m_v_charge;
);

DISCRETE_CLASS_STEP_RESET(dsd_555_cc, 1,
	unsigned int    m_type;                     /* type of 555cc circuit */
	int             m_output_type;
	int             m_output_is_ac;
	double          m_ac_shift;                 /* DC shift needed to make waveform ac */
	int             m_flip_flop;                /* 555 flip/flop output state */
	double          m_cap_voltage;              /* voltage on cap */
	double          m_threshold;
	double          m_trigger;
	double          m_v_out_high;               /* Logic 1 voltage level */
	double          m_v_cc_source;
	int             m_has_rc_nodes;
	double          m_exp_bleed;
	double          m_exp_charge;
	double          m_exp_discharge;
	double          m_exp_discharge_01;
	double          m_exp_discharge_no_i;
	double          m_t_rc_charge;
	double          m_t_rc_discharge;
	double          m_t_rc_discharge_01;
	double          m_t_rc_discharge_no_i;
);

DISCRETE_CLASS_STEP_RESET(dsd_555_vco1, 1,
	int             m_ctrlv_is_node;
	int             m_output_type;
	int             m_output_is_ac;
	double          m_ac_shift;                 /* DC shift needed to make waveform ac */
	int             m_flip_flop;                /* flip/flop output state */
	double          m_v_out_high;               /* 555 high voltage */
	double          m_threshold;                /* falling threshold */
	double          m_trigger;                  /* rising threshold */
	double          m_i_charge;                 /* charge current */
	double          m_i_discharge;              /* discharge current */
	double          m_cap_voltage;              /* current capacitor voltage */
);

DISCRETE_CLASS_STEP_RESET(dsd_566, 1,
	//unsigned int    m_state[2];                 /* keeps track of excess flip_flop changes during the current step */
	int             m_flip_flop;                /* 566 flip/flop output state */
	double          m_cap_voltage;              /* voltage on cap */
	double          m_v_sqr_low;                /* voltage for a squarewave at low */
	double          m_v_sqr_high;               /* voltage for a squarewave at high */
	double          m_v_sqr_diff;
	double          m_threshold_low;            /* falling threshold */
	double          m_threshold_high;           /* rising threshold */
	double          m_ac_shift;                 /* used to fake AC */
	double          m_v_osc_stable;
	double          m_v_osc_stop;
	int             m_fake_ac;
	int             m_out_type;
);

DISCRETE_CLASS_STEP_RESET(dsd_ls624, 1,
	double          m_exponent;
	double          m_t_used;
	double          m_v_cap_freq_in;
	double          m_v_freq_scale;
	double          m_v_rng_scale;
	int             m_flip_flop;
	int             m_has_freq_in_cap;
	int             m_out_type;
);

#endif /* __DISC_WAV_H__ */
