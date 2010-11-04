/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by Keith Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *  (c) D.Renaud 2003-2004
 *
 ************************************************************************
 *
 * DST_ADDDER            - Multichannel adder
 * DST_BITS_DECODE       - Decode Bits from input node
 * DST_CLAMP             - Simple signal clamping circuit
 * DST_COMP_ADDER        - Selectable parallel component circuit
 * DST_DAC_R1            - R1 Ladder DAC with cap filtering
 * DST_DIODE_MIX         - Diode mixer
 * DST_DIVIDE            - Division function
 * DST_GAIN              - Gain Factor
 * DST_INTEGRATE         - Integration circuits
 * DST_LOGIC_INV         - Logic level invertor
 * DST_LOGIC_AND         - Logic AND gate 4 input
 * DST_LOGIC_NAND        - Logic NAND gate 4 input
 * DST_LOGIC_OR          - Logic OR gate 4 input
 * DST_LOGIC_NOR         - Logic NOR gate 4 input
 * DST_LOGIC_XOR         - Logic XOR gate 2 input
 * DST_LOGIC_NXOR        - Logic NXOR gate 2 input
 * DST_LOGIC_DFF         - Logic D-type flip/flop
 * DST_LOGIC_JKFF        - Logic JK-type flip/flop
 * DST_LOGIC_SHIFT       - Logic Shift Register
 * DST_LOOKUP_TABLE      - Return value from lookup table
 * DST_MIXER             - Final Mixer Stage
 * DST_MULTIPLEX         - 1 of x Multiplexer/switch
 * DST_ONESHOT           - One shot pulse generator
 * DST_RAMP              - Ramp up/down
 * DST_SAMPHOLD          - Sample & Hold Implementation
 * DST_SWITCH            - Switch implementation
 * DST_ASWITCH           - Analog switch
 * DST_TRANSFORM         - Multiple math functions
 * DST_OP_AMP            - Op Amp circuits
 * DST_OP_AMP_1SHT       - Op Amp One Shot
 * DST_TVCA_OP_AMP       - Triggered op amp voltage controlled amplifier
 * DST_XTIME_BUFFER      - Buffer/Invertor gate implementation using X_TIME
 * DST_XTIME_AND         - AND/NAND gate implementation using X_TIME
 * DST_XTIME_OR          - OR/NOR gate implementation using X_TIME
 * DST_XTIME_XOR         - XOR/XNOR gate implementation using X_TIME
 *
 ************************************************************************/

#include <float.h>

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

struct dst_shift_context
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


/************************************************************************
 *
 * DST_ADDER - This is a 4 channel input adder with enable function
 *
 * input[0]    - Enable input value
 * input[1]    - Channel0 input value
 * input[2]    - Channel1 input value
 * input[3]    - Channel2 input value
 * input[4]    - Channel3 input value
 *
 ************************************************************************/
#define DST_ADDER__ENABLE	DISCRETE_INPUT(0)
#define DST_ADDER__IN0		DISCRETE_INPUT(1)
#define DST_ADDER__IN1		DISCRETE_INPUT(2)
#define DST_ADDER__IN2		DISCRETE_INPUT(3)
#define DST_ADDER__IN3		DISCRETE_INPUT(4)

DISCRETE_STEP(dst_adder)
{
	if(DST_ADDER__ENABLE)
	{
		node->output[0] = DST_ADDER__IN0 + DST_ADDER__IN1 + DST_ADDER__IN2 + DST_ADDER__IN3;
	}
	else
	{
		node->output[0]=0;
	}
}


/************************************************************************
 *
 * DST_COMP_ADDER  - Selectable parallel component adder
 *
 * input[0]    - Bit Select
 *
 * Also passed discrete_comp_adder_table structure
 *
 * Mar 2004, D Renaud.
 ************************************************************************/
#define DST_COMP_ADDER__SELECT	DISCRETE_INPUT(0)

DISCRETE_STEP(dst_comp_adder)
{
	DISCRETE_DECLARE_CONTEXT(dst_comp_adder)
	int select;

	select = (int)DST_COMP_ADDER__SELECT;
	assert(select < 256);
	node->output[0] = context->total[select];
}

DISCRETE_RESET(dst_comp_adder)
{
	DISCRETE_DECLARE_CONTEXT(dst_comp_adder)
	DISCRETE_DECLARE_INFO(discrete_comp_adder_table)

	int i, bit;
	int bit_length = info->length;
	int length = 1 << bit_length;

	assert(length <= 256);

	/* pre-calculate all possible values to speed up step routine */
	for(i = 0; i < length; i++)
	{
		switch (info->type)
		{
			case DISC_COMP_P_CAPACITOR:
				context->total[i] = info->cDefault;
				for(bit = 0; bit < bit_length; bit++)
				{
					if (i & (1 << bit))
						context->total[i] += info->c[bit];
				}
				break;
			case DISC_COMP_P_RESISTOR:
				context->total[i] = (info->cDefault != 0) ? 1.0 / info->cDefault : 0;
				for(bit = 0; bit < bit_length; bit++)
				{
					if ((i & (1 << bit)) && (info->c[bit] != 0))
						context->total[i] += 1.0 / info->c[bit];
				}
				if (context->total[i] != 0)
					context->total[i] = 1.0 / context->total[i];
				break;
		}
	}
	node->output[0] = context->total[0];
}

/************************************************************************
 *
 * DST_CLAMP - Simple signal clamping circuit
 *
 * input[0]    - Input value
 * input[1]    - Minimum value
 * input[2]    - Maximum value
 *
 ************************************************************************/
#define DST_CLAMP__IN		DISCRETE_INPUT(0)
#define DST_CLAMP__MIN		DISCRETE_INPUT(1)
#define DST_CLAMP__MAX		DISCRETE_INPUT(2)

DISCRETE_STEP(dst_clamp)
{
	if (DST_CLAMP__IN < DST_CLAMP__MIN) node->output[0] = DST_CLAMP__MIN;
	else if (DST_CLAMP__IN > DST_CLAMP__MAX) node->output[0] = DST_CLAMP__MAX;
	else node->output[0]= DST_CLAMP__IN;
}


/************************************************************************
 *
 * DST_DAC_R1 - R1 Ladder DAC with cap smoothing
 *
 * input[0]    - Binary Data Input
 * input[1]    - Data On Voltage (3.4 for TTL)
 *
 * also passed discrete_dac_r1_ladder structure
 *
 * Mar 2004, D Renaud.
 * Nov 2010, D Renaud. - optimized for speed
 ************************************************************************/
#define DST_DAC_R1__DATA		DISCRETE_INPUT(0)
#define DST_DAC_R1__VON			DISCRETE_INPUT(1)

DISCRETE_STEP(dst_dac_r1)
{
	DISCRETE_DECLARE_CONTEXT(dst_dac_r1)

	int		data = (int)DST_DAC_R1__DATA;
	double	v = context->v_step[data];
	double	x_time = DST_DAC_R1__DATA - data;
	double	last_v = context->last_v;

	context->last_v = v;

	if (x_time > 0)
		v = x_time * (v - last_v) + last_v;

	/* Filter if needed, else just output voltage */
	if (context->has_c_filter)
	{
		double out = node->output[0];
		double v_diff = v - out;
		/* optimization - if charged close enough to voltage */
		if (fabs(v_diff) < 0.000001)
			node->output[0] = v;
		else
		{
			out += v_diff * context->exponent;
			node->output[0] = out;
		}
	}
	else
		node->output[0] = v;
}

DISCRETE_RESET(dst_dac_r1)
{
	DISCRETE_DECLARE_CONTEXT(dst_dac_r1)
	DISCRETE_DECLARE_INFO(discrete_dac_r1_ladder)

	int	bit;
	int ladderLength = info->ladderLength;
	int total_steps = 1 << ladderLength;
	double r_total = 0;
	double i_bias;
	double v_on = DST_DAC_R1__VON;

	context->last_v = 0;

	/* Calculate the Millman current of the bias circuit */
	if (info->rBias > 0)
		i_bias = info->vBias / info->rBias;
	else
		i_bias = 0;

	/*
     * We will do a small amount of error checking.
     * But if you are an idiot and pass a bad ladder table
     * then you deserve a crash.
     */
	if (ladderLength < 2 && info->rBias == 0 && info->rGnd == 0)
	{
		/* You need at least 2 resistors for a ladder */
		discrete_log(node->info, "dst_dac_r1_reset - Ladder length too small");
	}
	if (ladderLength > DISC_LADDER_MAXRES )
	{
		discrete_log(node->info, "dst_dac_r1_reset - Ladder length exceeds DISC_LADDER_MAXRES");
	}

	/*
     * Calculate the total of all resistors in parallel.
     * This is the combined resistance of the voltage sources.
     * This is used for the charging curve.
     */
	for(bit = 0; bit < ladderLength; bit++)
	{
		if (info->r[bit] > 0)
			r_total += 1.0 / info->r[bit];
	}
	if (info->rBias > 0) r_total += 1.0 / info->rBias;
	if (info->rGnd > 0)  r_total += 1.0 / info->rGnd;
	r_total = 1.0 / r_total;

	node->output[0] = 0;

	if (info->cFilter > 0)
	{
		context->has_c_filter = 1;
		/* Setup filter constant */
		context->exponent = RC_CHARGE_EXP(r_total * info->cFilter);
	}
	else
		context->has_c_filter = 0;

	/* pre-calculate all possible values to speed up step routine */
	for(int i = 0; i < total_steps; i++)
	{
		double i_total = i_bias;
		for (bit = 0; bit < ladderLength; bit++)
		{
			/* Add up currents of ON circuits per Millman. */

			/* ignore if no resistor present */
			if (EXPECTED(info->r[bit] > 0))
			{
				double i_bit;
				int bit_val = (i >> bit) & 0x01;

				if (bit_val != 0)
					i_bit   = v_on / info->r[bit];
				else
					i_bit = 0;
				i_total += i_bit;
			}
		}
		context->v_step[i] = i_total * r_total;
	}
}


/************************************************************************
*
 * DST_DIODE_MIX  - Diode Mixer
 *
 * input[0]    - Input 0
 * .....
 *
 * Dec 2004, D Renaud.
 ************************************************************************/
#define DST_DIODE_MIX_INP_OFFSET	0
#define DST_DIODE_MIX__INP(addr)	DISCRETE_INPUT(DST_DIODE_MIX_INP_OFFSET + addr)

DISCRETE_STEP(dst_diode_mix)
{
	DISCRETE_DECLARE_CONTEXT(dst_diode_mix)

	double	val, max = 0;
	int		addr;

	for (addr = 0; addr < context->size; addr++)
	{
		val = DST_DIODE_MIX__INP(addr) - context->v_junction[addr];
		if (val > max) max = val;
	}
	if (max < 0) max = 0;
	node->output[0] = max;
}

DISCRETE_RESET(dst_diode_mix)
{
	DISCRETE_DECLARE_CONTEXT(dst_diode_mix)
	DISCRETE_DECLARE_INFO(double)

	int		addr;

	context->size = node->active_inputs - DST_DIODE_MIX_INP_OFFSET;
	assert(context->size <= 8);

	for (addr = 0; addr < context->size; addr++)
	{
		if (info == NULL)
		{
			/* setup default junction voltage */
			context->v_junction[addr] = 0.5;
		}
		else
		{
			/* use supplied junction voltage */
			context->v_junction[addr] = *info++;
		}
	}
	DISCRETE_STEP_CALL(dst_diode_mix);
}


/************************************************************************
 *
 * DST_DIVIDE  - Programmable divider with enable
 *
 * input[0]    - Enable input value
 * input[1]    - Channel0 input value
 * input[2]    - Divisor
 *
 ************************************************************************/
#define DST_DIVIDE__ENABLE	DISCRETE_INPUT(0)
#define DST_DIVIDE__IN		DISCRETE_INPUT(1)
#define DST_DIVIDE__DIV		DISCRETE_INPUT(2)

