// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Couriersud,Derrick Renaud,Frank Palazzolo
#ifndef MAME_SOUND_DISC_WAV_H
#define MAME_SOUND_DISC_WAV_H

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

DISCRETE_CLASS_STEP_RESET(dss_counter, 1,
	int             m_clock_type = 0;
	int             m_out_type = 0;
	int             m_is_7492 = 0;
	int             m_last_clock = 0;
	uint32_t        m_last_count = 0;
	//uint32_t        m_last = 0;             /* Last clock state */
	uint32_t        m_min = 0;
	uint32_t        m_max = 0;
	uint32_t        m_diff = 0;
	double          m_t_left = 0.0;         /* time unused during last sample in seconds */
);

DISCRETE_CLASS_STEP_RESET(dss_lfsr_noise, 2,
	unsigned int    m_lfsr_reg = 0;
	int             m_last = 0;             /* Last clock state */
	double          m_t_clock = 0.0;        /* fixed counter clock in seconds */
	double          m_t_left = 0.0;         /* time unused during last sample in seconds */
	//double          m_sample_step = 0.0;
	//double          m_t = 0.0;
	uint8_t         m_reset_on_high = 0;
	uint8_t         m_invert_output = 0;
	uint8_t         m_out_is_f0 = 0;
	uint8_t         m_out_lfsr_reg = 0;
);

DISCRETE_CLASS_STEP_RESET(dss_noise, 2,
	double          m_phase = 0.0;
);

DISCRETE_CLASS_STEP_RESET(dss_note, 1,
	int             m_clock_type = 0;
	int             m_out_type = 0;
	int             m_last = 0;             /* Last clock state */
	double          m_t_clock = 0.0;        /* fixed counter clock in seconds */
	double          m_t_left = 0.0;         /* time unused during last sample in seconds */
	int             m_max1 = 0;             /* Max 1 Count stored as int for easy use. */
	int             m_max2 = 0;             /* Max 2 Count stored as int for easy use. */
	int             m_count1 = 0;           /* current count1 */
	int             m_count2 = 0;           /* current count2 */
);

DISCRETE_CLASS_STEP_RESET(dss_sawtoothwave, 1,
	double          m_phase = 0.0;
	int             m_type = 0;
);

DISCRETE_CLASS_STEP_RESET(dss_sinewave, 1,
	double          m_phase = 0.0;
);

DISCRETE_CLASS_STEP_RESET(dss_squarewave, 1,
	double          m_phase = 0.0;
	double          m_trigger = 0.0;
);
DISCRETE_CLASS_STEP_RESET(dss_squarewfix, 1,
	int             m_flip_flop = 0;
	double          m_sample_step = 0.0;
	double          m_t_left = 0.0;
	double          m_t_off = 0.0;
	double          m_t_on = 0.0;
);

DISCRETE_CLASS_STEP_RESET(dss_squarewave2, 1,
	double          m_phase = 0.0;
	double          m_trigger = 0.0;
);

DISCRETE_CLASS_STEP_RESET(dss_trianglewave, 1,
	double          m_phase = 0.0;
);

/* Component specific modules */

#define DSS_INV_TAB_SIZE    500
#define DEFAULT_CD40XX_VALUES(_vB)  (_vB),(_vB)*0.02,(_vB)*0.98,(_vB)/5.0*1.5,(_vB)/5.0*3.5, 0.1

class DISCRETE_CLASS_NAME(dss_inverter_osc): public discrete_base_node, public discrete_step_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dss_inverter_osc, base)
	DISCRETE_CLASS_DESTRUCTOR(dss_inverter_osc)
public:
	struct description
	{
		double  vB = 0.0;
		double  vOutLow = 0.0;
		double  vOutHigh = 0.0;
		double  vInFall = 0.0;  // voltage that triggers the gate input to go low (0V) on fall
		double  vInRise = 0.0;  // voltage that triggers the gate input to go high (vGate) on rise
		double  clamp = 0.0;    // voltage is clamped to -clamp ... vb+clamp if clamp>= 0;
		int     options = 0.0;  // bitmapped options
	};
	enum {
		IS_TYPE1 = 0x00,
		IS_TYPE2 = 0x01,
		IS_TYPE3 = 0x02,
		IS_TYPE4 = 0x03,
		IS_TYPE5 = 0x04,
		TYPE_MASK = 0x0f,
		OUT_IS_LOGIC = 0x10
	};
	void step() override;
	void reset() override;
protected:
	inline double tftab(double x);
	inline double tf(double x);
private:
	DISCRETE_CLASS_INPUT(I_ENABLE,  0);
	DISCRETE_CLASS_INPUT(I_MOD,     1);
	DISCRETE_CLASS_INPUT(I_RC,      2);
	DISCRETE_CLASS_INPUT(I_RP,      3);
	DISCRETE_CLASS_INPUT(I_C,       4);
	DISCRETE_CLASS_INPUT(I_R2,      5);

	double          mc_v_cap = 0.0;
	double          mc_v_g2_old = 0.0;
	double          mc_w = 0.0;
	double          mc_wc = 0.0;
	double          mc_rp = 0.0;
	double          mc_r1 = 0.0;
	double          mc_r2 = 0.0;
	double          mc_c = 0.0;
	double          mc_tf_a = 0.0;
	double          mc_tf_b = 0.0;
	double          mc_tf_tab[DSS_INV_TAB_SIZE]{ 0.0 };
};

DISCRETE_CLASS_STEP_RESET(dss_op_amp_osc, 1,
	const double *  m_r[8]{ nullptr };      /* pointers to resistor values */
	int             m_type = 0;
	uint8_t         m_flip_flop = 0;        /* flip/flop output state */
	uint8_t         m_flip_flop_xor = 0;    /* flip_flop ^ flip_flop_xor, 0 = discharge, 1 = charge */
	uint8_t         m_output_type = 0;
	uint8_t         m_has_enable = 0;
	double          m_v_out_high = 0.0;
	double          m_threshold_low = 0.0;  /* falling threshold */
	double          m_threshold_high = 0.0; /* rising threshold */
	double          m_v_cap = 0.0;          /* current capacitor voltage */
	double          m_r_total = 0.0;        /* all input resistors in parallel */
	double          m_i_fixed = 0.0;        /* fixed current at the input */
	double          m_i_enable = 0.0;       /* fixed current at the input if enabled */
	double          m_temp1 = 0.0;          /* Multi purpose */
	double          m_temp2 = 0.0;          /* Multi purpose */
	double          m_temp3 = 0.0;          /* Multi purpose */
	double          m_is_linear_charge = 0.0;
	double          m_charge_rc[2]{ 0.0 };
	double          m_charge_exp[2]{ 0.0 };
	double          m_charge_v[2]{ 0.0 };
);

DISCRETE_CLASS_STEP_RESET(dss_schmitt_osc, 1,
	double          m_ration_in = 0.0;      /* ratio of total charging voltage that comes from the input */
	double          m_ratio_feedback = 0.0; /* ratio of total charging voltage that comes from the feedback */
	double          m_v_cap = 0.0;          /* current capacitor voltage */
	double          m_rc = 0.0;             /* r*c */
	double          m_exponent = 0.0;
	int             m_state = 0;            /* state of the output */
	int             m_enable_type = 0;
	uint8_t         m_input_is_voltage = 0;
);

/* Not yet implemented */
DISCRETE_CLASS_STEP_RESET(dss_adsrenv,  1,
	//double          m_phase = 0.0;
);


#endif // MAME_SOUND_DISC_WAV_H
