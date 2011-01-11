#pragma once

#ifndef __DISC_CLS_H__
#define __DISC_CLS_H__

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
 *
 ***********************************************************************/

#define DISCRETE_CLASS_NAME(_name) _name ## _node

#define DISCRETE_CLASS_CONSTRUCTOR(_name, _ctxsize)				\
		DISCRETE_CLASS_NAME(_name)(discrete_device * pdev, const discrete_sound_block *block) \
		 : discrete_base_node(pdev, block) {  m_context_size = _ctxsize; }

#define  DISCRETE_CLASS_STEP_RESET(_name, _ctxsize, _maxout)	\
class DISCRETE_CLASS_NAME(_name): public discrete_base_node	\
{														\
public:													\
	DISCRETE_CLASS_CONSTRUCTOR(_name, _ctxsize)					\
	void step(void);									\
	void reset(void);									\
	int max_output(void) { return _maxout; }			\
}

#define DISCRETE_CLASS_STEP(_name, _ctxsize, _maxout)	 						\
class DISCRETE_CLASS_NAME(_name): public discrete_base_node	\
{														\
public:													\
	DISCRETE_CLASS_CONSTRUCTOR(_name, _ctxsize)					\
	   \
	void step(void);									\
	void reset(void)			{ this->step(); }		\
	int max_output(void) { return _maxout; }			\
}

#define  DISCRETE_CLASS_RESET(_name, _ctxsize, _maxout) 					\
class DISCRETE_CLASS_NAME(_name): public discrete_base_node	\
{														\
public:													\
	DISCRETE_CLASS_CONSTRUCTOR(_name, _ctxsize)					\
	void step(void) { }									\
	void reset(void);									\
	bool is_stepping(void) { return false; }			\
	int max_output(void) { return _maxout; }			\
}

#define  DISCRETE_CLASS(_name, _ctxsize, _maxout) 							\
class DISCRETE_CLASS_NAME(_name): public discrete_base_node	\
{														\
public:													\
	DISCRETE_CLASS_CONSTRUCTOR(_name, _ctxsize)					\
	void step(void);									\
	void reset(void);									\
	void start(void);									\
	void stop(void);									\
	bool is_stepping(void);								\
	int max_output(void) { return _maxout; }			\
}

class DISCRETE_CLASS_NAME(special): public discrete_base_node
{
public:
	DISCRETE_CLASS_CONSTRUCTOR(special, 0)
	void step(void) { }
	bool is_stepping(void) { return false; }
	int max_output(void) { return 0; }
};

class DISCRETE_CLASS_NAME(unimplemented): public discrete_base_node
{
public:
	DISCRETE_CLASS_CONSTRUCTOR(unimplemented, 0)
	void step(void) { }
	bool is_stepping(void) { return false; }
	int max_output(void) { return 0; }
};

#define DSS_INV_TAB_SIZE	500

struct dss_adjustment_context
{
	const input_port_config *port;
	INT32		lastpval;
	INT32		pmin;
	double		pscale;
	double		min;
	double		scale;
};

struct dss_input_context
{
	stream_sample_t *ptr;			/* current in ptr for stream */
	double		gain;				/* node gain */
	double		offset;				/* node offset */
	UINT8		data;				/* data written */
	UINT8		is_stream;
	UINT8		is_buffered;
	UINT32		stream_in_number;
	/* the buffer stream */
	sound_stream *buffer_stream;
};


struct dss_lfsr_noise_context
{
	unsigned int	lfsr_reg;
	int				last;		/* Last clock state */
	double			t_clock;	/* fixed counter clock in seconds */
	double			t_left;		/* time unused during last sample in seconds */
	double			sample_step;
	double			t;
	UINT8			reset_on_high;
	UINT8			invert_output;
	UINT8			out_is_f0;
	UINT8			out_lfsr_reg;
};

struct dss_noise_context
{
	double phase;
};

struct dss_note_context
{
	int		clock_type;
	int		out_type;
	int		last;		/* Last clock state */
	double	t_clock;	/* fixed counter clock in seconds */
	double	t_left;		/* time unused during last sample in seconds */
	int		max1;		/* Max 1 Count stored as int for easy use. */
	int		max2;		/* Max 2 Count stored as int for easy use. */
	int		count1;		/* current count1 */
	int		count2;		/* current count2 */
};