DISCRETE_STEP(dst_divide)
{
	if(DST_DIVIDE__ENABLE)
	{
		if(DST_DIVIDE__DIV == 0)
		{
			node->output[0 ]= DBL_MAX;	/* Max out but don't break */
			discrete_log(node->info, "dst_divider_step() - Divide by Zero attempted in NODE_%02d.\n",NODE_BLOCKINDEX(node));
		}
		else
		{
			node->output[0]= DST_DIVIDE__IN / DST_DIVIDE__DIV;
		}
	}
	else
	{
		node->output[0]=0;
	}
}


/************************************************************************
 *
 * DST_GAIN - This is a programmable gain module with enable function
 *
 * input[0]    - Channel0 input value
 * input[1]    - Gain value
 * input[2]    - Final addition offset
 *
 ************************************************************************/
#define DST_GAIN__IN		DISCRETE_INPUT(0)
#define DST_GAIN__GAIN		DISCRETE_INPUT(1)
#define DST_GAIN__OFFSET	DISCRETE_INPUT(2)

DISCRETE_STEP(dst_gain)
{
		node->output[0]  = DST_GAIN__IN * DST_GAIN__GAIN + DST_GAIN__OFFSET;
}


/************************************************************************
 *
 * DST_INTEGRATE - Integration circuits
 *
 * input[0] - Trigger 0
 * input[1] - Trigger 1
 *
 * also passed discrete_integrate_info structure
 *
 * Mar 2004, D Renaud.
 ************************************************************************/
#define DST_INTEGRATE__TRG0	DISCRETE_INPUT(0)
#define DST_INTEGRATE__TRG1	DISCRETE_INPUT(1)

static int dst_trigger_function(int trig0, int trig1, int trig2, int function)
{
	int result = 1;
	switch (function)
	{
		case DISC_OP_AMP_TRIGGER_FUNCTION_TRG0:
			result = trig0;
			break;
		case DISC_OP_AMP_TRIGGER_FUNCTION_TRG0_INV:
			result = !trig0;
			break;
		case DISC_OP_AMP_TRIGGER_FUNCTION_TRG1:
			result = trig1;
			break;
		case DISC_OP_AMP_TRIGGER_FUNCTION_TRG1_INV:
			result = !trig1;
			break;
		case DISC_OP_AMP_TRIGGER_FUNCTION_TRG2:
			result = trig2;
			break;
		case DISC_OP_AMP_TRIGGER_FUNCTION_TRG2_INV:
			result = !trig2;
			break;
		case DISC_OP_AMP_TRIGGER_FUNCTION_TRG01_AND:
			result = trig0 && trig1;
			break;
		case DISC_OP_AMP_TRIGGER_FUNCTION_TRG01_NAND:
			result = !(trig0 && trig1);
			break;
	}

	return (result);
}

DISCRETE_STEP(dst_integrate)
{
	DISCRETE_DECLARE_CONTEXT(dst_integrate)
	DISCRETE_DECLARE_INFO(discrete_integrate_info)

	int		trig0, trig1;
	double	i_neg = 0;	/* current into - input */
	double	i_pos = 0;	/* current into + input */

	switch (info->type)
	{
		case DISC_INTEGRATE_OP_AMP_1:
			if (DST_INTEGRATE__TRG0 != 0)
			{
				/* This forces the cap to completely charge,
                 * and the output to go to it's max value.
                 */
				node->output[0] = context->v_max_out;
				return;
			}
			node->output[0] -= context->change;
			break;

		case DISC_INTEGRATE_OP_AMP_1 | DISC_OP_AMP_IS_NORTON:
			i_neg = context->v_max_in / info->r1;
			i_pos = (DST_INTEGRATE__TRG0 - OP_AMP_NORTON_VBE) / info->r2;
			if (i_pos < 0) i_pos = 0;
			node->output[0] += (i_pos - i_neg) / node->info->sample_rate / info->c;
			break;

		case DISC_INTEGRATE_OP_AMP_2 | DISC_OP_AMP_IS_NORTON:
			trig0  = (int)DST_INTEGRATE__TRG0;
			trig1  = (int)DST_INTEGRATE__TRG1;
			i_neg  = dst_trigger_function(trig0, trig1, 0, info->f0) ? context->v_max_in_d / info->r1 : 0;
			i_pos  = dst_trigger_function(trig0, trig1, 0, info->f1) ? context->v_max_in / info->r2 : 0;
			i_pos += dst_trigger_function(trig0, trig1, 0, info->f2) ? context->v_max_in_d / info->r3 : 0;
			node->output[0] += (i_pos - i_neg) / node->info->sample_rate / info->c;
			break;
	}

	/* Clip the output. */
	if (node->output[0] < 0) node->output[0] = 0;
	if (node->output[0] > context->v_max_out) node->output[0] = context->v_max_out;
}

DISCRETE_RESET(dst_integrate)
{
	DISCRETE_DECLARE_CONTEXT(dst_integrate)
	DISCRETE_DECLARE_INFO(discrete_integrate_info)

	double	i, v;

	if (info->type & DISC_OP_AMP_IS_NORTON)
	{
		context->v_max_out  = info->vP - OP_AMP_NORTON_VBE;
		context->v_max_in   = info->v1 - OP_AMP_NORTON_VBE;
		context->v_max_in_d = context->v_max_in - OP_AMP_NORTON_VBE;
	}
	else
	{
		context->v_max_out =  info->vP - OP_AMP_VP_RAIL_OFFSET;

		v = info->v1 * info->r3 / (info->r2 + info->r3);	/* vRef */
		v = info->v1 - v;	/* actual charging voltage */
		i = v / info->r1;
		context->change = i / node->info->sample_rate / info->c;
	}
	node->output[0] = 0;
}


/************************************************************************
 *
 * DST_LOGIC_INV - Logic invertor gate implementation
 *
 * input[0]    - Enable
 * input[1]    - input[0] value
 *
 ************************************************************************/
#define DST_LOGIC_INV__IN		DISCRETE_INPUT(0)

DISCRETE_STEP(dst_logic_inv)
{
	node->output[0] = DST_LOGIC_INV__IN ? 0.0 : 1.0;
}

/************************************************************************
 *
 * DST_BITS_DECODE - Decode Bits from input node
 *
 ************************************************************************/
#define DST_BITS_DECODE__IN		DISCRETE_INPUT(0)
#define DST_BITS_DECODE__FROM	DISCRETE_INPUT(1)
#define DST_BITS_DECODE__TO		DISCRETE_INPUT(2)
#define DST_BITS_DECODE__VOUT	DISCRETE_INPUT(3)

DISCRETE_STEP(dst_bits_decode)
{
	DISCRETE_DECLARE_CONTEXT(dst_bits_decode)

	int new_val = DST_BITS_DECODE__IN;
	int last_val = context->last_val;
	int last_had_x_time = context->last_had_x_time;

	if (last_val != new_val || last_had_x_time)
	{
		int i, new_bit, last_bit, last_bit_had_x_time, bit_changed;
		double x_time = DST_BITS_DECODE__IN - new_val;
		int from = context->from;
		int count = context->count;
		int decode_x_time = context->decode_x_time;
		int has_x_time = x_time > 0 ? 1 : 0;
		double out = 0;
		double v_out = DST_BITS_DECODE__VOUT;

		for (i = 0; i < count; i++ )
		{
			new_bit = (new_val >> (i + from)) & 1;
			last_bit = (last_val >> (i + from)) & 1;
			last_bit_had_x_time = (last_had_x_time >> (i + from)) & 1;
			bit_changed = last_bit != new_bit ? 1 : 0;

			if (!bit_changed && !last_bit_had_x_time)
				continue;

			if (decode_x_time)
			{
				out = new_bit;
				if (bit_changed)
					out += x_time;
			}
			else
			{
				out = v_out;
				if (has_x_time && bit_changed)
				{
					if (new_bit)
						out *= x_time;
					else
						out *= (1.0 - x_time);
				}
				else
					out *= new_bit;
			}
			node->output[i] = out;
			if (has_x_time && bit_changed)
				/* set */
				context->last_had_x_time |= 1 << (i + from);
			else
				/* clear */
				context->last_had_x_time &= ~(1 << (i + from));
		}
		context->last_val = new_val;
	}
}

DISCRETE_RESET(dst_bits_decode)
{
	DISCRETE_DECLARE_CONTEXT(dst_bits_decode)

	context->from = DST_BITS_DECODE__FROM;
	context->count = DST_BITS_DECODE__TO - context->from + 1;
	if (DST_BITS_DECODE__VOUT == 0)
		context->decode_x_time = 1;
	else
		context->decode_x_time = 0;
	context->last_had_x_time = 0;

	DISCRETE_STEP_CALL(dst_bits_decode);
}


/************************************************************************
 *
 * DST_LOGIC_AND - Logic AND gate implementation
 *
 * input[0]    - input[0] value
 * input[1]    - input[1] value
 * input[2]    - input[2] value
 * input[3]    - input[3] value
 *
 ************************************************************************/
#define DST_LOGIC_AND__IN0		DISCRETE_INPUT(0)
#define DST_LOGIC_AND__IN1		DISCRETE_INPUT(1)
#define DST_LOGIC_AND__IN2		DISCRETE_INPUT(2)
#define DST_LOGIC_AND__IN3		DISCRETE_INPUT(3)

DISCRETE_STEP(dst_logic_and)
{
	node->output[0] = (DST_LOGIC_AND__IN0 && DST_LOGIC_AND__IN1 && DST_LOGIC_AND__IN2 && DST_LOGIC_AND__IN3)? 1.0 : 0.0;
}

/************************************************************************
 *
 * DST_LOGIC_NAND - Logic NAND gate implementation
 *
 * input[0]    - input[0] value
 * input[1]    - input[1] value
 * input[2]    - input[2] value
 * input[3]    - input[3] value
 *
 ************************************************************************/
#define DST_LOGIC_NAND__IN0		DISCRETE_INPUT(0)
#define DST_LOGIC_NAND__IN1		DISCRETE_INPUT(1)
#define DST_LOGIC_NAND__IN2		DISCRETE_INPUT(2)
#define DST_LOGIC_NAND__IN3		DISCRETE_INPUT(3)

DISCRETE_STEP(dst_logic_nand)
{
	node->output[0]= (DST_LOGIC_NAND__IN0 && DST_LOGIC_NAND__IN1 && DST_LOGIC_NAND__IN2 && DST_LOGIC_NAND__IN3)? 0.0 : 1.0;
}

/************************************************************************
 *
 * DST_LOGIC_OR  - Logic OR  gate implementation
 *
 * input[0]    - input[0] value
 * input[1]    - input[1] value
 * input[2]    - input[2] value
 * input[3]    - input[3] value
 *
 ************************************************************************/
#define DST_LOGIC_OR__IN0		DISCRETE_INPUT(0)
#define DST_LOGIC_OR__IN1		DISCRETE_INPUT(1)
#define DST_LOGIC_OR__IN2		DISCRETE_INPUT(2)
#define DST_LOGIC_OR__IN3		DISCRETE_INPUT(3)

DISCRETE_STEP(dst_logic_or)
{
	node->output[0] = (DST_LOGIC_OR__IN0 || DST_LOGIC_OR__IN1 || DST_LOGIC_OR__IN2 || DST_LOGIC_OR__IN3) ? 1.0 : 0.0;
}

/************************************************************************
 *
 * DST_LOGIC_NOR - Logic NOR gate implementation
 *
 * input[0]    - input[0] value
 * input[1]    - input[1] value
 * input[2]    - input[2] value
 * input[3]    - input[3] value
 *
 ************************************************************************/
#define DST_LOGIC_NOR__IN0		DISCRETE_INPUT(0)
#define DST_LOGIC_NOR__IN1		DISCRETE_INPUT(1)
#define DST_LOGIC_NOR__IN2		DISCRETE_INPUT(2)
#define DST_LOGIC_NOR__IN3		DISCRETE_INPUT(3)

