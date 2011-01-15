#pragma once

#ifndef __DISC_WAV_H__
#define __DISC_WAV_H__

/***********************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by Keith Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *
 *  Coding started in November 2000
 *
 *  Additions/bugfix February 2003 - D.Renaud, F.Palazzolo, K.Wilkins
 *  Discrete parallel tasks 2009 - Couriersud
 *  Discrete classes 2010		 - Couriersud
 *
 ***********************************************************************/

#include "discrete.h"

DISCRETE_CLASS_STEP_RESETA(dss_counter, 1,
	int		m_clock_type;
	int		m_out_type;
	int		m_is_7492;
	int		m_last_clock;
	UINT32	m_last_count;
	UINT32	m_last;		/* Last clock state */
	UINT32	m_min;
	UINT32	m_max;
	UINT32	m_diff;
	double	m_t_left;		/* time unused during last sample in seconds */
);

DISCRETE_CLASS_STEP_RESETA(dss_lfsr_noise, 2,
	unsigned int	m_lfsr_reg;
	int				m_last;		/* Last clock state */
	double			m_t_clock;	/* fixed counter clock in seconds */
	double			m_t_left;		/* time unused during last sample in seconds */
	double			m_sample_step;
	double			m_t;
	UINT8			m_reset_on_high;
	UINT8			m_invert_output;
	UINT8			m_out_is_f0;
	UINT8			m_out_lfsr_reg;
);

DISCRETE_CLASS_STEP_RESETA(dss_noise, 2,
	double m_phase;
);

DISCRETE_CLASS_STEP_RESETA(dss_note, 1,
	int		m_clock_type;
	int		m_out_type;
	int		m_last;		/* Last clock state */
	double	m_t_clock;	/* fixed counter clock in seconds */
	double	m_t_left;		/* time unused during last sample in seconds */
	int		m_max1;		/* Max 1 Count stored as int for easy use. */
	int		m_max2;		/* Max 2 Count stored as int for easy use. */
	int		m_count1;		/* current count1 */
	int		m_count2;		/* current count2 */
);

DISCRETE_CLASS_STEP_RESETA(dss_sawtoothwave, 1,
	double	m_phase;
	int		m_type;
);

DISCRETE_CLASS_STEP_RESETA(dss_sinewave, 1,
	double	m_phase;
);

DISCRETE_CLASS_STEP_RESETA(dss_squarewave, 1,
	double	m_phase;
	double	m_trigger;
);
DISCRETE_CLASS_STEP_RESETA(dss_squarewfix, 1,
	int		m_flip_flop;
	double	m_sample_step;
	double	m_t_left;
	double	m_t_off;
	double	m_t_on;
);

DISCRETE_CLASS_STEP_RESETA(dss_squarewave2, 1,
	double	m_phase;
	double	m_trigger;
);

DISCRETE_CLASS_STEP_RESETA(dss_trianglewave, 1,
	double	m_phase;
);
/* Component specific modules */

#define DSS_INV_TAB_SIZE	500

class DISCRETE_CLASS_NAME(dss_inverter_osc): public discrete_base_node, public discrete_step_interface
{
protected:
	inline double tftab(double x);
	inline double tf(double x);
public:
	DISCRETE_CLASS_CONSTRUCTOR(dss_inverter_osc, base)
	void step(void);
	void reset(void);
private:
	double	mc_v_cap;
	double  mc_v_g2_old;
	double	mc_w;
	double  mc_wc;
	double	mc_rp;
	double  mc_r1;
	double  mc_r2;
	double  mc_c;
	double	mc_tf_a;
	double	mc_tf_b;
	double  mc_tf_tab[DSS_INV_TAB_SIZE];
};

DISCRETE_CLASS_STEP_RESETA(dss_op_amp_osc, 1,
	const double *m_r1;		/* pointers to resistor values */
	const double *m_r2;
	const double *m_r3;
	const double *m_r4;
	const double *m_r5;
	const double *m_r6;
	const double *m_r7;
	const double *m_r8;
	int			m_type;
	UINT8		m_flip_flop;		/* flip/flop output state */
	UINT8		m_flip_flop_xor;	/* flip_flop ^ flip_flop_xor, 0 = discharge, 1 = charge */
	UINT8		m_output_type;
	UINT8		m_has_enable;
	double		m_v_out_high;
	double		m_threshold_low;	/* falling threshold */
	double		m_threshold_high;	/* rising threshold */
	double		m_v_cap;			/* current capacitor voltage */
	double		m_r_total;		/* all input resistors in parallel */
	double		m_i_fixed;		/* fixed current at the input */
	double		m_i_enable;		/* fixed current at the input if enabled */
	double		m_temp1;			/* Multi purpose */
	double		m_temp2;			/* Multi purpose */
	double		m_temp3;			/* Multi purpose */
	double		m_is_linear_charge;
	double		m_charge_rc[2];
	double		m_charge_exp[2];
	double		m_charge_v[2];
);

DISCRETE_CLASS_STEP_RESETA(dss_schmitt_osc, 1,
	double		m_ration_in;			/* ratio of total charging voltage that comes from the input */
	double		m_ratio_feedback;	/* ratio of total charging voltage that comes from the feedback */
	double		m_v_cap;				/* current capacitor voltage */
	double		m_rc;					/* r*c */
	double		m_exponent;
	int			m_state;				/* state of the output */
	int			m_enable_type;
	UINT8		m_input_is_voltage;
);

/* Not yet implemented */
DISCRETE_CLASS_STEP_RESETA(dss_adsrenv,  1,
	double		m_phase;
);


#endif /* __DISC_WAV_H__ */