struct dss_op_amp_osc_context
{
	const double *r1;		/* pointers to resistor values */
	const double *r2;
	const double *r3;
	const double *r4;
	const double *r5;
	const double *r6;
	const double *r7;
	const double *r8;
	int		type;
	UINT8	flip_flop;		/* flip/flop output state */
	UINT8	flip_flop_xor;	/* flip_flop ^ flip_flop_xor, 0 = discharge, 1 = charge */
	UINT8	output_type;
	UINT8	has_enable;
	double	v_out_high;
	double	threshold_low;	/* falling threshold */
	double	threshold_high;	/* rising threshold */
	double	v_cap;			/* current capacitor voltage */
	double	r_total;		/* all input resistors in parallel */
	double	i_fixed;		/* fixed current at the input */
	double	i_enable;		/* fixed current at the input if enabled */
	double	temp1;			/* Multi purpose */
	double	temp2;			/* Multi purpose */
	double	temp3;			/* Multi purpose */
	double	is_linear_charge;
	double	charge_rc[2];
	double	charge_exp[2];
	double	charge_v[2];
};

struct dss_sawtoothwave_context
{
	double	phase;
	int		type;
};

struct dss_schmitt_osc_context
{
	double	ration_in;			/* ratio of total charging voltage that comes from the input */
	double	ratio_feedback;	/* ratio of total charging voltage that comes from the feedback */
	double	v_cap;				/* current capacitor voltage */
	double	rc;					/* r*c */
	double	exponent;
	int		state;				/* state of the output */
	int		enable_type;
	UINT8	input_is_voltage;
};

struct dss_sinewave_context
{
	double phase;
};

struct dss_squarewave_context
{
	double phase;
	double trigger;
};

struct dss_squarewfix_context
{
	int		flip_flop;
	double	sample_step;
	double	t_left;
	double	t_off;
	double	t_on;
};

struct dss_trianglewave_context
{
	double phase;
};

struct dss_adsrenv_context
{
	double phase;
};

struct dss_counter_context
{
	int		clock_type;
	int		out_type;
	int		is_7492;
	int		last_clock;
	UINT32	last_count;
	UINT32	last;		/* Last clock state */
	UINT32	min;
	UINT32	max;
	UINT32	diff;
	double	t_left;		/* time unused during last sample in seconds */
};


struct dst_filter1_context
{
	double x1;		/* x[k-1], previous input value */
	double y1;		/* y[k-1], previous output value */
	double a1;		/* digital filter coefficients, denominator */
	double b0, b1;		/* digital filter coefficients, numerator */
};

struct dst_filter2_context
{
	double x1, x2;		/* x[k-1], x[k-2], previous 2 input values */
	double y1, y2;		/* y[k-1], y[k-2], previous 2 output values */
	double a1, a2;		/* digital filter coefficients, denominator */
	double b0, b1, b2;	/* digital filter coefficients, numerator */
};

struct dst_sallen_key_context
{
	double x1, x2;		/* x[k-1], x[k-2], previous 2 input values */
	double y1, y2;		/* y[k-1], y[k-2], previous 2 output values */
	double a1, a2;		/* digital filter coefficients, denominator */
	double b0, b1, b2;	/* digital filter coefficients, numerator */
};

struct dst_op_amp_filt_context
{
	int		type;		/* What kind of filter */
	int		is_norton;	/* 1 = Norton op-amps */
	double	vRef;
	double	vP;
	double	vN;
	double	rTotal;		/* All input resistance in parallel. */
	double	iFixed;		/* Current supplied by r3 & r4 if used. */
	double	exponentC1;
	double	exponentC2;
	double	exponentC3;
	double	rRatio;		/* divide ratio of resistance network */
	double	vC1;		/* Charge on C1 */
	double	vC1b;		/* Charge on C1, part of C1 charge if needed */
	double	vC2;		/* Charge on C2 */
	double	vC3;		/* Charge on C2 */
	double	gain;		/* Gain of the filter */
	double  x1, x2;		/* x[k-1], x[k-2], previous 2 input values */
	double  y1, y2;		/* y[k-1], y[k-2], previous 2 output values */
	double  a1,a2;		/* digital filter coefficients, denominator */
	double  b0,b1,b2;	/* digital filter coefficients, numerator */
};

struct dst_rc_circuit_1_context
{
	double v_cap;
	double v_charge_1_2;
	double v_drop;
	double exp_1;
	double exp_1_2;
	double exp_2;
};