DISCRETE_STEP(dst_logic_nor)
{
	node->output[0] = (DST_LOGIC_NOR__IN0 || DST_LOGIC_NOR__IN1 || DST_LOGIC_NOR__IN2 || DST_LOGIC_NOR__IN3) ? 0.0 : 1.0;
}

/************************************************************************
 *
 * DST_LOGIC_XOR - Logic XOR gate implementation
 *
 * input[0]    - input[0] value
 * input[1]    - input[1] value
 *
 ************************************************************************/
#define DST_LOGIC_XOR__IN0		DISCRETE_INPUT(0)
#define DST_LOGIC_XOR__IN1		DISCRETE_INPUT(1)

DISCRETE_STEP(dst_logic_xor)
{
	node->output[0] = ((DST_LOGIC_XOR__IN0 && !DST_LOGIC_XOR__IN1) || (!DST_LOGIC_XOR__IN0 && DST_LOGIC_XOR__IN1)) ? 1.0 : 0.0;
}

/************************************************************************
 *
 * DST_LOGIC_NXOR - Logic NXOR gate implementation
 *
 * input[0]    - input[0] value
 * input[1]    - input[1] value
 *
 ************************************************************************/
#define DST_LOGIC_XNOR__IN0		DISCRETE_INPUT(0)
#define DST_LOGIC_XNOR__IN1		DISCRETE_INPUT(1)

DISCRETE_STEP(dst_logic_nxor)
{
	node->output[0] = ((DST_LOGIC_XNOR__IN0 && !DST_LOGIC_XNOR__IN1) || (!DST_LOGIC_XNOR__IN0 && DST_LOGIC_XNOR__IN1)) ? 0.0 : 1.0;
}


/************************************************************************
 *
 * DST_LOGIC_DFF - Standard D-type flip-flop implementation
 *
 * input[0]    - /Reset
 * input[1]    - /Set
 * input[2]    - clock
 * input[3]    - data
 *
 ************************************************************************/
#define DST_LOGIC_DFF__RESET	!DISCRETE_INPUT(0)
#define DST_LOGIC_DFF__SET		!DISCRETE_INPUT(1)
#define DST_LOGIC_DFF__CLOCK	 DISCRETE_INPUT(2)
#define DST_LOGIC_DFF__DATA 	 DISCRETE_INPUT(3)

DISCRETE_STEP(dst_logic_dff)
{
	DISCRETE_DECLARE_CONTEXT(dst_flipflop)

	int clk = (int)DST_LOGIC_DFF__CLOCK;

	if (DST_LOGIC_DFF__RESET)
		node->output[0] = 0;
	else if (DST_LOGIC_DFF__SET)
		node->output[0] = 1;
	else if (!context->last_clk && clk)	/* low to high */
		node->output[0] = DST_LOGIC_DFF__DATA;
	context->last_clk = clk;
}

DISCRETE_RESET(dst_logic_ff)
{
	DISCRETE_DECLARE_CONTEXT(dst_flipflop)


	context->last_clk = 0;
	node->output[0]   = 0;
}


/************************************************************************
 *
 * DST_LOGIC_JKFF - Standard JK-type flip-flop implementation
 *
 * input[0]    - /Reset
 * input[1]    - /Set
 * input[2]    - clock
 * input[3]    - J
 * input[4]    - K
 *
 ************************************************************************/
#define DST_LOGIC_JKFF__RESET	!DISCRETE_INPUT(0)
#define DST_LOGIC_JKFF__SET		!DISCRETE_INPUT(1)
#define DST_LOGIC_JKFF__CLOCK	 DISCRETE_INPUT(2)
#define DST_LOGIC_JKFF__J		 DISCRETE_INPUT(3)
#define DST_LOGIC_JKFF__K		 DISCRETE_INPUT(4)

DISCRETE_STEP(dst_logic_jkff)
{
	DISCRETE_DECLARE_CONTEXT(dst_flipflop)

	int clk = (int)DST_LOGIC_JKFF__CLOCK;
	int j   = (int)DST_LOGIC_JKFF__J;
	int k   = (int)DST_LOGIC_JKFF__K;

	if (DST_LOGIC_JKFF__RESET)
		node->output[0] = 0;
	else if (DST_LOGIC_JKFF__SET)
		node->output[0] = 1;
	else if (context->last_clk && !clk)	/* high to low */
	{
		if (!j)
		{
			/* J=0, K=0 - Hold */
			if (k)
				/* J=0, K=1 - Reset */
				node->output[0] = 0;
		}
		else
		{
			if (!k)
				/* J=1, K=0 - Set */
				node->output[0] = 1;
			else
				/* J=1, K=1 - Toggle */
				node->output[0] = !(int)node->output[0];
		}
	}
	context->last_clk = clk;
}


/************************************************************************
 *
 * DST_LOGIC_SHIFT - Shift Register implementation
 *
 ************************************************************************/
#define DST_LOGIC_SHIFT__IN			DISCRETE_INPUT(0)
#define DST_LOGIC_SHIFT__RESET		DISCRETE_INPUT(1)
#define DST_LOGIC_SHIFT__CLK		DISCRETE_INPUT(2)
#define DST_LOGIC_SHIFT__SIZE		DISCRETE_INPUT(3)
#define DST_LOGIC_SHIFT__OPTIONS	DISCRETE_INPUT(4)

DISCRETE_STEP(dst_logic_shift)
{
	DISCRETE_DECLARE_CONTEXT(dst_shift)

	double	cycles;
	double	ds_clock;
	int		clock = 0, inc = 0;

	int input_bit = (DST_LOGIC_SHIFT__IN != 0) ? 1 : 0;
	ds_clock = DST_LOGIC_SHIFT__CLK;
	if (context->clock_type == DISC_CLK_IS_FREQ)
	{
		/* We need to keep clocking the internal clock even if in reset. */
		cycles = (context->t_left + node->info->sample_time) * ds_clock;
		inc    = (int)cycles;
		context->t_left = (cycles - inc) / ds_clock;
	}
	else
	{
		clock  = (int)ds_clock;
	}

	/* If reset enabled then set output to the reset value.  No x_time in reset. */
	if(((DST_LOGIC_SHIFT__RESET == 0) ? 0 : 1) == context->reset_on_high)
	{
		context->shift_data = 0;
		node->output[0] = 0;
		return;
	}

	/* increment clock */
	switch (context->clock_type)
	{
		case DISC_CLK_ON_F_EDGE:
		case DISC_CLK_ON_R_EDGE:
			/* See if the clock has toggled to the proper edge */
			clock = (clock != 0);
			if (context->last != clock)
			{
				context->last = clock;
				if (context->clock_type == clock)
				{
					/* Toggled */
					inc = 1;
				}
			}
			break;

		case DISC_CLK_BY_COUNT:
			/* Clock number of times specified. */
			inc = clock;
			break;
	}

	if (inc > 0)
	{
		if (context->shift_r)
		{
			context->shift_data >>= 1;
			context->shift_data |= input_bit << ((int)DST_LOGIC_SHIFT__SIZE - 1);
			inc--;
			context->shift_data >>= inc;
		}
		else
		{
			context->shift_data <<= 1;
			context->shift_data |= input_bit;
			inc--;
			context->shift_data <<= inc;
		}
		context->shift_data &= context->bit_mask;
	}

	node->output[0] = context->shift_data;
}

DISCRETE_RESET(dst_logic_shift)
{
	DISCRETE_DECLARE_CONTEXT(dst_shift)

	context->bit_mask = (1 << (int)DST_LOGIC_SHIFT__SIZE) - 1;
	context->clock_type = (int)DST_LOGIC_SHIFT__OPTIONS & DISC_CLK_MASK;
	context->reset_on_high = ((int)DST_LOGIC_SHIFT__OPTIONS & DISC_LOGIC_SHIFT__RESET_H) ? 1 : 0;
	context->shift_r = ((int)DST_LOGIC_SHIFT__OPTIONS & DISC_LOGIC_SHIFT__RIGHT)  ? 1 : 0;

	context->t_left  = 0;
	context->last = 0;
	context->shift_data   = 0;
	node->output[0]  = 0;
}

/************************************************************************
 *
 * DST_LOOKUP_TABLE  - Return value from lookup table
 *
 * input[0]    - Input 1
 * input[1]    - Table size
 *
 * Also passed address of the lookup table
 *
 * Feb 2007, D Renaud.
 ************************************************************************/
#define DST_LOOKUP_TABLE__IN		DISCRETE_INPUT(0)
#define DST_LOOKUP_TABLE__SIZE		DISCRETE_INPUT(1)

DISCRETE_STEP(dst_lookup_table)
{
	DISCRETE_DECLARE_INFO(double)

	int	addr = DST_LOOKUP_TABLE__IN;

	if (addr < 0 || addr >= DST_LOOKUP_TABLE__SIZE)
		node->output[0] = 0;
	else
		node->output[0] = info[addr];
}


/************************************************************************
 *
 * DST_MIXER  - Mixer/Gain stage
 *
 * input[0]    - Enable input value
 * input[1]    - Input 1
 * input[2]    - Input 2
 * input[3]    - Input 3
 * input[4]    - Input 4
 * input[5]    - Input 5
 * input[6]    - Input 6
 * input[7]    - Input 7
 * input[8]    - Input 8
 *
 * Also passed discrete_mixer_info structure
 *
 * Mar 2004, D Renaud.
 ************************************************************************/
/*
 * The input resistors can be a combination of static values and nodes.
 * If a node is used then its value is in series with the static value.
 * Also if a node is used and its value is 0, then that means the
 * input is disconnected from the circuit.
 *
 * There are 3 basic types of mixers, defined by the 2 types.  The
 * op amp mixer is further defined by the prescence of rI.  This is a
 * brief explaination.
 *
 * DISC_MIXER_IS_RESISTOR
 * The inputs are high pass filtered if needed, using (rX || rF) * cX.
 * Then Millman is used for the voltages.
 * r = (1/rF + 1/r1 + 1/r2...)
 * i = (v1/r1 + v2/r2...)
 * v = i * r
 *
 * DISC_MIXER_IS_OP_AMP - no rI
 * This is just a summing circuit.
 * The inputs are high pass filtered if needed, using rX * cX.
 * Then a modified Millman is used for the voltages.
 * i = ((vRef - v1)/r1 + (vRef - v2)/r2...)
 * v = i * rF
 *
 * DISC_MIXER_IS_OP_AMP_WITH_RI
 * The inputs are high pass filtered if needed, using (rX + rI) * cX.
 * Then Millman is used for the voltages including vRef/rI.
 * r = (1/rI + 1/r1 + 1/r2...)
 * i = (vRef/rI + v1/r1 + v2/r2...)
 * The voltage is then modified by an inverting amp formula.
 * v = vRef + (rF/rI) * (vRef - (i * r))
 */
#define DST_MIXER__ENABLE		DISCRETE_INPUT(0)
#define DST_MIXER__IN(bit)		DISCRETE_INPUT(bit + 1)

