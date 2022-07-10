// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Couriersud,Derrick Renaud,Frank Palazzolo
#ifndef MAME_SOUND_DISC_MTH_H
#define MAME_SOUND_DISC_MTH_H

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

DISCRETE_CLASS_STEP(dst_adder, 1, /* no context */ );

DISCRETE_CLASS_STEP(dst_clamp, 1, /* no context */ );

DISCRETE_CLASS_STEP(dst_divide, 1, /* no context */ );

DISCRETE_CLASS_STEP(dst_gain, 1, /* no context */ );

DISCRETE_CLASS_STEP(dst_logic_inv, 1, /* no context */ );

DISCRETE_CLASS_STEP_RESET(dst_bits_decode, 8,
	int             m_count = 0;
	int             m_decode_x_time = 0;
	int             m_from = 0;
	int             m_last_val = 0;
	int             m_last_had_x_time = 0;
);

DISCRETE_CLASS_STEP(dst_logic_and, 1, /* no context */ );

DISCRETE_CLASS_STEP(dst_logic_nand, 1, /* no context */ );

DISCRETE_CLASS_STEP(dst_logic_or, 1, /* no context */ );

DISCRETE_CLASS_STEP(dst_logic_nor, 1, /* no context */ );

DISCRETE_CLASS_STEP(dst_logic_xor, 1, /* no context */ );

DISCRETE_CLASS_STEP(dst_logic_nxor, 1, /* no context */ );

DISCRETE_CLASS_STEP_RESET(dst_logic_dff, 1,
	int             m_last_clk = 0;
);

DISCRETE_CLASS_STEP_RESET(dst_logic_jkff, 1,
	double          m_v_out = 0;
	int             m_last_clk = 0;
);

DISCRETE_CLASS_STEP_RESET(dst_logic_shift, 1,
	double          m_t_left = 0.0;             /* time unused during last sample in seconds */
	uint32_t        m_shift_data = 0;
	uint32_t        m_bit_mask = 0;
	uint8_t         m_clock_type = 0;
	uint8_t         m_reset_on_high = 0;
	uint8_t         m_shift_r = 0;
	uint8_t         m_last = 0;
);

DISCRETE_CLASS_STEP(dst_lookup_table, 1, /* no context */ );

DISCRETE_CLASS_STEP_RESET(dst_multiplex, 1,
	int             m_size = 0;
);

DISCRETE_CLASS_STEP_RESET(dst_oneshot, 1,
	double          m_countdown = 0.0;
	int             m_state = 0;
	int             m_last_trig = 0;
	int             m_type = 0;
);

DISCRETE_CLASS_STEP_RESET(dst_ramp, 1,
	double          m_v_out = 0.0;
	double          m_step = 0.0;
	int             m_dir = 0;              /* 1 if End is higher then Start */
	int             m_last_en = 0;          /* Keep track of the last enable value */
);

DISCRETE_CLASS_STEP_RESET(dst_samphold, 1,
	double          m_last_input = 0.0;
	int             m_clocktype = 0;
);

DISCRETE_CLASS_STEP(dst_switch, 1, /* no context */ );

DISCRETE_CLASS_STEP(dst_aswitch, 1, /* no context */ );

DISCRETE_CLASS_STEP(, 1, /* no context */ );
class DISCRETE_CLASS_NAME(dst_transform): public discrete_base_node, public discrete_step_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dst_transform, base)
	DISCRETE_CLASS_DESTRUCTOR(dst_transform)
public:
	enum token {
		TOK_END = 0,
		TOK_MULT,
		TOK_DIV,
		TOK_ADD,
		TOK_MINUS,
		TOK_0,
		TOK_1,
		TOK_2,
		TOK_3,
		TOK_4,
		TOK_DUP,
		TOK_ABS,         /* absolute value */
		TOK_NEG,         /* * -1 */
		TOK_NOT,         /* Logical NOT of Last Value */
		TOK_EQUAL,       /* Logical = */
		TOK_GREATER,     /* Logical > */
		TOK_LESS,        /* Logical < */
		TOK_AND,         /* Bitwise AND */
		TOK_OR,          /* Bitwise OR */
		TOK_XOR          /* Bitwise XOR */
	};
	void step() override;
	void reset() override;
private:
	DISCRETE_CLASS_INPUT(I_IN0,     0);
	DISCRETE_CLASS_INPUT(I_IN1,     1);
	DISCRETE_CLASS_INPUT(I_IN2,     2);
	DISCRETE_CLASS_INPUT(I_IN3,     3);
	DISCRETE_CLASS_INPUT(I_IN4,     4);
	enum token precomp[32]{ TOK_END };
};

/* Component specific */

DISCRETE_CLASS_STEP_RESET(dst_comp_adder, 1,
	double          m_total[256]{ 0.0 };
);

