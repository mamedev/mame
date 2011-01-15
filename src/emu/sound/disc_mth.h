#pragma once

#ifndef __DISC_MTH_H__
#define __DISC_MTH_H__

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

DISCRETE_CLASS_STEPA(dst_adder, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_clamp, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_divide, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_gain, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_logic_inv, 1, /* no context */ );

DISCRETE_CLASS_STEP_RESETA(dst_bits_decode, 8,
	int m_count;
	int m_decode_x_time;
	int m_from;
	int m_last_val;
	int m_last_had_x_time;
);

DISCRETE_CLASS_STEPA(dst_logic_and, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_logic_nand, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_logic_or, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_logic_nor, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_logic_xor, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_logic_nxor, 1, /* no context */ );

DISCRETE_CLASS_STEP_RESETA(dst_logic_dff, 1,
	int m_last_clk;
);

DISCRETE_CLASS_STEP_RESETA(dst_logic_jkff, 1,
	int m_last_clk;
);

DISCRETE_CLASS_STEP_RESETA(dst_logic_shift, 1,
	double	m_t_left;		/* time unused during last sample in seconds */
	UINT32	m_shift_data;
	UINT32	m_bit_mask;
	UINT8	m_clock_type;
	UINT8	m_reset_on_high;
	UINT8	m_shift_r;
	UINT8	m_last;
);

DISCRETE_CLASS_STEPA(dst_lookup_table, 1, /* no context */ );

DISCRETE_CLASS_STEP_RESETA(dst_multiplex, 1,
	int m_size;
);

DISCRETE_CLASS_STEP_RESETA(dst_oneshot, 1,
	double	m_countdown;
	int		m_state;
	int		m_last_trig;
	int		m_type;
);

DISCRETE_CLASS_STEP_RESETA(dst_ramp, 1,
	double	m_step;
	int		m_dir;		/* 1 if End is higher then Start */
	int		m_last_en;	/* Keep track of the last enable value */
);

DISCRETE_CLASS_STEP_RESETA(dst_samphold, 1,
	double m_last_input;
	int		m_clocktype;
);

DISCRETE_CLASS_STEPA(dst_switch, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_aswitch, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_transform, 1, /* no context */ );

/* Component specific */

DISCRETE_CLASS_STEP_RESETA(dst_comp_adder, 1,
	double	m_total[256];
);

DISCRETE_CLASS_STEP_RESETA(dst_dac_r1, 1,
	double	m_exponent;
	double	m_last_v;
	double	m_v_step[256];
	int		m_has_c_filter;
);

DISCRETE_CLASS_STEP_RESETA(dst_diode_mix, 1,
	int		m_size;
	double	m_v_junction[8];
);

DISCRETE_CLASS_STEP_RESETA(dst_integrate, 1,
	double	m_change;
	double	m_v_max_in;	/* v1 - norton VBE */
	double	m_v_max_in_d;	/* v1 - norton VBE - diode drop */
	double	m_v_max_out;
);

#define DISC_MIXER_MAX_INPS	8
DISCRETE_CLASS_STEP_RESETA(dst_mixer, 1,
	int		m_type;
	int		m_size;
	int		m_r_node_bit_flag;
	int		m_c_bit_flag;
	double	m_r_total;
	double *m_r_node[DISC_MIXER_MAX_INPS];		/* Either pointer to resistance node output OR NULL */
	double	m_r_last[DISC_MIXER_MAX_INPS];
	double	m_exponent_rc[DISC_MIXER_MAX_INPS];	/* For high pass filtering cause by cIn */
	double	m_v_cap[DISC_MIXER_MAX_INPS];			/* cap voltage of each input */
	double	m_exponent_c_f;			/* Low pass on mixed inputs */
	double	m_exponent_c_amp;			/* Final high pass caused by out cap and amp input impedance */
	double	m_v_cap_f;				/* cap voltage of cF */
	double	m_v_cap_amp;				/* cap voltage of cAmp */
	double	m_gain;					/* used for DISC_MIXER_IS_OP_AMP_WITH_RI */
);

DISCRETE_CLASS_STEP_RESETA(dst_op_amp, 1,
	UINT8	m_has_cap;
	UINT8	m_has_r1;
	UINT8	m_has_r4;
	double	m_v_max;
	double	m_i_fixed;
	double	m_v_cap;
	double	m_exponent;
);

DISCRETE_CLASS_STEP_RESETA(dst_op_amp_1sht, 1,
	double	m_i_fixed;
	double	m_v_max;
	double	m_r34ratio;
	double	m_v_cap1;
	double	m_v_cap2;
	double	m_exponent1c;
	double	m_exponent1d;
	double	m_exponent2;
);

DISCRETE_CLASS_STEP_RESETA(dst_tvca_op_amp, 1,
	double	m_v_out_max;		/* Maximum output voltage */
	double	m_v_trig[2];		/* Voltage used to charge cap1 based on function F3 */
	double	m_v_trig2;			/* Voltage used to charge cap2 */
	double	m_v_trig3;			/* Voltage used to charge cap3 */
	double	m_i_fixed;		/* Fixed current going into - input */
	double	m_exponent_c[2];	/* Charge exponents based on function F3 */
	double	m_exponent_d[2];	/* Discharge exponents based on function F3 */
	double	m_exponent2[2];	/* Discharge/charge exponents based on function F4 */
	double	m_exponent3[2];	/* Discharge/charge exponents based on function F5 */
	double	m_exponent4;		/* Discharge/charge exponents for c4 */
	double	m_v_cap1;			/* charge on cap c1 */
	double	m_v_cap2;			/* charge on cap c2 */
	double	m_v_cap3;			/* charge on cap c3 */
	double	m_v_cap4;			/* charge on cap c4 */
	double	m_r67;			/* = r6 + r7 (for easy use later) */
	UINT8	m_has_c4;
	UINT8	m_has_r4;
);

DISCRETE_CLASS_STEPA(dst_xtime_buffer, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_xtime_and, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_xtime_or, 1, /* no context */ );

DISCRETE_CLASS_STEPA(dst_xtime_xor, 1, /* no context */ );


#endif /* __DISC_WAV_H__ */