DISCRETE_STEP(dst_mixer)
{
	DISCRETE_DECLARE_CONTEXT(dst_mixer)
	DISCRETE_DECLARE_INFO(discrete_mixer_desc)

	double	v, vTemp, r_total, rTemp, rTemp2 = 0;
	double	i = 0;		/* total current of inputs */
	int		bit, connected;

	/* put commonly used stuff in local variables for speed */
	int		r_node_bit_flag = context->r_node_bit_flag;
	int		c_bit_flag = context->c_bit_flag;
	int		bit_mask = 1;
	int		has_rF = (info->rF != 0);
	int		type = context->type;
	double	v_ref = info->vRef;
	double	rI = info->rI;

	if (EXPECTED(DST_MIXER__ENABLE))
	{
		r_total = context->r_total;

		if (UNEXPECTED(context->r_node_bit_flag != 0))
		{
			/* loop and do any high pass filtering for connected caps */
			/* but first see if there is an r_node for the current path */
			/* if so, then the exponents need to be re-calculated */
			for (bit = 0; bit < context->size; bit++)
			{
				rTemp     = info->r[bit];
				connected = 1;
				vTemp     = DST_MIXER__IN(bit);

				/* is there a resistor? */
				if (r_node_bit_flag & bit_mask)
				{
					/* a node has the possibility of being disconnected from the circuit. */
					if (*context->r_node[bit] == 0)
						connected = 0;
					else
					{
						/* value currently holds resistance */
						rTemp   += *context->r_node[bit];
						r_total += 1.0 / rTemp;
						/* is there a capacitor? */
						if (c_bit_flag & bit_mask)
						{
							switch (type)
							{
								case DISC_MIXER_IS_RESISTOR:
									/* is there an rF? */
									if (has_rF)
									{
										rTemp2 = RES_2_PARALLEL(rTemp, info->rF);
										break;
									}
									/* else, fall through and just use the resistor value */
								case DISC_MIXER_IS_OP_AMP:
									rTemp2 = rTemp;
									break;
								case DISC_MIXER_IS_OP_AMP_WITH_RI:
									rTemp2 = rTemp + rI;
									break;
							}
							/* Re-calculate exponent if resistor is a node and has changed value */
							if (*context->r_node[bit] != context->r_last[bit])
							{
								context->exponent_rc[bit] =  RC_CHARGE_EXP(rTemp2 * info->c[bit]);
								context->r_last[bit] = *context->r_node[bit];
							}
						}
					}
				}

				if (connected)
				{
					/* is there a capacitor? */
					if (c_bit_flag & bit_mask)
					{
						/* do input high pass filtering if needed. */
						context->v_cap[bit] += (vTemp - v_ref - context->v_cap[bit]) * context->exponent_rc[bit];
						vTemp -= context->v_cap[bit];
					}
					i += ((type == DISC_MIXER_IS_OP_AMP) ? v_ref - vTemp : vTemp) / rTemp;
				}
			bit_mask = bit_mask << 1;
			}
		}
		else if (UNEXPECTED(c_bit_flag != 0))
		{
			/* no r_nodes, so just do high pass filtering */
			for (bit = 0; bit < context->size; bit++)
			{
				vTemp = DST_MIXER__IN(bit);

				if (c_bit_flag & (1 << bit))
				{
					/* do input high pass filtering if needed. */
					context->v_cap[bit] += (vTemp - v_ref - context->v_cap[bit]) * context->exponent_rc[bit];
					vTemp -= context->v_cap[bit];
				}
				i += ((type == DISC_MIXER_IS_OP_AMP) ? v_ref - vTemp : vTemp) / info->r[bit];
			}
		}
		else
		{
			/* no r_nodes or c_nodes, mixing only */
			if (UNEXPECTED(type == DISC_MIXER_IS_OP_AMP))
			{
				for (bit = 0; bit < context->size; bit++)
					i += ( v_ref - DST_MIXER__IN(bit) ) / info->r[bit];
			}
			else
			{
				for (bit = 0; bit < context->size; bit++)
					i += DST_MIXER__IN(bit) / info->r[bit];
			}
		}

		if (UNEXPECTED(type == DISC_MIXER_IS_OP_AMP_WITH_RI))
			i += v_ref / rI;

		r_total = 1.0 / r_total;

		/* If resistor network or has rI then Millman is used.
         * If op-amp then summing formula is used. */
		v = i * ((type == DISC_MIXER_IS_OP_AMP) ? info->rF : r_total);

		if (UNEXPECTED(type == DISC_MIXER_IS_OP_AMP_WITH_RI))
			v = v_ref + (context->gain * (v_ref - v));

		/* Do the low pass filtering for cF */
		if (EXPECTED(info->cF != 0))
		{
			if (UNEXPECTED(r_node_bit_flag != 0))
			{
				/* Re-calculate exponent if resistor nodes are used */
				context->exponent_c_f =  RC_CHARGE_EXP(r_total * info->cF);
			}
			context->v_cap_f += (v - v_ref - context->v_cap_f) * context->exponent_c_f;
			v = context->v_cap_f;
		}

		/* Do the high pass filtering for cAmp */
		if (EXPECTED(info->cAmp != 0))
		{
			context->v_cap_amp += (v - context->v_cap_amp) * context->exponent_c_amp;
			v -= context->v_cap_amp;
		}
		node->output[0] = v * info->gain;
	}
	else
	{
		node->output[0] = 0;
	}
}


DISCRETE_RESET(dst_mixer)
{
	DISCRETE_DECLARE_CONTEXT(dst_mixer)
	DISCRETE_DECLARE_INFO(discrete_mixer_desc)

	node_description *r_node;

	int		bit;
	double	rTemp = 0;

	/* link to r_node outputs */
	context->r_node_bit_flag = 0;
	for (bit = 0; bit < 8; bit++)
	{
		r_node = discrete_find_node(node->info, info->r_node[bit]);
		if (r_node != NULL)
		{
			context->r_node[bit] = &(r_node->output[NODE_CHILD_NODE_NUM(info->r_node[bit])]);
			context->r_node_bit_flag |= 1 << bit;
		}
		else
			context->r_node[bit] = NULL;

		/* flag any caps */
		if (info->c[bit] != 0)
			context->c_bit_flag |= 1 << bit;
	}

	context->size = node->active_inputs - 1;

	/*
     * THERE IS NO ERROR CHECKING!!!!!!!!!
     * If you pass a bad ladder table
     * then you deserve a crash.
     */

	context->type = info->type;
	if ((info->type == DISC_MIXER_IS_OP_AMP) && (info->rI != 0))
		context->type = DISC_MIXER_IS_OP_AMP_WITH_RI;

	/*
     * Calculate the total of all resistors in parallel.
     * This is the combined resistance of the voltage sources.
     * Also calculate the exponents while we are here.
     */
	context->r_total = 0;
	for(bit = 0; bit < context->size; bit++)
	{
		if ((info->r[bit] != 0) && !info->r_node[bit] )
		{
			context->r_total += 1.0 / info->r[bit];
		}

		context->v_cap[bit]       = 0;
		context->exponent_rc[bit] = 0;
		if ((info->c[bit] != 0)  && !info->r_node[bit])
		{
			switch (context->type)
			{
				case DISC_MIXER_IS_RESISTOR:
					/* is there an rF? */
					if (info->rF != 0)
					{
						rTemp = 1.0 / ((1.0 / info->r[bit]) + (1.0 / info->rF));
						break;
					}
					/* else, fall through and just use the resistor value */
				case DISC_MIXER_IS_OP_AMP:
					rTemp = info->r[bit];
					break;
				case DISC_MIXER_IS_OP_AMP_WITH_RI:
					rTemp = info->r[bit] + info->rI;
					break;
			}
			/* Setup filter constants */
			context->exponent_rc[bit] = RC_CHARGE_EXP(rTemp * info->c[bit]);
		}
	}

	if (info->rF != 0)
	{
		if (context->type == DISC_MIXER_IS_RESISTOR) context->r_total += 1.0 / info->rF;
	}
	if (context->type == DISC_MIXER_IS_OP_AMP_WITH_RI) context->r_total += 1.0 / info->rI;

	context->v_cap_f      = 0;
	context->exponent_c_f = 0;
	if (info->cF != 0)
	{
		/* Setup filter constants */
		context->exponent_c_f = RC_CHARGE_EXP(((info->type == DISC_MIXER_IS_OP_AMP) ? info->rF : (1.0 / context->r_total)) * info->cF);
	}

	context->v_cap_amp      = 0;
	context->exponent_c_amp = 0;
	if (info->cAmp != 0)
	{
		/* Setup filter constants */
		/* We will use 100k ohms as an average final stage impedance. */
		/* Your amp/speaker system will have more effect on incorrect filtering then any value used here. */
		context->exponent_c_amp = RC_CHARGE_EXP(RES_K(100) * info->cAmp);
	}

	if (context->type == DISC_MIXER_IS_OP_AMP_WITH_RI) context->gain = info->rF / info->rI;

	node->output[0] = 0;
}


/************************************************************************
 *
 * DST_MULTIPLEX - 1 of x multiplexer/switch
 *
 * input[0]    - switch position
 * input[1]    - input[0]
 * input[2]    - input[1]
 * .....
 *
 * Dec 2004, D Renaud.
 ************************************************************************/
#define DST_MULTIPLEX__ADDR			DISCRETE_INPUT(0)
#define DST_MULTIPLEX__INP(addr)	DISCRETE_INPUT(1 + addr)

DISCRETE_STEP(dst_multiplex)
{
	DISCRETE_DECLARE_CONTEXT(dst_size)

	int addr;

	addr = DST_MULTIPLEX__ADDR;	/* FP to INT */
	if ((addr >= 0) && (addr < context->size))
	{
		node->output[0] = DST_MULTIPLEX__INP(addr);
	}
	else
	{
		/* Bad address.  We will leave the output alone. */
		discrete_log(node->info, "NODE_%02d - Address = %d. Out of bounds\n", NODE_BLOCKINDEX(node), addr);
	}
}

DISCRETE_RESET(dst_multiplex)
{
	DISCRETE_DECLARE_CONTEXT(dst_size)

	context->size = node->active_inputs - 1;

	DISCRETE_STEP_CALL(dst_multiplex);
}


/************************************************************************
 *
 * DST_ONESHOT - Usage of node_description values for one shot pulse
 *
 * input[0]    - Reset value
 * input[1]    - Trigger value
 * input[2]    - Amplitude value
 * input[3]    - Width of oneshot pulse
 * input[4]    - type R/F edge, Retriggerable?
 *
 * Complete re-write Jan 2004, D Renaud.
 ************************************************************************/
#define DST_ONESHOT__RESET	DISCRETE_INPUT(0)
#define DST_ONESHOT__TRIG	DISCRETE_INPUT(1)
#define DST_ONESHOT__AMP	DISCRETE_INPUT(2)
#define DST_ONESHOT__WIDTH	DISCRETE_INPUT(3)
#define DST_ONESHOT__TYPE	(int)DISCRETE_INPUT(4)

DISCRETE_STEP(dst_oneshot)
{
	DISCRETE_DECLARE_CONTEXT(dst_oneshot)

	int trigger = (DST_ONESHOT__TRIG != 0);

	/* If the state is triggered we will need to countdown later */
	int do_count = context->state;

	if (UNEXPECTED(DST_ONESHOT__RESET))
	{
		/* Hold in Reset */
		node->output[0] = 0;
		context->state  = 0;
	}
	else
	{
		/* are we at an edge? */
		if (UNEXPECTED(trigger != context->last_trig))
		{
			/* There has been a trigger edge */
			context->last_trig = trigger;

			/* Is it the proper edge trigger */
			if ((context->type & DISC_ONESHOT_REDGE) ? trigger : !trigger)
			{
				if (!context->state)
				{
					/* We have first trigger */
					context->state     = 1;
					node->output[0]    = (context->type & DISC_OUT_ACTIVE_LOW) ? 0 : DST_ONESHOT__AMP;
					context->countdown = DST_ONESHOT__WIDTH;
				}
				else
				{
					/* See if we retrigger */
					if (context->type & DISC_ONESHOT_RETRIG)
					{
						/* Retrigger */
						context->countdown = DST_ONESHOT__WIDTH;
						do_count = 0;
					}
				}
			}
		}

		if (UNEXPECTED(do_count))
		{
			context->countdown -= node->info->sample_time;
			if(context->countdown <= 0.0)
			{
				node->output[0]    = (context->type & DISC_OUT_ACTIVE_LOW) ? DST_ONESHOT__AMP : 0;
				context->countdown = 0;
				context->state     = 0;
			}
		}
	}
}


DISCRETE_RESET(dst_oneshot)
{
	DISCRETE_DECLARE_CONTEXT(dst_oneshot)

	context->countdown = 0;
	context->state     = 0;

	context->last_trig = 0;
	context->type = DST_ONESHOT__TYPE;

	node->output[0] = (context->type & DISC_OUT_ACTIVE_LOW) ? DST_ONESHOT__AMP : 0;
}