struct dst_rcdisc_context
{
	int state;
	double t;           /* time */
	double exponent0;
	double exponent1;
	double v_cap;		/* rcdisc5 */
	double v_diode;		/* rcdisc3 */
};

struct dst_rcdisc_mod_context
{
	double	v_cap;
	double	exp_low[2];
	double	exp_high[4];
	double	gain[2];
	double	vd_gain[4];
};

struct dst_rcdisc4_context
{
	int		type;
	double	max_out;
	double	vC1;
	double	v[2];
	double	exp[2];
};

struct dst_rcfilter_context
{
	double	vCap;
	double	rc;
	double	exponent;
	UINT8	has_rc_nodes;
	UINT8	is_fast;
};

struct dst_crfilter_context
{
	double	vCap;
	double	rc;
	double	exponent;
	UINT8	has_rc_nodes;
	UINT8	is_fast;
};

struct dst_rcfilter_sw_context
{
	double	vCap[4];
	double	exp[4];
	double  exp0;	/* fast case bit 0 */
	double  exp1;	/* fast case bit 1 */
	double  factor; /* fast case */
	double  f1[16];
	double  f2[16];
};

struct dst_rcintegrate_context
{
	int		type;
	double	gain_r1_r2;
	double	f;				/* r2,r3 gain */
	double	vCap;
	double	vCE;
	double	exponent0;
	double	exponent1;
	double	exp_exponent0;
	double	exp_exponent1;
	double	c_exp0;
	double	c_exp1;
};

struct dso_csvlog_context
{
	FILE *csv_file;
	INT64 sample_num;
	char name[32];
};

struct dso_wavlog_context
{
	wav_file *wavfile;
	char name[32];
};

struct dsd_555_astbl_context
{
	int		use_ctrlv;
	int		output_type;
	int		output_is_ac;
	double	ac_shift;			/* DC shift needed to make waveform ac */
	int		flip_flop;			/* 555 flip/flop output state */
	double	cap_voltage;		/* voltage on cap */
	double	threshold;
	double	trigger;
	double	v_out_high;			/* Logic 1 voltage level */
	double	v_charge;
	double *v_charge_node;		/* point to output of node */
	int		has_rc_nodes;
	double	exp_bleed;
	double	exp_charge;
	double	exp_discharge;
	double	t_rc_bleed;
	double	t_rc_charge;
	double	t_rc_discharge;
	double	last_r1;
	double	last_r2;
	double	last_c;
};

struct dsd_555_mstbl_context
{
	int		trig_is_logic;
	int		trig_discharges_cap;
	int		output_type;
	double	ac_shift;				/* DC shift needed to make waveform ac */
	int		flip_flop;				/* 555 flip/flop output state */
	int		has_rc_nodes;
	double	exp_charge;
	double	cap_voltage;			/* voltage on cap */
	double	threshold;
	double	trigger;
	double	v_out_high;				/* Logic 1 voltage level */
	double	v_charge;
};

struct dsd_555_cc_context
{
	unsigned int	type;			/* type of 555cc circuit */
	int				output_type;
	int				output_is_ac;
	double			ac_shift;		/* DC shift needed to make waveform ac */
	int				flip_flop;		/* 555 flip/flop output state */
	double			cap_voltage;	/* voltage on cap */
	double			threshold;
	double			trigger;
	double			v_out_high;		/* Logic 1 voltage level */
	double			v_cc_source;
	int				has_rc_nodes;
	double			exp_bleed;
	double			exp_charge;
	double			exp_discharge;
	double			exp_discharge_01;
	double			exp_discharge_no_i;
	double			t_rc_charge;
	double			t_rc_discharge;
	double			t_rc_discharge_01;
	double			t_rc_discharge_no_i;
};

struct dsd_555_vco1_context
{
	int		ctrlv_is_node;
	int		output_type;
	int		output_is_ac;
	double	ac_shift;			/* DC shift needed to make waveform ac */
	int		flip_flop;			/* flip/flop output state */
	double	v_out_high;			/* 555 high voltage */
	double	threshold;			/* falling threshold */
	double	trigger;			/* rising threshold */
	double	i_charge;			/* charge current */
	double	i_discharge;		/* discharge current */
	double	cap_voltage;		/* current capacitor voltage */
};