DISCRETE_CLASS_STEP_RESET(dst_dac_r1, 1,
	double          m_v_out = 0.0;
	double          m_exponent = 0.0;
	double          m_last_v = 0.0;
	double          m_v_step[256]{ 0.0 };
	int             m_has_c_filter = 0;
);

DISCRETE_CLASS_STEP_RESET(dst_diode_mix, 1,
	int             m_size = 0;
	double          m_v_junction[8]{ 0.0 };
);

DISCRETE_CLASS_STEP_RESET(dst_integrate, 1,
	double          m_v_out = 0.0;
	double          m_change = 0.0;
	double          m_v_max_in = 0.0;       /* v1 - norton VBE */
	double          m_v_max_in_d = 0.0;     /* v1 - norton VBE - diode drop */
	double          m_v_max_out = 0.0;
);

#define DISC_MIXER_MAX_INPS 8
DISCRETE_CLASS_STEP_RESET(dst_mixer, 1,
	int             m_type = 0;
	int             m_size = 0;
	int             m_r_node_bit_flag = 0;
	int             m_c_bit_flag = 0;
	double          m_r_total = 0.0;
	const double *  m_r_node[DISC_MIXER_MAX_INPS]{ nullptr };      /* Either pointer to resistance node output OR nullptr */
	double          m_r_last[DISC_MIXER_MAX_INPS]{ 0.0 };
	double          m_exponent_rc[DISC_MIXER_MAX_INPS]{ 0.0 }; /* For high pass filtering cause by cIn */
	double          m_v_cap[DISC_MIXER_MAX_INPS]{ 0.0 };       /* cap voltage of each input */
	double          m_exponent_c_f = 0.0;   /* Low pass on mixed inputs */
	double          m_exponent_c_amp = 0.0; /* Final high pass caused by out cap and amp input impedance */
	double          m_v_cap_f = 0.0;        /* cap voltage of cF */
	double          m_v_cap_amp = 0.0;      /* cap voltage of cAmp */
	double          m_gain = 0.0;           /* used for DISC_MIXER_IS_OP_AMP_WITH_RI */
);

DISCRETE_CLASS_STEP_RESET(dst_op_amp, 1,
	uint8_t         m_has_cap = 0;
	uint8_t         m_has_r1 = 0;
	uint8_t         m_has_r4 = 0;
	double          m_v_max = 0.0;
	double          m_i_fixed = 0.0;
	double          m_v_cap = 0.0;
	double          m_exponent = 0.0;
);

DISCRETE_CLASS_STEP_RESET(dst_op_amp_1sht, 1,
	double          m_v_out = 0.0;
	double          m_i_fixed = 0.0;
	double          m_v_max = 0.0;
	double          m_r34ratio = 0.0;
	double          m_v_cap1 = 0.0;
	double          m_v_cap2 = 0.0;
	double          m_exponent1c = 0.0;
	double          m_exponent1d = 0.0;
	double          m_exponent2 = 0.0;
);

DISCRETE_CLASS_STEP_RESET(dst_tvca_op_amp, 1,
	double          m_v_out_max = 0.0;      /* Maximum output voltage */
	double          m_v_trig[2]{ 0.0 };     /* Voltage used to charge cap1 based on function F3 */
	double          m_v_trig2 = 0.0;        /* Voltage used to charge cap2 */
	double          m_v_trig3 = 0.0;        /* Voltage used to charge cap3 */
	double          m_i_fixed = 0.0;        /* Fixed current going into - input */
	double          m_exponent_c[2]{ 0.0 }; /* Charge exponents based on function F3 */
	double          m_exponent_d[2]{ 0.0 }; /* Discharge exponents based on function F3 */
	double          m_exponent2[2]{ 0.0 };  /* Discharge/charge exponents based on function F4 */
	double          m_exponent3[2]{ 0.0 };  /* Discharge/charge exponents based on function F5 */
	double          m_exponent4 = 0.0;      /* Discharge/charge exponents for c4 */
	double          m_v_cap1 = 0.0;         /* charge on cap c1 */
	double          m_v_cap2 = 0.0;         /* charge on cap c2 */
	double          m_v_cap3 = 0.0;         /* charge on cap c3 */
	double          m_v_cap4 = 0.0;         /* charge on cap c4 */
	double          m_r67 = 0.0;            /* = r6 + r7 (for easy use later) */
	uint8_t         m_has_c4 = 0;
	uint8_t         m_has_r4 = 0;
);

DISCRETE_CLASS_STEP(dst_xtime_buffer, 1, /* no context */ );

DISCRETE_CLASS_STEP(dst_xtime_and, 1, /* no context */ );

DISCRETE_CLASS_STEP(dst_xtime_or, 1, /* no context */ );

DISCRETE_CLASS_STEP(dst_xtime_xor, 1, /* no context */ );


#endif // MAME_SOUND_DISC_MTH_H