/************************************************************************
 *
 * DST_RAMP - Ramp up/down model usage
 *
 * input[0]    - Enable ramp
 * input[1]    - Ramp Reverse/Forward switch
 * input[2]    - Gradient, change/sec
 * input[3]    - Start value
 * input[4]    - End value
 * input[5]    - Clamp value when disabled
 *
 ************************************************************************/
#define DST_RAMP__ENABLE	DISCRETE_INPUT(0)
#define DST_RAMP__DIR		DISCRETE_INPUT(1)
#define DST_RAMP__GRAD		DISCRETE_INPUT(2)
#define DST_RAMP__START		DISCRETE_INPUT(3)
#define DST_RAMP__END		DISCRETE_INPUT(4)
#define DST_RAMP__CLAMP		DISCRETE_INPUT(5)

DISCRETE_STEP(dst_ramp)
{
	DISCRETE_DECLARE_CONTEXT(dst_ramp)

	if(DST_RAMP__ENABLE)
	{
		if (!context->last_en)
		{
			context->last_en = 1;
			node->output[0]  = DST_RAMP__START;
		}
		if(context->dir ? DST_RAMP__DIR : !DST_RAMP__DIR) node->output[0]+=context->step;
		else node->output[0] -= context->step;
		/* Clamp to min/max */
		if(context->dir ? (node->output[0] < DST_RAMP__START)
				: (node->output[0] > DST_RAMP__START)) node->output[0] = DST_RAMP__START;
		if(context->dir ? (node->output[0] > DST_RAMP__END)
				: (node->output[0] < DST_RAMP__END)) node->output[0] = DST_RAMP__END;
	}
	else
	{
		context->last_en = 0;
		/* Disabled so clamp to output */
		node->output[0] = DST_RAMP__CLAMP;
	}
}

DISCRETE_RESET(dst_ramp)
{
	DISCRETE_DECLARE_CONTEXT(dst_ramp)

	node->output[0]  = DST_RAMP__CLAMP;
	context->step    = DST_RAMP__GRAD / node->info->sample_rate;
	context->dir     = ((DST_RAMP__END - DST_RAMP__START) == abs(DST_RAMP__END - DST_RAMP__START));
	context->last_en = 0;
}


/************************************************************************
 *
 * DST_SAMPHOLD - Sample & Hold Implementation
 *
 * input[0]    - input[0] value
 * input[1]    - clock node
 * input[2]    - clock type
 *
 ************************************************************************/
#define DST_SAMPHOLD__IN0		DISCRETE_INPUT(0)
#define DST_SAMPHOLD__CLOCK		DISCRETE_INPUT(1)
#define DST_SAMPHOLD__TYPE		DISCRETE_INPUT(2)

DISCRETE_STEP(dst_samphold)
{
	DISCRETE_DECLARE_CONTEXT(dst_samphold)

	switch(context->clocktype)
	{
		case DISC_SAMPHOLD_REDGE:
			/* Clock the whole time the input is rising */
			if (DST_SAMPHOLD__CLOCK > context->last_input) node->output[0] = DST_SAMPHOLD__IN0;
			break;
		case DISC_SAMPHOLD_FEDGE:
			/* Clock the whole time the input is falling */
			if(DST_SAMPHOLD__CLOCK < context->last_input) node->output[0] = DST_SAMPHOLD__IN0;
			break;
		case DISC_SAMPHOLD_HLATCH:
			/* Output follows input if clock != 0 */
			if( DST_SAMPHOLD__CLOCK) node->output[0] = DST_SAMPHOLD__IN0;
			break;
		case DISC_SAMPHOLD_LLATCH:
			/* Output follows input if clock == 0 */
			if (DST_SAMPHOLD__CLOCK == 0) node->output[0] = DST_SAMPHOLD__IN0;
			break;
		default:
			discrete_log(node->info, "dst_samphold_step - Invalid clocktype passed");
			break;
	}
	/* Save the last value */
	context->last_input = DST_SAMPHOLD__CLOCK;
}

DISCRETE_RESET(dst_samphold)
{
	DISCRETE_DECLARE_CONTEXT(dst_samphold)

	node->output[0]     =  0;
	context->last_input = -1;
	/* Only stored in here to speed up and save casting in the step function */
	context->clocktype = (int)DST_SAMPHOLD__TYPE;
	DISCRETE_STEP_CALL(dst_samphold);
}


/************************************************************************
 *
 * DST_SWITCH - Programmable 2 pole switch module with enable function
 *
 * input[0]    - Enable input value
 * input[1]    - switch position
 * input[2]    - input[0]
 * input[3]    - input[1]
 *
 ************************************************************************/
#define DST_SWITCH__ENABLE	DISCRETE_INPUT(0)
#define DST_SWITCH__SWITCH	DISCRETE_INPUT(1)
#define DST_SWITCH__IN0		DISCRETE_INPUT(2)
#define DST_SWITCH__IN1		DISCRETE_INPUT(3)

DISCRETE_STEP(dst_switch)
{
	if(DST_SWITCH__ENABLE)
	{
		node->output[0] = DST_SWITCH__SWITCH ? DST_SWITCH__IN1 : DST_SWITCH__IN0;
	}
	else
	{
		node->output[0] = 0;
	}
}

/************************************************************************
 *
 * DST_ASWITCH - Analog switch
 *
 * input[1]    - Control
 * input[2]    - Input
 * input[3]    - Threshold for enable
 *
 ************************************************************************/
#define DST_ASWITCH__CTRL		DISCRETE_INPUT(0)
#define DST_ASWITCH__IN			DISCRETE_INPUT(1)
#define DST_ASWITCH__THRESHOLD	DISCRETE_INPUT(2)


DISCRETE_STEP(dst_aswitch)
{
	node->output[0] = DST_ASWITCH__CTRL > DST_ASWITCH__THRESHOLD ? DST_ASWITCH__IN : 0;
}

/************************************************************************
 *
 * DST_TRANSFORM - Programmable math module
 *
 * input[0]    - Channel0 input value
 * input[1]    - Channel1 input value
 * input[2]    - Channel2 input value
 * input[3]    - Channel3 input value
 * input[4]    - Channel4 input value
 *
 ************************************************************************/
#define DST_TRANSFORM__IN0		DISCRETE_INPUT(0)
#define DST_TRANSFORM__IN1		DISCRETE_INPUT(1)
#define DST_TRANSFORM__IN2		DISCRETE_INPUT(2)
#define DST_TRANSFORM__IN3		DISCRETE_INPUT(3)
#define DST_TRANSFORM__IN4		DISCRETE_INPUT(4)

#define MAX_TRANS_STACK	16

INLINE double dst_transform_pop(double *stack, int *pointer)
{
	//decrement THEN read
	assert(*pointer > 0);
	(*pointer)--;
	return stack[*pointer];
}

INLINE void dst_transform_push(double *stack, int *pointer, double value)
{
	//Store THEN increment
	assert(*pointer < MAX_TRANS_STACK);
	stack[(*pointer)++] = value;
}

DISCRETE_STEP(dst_transform)
{
	double	trans_stack[MAX_TRANS_STACK];
	double	number1,top;
	int		trans_stack_ptr = 0;

	const char *fPTR = (const char *)node->custom;

	top = HUGE_VAL;

	while(*fPTR != 0)
	{
		switch (*fPTR++)
		{
			case '*':
				number1 = dst_transform_pop(trans_stack, &trans_stack_ptr);
				top = number1 * top;
				break;
			case '/':
				number1 = dst_transform_pop(trans_stack, &trans_stack_ptr);
				top = number1 / top;
				break;
			case '+':
				number1=dst_transform_pop(trans_stack, &trans_stack_ptr);
				top = number1 + top;
				break;
			case '-':
				number1 = dst_transform_pop(trans_stack, &trans_stack_ptr);
				top = number1 - top;
				break;
			case '0':
				dst_transform_push(trans_stack, &trans_stack_ptr, top);
				top = DST_TRANSFORM__IN0;
				break;
			case '1':
				dst_transform_push(trans_stack, &trans_stack_ptr, top);
				top = DST_TRANSFORM__IN1;
				break;
			case '2':
				dst_transform_push(trans_stack, &trans_stack_ptr, top);
				top = DST_TRANSFORM__IN2;
				break;
			case '3':
				dst_transform_push(trans_stack, &trans_stack_ptr, top);
				top = DST_TRANSFORM__IN3;
				break;
			case '4':
				dst_transform_push(trans_stack, &trans_stack_ptr, top);
				top = DST_TRANSFORM__IN4;
				break;
			case 'P':
				dst_transform_push(trans_stack, &trans_stack_ptr, top);
				break;
			case 'a':	/* absolute value */
				top = fabs(top);
				break;
			case 'i':	/* * -1 */
				top = -top;
				break;
			case '!':	/* Logical NOT of Last Value */
				top = !top;
				break;
			case '=':	/* Logical = */
				number1 = dst_transform_pop(trans_stack, &trans_stack_ptr);
				top = (int)number1 == (int)top;
				break;
			case '>':	/* Logical > */
				number1 = dst_transform_pop(trans_stack, &trans_stack_ptr);
				top = number1 > top;
				break;
			case '<':	/* Logical < */
				number1 = dst_transform_pop(trans_stack, &trans_stack_ptr);
				top = number1 < top;
				break;
			case '&':	/* Bitwise AND */
				number1 = dst_transform_pop(trans_stack, &trans_stack_ptr);
				top = (int)number1 & (int)top;
				break;
			case '|':	/* Bitwise OR */
				number1 = dst_transform_pop(trans_stack, &trans_stack_ptr);
				top = (int)number1 | (int)top;
				break;
			case '^':	/* Bitwise XOR */
				number1 = dst_transform_pop(trans_stack, &trans_stack_ptr);
				top = (int)number1 ^ (int)top;
				break;
			default:
				discrete_log(node->info, "dst_transform_step - Invalid function type/variable passed: %s",(const char *)node->custom);
				/* that is enough to fatalerror */
				fatalerror("dst_transform_step - Invalid function type/variable passed: %s", (const char *)node->custom);
				break;
		}
	}
	node->output[0] = top;
}


/************************************************************************
 *
 * DST_OP_AMP - op amp circuits
 *
 * input[0] - Enable
 * input[1] - Input 0
 * input[2] - Input 1
 *
 * also passed discrete_op_amp_info structure
 *
 * Mar 2007, D Renaud.
 ************************************************************************/
#define DST_OP_AMP__ENABLE	DISCRETE_INPUT(0)
#define DST_OP_AMP__INP0	DISCRETE_INPUT(1)
#define DST_OP_AMP__INP1	DISCRETE_INPUT(2)

DISCRETE_STEP(dst_op_amp)
{
	DISCRETE_DECLARE_CONTEXT(dst_op_amp)
	DISCRETE_DECLARE_INFO(discrete_op_amp_info)

	double i_pos = 0;
	double i_neg = 0;
	double i    = 0;

	if (DST_OP_AMP__ENABLE)
	{
		switch (info->type)
		{
			case DISC_OP_AMP_IS_NORTON:
				/* work out neg pin current */
				if  (context->has_r1)
				{
					i_neg = (DST_OP_AMP__INP0 - OP_AMP_NORTON_VBE) / info->r1;
					if (i_neg < 0) i_neg = 0;
				}
				i_neg += context->i_fixed;

				/* work out neg pin current */
				i_pos = (DST_OP_AMP__INP1 - OP_AMP_NORTON_VBE) / info->r2;
				if (i_pos < 0) i_pos = 0;

				/* work out current across r4 */
				i = i_pos - i_neg;

				if (context->has_cap)
				{
					if (context->has_r4)
					{
						/* voltage across r4 charging cap */
						i *= info->r4;
						/* exponential charge */
						context->v_cap += (i - context->v_cap) * context->exponent;
					}
					else
						/* linear charge */
						context->v_cap += i / context->exponent;
					node->output[0] = context->v_cap;
				}
				else
					if (context->has_r4)
						node->output[0] = i * info->r4;
					else
						/* output just swings to rail when there is no r4 */
						if (i > 0)
							node->output[0] = context->v_max;
						else
							node->output[0] = 0;

				/* clamp output */
				if (node->output[0] > context->v_max) node->output[0] = context->v_max;
				else if (node->output[0] < info->vN) node->output[0] = info->vN;
				context->v_cap = node->output[0];
				break;

			default:
				node->output[0] = 0;
		}
	}
	else
		node->output[0] = 0;
}