struct dsd_566_context
{
	unsigned int state[2];			/* keeps track of excess flip_flop changes during the current step */
	int			flip_flop;			/* 566 flip/flop output state */
	double		cap_voltage;		/* voltage on cap */
	double		v_sqr_low;			/* voltage for a squarewave at low */
	double		v_sqr_high;			/* voltage for a squarewave at high */
	double		v_sqr_diff;
	double		threshold_low;		/* falling threshold */
	double		threshold_high;		/* rising threshold */
	double		ac_shift;			/* used to fake AC */
	double		v_osc_stable;
	double		v_osc_stop;
	int			fake_ac;
	int			out_type;
};

struct dsd_ls624_context
{
	double	exponent;
	double	t_used;
	double	v_cap_freq_in;
	double	v_freq_scale;
	double	v_rng_scale;
	int		flip_flop;
	int		has_freq_in_cap;
	int		out_type;
};

struct dst_comp_adder_context
{
	double	total[256];
};

struct dst_bits_decode_context
{
	int count;
	int decode_x_time;
	int from;
	int last_val;
	int last_had_x_time;
};

struct dst_dac_r1_context
{
	double	exponent;
	double	last_v;
	double	v_step[256];
	int		has_c_filter;
};

struct dst_diode_mix_context
{
	int		size;
	double	v_junction[8];
};

struct dst_flipflop_context
{
	int last_clk;
};

struct dst_integrate_context
{
	double	change;
	double	v_max_in;	/* v1 - norton VBE */
	double	v_max_in_d;	/* v1 - norton VBE - diode drop */
	double	v_max_out;
};

#define DISC_MIXER_MAX_INPS	8

struct dst_mixer_context
{
	int		type;
	int		size;
	int		r_node_bit_flag;
	int		c_bit_flag;
	double	r_total;
	double *r_node[DISC_MIXER_MAX_INPS];		/* Either pointer to resistance node output OR NULL */
	double	r_last[DISC_MIXER_MAX_INPS];
	double	exponent_rc[DISC_MIXER_MAX_INPS];	/* For high pass filtering cause by cIn */
	double	v_cap[DISC_MIXER_MAX_INPS];			/* cap voltage of each input */
	double	exponent_c_f;			/* Low pass on mixed inputs */
	double	exponent_c_amp;			/* Final high pass caused by out cap and amp input impedance */
	double	v_cap_f;				/* cap voltage of cF */
	double	v_cap_amp;				/* cap voltage of cAmp */
	double	gain;					/* used for DISC_MIXER_IS_OP_AMP_WITH_RI */
};

struct dst_oneshot_context
{
	double	countdown;
	int		state;
	int		last_trig;
	int		type;
};

struct dst_ramp_context
{
	double	step;
	int		dir;		/* 1 if End is higher then Start */
	int		last_en;	/* Keep track of the last enable value */
};

struct dst_samphold_context
{
	double last_input;
	int clocktype;
};

struct dst_logic_shift_context
{
	double	t_left;		/* time unused during last sample in seconds */
	UINT32	shift_data;
	UINT32	bit_mask;
	UINT8	clock_type;
	UINT8	reset_on_high;
	UINT8	shift_r;
	UINT8	last;
};

struct dst_size_context
{
	int size;
};

struct dst_multiplex_context
{
	int size;
};

struct dst_op_amp_context
{
	UINT8	has_cap;
	UINT8	has_r1;
	UINT8	has_r4;
	double	v_max;
	double	i_fixed;
	double	v_cap;
	double	exponent;
};

struct dst_op_amp_1sht_context
{
	double	i_fixed;
	double	v_max;
	double	r34ratio;
	double	v_cap1;
	double	v_cap2;
	double	exponent1c;
	double	exponent1d;
	double	exponent2;
};

struct dst_tvca_op_amp_context
{
	double	v_out_max;		/* Maximum output voltage */
	double	v_trig[2];		/* Voltage used to charge cap1 based on function F3 */
	double	v_trig2;			/* Voltage used to charge cap2 */
	double	v_trig3;			/* Voltage used to charge cap3 */
	double	i_fixed;		/* Fixed current going into - input */
	double	exponent_c[2];	/* Charge exponents based on function F3 */
	double	exponent_d[2];	/* Discharge exponents based on function F3 */
	double	exponent2[2];	/* Discharge/charge exponents based on function F4 */
	double	exponent3[2];	/* Discharge/charge exponents based on function F5 */
	double	exponent4;		/* Discharge/charge exponents for c4 */
	double	v_cap1;			/* charge on cap c1 */
	double	v_cap2;			/* charge on cap c2 */
	double	v_cap3;			/* charge on cap c3 */
	double	v_cap4;			/* charge on cap c4 */
	double	r67;			/* = r6 + r7 (for easy use later) */
	UINT8	has_c4;
	UINT8	has_r4;
};


struct dst_rcdisc2_context
{
	double x1;			/* x[k-1], last input value */
	double y1;			/* y[k-1], last output value */
	double a1_0, b0_0, b1_0;	/* digital filter coefficients, filter #1 */
	double a1_1, b0_1, b1_1;	/* digital filter coefficients, filter #2 */
};
;
DISCRETE_CLASS_STEP_RESET(dso_output, 0, 0);
DISCRETE_CLASS(dso_csvlog, sizeof(struct dso_csvlog_context), 0);
DISCRETE_CLASS(dso_wavlog, sizeof(struct dso_wavlog_context), 0);

DISCRETE_CLASS(dso_task_start, 0, 0);
DISCRETE_CLASS_STEP_RESET(dso_task_end, 0, 0);

DISCRETE_CLASS_STEP_RESET(dss_adjustment, sizeof(struct dss_adjustment_context), 1);
DISCRETE_CLASS_RESET(dss_constant, 0, 1);
DISCRETE_CLASS_RESET(dss_input_data, sizeof(struct dss_input_context), 1);
DISCRETE_CLASS_RESET(dss_input_logic, sizeof(struct dss_input_context), 1);
DISCRETE_CLASS_RESET(dss_input_not, sizeof(struct dss_input_context), 1);
DISCRETE_CLASS_STEP_RESET(dss_input_pulse, sizeof(struct dss_input_context), 1);
DISCRETE_CLASS(dss_input_stream, sizeof(struct dss_input_context), 1);

/* from disc_wav.c */
/* Generic modules */
DISCRETE_CLASS_STEP_RESET(dss_counter, sizeof(struct dss_counter_context), 1);
DISCRETE_CLASS_STEP_RESET(dss_lfsr_noise, sizeof(struct dss_lfsr_noise_context), 1);
DISCRETE_CLASS_STEP_RESET(dss_noise, sizeof(struct dss_noise_context), 2);
DISCRETE_CLASS_STEP_RESET(dss_note, sizeof(struct dss_note_context), 1);
DISCRETE_CLASS_STEP_RESET(dss_sawtoothwave, sizeof(struct dss_sawtoothwave_context), 1);
DISCRETE_CLASS_STEP_RESET(dss_sinewave, sizeof(struct dss_sinewave_context), 1);
DISCRETE_CLASS_STEP_RESET(dss_squarewave, sizeof(struct dss_squarewave_context), 1);
DISCRETE_CLASS_STEP_RESET(dss_squarewfix, sizeof(struct dss_squarewfix_context), 1);
DISCRETE_CLASS_STEP_RESET(dss_squarewave2, sizeof(struct dss_squarewave_context), 1);
DISCRETE_CLASS_STEP_RESET(dss_trianglewave, sizeof(struct dss_trianglewave_context), 1);
/* Component specific modules */