DISCRETE_RESET(dst_op_amp)
{
	DISCRETE_DECLARE_CONTEXT(dst_op_amp)
	DISCRETE_DECLARE_INFO(discrete_op_amp_info)

	context->has_r1 = info->r1 > 0;
	context->has_r4 = info->r4 > 0;

	context->v_max = info->vP - OP_AMP_NORTON_VBE;

	context->v_cap = 0;
	if (info->c > 0)
	{
		context->has_cap = 1;
		/* Setup filter constants */
		if (context->has_r4)
		{
			/* exponential charge */
			context->exponent = RC_CHARGE_EXP(info->r4 * info->c);
		}
		else
			/* linear charge */
			context->exponent = node->info->sample_rate * info->c;
	}

	if (info->r3 > 0)
		context->i_fixed = (info->vP - OP_AMP_NORTON_VBE) / info->r3;
	else
		context->i_fixed = 0;
}


/************************************************************************
 *
 * DST_OP_AMP_1SHT - op amp one shot circuits
 *
 * input[0] - Trigger
 *
 * also passed discrete_op_amp_1sht_info structure
 *
 * Mar 2007, D Renaud.
 ************************************************************************/
#define DST_OP_AMP_1SHT__TRIGGER	DISCRETE_INPUT(0)

DISCRETE_STEP(dst_op_amp_1sht)
{
	DISCRETE_DECLARE_CONTEXT(dst_op_amp_1sht)
	DISCRETE_DECLARE_INFO(discrete_op_amp_1sht_info)

	double i_pos;
	double i_neg;
	double v;

	/* update trigger circuit */
	i_pos  = (DST_OP_AMP_1SHT__TRIGGER - context->v_cap2) / info->r2;
	i_pos += node->output[0] / info->r5;
	context->v_cap2 += (DST_OP_AMP_1SHT__TRIGGER - context->v_cap2) * context->exponent2;

	/* calculate currents and output */
	i_neg = (context->v_cap1 - OP_AMP_NORTON_VBE) / info->r3;
	if (i_neg < 0) i_neg = 0;
	i_neg += context->i_fixed;

	if (i_pos > i_neg) node->output[0] = context->v_max;
	else node->output[0] = info->vN;

	/* update c1 */
	/* rough value of voltage at anode of diode if discharging */
	v = node->output[0] + 0.6;
	if (context->v_cap1 > node->output[0])
	{
		/* discharge */
		if (context->v_cap1 > v)
			/* immediate discharge through diode */
			context->v_cap1 = v;
		else
			/* discharge through r4 */
			context->v_cap1 += (node->output[0] - context->v_cap1) * context->exponent1d;
	}
	else
		/* charge */
		context->v_cap1 += ((node->output[0] - OP_AMP_NORTON_VBE) * context->r34ratio + OP_AMP_NORTON_VBE - context->v_cap1) * context->exponent1c;
}

DISCRETE_RESET(dst_op_amp_1sht)
{
	DISCRETE_DECLARE_CONTEXT(dst_op_amp_1sht)
	DISCRETE_DECLARE_INFO(discrete_op_amp_1sht_info)

	context->exponent1c = RC_CHARGE_EXP(RES_2_PARALLEL(info->r3, info->r4) * info->c1);
	context->exponent1d = RC_CHARGE_EXP(info->r4 * info->c1);
	context->exponent2  = RC_CHARGE_EXP(info->r2 * info->c2);
	context->i_fixed  = (info->vP - OP_AMP_NORTON_VBE) / info->r1;
	context->v_cap1   = context->v_cap2 = 0;
	context->v_max    = info->vP - OP_AMP_NORTON_VBE;
	context->r34ratio = info->r3 / (info->r3 + info->r4);
}


/************************************************************************
 *
 * DST_TVCA_OP_AMP - trigged op-amp VCA
 *
 * input[0] - Trigger 0
 * input[1] - Trigger 1
 * input[2] - Trigger 2
 * input[3] - Input 0
 * input[4] - Input 1
 *
 * also passed discrete_op_amp_tvca_info structure
 *
 * Mar 2004, D Renaud.
 ************************************************************************/
#define DST_TVCA_OP_AMP__TRG0	DISCRETE_INPUT(0)
#define DST_TVCA_OP_AMP__TRG1	DISCRETE_INPUT(1)
#define DST_TVCA_OP_AMP__TRG2	DISCRETE_INPUT(2)
#define DST_TVCA_OP_AMP__INP0	DISCRETE_INPUT(3)
#define DST_TVCA_OP_AMP__INP1	DISCRETE_INPUT(4)

DISCRETE_STEP(dst_tvca_op_amp)
{
	DISCRETE_DECLARE_CONTEXT(dst_tvca_op_amp)
	DISCRETE_DECLARE_INFO(discrete_op_amp_tvca_info)

	int		trig0, trig1, trig2, f3;
	double	i2 = 0;		/* current through r2 */
	double	i3 = 0;		/* current through r3 */
	double	i_neg = 0;	/* current into - input */
	double	i_pos = 0;	/* current into + input */
	double	i_out = 0;	/* current at output */

	trig0 = (int)DST_TVCA_OP_AMP__TRG0;
	trig1 = (int)DST_TVCA_OP_AMP__TRG1;
	trig2 = (int)DST_TVCA_OP_AMP__TRG2;
	f3 = dst_trigger_function(trig0, trig1, trig2, info->f3);

	if ((info->r2 != 0) && dst_trigger_function(trig0, trig1, trig2, info->f0))
		{
			/* r2 is present, so we assume Input 0 is connected and valid. */
			i2 = (DST_TVCA_OP_AMP__INP0 - OP_AMP_NORTON_VBE) / info->r2;
			if ( i2 < 0) i2 = 0;
		}

	if ((info->r3 != 0) && dst_trigger_function(trig0, trig1, trig2, info->f1))
		{
			/* r2 is present, so we assume Input 1 is connected and valid. */
			/* Function F1 is not grounding the circuit. */
			i3 = (DST_TVCA_OP_AMP__INP1 - OP_AMP_NORTON_VBE) / info->r3;
			if ( i3 < 0) i3 = 0;
		}

	/* Calculate current going in to - input. */
	i_neg = context->i_fixed + i2 + i3;

	/* Update the c1 cap voltage. */
	if (dst_trigger_function(trig0, trig1, trig2, info->f2))
	{
		/* F2 is not grounding the circuit so we charge the cap. */
		context->v_cap1 += (context->v_trig[f3] - context->v_cap1) * context->exponent_c[f3];
	}
	else
	{
		/* F2 is at ground.  The diode blocks this so F2 and r5 are out of circuit.
         * So now the discharge rate is dependent upon F3.
         * If F3 is at ground then we discharge to 0V through r6.
         * If F3 is out of circuit then we discharge to OP_AMP_NORTON_VBE through r6+r7. */
		context->v_cap1 += ((f3 ? OP_AMP_NORTON_VBE : 0.0) - context->v_cap1) * context->exponent_d[f3];
	}

	/* Calculate c1 current going in to + input. */
	i_pos = (context->v_cap1 - OP_AMP_NORTON_VBE) / context->r67;
	if ((i_pos < 0) || !f3) i_pos = 0;

	/* Update the c2 cap voltage and current. */
	if (info->r9 != 0)
	{
		f3 = dst_trigger_function(trig0, trig1, trig2, info->f4);
		context->v_cap2 += ((f3 ? context->v_trig2 : 0) - context->v_cap2) * context->exponent2[f3];
		i_pos += context->v_cap2 / info->r9;
	}

	/* Update the c3 cap voltage and current. */
	if (info->r11 != 0)
	{
		f3 = dst_trigger_function(trig0, trig1, trig2, info->f5);
		context->v_cap3 += ((f3 ? context->v_trig3 : 0) - context->v_cap3) * context->exponent3[f3];
		i_pos += context->v_cap3 / info->r11;
	}

	/* Calculate output current. */
	i_out = i_pos - i_neg;
	if (i_out < 0) i_out = 0;

	/* Convert to voltage for final output. */
	if (context->has_c4)
	{
		if (context->has_r4)
		{
			/* voltage across r4 charging cap */
			i_out *= info->r4;
			/* exponential charge */
			context->v_cap4 += (i_out - context->v_cap4) * context->exponent4;
		}
		else
		/* linear charge */
			context->v_cap4 += i_out / context->exponent4;
		if (context->v_cap4 < 0)
			context->v_cap4 = 0;
		node->output[0] = context->v_cap4;
	}
	else
		node->output[0] = i_out * info->r4;



	/* Clip the output if needed. */
	if (node->output[0] > context->v_out_max) node->output[0] = context->v_out_max;
}

DISCRETE_RESET(dst_tvca_op_amp)
{
	DISCRETE_DECLARE_CONTEXT(dst_tvca_op_amp)
	DISCRETE_DECLARE_INFO(discrete_op_amp_tvca_info)

	context->r67 = info->r6 + info->r7;

	context->v_out_max = info->vP - OP_AMP_NORTON_VBE;
	/* This is probably overkill because R5 is usually much lower then r6 or r7,
     * but it is better to play it safe. */
	context->v_trig[0] = (info->v1 - 0.6) * RES_VOLTAGE_DIVIDER(info->r5, info->r6);
	context->v_trig[1] = (info->v1 - 0.6 - OP_AMP_NORTON_VBE) * RES_VOLTAGE_DIVIDER(info->r5, context->r67) + OP_AMP_NORTON_VBE;
	context->i_fixed   = context->v_out_max / info->r1;

	context->v_cap1 = 0;
	/* Charge rate thru r5 */
	/* There can be a different charge rates depending on function F3. */
	context->exponent_c[0] = RC_CHARGE_EXP(RES_2_PARALLEL(info->r5, info->r6) * info->c1);
	context->exponent_c[1] = RC_CHARGE_EXP(RES_2_PARALLEL(info->r5, context->r67) * info->c1);
	/* Discharge rate thru r6 + r7 */
	context->exponent_d[1] = RC_CHARGE_EXP(context->r67 * info->c1);
	/* Discharge rate thru r6 */
	if (info->r6 != 0)
	{
		context->exponent_d[0] = RC_CHARGE_EXP(info->r6 * info->c1);
	}
	context->v_cap2       = 0;
	context->v_trig2      = (info->v2 - 0.6 - OP_AMP_NORTON_VBE) * RES_VOLTAGE_DIVIDER(info->r8, info->r9);
	context->exponent2[0] = RC_CHARGE_EXP(info->r9 * info->c2);
	context->exponent2[1] = RC_CHARGE_EXP(RES_2_PARALLEL(info->r8, info->r9) * info->c2);
	context->v_cap3       = 0;
	context->v_trig3      = (info->v3 - 0.6 - OP_AMP_NORTON_VBE) * RES_VOLTAGE_DIVIDER(info->r10, info->r11);
	context->exponent3[0] = RC_CHARGE_EXP(info->r11 * info->c3);
	context->exponent3[1] = RC_CHARGE_EXP(RES_2_PARALLEL(info->r10, info->r11) * info->c3);
	context->v_cap4       = 0;
	if (info->r4 != 0) context->has_r4 = 1;
	if (info->c4 != 0) context->has_c4 = 1;
	if (context->has_r4 && context->has_c4)
		context->exponent4    = RC_CHARGE_EXP(info->r4 * info->c4);

	DISCRETE_STEP_CALL(dst_tvca_op_amp);
}