class DISCRETE_CLASS_NAME(dss_inverter_osc): public discrete_base_node
{
protected:
	inline double tftab(double x);
	inline double tf(double x);
public:
	DISCRETE_CLASS_CONSTRUCTOR(dss_inverter_osc, 0)
	void step(void);
	void reset(void);
	int max_output(void) { return 1; }
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


DISCRETE_CLASS_STEP_RESET(dss_op_amp_osc, sizeof(struct dss_op_amp_osc_context), 1);
DISCRETE_CLASS_STEP_RESET(dss_schmitt_osc, sizeof(struct dss_schmitt_osc_context), 1);
/* Not yet implemented */
DISCRETE_CLASS_STEP_RESET(dss_adsrenv, sizeof(struct dss_adsrenv_context), 1);

/* from disc_mth.c */
/* Generic modules */
DISCRETE_CLASS_STEP(dst_adder, 0, 1);
DISCRETE_CLASS_STEP(dst_clamp, 0, 1);
DISCRETE_CLASS_STEP(dst_divide,0, 1);
DISCRETE_CLASS_STEP(dst_gain, 0, 1);
DISCRETE_CLASS_STEP(dst_logic_inv, 0, 1);
DISCRETE_CLASS_STEP_RESET(dst_bits_decode, sizeof(struct dst_bits_decode_context), 8);
DISCRETE_CLASS_STEP(dst_logic_and, 0, 1);
DISCRETE_CLASS_STEP(dst_logic_nand, 0, 1);
DISCRETE_CLASS_STEP(dst_logic_or, 0, 1);
DISCRETE_CLASS_STEP(dst_logic_nor, 0, 1);
DISCRETE_CLASS_STEP(dst_logic_xor, 0, 1);
DISCRETE_CLASS_STEP(dst_logic_nxor, 0, 1);
DISCRETE_CLASS_STEP_RESET(dst_logic_dff, sizeof(struct dst_flipflop_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_logic_jkff, sizeof(struct dst_flipflop_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_logic_shift, sizeof(struct dst_logic_shift_context), 1);
DISCRETE_CLASS_STEP(dst_lookup_table, 0, 1);
DISCRETE_CLASS_STEP_RESET(dst_multiplex, sizeof(struct dst_multiplex_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_oneshot, sizeof(struct dst_oneshot_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_ramp, sizeof(struct dst_ramp_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_samphold, sizeof(struct dst_samphold_context), 1);
DISCRETE_CLASS_STEP(dst_switch, 0, 1);
DISCRETE_CLASS_STEP(dst_aswitch, 0, 1);
DISCRETE_CLASS_STEP(dst_transform, 0, 1);
/* Component specific */
DISCRETE_CLASS_STEP_RESET(dst_comp_adder, sizeof(struct dst_comp_adder_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_dac_r1, sizeof(struct dst_dac_r1_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_diode_mix, sizeof(struct dst_diode_mix_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_integrate, sizeof(struct dst_integrate_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_mixer, sizeof(struct dst_mixer_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_op_amp, sizeof(struct dst_op_amp_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_op_amp_1sht, sizeof(struct dst_op_amp_1sht_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_tvca_op_amp, sizeof(struct dst_tvca_op_amp_context), 1);
DISCRETE_CLASS_STEP(dst_xtime_buffer, 0, 1);
DISCRETE_CLASS_STEP(dst_xtime_and, 0, 1);
DISCRETE_CLASS_STEP(dst_xtime_or, 0, 1);
DISCRETE_CLASS_STEP(dst_xtime_xor, 0, 1);

/* from disc_flt.c */
/* Generic modules */
DISCRETE_CLASS_STEP_RESET(dst_filter1, sizeof(struct dst_filter1_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_filter2, sizeof(struct dst_filter2_context), 1);
/* Component specific modules */
DISCRETE_CLASS_STEP_RESET(dst_sallen_key, sizeof(struct dst_sallen_key_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_crfilter, sizeof(struct dst_crfilter_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_op_amp_filt, sizeof(struct dst_op_amp_filt_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_rc_circuit_1, sizeof(struct dst_rc_circuit_1_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_rcdisc, sizeof(struct dst_rcdisc_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_rcdisc2, sizeof(struct dst_rcdisc_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_rcdisc3, sizeof(struct dst_rcdisc_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_rcdisc4, sizeof(struct dst_rcdisc4_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_rcdisc5, sizeof(struct dst_rcdisc_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_rcintegrate, sizeof(struct dst_rcintegrate_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_rcdisc_mod, sizeof(struct dst_rcdisc_mod_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_rcfilter, sizeof(struct dst_rcfilter_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_rcfilter_sw, sizeof(struct dst_rcfilter_sw_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_rcdiscN, sizeof(struct dst_filter1_context), 1);
DISCRETE_CLASS_STEP_RESET(dst_rcdisc2N, sizeof(struct dst_rcdisc2_context), 1);
/* from disc_dev.c */
/* generic modules */
/* Component specific modules */
DISCRETE_CLASS_STEP_RESET(dsd_555_astbl, sizeof(struct dsd_555_astbl_context), 1);
DISCRETE_CLASS_STEP_RESET(dsd_555_mstbl, sizeof(struct dsd_555_mstbl_context), 1);
DISCRETE_CLASS_STEP_RESET(dsd_555_cc, sizeof(struct dsd_555_cc_context), 1);
DISCRETE_CLASS_STEP_RESET(dsd_555_vco1, sizeof(struct dsd_555_vco1_context), 1);
DISCRETE_CLASS_STEP_RESET(dsd_566, sizeof(struct dsd_566_context), 1);
DISCRETE_CLASS_STEP_RESET(dsd_ls624, sizeof(struct dsd_ls624_context), 1);
/* must be the last one */



#endif /* __DISCRETE_H__ */