/* the different logic and xtime states */
enum
{
	XTIME__IN0_0__IN1_0__IN0_NOX__IN1_NOX = 0,
	XTIME__IN0_0__IN1_0__IN0_NOX__IN1_X,
	XTIME__IN0_0__IN1_0__IN0_X__IN1_NOX,
	XTIME__IN0_0__IN1_0__IN0_X__IN1_X,
	XTIME__IN0_0__IN1_1__IN0_NOX__IN1_NOX,
	XTIME__IN0_0__IN1_1__IN0_NOX__IN1_X,
	XTIME__IN0_0__IN1_1__IN0_X__IN1_NOX,
	XTIME__IN0_0__IN1_1__IN0_X__IN1_X,
	XTIME__IN0_1__IN1_0__IN0_NOX__IN1_NOX,
	XTIME__IN0_1__IN1_0__IN0_NOX__IN1_X,
	XTIME__IN0_1__IN1_0__IN0_X__IN1_NOX,
	XTIME__IN0_1__IN1_0__IN0_X__IN1_X,
	XTIME__IN0_1__IN1_1__IN0_NOX__IN1_NOX,
	XTIME__IN0_1__IN1_1__IN0_NOX__IN1_X,
	XTIME__IN0_1__IN1_1__IN0_X__IN1_NOX,
	XTIME__IN0_1__IN1_1__IN0_X__IN1_X
};


/************************************************************************
 *
 * DST_XTIME_BUFFER - Buffer/Invertor gate implementation using X_TIME
 *
 * If OUT_LOW and OUT_HIGH are defined then the output will be energy.
 * If they are both 0, then the output will be X_TIME logic.
 *
 ************************************************************************/
#define DST_XTIME_BUFFER__IN			DISCRETE_INPUT(0)
#define DST_XTIME_BUFFER_OUT_LOW		DISCRETE_INPUT(1)
#define DST_XTIME_BUFFER_OUT_HIGH		DISCRETE_INPUT(2)
#define DST_XTIME_BUFFER_INVERT			DISCRETE_INPUT(3)

DISCRETE_STEP(dst_xtime_buffer)
{
	int	in0 = (int)DST_XTIME_BUFFER__IN;
	int	out = in0;
	int out_is_energy = 1;

	double x_time = DST_XTIME_BUFFER__IN - in0;

	double out_low = DST_XTIME_BUFFER_OUT_LOW;
	double out_high = DST_XTIME_BUFFER_OUT_HIGH;

	if (out_low ==0 && out_high == 0)
		out_is_energy = 0;

	if (DST_XTIME_BUFFER_INVERT != 0)
		out ^= 1;

	if (out_is_energy)
	{
		if (x_time > 0)
		{
			double diff = out_high - out_low;
			diff = out ? diff * x_time : diff * (1.0 - x_time);
			node->output[0] = out_low + diff;
		}
		else
			node->output[0] = out ? out_high : out_low;
	}
	else
		node->output[0] = out + x_time;
}


/************************************************************************
 *
 * DST_XTIME_AND - AND/NAND gate implementation using X_TIME
 *
 * If OUT_LOW and OUT_HIGH are defined then the output will be energy.
 * If they are both 0, then the output will be X_TIME logic.
 *
 ************************************************************************/
#define DST_XTIME_AND__IN0			DISCRETE_INPUT(0)
#define DST_XTIME_AND__IN1			DISCRETE_INPUT(1)
#define DST_XTIME_AND_OUT_LOW		DISCRETE_INPUT(2)
#define DST_XTIME_AND_OUT_HIGH		DISCRETE_INPUT(3)
#define DST_XTIME_AND_INVERT		DISCRETE_INPUT(4)

DISCRETE_STEP(dst_xtime_and)
{
	int	in0 = (int)DST_XTIME_AND__IN0;
	int	in1 = (int)DST_XTIME_AND__IN1;
	int	out = 0;
	int out_is_energy = 1;

	double x_time = 0;
	double x_time0 = DST_XTIME_AND__IN0 - in0;
	double x_time1 = DST_XTIME_AND__IN1 - in1;

	int in0_has_xtime = x_time0 > 0 ? 1 : 0;
	int in1_has_xtime = x_time1 > 0 ? 1 : 0;

	double out_low = DST_XTIME_AND_OUT_LOW;
	double out_high = DST_XTIME_AND_OUT_HIGH;

	if (out_low ==0 && out_high == 0)
		out_is_energy = 0;

	switch ((in0 << 3) | (in1 << 2) | (in0_has_xtime < 1) | in1_has_xtime)
	{
		// these are all 0
		//case XTIME__IN0_0__IN1_0__IN0_NOX__IN1_NOX:
		//case XTIME__IN0_0__IN1_1__IN0_NOX__IN1_NOX:
		//case XTIME__IN0_1__IN1_0__IN0_NOX__IN1_NOX:
		//case XTIME__IN0_0__IN1_0__IN0_NOX__IN1_X:
		//case XTIME__IN0_0__IN1_0__IN0_X__IN1_NOX:
		//case XTIME__IN0_0__IN1_1__IN0_NOX__IN1_X:
		//case XTIME__IN0_1__IN1_0__IN0_X__IN1_NOX:
		//	break;

		case XTIME__IN0_1__IN1_1__IN0_NOX__IN1_NOX:
			out = 1;
			break;

		case XTIME__IN0_0__IN1_1__IN0_X__IN1_NOX:
			/*
			 * in0  1   ------
			 *      0         -------
			 *          ...^....^...
			 *
			 * in1  1   -------------
			 *      0
			 *          ...^....^...
			 *
			 * out  1   ------
			 *      0         ------
			 *          ...^....^...
			 */
			x_time = x_time0;
			break;

		case XTIME__IN0_1__IN1_0__IN0_NOX__IN1_X:
			/*
			 * in0  1   -------------
			 *      0
			 *          ...^....^...
			 *
			 * in1  1   ------
			 *      0         -------
			 *          ...^....^...
			 *
			 * out  1   ------
			 *      0         ------
			 *          ...^....^...
			 */
			x_time = x_time1;
			break;

		case XTIME__IN0_0__IN1_0__IN0_X__IN1_X:
			/*
			 * in0  1   -----              -------
			 *      0        --------             ------
			 *          ...^....^...       ...^....^...
			 *
			 * in1  1   -------            -----
			 *      0          ------           --------
			 *          ...^....^...       ...^....^...
			 *
			 * out  1   -----              -----
			 *      0        -------            -------
			 *          ...^....^...       ...^....^...
			 */
			// use x_time of input that went to 0 first/longer
			if (x_time0 >= x_time1)
				x_time = x_time0;
			else
				x_time = x_time1;
			break;

		case XTIME__IN0_0__IN1_1__IN0_X__IN1_X:
			/*
			 * in0  1   -------           -----
			 *      0          -----           -------
			 *          ...^....^...      ...^....^...
			 *
			 * in1  1        -------             -----
			 *      0   -----             -------
			 *          ...^....^...      ...^....^...
			 *
			 * out  1        --
			 *      0   -----  -----      ------------
			 *          ...^....^...      ...^....^...
			 */
			// may have went high for a bit in this cycle
			//if (x_time0 < x_time1)
			//	x_time = time1 - x_time0;
			break;

		case XTIME__IN0_1__IN1_0__IN0_X__IN1_X:
			/*
			 * in0  1        -------             -----
			 *      0   -----             -------
			 *          ...^....^...      ...^....^...
			 *
			 * in1  1   -------           -----
			 *      0          -----           -------
			 *          ...^....^...      ...^....^...
			 *
			 * out  1        --
			 *      0   -----  -----      ------------
			 *          ...^....^...      ...^....^...
			 */
			// may have went high for a bit in this cycle
			//if (x_time0 > x_time1)
			//	x_time = x_time0 - x_time1;
			break;

		case XTIME__IN0_1__IN1_1__IN0_NOX__IN1_X:
			/*
			 * in0  1   ------------
			 *      0
			 *          ...^....^...
			 *
			 * in1  1         ------
			 *      0   ------
			 *          ...^....^...
			 *
			 * out  1         ------
			 *      0   ------
			 *          ...^....^...
			 */
			out = 1;
			x_time = x_time1;
			break;

		case XTIME__IN0_1__IN1_1__IN0_X__IN1_NOX:
			/*
			 * in1  0         ------
			 *      0   ------
			 *          ...^....^...
			 *
			 * in1  1   ------------
			 *      0
			 *          ...^....^...
			 *
			 * out  1         ------
			 *      0   ------
			 *          ...^....^...
			 */
			out = 1;
			x_time = x_time0;
			break;

		case XTIME__IN0_1__IN1_1__IN0_X__IN1_X:
			/*
			 * in0  1         ------          --------
			 *      0   ------            ----
			 *          ...^....^...      ...^....^...
			 *
			 * in1  1       --------            ------
			 *      0   ----              ------
			 *          ...^....^...      ...^....^...
			 *
			 * out  1         ------            ------
			 *      0   ------            ------
			 *          ...^....^...      ...^....^...
			 */
			out = 1;
			if (x_time0 < x_time1)
				x_time = x_time0;
			else
				x_time = x_time1;
			break;
	}

	if (DST_XTIME_AND_INVERT != 0)
		out ^= 1;

	if (out_is_energy)
	{
		if (x_time > 0)
		{
			double diff = out_high - out_low;
			diff = out ? diff * x_time : diff * (1.0 - x_time);
			node->output[0] = out_low + diff;
		}
		else
			node->output[0] = out ? out_high : out_low;
	}
	else
		node->output[0] = out + x_time;
}


/************************************************************************
 *
 * DST_XTIME_OR - OR/NOR gate implementation using X_TIME
 *
 * If OUT_LOW and OUT_HIGH are defined then the output will be energy.
 * If they are both 0, then the output will be X_TIME logic.
 *
 ************************************************************************/
#define DST_XTIME_OR__IN0			DISCRETE_INPUT(0)
#define DST_XTIME_OR__IN1			DISCRETE_INPUT(1)
#define DST_XTIME_OR_OUT_LOW		DISCRETE_INPUT(2)
#define DST_XTIME_OR_OUT_HIGH		DISCRETE_INPUT(3)
#define DST_XTIME_OR_INVERT			DISCRETE_INPUT(4)

DISCRETE_STEP(dst_xtime_or)
{
	int	in0 = (int)DST_XTIME_OR__IN0;
	int	in1 = (int)DST_XTIME_OR__IN1;
	int	out = 1;
	int out_is_energy = 1;

	double x_time = 0;
	double x_time0 = DST_XTIME_OR__IN0 - in0;
	double x_time1 = DST_XTIME_OR__IN1 - in1;

	int in0_has_xtime = x_time0 > 0 ? 1 : 0;
	int in1_has_xtime = x_time1 > 0 ? 1 : 0;

	double out_low = DST_XTIME_OR_OUT_LOW;
	double out_high = DST_XTIME_OR_OUT_HIGH;

	if (out_low ==0 && out_high == 0)
		out_is_energy = 0;

	switch ((in0 << 3) | (in1 << 2) | (in0_has_xtime < 1) | in1_has_xtime)
	{
		// these are all 1
		//case XTIME__IN0_1__IN1_1__IN0_NOX__IN1_NOX:
		//case XTIME__IN0_0__IN1_1__IN0_NOX__IN1_NOX:
		//case XTIME__IN0_1__IN1_0__IN0_NOX__IN1_NOX:
		//case XTIME__IN0_1__IN1_0__IN0_NOX__IN1_X:
		//case XTIME__IN0_0__IN1_1__IN0_X__IN1_NOX:
		//case XTIME__IN0_1__IN1_1__IN0_NOX__IN1_X:
		//case XTIME__IN0_1__IN1_1__IN0_X__IN1_NOX:
		//	break;

		case XTIME__IN0_0__IN1_0__IN0_NOX__IN1_NOX:
			out = 0;
			break;

		case XTIME__IN0_0__IN1_0__IN0_NOX__IN1_X:
			/*
			 * in0  1
			 *      0   -------------
			 *          ...^....^...
			 *
			 * in1  1   ------
			 *      0         -------
			 *          ...^....^...
			 *
			 * out  1   ------
			 *      0         ------
			 *          ...^....^...
			 */
			out = 0;
			x_time = x_time1;
			break;

		case XTIME__IN0_0__IN1_0__IN0_X__IN1_NOX:
			/*
			 * in0  1   ------
			 *      0         -------
			 *          ...^....^...
			 *
			 * in1  1
			 *      0   -------------
			 *          ...^....^...
			 *
			 * out  1   ------
			 *      0         ------
			 *          ...^....^...
			 */
			out = 0;
			x_time = x_time0;
			break;

		case XTIME__IN0_0__IN1_0__IN0_X__IN1_X:
			/*
			 * in0  1   -----              -------
			 *      0        --------             ------
			 *          ...^....^...       ...^....^...
			 *
			 * in1  1   -------            -----
			 *      0          ------           --------
			 *          ...^....^...       ...^....^...
			 *
			 * out  1   -------            -------
			 *      0          -----              -----
			 *          ...^....^...       ...^....^...
			 */
			out = 0;
			// use x_time of input that was 1 last/longer
			// this means at 0 for less x_time
			if (x_time0 > x_time1)
				x_time = x_time1;
			else
				x_time = x_time0;
			break;

		case XTIME__IN0_0__IN1_1__IN0_NOX__IN1_X:
			/*
			 * in0  1
			 *      0   ------------
			 *          ...^....^...
			 *
			 * in1  1         ------
			 *      0   ------
			 *          ...^....^...
			 *
			 * out  1         ------
			 *      0   ------
			 *          ...^....^...
			 */
			x_time = x_time1;
			break;

		case XTIME__IN0_1__IN1_0__IN0_X__IN1_NOX:
			/*
			 * in0  1         ------
			 *      0   ------
			 *          ...^....^...
			 *
			 * in1  1
			 *      0   ------------
			 *          ...^....^...
			 *
			 * out  1         ------
			 *      0   ------
			 *          ...^....^...
			 */
			x_time = x_time0;
			break;

		case XTIME__IN0_0__IN1_1__IN0_X__IN1_X:
			/*
			 * in0  1   -------           -----
			 *      0          -----           -------
			 *          ...^....^...      ...^....^...
			 *
			 * in1  1        -------             -----
			 *      0   -----             -------
			 *          ...^....^...      ...^....^...
			 *
			 * out  1   ------------      -----  -----
			 *      0                          --
			 *          ...^....^...      ...^....^...
			 */
			// if (x_time0 > x_time1)
				/* Not sure if it is better to use 1
				 * or the total energy which would smear the switch points together.
				 * Let's try just using 1 */
				//x_time = xtime_0 - xtime_1;
			break;

		case XTIME__IN0_1__IN1_0__IN0_X__IN1_X:
			/*
			 * in0  1        -------             -----
			 *      0   -----             -------
			 *          ...^....^...      ...^....^...
			 *
			 * in1  1   -------           -----
			 *      0          -----           -------
			 *          ...^....^...      ...^....^...
			 *
			 * out  1   ------------      -----  -----
			 *      0                          --
			 *          ...^....^...      ...^....^...
			 */
			//if (x_time0 < x_time1)
				/* Not sure if it is better to use 1
				 * or the total energy which would smear the switch points together.
				 * Let's try just using 1 */
				//x_time = xtime_1 - xtime_0;
			break;

		case XTIME__IN0_1__IN1_1__IN0_X__IN1_X:
			/*
			 * in0  1         ------          --------
			 *      0   ------            ----
			 *          ...^....^...      ...^....^...
			 *
			 * in1  1       --------            ------
			 *      0   ----              ------
			 *          ...^....^...      ...^....^...
			 *
			 * out  1       --------          --------
			 *      0   ----              ----
			 *          ...^....^...      ...^....^...
			 */
			if (x_time0 > x_time1)
				x_time = x_time0;
			else
				x_time = x_time1;
			break;
	}

	if (DST_XTIME_OR_INVERT != 0)
		out ^= 1;

	if (out_is_energy)
	{
		if (x_time > 0)
		{
			double diff = out_high - out_low;
			diff = out ? diff * x_time : diff * (1.0 - x_time);
			node->output[0] = out_low + diff;
		}
		else
			node->output[0] = out ? out_high : out_low;
	}
	else
		node->output[0] = out + x_time;
}


/************************************************************************
 *
 * DST_XTIME_XOR - XOR/XNOR gate implementation using X_TIME
 *
 * If OUT_LOW and OUT_HIGH are defined then the output will be energy.
 * If they are both 0, then the output will be X_TIME logic.
 *
 ************************************************************************/
#define DST_XTIME_XOR__IN0			DISCRETE_INPUT(0)
#define DST_XTIME_XOR__IN1			DISCRETE_INPUT(1)
#define DST_XTIME_XOR_OUT_LOW		DISCRETE_INPUT(2)
#define DST_XTIME_XOR_OUT_HIGH		DISCRETE_INPUT(3)
#define DST_XTIME_XOR_INVERT		DISCRETE_INPUT(4)

DISCRETE_STEP(dst_xtime_xor)
{
	int	in0 = (int)DST_XTIME_XOR__IN0;
	int	in1 = (int)DST_XTIME_XOR__IN1;
	int	out = 1;
	int out_is_energy = 1;

	double x_time = 0;
	double x_time0 = DST_XTIME_XOR__IN0 - in0;
	double x_time1 = DST_XTIME_XOR__IN1 - in1;

	int in0_has_xtime = x_time0 > 0 ? 1 : 0;
	int in1_has_xtime = x_time1 > 0 ? 1 : 0;

	double out_low = DST_XTIME_XOR_OUT_LOW;
	double out_high = DST_XTIME_XOR_OUT_HIGH;

	if (out_low ==0 && out_high == 0)
		out_is_energy = 0;

	switch ((in0 << 3) | (in1 << 2) | (in0_has_xtime < 1) | in1_has_xtime)
	{
		// these are all 1
		//case XTIME__IN0_0__IN1_1__IN0_NOX__IN1_NOX:
		//case XTIME__IN0_1__IN1_0__IN0_NOX__IN1_NOX:
		//	break;

		case XTIME__IN0_1__IN1_1__IN0_NOX__IN1_NOX:
		case XTIME__IN0_0__IN1_0__IN0_NOX__IN1_NOX:
			out = 0;
			break;

		case XTIME__IN0_1__IN1_0__IN0_X__IN1_NOX:
			/*
			 * in0  1         ------
			 *      0   ------
			 *          ...^....^...
			 *
			 * in1  1
			 *      0   ------------
			 *          ...^....^...
			 *
			 * out  1         ------
			 *      0   ------
			 *          ...^....^...
			 */
		case XTIME__IN0_0__IN1_1__IN0_X__IN1_NOX:
			/*
			 * in0  1   ------
			 *      0         -------
			 *          ...^....^...
			 *
			 * in1  1   -------------
			 *      0
			 *          ...^....^...
			 *
			 * out  1         ------
			 *      0   ------
			 *          ...^....^...
			 */
			x_time = x_time0;
			break;

		case XTIME__IN0_0__IN1_1__IN0_NOX__IN1_X:
			/*
			 * in0  1
			 *      0   ------------
			 *          ...^....^...
			 *
			 * in1  1         ------
			 *      0   ------
			 *          ...^....^...
			 *
			 * out  1         ------
			 *      0   ------
			 *          ...^....^...
			 */
		case XTIME__IN0_1__IN1_0__IN0_NOX__IN1_X:
			/*
			 * in0  1   -------------
			 *      0
			 *          ...^....^...
			 *
			 * in1  1   ------
			 *      0         -------
			 *          ...^....^...
			 *
			 * out  1         ------
			 *      0   ------
			 *          ...^....^...
			 */
			x_time = x_time1;
			break;

		case XTIME__IN0_0__IN1_0__IN0_X__IN1_NOX:
			/*
			 * in0  1   ------
			 *      0         ------
			 *          ...^....^...
			 *
			 * in1  1
			 *      0   ------------
			 *          ...^....^...
			 *
			 * out  1   ------
			 *      0         ------
			 *          ...^....^...
			 */
		case XTIME__IN0_1__IN1_1__IN0_X__IN1_NOX:
			/*
			 * in1  0         ------
			 *      0   ------
			 *          ...^....^...
			 *
			 * in1  1   ------------
			 *      0
			 *          ...^....^...
			 *
			 * out  1   ------
			 *      0         ------
			 *          ...^....^...
			 */
			out = 0;
			x_time = x_time0;
			break;

		case XTIME__IN0_0__IN1_0__IN0_NOX__IN1_X:
			/*
			 * in0  1
			 *      0   ------------
			 *          ...^....^...
			 *
			 * in1  1   ------
			 *      0         ------
			 *          ...^....^...
			 *
			 * out  1   ------
			 *      0         ------
			 *          ...^....^...
			 */
		case XTIME__IN0_1__IN1_1__IN0_NOX__IN1_X:
			/*
			 * in0  1   ------------
			 *      0
			 *          ...^....^...
			 *
			 * in1  1         ------
			 *      0   ------
			 *          ...^....^...
			 *
			 * out  1   ------
			 *      0         ------
			 *          ...^....^...
			 */
			out = 0;
			x_time = x_time1;
			break;

		case XTIME__IN0_0__IN1_0__IN0_X__IN1_X:
			/*
			 * in0  1   -----              -------
			 *      0        -------              -----
			 *          ...^....^...       ...^....^...
			 *
			 * in1  1   -------            -----
			 *      0          -----            -------
			 *          ...^....^...       ...^....^...
			 *
			 * out  1        --                 --
			 *      0   -----  -----       -----  -----
			 *          ...^....^...       ...^....^...
			 */
		case XTIME__IN0_1__IN1_1__IN0_X__IN1_X:
			/*
			 * in0  1         ------          --------
			 *      0   ------            ----
			 *          ...^....^...      ...^....^...
			 *
			 * in1  1       --------            ------
			 *      0   ----              ------
			 *          ...^....^...      ...^....^...
			 *
			 * out  1       --                --
			 *      0   ----  ------      ----  ------
			 *          ...^....^...      ...^....^...
			 */
			out = 0;
			/* Not sure if it is better to use 0
			 * or the total energy which would smear the switch points together.
			 * Let's try just using 0 */
			// x_time = abs(x_time0 - x_time1);
			break;

		case XTIME__IN0_0__IN1_1__IN0_X__IN1_X:
			/*
			 * in0  1   -------           -----
			 *      0          -----           -------
			 *          ...^....^...      ...^....^...
			 *
			 * in1  1        -------             -----
			 *      0   -----             -------
			 *          ...^....^...      ...^....^...
			 *
			 * out  1   -----  -----      -----  -----
			 *      0        --                --
			 *          ...^....^...      ...^....^...
			 */
		case XTIME__IN0_1__IN1_0__IN0_X__IN1_X:
			/*
			 * in0  1        -------             -----
			 *      0   -----             -------
			 *          ...^....^...      ...^....^...
			 *
			 * in1  1   -------           -----
			 *      0          -----           -------
			 *          ...^....^...      ...^....^...
			 *
			 * out  1   -----  -----      -----  -----
			 *      0        --                --
			 *          ...^....^...      ...^....^...
			 */
			/* Not sure if it is better to use 1
			 * or the total energy which would smear the switch points together.
			 * Let's try just using 1 */
			// x_time = 1.0 - abs(x_time0 - x_time1);
			break;
}

	if (DST_XTIME_XOR_INVERT != 0)
		out ^= 1;

	if (out_is_energy)
	{
		if (x_time > 0)
		{
			double diff = out_high - out_low;
			diff = out ? diff * x_time : diff * (1.0 - x_time);
			node->output[0] = out_low + diff;
		}
		else
			node->output[0] = out ? out_high : out_low;
	}
	else
		node->output[0] = out + x_time;
}
