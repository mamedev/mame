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
 *
 ************************************************************************/

#include <float.h>

struct dst_dac_r1_context
{
	double	iBias;		// current of the bias circuit
	double	exponent;	// smoothing curve
	double	rTotal;		// all resistors in parallel
};

struct dst_flipflop_context
{
	int last_clk;
};

struct dst_integrate_context
{
	double	change;
	double	vMaxIn;		// v1 - norton VBE
	double	vMaxInD;	// v1 - norton VBE - diode drop
	double	vMaxOut;
};

#define DISC_MIXER_MAX_INPS	8

struct dst_mixer_context
{
	int	type;
	int	size;
	double	rTotal;
	double *rNode[DISC_MIXER_MAX_INPS];			// Either pointer to resistance node output OR NULL
	double	exponent_rc[DISC_MIXER_MAX_INPS];	// For high pass filtering cause by cIn
	double	vCap[DISC_MIXER_MAX_INPS];		// cap voltage of each input
	double	exponent_cF;				// Low pass on mixed inputs
	double	exponent_cAmp;				// Final high pass caused by out cap and amp input impedance
	double	vCapF;					// cap voltage of cF
	double	vCapAmp;				// cap voltage of cAmp
	double	gain;					// used for DISC_MIXER_IS_OP_AMP_WITH_RI
};

struct dst_oneshot_context
{
	double countdown;
	int state;
	int lastTrig;
};

struct dss_ramp_context
{
	double step;
	int dir;		/* 1 if End is higher then Start */
	int last_en;	/* Keep track of the last enable value */
};

struct dst_samphold_context
{
	double lastinput;
	int clocktype;
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
	double	vMax;
	double	iFixed;
	double	vCap;
	double	exponent;
};

struct dst_op_amp_1sht_context
{
	double	iFixed;
	double	vMax;
	double	r34ratio;
	double	vCap1;
	double	vCap2;
	double	exponent1c;
	double	exponent1d;
	double	exponent2;
};

struct dst_tvca_op_amp_context
{
	double	vOutMax;		// Maximum output voltage
	double	vTrig[2];		// Voltage used to charge cap1 based on function F3
	double	vTrig2;			// Voltage used to charge cap2
	double	vTrig3;			// Voltage used to charge cap3
	double	iFixed;			// Fixed current going into - input
	double	exponentC[2];	// Charge exponents based on function F3
	double	exponentD[2];	// Discharge exponents based on function F3
	double	exponent2[2];	// Discharge/charge exponents based on function F4
	double	exponent3[2];	// Discharge/charge exponents based on function F5
	double	vCap1;			// charge on cap c1
	double	vCap2;			// charge on cap c2
	double	vCap3;			// charge on cap c3
	double	r67;			// = r6 + r7 (for easy use later)
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
#define DST_ADDER__ENABLE	(*(node->input[0]))
#define DST_ADDER__IN0		(*(node->input[1]))
#define DST_ADDER__IN1		(*(node->input[2]))
#define DST_ADDER__IN2		(*(node->input[3]))
#define DST_ADDER__IN3		(*(node->input[4]))

void dst_adder_step(node_description *node)
{
	if(DST_ADDER__ENABLE)
	{
		node->output = DST_ADDER__IN0 + DST_ADDER__IN1 + DST_ADDER__IN2 + DST_ADDER__IN3;
	}
	else
	{
		node->output=0;
	}
}


/************************************************************************
 *
 * DST_COMP_ADDER  - Selectable parallel component adder
 *
 * input[0]    - Enable input value
 * input[1]    - Bit Select
 *
 * Also passed discrete_comp_adder_table structure
 *
 * Mar 2004, D Renaud.
 ************************************************************************/
#define DST_COMP_ADDER__ENABLE	(*(node->input[0]))
#define DST_COMP_ADDER__SELECT	(int)(*(node->input[1]))

void dst_comp_adder_step(node_description *node)
{
	const discrete_comp_adder_table *info = node->custom;
	int bit;

	if(DST_COMP_ADDER__ENABLE)
	{
		switch (info->type)
		{
			case DISC_COMP_P_CAPACITOR:
				node->output = info->cDefault;
				for(bit=0; bit < info->length; bit++)
				{
					if (DST_COMP_ADDER__SELECT & (1 << bit)) node->output += info->c[bit];
				}
				break;
			case DISC_COMP_P_RESISTOR:
				node->output = info->cDefault ? 1.0 / info->cDefault : 0;
				for(bit=0; bit < info->length; bit++)
				{
					if (DST_COMP_ADDER__SELECT & (1 << bit)) node->output += 1.0 / info->c[bit];
				}
				if (node->output != 0) node->output = 1.0 / node->output;
				break;
		}
	}
	else
	{
		node->output = 0;
	}
}


/************************************************************************
 *
 * DST_CLAMP - Simple signal clamping circuit
 *
 * input[0]    - Enable ramp
 * input[1]    - Input value
 * input[2]    - Minimum value
 * input[3]    - Maximum value
 * input[4]    - Clamp output when disabled
 *
 ************************************************************************/
#define DST_CLAMP__ENABLE	(*(node->input[0]))
#define DST_CLAMP__IN		(*(node->input[1]))
#define DST_CLAMP__MIN		(*(node->input[2]))
#define DST_CLAMP__MAX		(*(node->input[3]))
#define DST_CLAMP__CLAMP	(*(node->input[4]))

void dst_clamp_step(node_description *node)
{
	if(DST_CLAMP__ENABLE)
	{
		if(DST_CLAMP__IN < DST_CLAMP__MIN) node->output = DST_CLAMP__MIN;
		else if(DST_CLAMP__IN > DST_CLAMP__MAX) node->output = DST_CLAMP__MAX;
		else node->output= DST_CLAMP__IN;
	}
	else
	{
		node->output = DST_CLAMP__CLAMP;
	}
}


/************************************************************************
 *
 * DST_DAC_R1 - R1 Ladder DAC with cap smoothing
 *
 * input[0]    - Enable
 * input[1]    - Binary Data Input
 * input[2]    - Data On Voltage (3.4 for TTL)
 *
 * also passed discrete_dac_r1_ladder structure
 *
 * Mar 2004, D Renaud.
 ************************************************************************/
#define DST_DAC_R1__ENABLE		(*(node->input[0]))
#define DST_DAC_R1__DATA		(int)(*(node->input[1]))
#define DST_DAC_R1__VON			(*(node->input[2]))

void dst_dac_r1_step(node_description *node)
{
	const discrete_dac_r1_ladder *info = node->custom;
	struct dst_dac_r1_context *context = node->context;

	int	bit;
	double	v;
	double	i;

	i = context->iBias;

	if (DST_DAC_R1__ENABLE)
	{
		for (bit=0; bit < info->ladderLength; bit++)
		{
			/* Add up currents of ON circuits per Millman. */
			/* Off, being 0V and having no current, can be ignored. */
			if ((DST_DAC_R1__DATA & (1 << bit)) && info->r[bit])
				i += DST_DAC_R1__VON / info->r[bit];
		}

		v = i * context->rTotal;

		/* Filter if needed, else just output voltage */
		node->output = info->cFilter ? node->output + ((v - node->output) * context->exponent) : v;
	}
	else
	{
		/*
         * If module is disabled we will just leave the voltage where it was.
         * We may want to set it to 0 in the future, but we will probably never
         * disable this module.
         */
	}
}

void dst_dac_r1_reset(node_description *node)
{
	const discrete_dac_r1_ladder *info = node->custom;
	struct dst_dac_r1_context *context = node->context;

	int	bit;

	/* Calculate the Millman current of the bias circuit */
	if (info->rBias)
		context->iBias = info->vBias / info->rBias;
	else
		context->iBias = 0;

	/*
     * We will do a small amount of error checking.
     * But if you are an idiot and pass a bad ladder table
     * then you deserve a crash.
     */
	if (info->ladderLength < 2)
	{
		/* You need at least 2 resistors for a ladder */
		discrete_log("dst_dac_r1_reset - Ladder length too small");
	}
	if (info->ladderLength > DISC_LADDER_MAXRES )
	{
		discrete_log("dst_dac_r1_reset - Ladder length exceeds DISC_LADDER_MAXRES");
	}

	/*
     * Calculate the total of all resistors in parallel.
     * This is the combined resistance of the voltage sources.
     * This is used for the charging curve.
     */
	context->rTotal = 0;
	for(bit=0; bit < info->ladderLength; bit++)
	{
		if (info->r[bit])
			context->rTotal += 1.0 / info->r[bit];
	}
	if (info->rBias) context->rTotal += 1.0 / info->rBias;
	if (info->rGnd) context->rTotal += 1.0 / info->rGnd;
	context->rTotal = 1.0 / context->rTotal;

	node->output = 0;

	if (info->cFilter)
	{
		/* Setup filter constants */
		context->exponent = -1.0 / (context->rTotal * info->cFilter  * discrete_current_context->sample_rate);
		context->exponent = 1.0 - exp(context->exponent);
	}
}


/************************************************************************
*
 * DST_DIODE_MIX  - Diode Mixer
 *
 * input[0]    - Enable input value
 * input[1]    - Diode junction voltage drop
 * input[2]    - Input 0
 * .....
 *
 * Dec 2004, D Renaud.
 ************************************************************************/
#define DST_DIODE_MIX__ENABLE		(*(node->input[0]))
#define DST_DIODE_MIX__VJUNC		(*(node->input[1]))
#define DST_DIODE_MIX__INP(addr)	(*(node->input[2 + addr]))

void dst_diode_mix_step(node_description *node)
{
	struct dst_size_context *context = node->context;
	double max = 0;
	int addr;

	if (DST_DIODE_MIX__ENABLE)
	{
		for (addr = 0; addr < context->size; addr++)
		{
			if (DST_DIODE_MIX__INP(addr) > max) max = DST_DIODE_MIX__INP(addr);
		}
		node->output = max - DST_DIODE_MIX__VJUNC;
		if (node->output < 0) node->output = 0;
	}
	else
	{
		node->output = 0;
	}
}

void dst_diode_mix_reset(node_description *node)
{
	struct dst_size_context *context = node->context;

	context->size = node->active_inputs - 2;

	dst_diode_mix_step(node);
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
#define DST_DIVIDE__ENABLE	(*(node->input[0]))
#define DST_DIVIDE__IN		(*(node->input[1]))
#define DST_DIVIDE__DIV		(*(node->input[2]))

void dst_divide_step(node_description *node)
{
	if(DST_DIVIDE__ENABLE)
	{
		if(DST_DIVIDE__DIV == 0)
		{
			node->output=DBL_MAX;	/* Max out but don't break */
			discrete_log("dst_divider_step() - Divide by Zero attempted in NODE_%02d.\n",node->node-NODE_START);
		}
		else
		{
			node->output= DST_DIVIDE__IN / DST_DIVIDE__DIV;
		}
	}
	else
	{
		node->output=0;
	}
}


/************************************************************************
 *
 * DST_GAIN - This is a programmable gain module with enable function
 *
 * input[0]    - Enable input value
 * input[1]    - Channel0 input value
 * input[2]    - Gain value
 * input[3]    - Final addition offset
 *
 ************************************************************************/
#define DST_GAIN__ENABLE	(*(node->input[0]))
#define DST_GAIN__IN		(*(node->input[1]))
#define DST_GAIN__GAIN		(*(node->input[2]))
#define DST_GAIN__OFFSET	(*(node->input[3]))

void dst_gain_step(node_description *node)
{
	if(DST_GAIN__ENABLE)
	{
		node->output = DST_GAIN__IN * DST_GAIN__GAIN;
		node->output += DST_GAIN__OFFSET;
	}
	else
	{
		node->output=0;
	}
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
#define DST_INTEGRATE__TRG0	(*(node->input[0]))
#define DST_INTEGRATE__TRG1	(*(node->input[1]))

int dst_trigger_function(int trig0, int trig1, int trig2, int function)
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

void dst_integrate_step(node_description *node)
{
	const discrete_integrate_info *info = node->custom;
	struct dst_integrate_context *context = node->context;

	int		trig0, trig1;
	double	iNeg = 0;	// current into - input
	double	iPos = 0;	// current into + input

	switch (info->type)
	{
		case DISC_INTEGRATE_OP_AMP_1:
			if (DST_INTEGRATE__TRG0 != 0)
			{
				/* This forces the cap to completely charge,
                 * and the output to go to it's max value.
                 */
				node->output = context->vMaxOut;
				return;
			}
			node->output -= context->change;
			break;

		case DISC_INTEGRATE_OP_AMP_1 | DISC_OP_AMP_IS_NORTON:
			iNeg = context->vMaxIn / info->r1;
			iPos = (DST_INTEGRATE__TRG0 - OP_AMP_NORTON_VBE) / info->r2;
			if (iPos < 0) iPos = 0;
			node->output += (iPos - iNeg) / discrete_current_context->sample_rate / info->c;
			break;

		case DISC_INTEGRATE_OP_AMP_2 | DISC_OP_AMP_IS_NORTON:
			trig0 = (int)DST_INTEGRATE__TRG0;
			trig1 = (int)DST_INTEGRATE__TRG1;
			iNeg = dst_trigger_function(trig0, trig1, 0, info->f0) ? context->vMaxInD / info->r1 : 0;
			iPos =  dst_trigger_function(trig0, trig1, 0, info->f1) ? context->vMaxIn / info->r2 : 0;
			iPos +=  dst_trigger_function(trig0, trig1, 0, info->f2) ? context->vMaxInD / info->r3 : 0;
			node->output += (iPos - iNeg) / discrete_current_context->sample_rate / info->c;
			break;
	}

	/* Clip the output. */
	if (node->output < 0) node->output = 0;
	if (node->output > context->vMaxOut) node->output = context->vMaxOut;
}

void dst_integrate_reset(node_description *node)
{
	const discrete_integrate_info *info = node->custom;
	struct dst_integrate_context *context = node->context;
	double	i, v;

	if (info->type & DISC_OP_AMP_IS_NORTON)
	{
		context->vMaxOut = info->vP - OP_AMP_NORTON_VBE;
		context->vMaxIn  = info->v1 - OP_AMP_NORTON_VBE;
		context->vMaxInD = context->vMaxIn - OP_AMP_NORTON_VBE;
	}
	else
	{
		context->vMaxOut =  info->vP - OP_AMP_VP_RAIL_OFFSET;

		v = info->v1 * info->r3 / (info->r2 + info->r3);	/* vRef */
		v = info->v1 - v;	/* actual charging voltage */
		i = v / info->r1;
		context->change = i / discrete_current_context->sample_rate / info->c;
	}
	node->output = 0;
}


/************************************************************************
 *
 * DST_LOGIC_INV - Logic invertor gate implementation
 *
 * input[0]    - Enable
 * input[1]    - input[0] value
 *
 ************************************************************************/
#define DST_LOGIC_INV__ENABLE	(*(node->input[0]))
#define DST_LOGIC_INV__IN		(*(node->input[1]))

void dst_logic_inv_step(node_description *node)
{
	if(DST_LOGIC_INV__ENABLE)
	{
		node->output = DST_LOGIC_INV__IN ? 0.0 : 1.0;
	}
	else
	{
		node->output=0.0;
	}
}

/************************************************************************
 *
 * DST_LOGIC_AND - Logic AND gate implementation
 *
 * input[0]    - Enable
 * input[1]    - input[0] value
 * input[2]    - input[1] value
 * input[3]    - input[2] value
 * input[4]    - input[3] value
 *
 ************************************************************************/
#define DST_LOGIC_AND__ENABLE	(*(node->input[0]))
#define DST_LOGIC_AND__IN0		(*(node->input[1]))
#define DST_LOGIC_AND__IN1		(*(node->input[2]))
#define DST_LOGIC_AND__IN2		(*(node->input[3]))
#define DST_LOGIC_AND__IN3		(*(node->input[4]))

void dst_logic_and_step(node_description *node)
{
	if(DST_LOGIC_AND__ENABLE)
	{
		node->output= (DST_LOGIC_AND__IN0 && DST_LOGIC_AND__IN1 && DST_LOGIC_AND__IN2 && DST_LOGIC_AND__IN3)? 1.0 : 0.0;
	}
	else
	{
		node->output=0.0;
	}
}

/************************************************************************
 *
 * DST_LOGIC_NAND - Logic NAND gate implementation
 *
 * input[0]    - Enable
 * input[1]    - input[0] value
 * input[2]    - input[1] value
 * input[3]    - input[2] value
 * input[4]    - input[3] value
 *
 ************************************************************************/
#define DST_LOGIC_NAND__ENABLE	(*(node->input[0]))
#define DST_LOGIC_NAND__IN0		(*(node->input[1]))
#define DST_LOGIC_NAND__IN1		(*(node->input[2]))
#define DST_LOGIC_NAND__IN2		(*(node->input[3]))
#define DST_LOGIC_NAND__IN3		(*(node->input[4]))

void dst_logic_nand_step(node_description *node)
{
	if(DST_LOGIC_NAND__ENABLE)
	{
		node->output= (DST_LOGIC_NAND__IN0 && DST_LOGIC_NAND__IN1 && DST_LOGIC_NAND__IN2 && DST_LOGIC_NAND__IN3)? 0.0 : 1.0;
	}
	else
	{
		node->output=0.0;
	}
}

/************************************************************************
 *
 * DST_LOGIC_OR  - Logic OR  gate implementation
 *
 * input[0]    - Enable
 * input[1]    - input[0] value
 * input[2]    - input[1] value
 * input[3]    - input[2] value
 * input[4]    - input[3] value
 *
 ************************************************************************/
#define DST_LOGIC_OR__ENABLE	(*(node->input[0]))
#define DST_LOGIC_OR__IN0		(*(node->input[1]))
#define DST_LOGIC_OR__IN1		(*(node->input[2]))
#define DST_LOGIC_OR__IN2		(*(node->input[3]))
#define DST_LOGIC_OR__IN3		(*(node->input[4]))

void dst_logic_or_step(node_description *node)
{
	if(DST_LOGIC_OR__ENABLE)
	{
		node->output = (DST_LOGIC_OR__IN0 || DST_LOGIC_OR__IN1 || DST_LOGIC_OR__IN2 || DST_LOGIC_OR__IN3) ? 1.0 : 0.0;
	}
	else
	{
		node->output=0.0;
	}
}

/************************************************************************
 *
 * DST_LOGIC_NOR - Logic NOR gate implementation
 *
 * input[0]    - Enable
 * input[1]    - input[0] value
 * input[2]    - input[1] value
 * input[3]    - input[2] value
 * input[4]    - input[3] value
 *
 ************************************************************************/
#define DST_LOGIC_NOR__ENABLE	(*(node->input[0]))
#define DST_LOGIC_NOR__IN0		(*(node->input[1]))
#define DST_LOGIC_NOR__IN1		(*(node->input[2]))
#define DST_LOGIC_NOR__IN2		(*(node->input[3]))
#define DST_LOGIC_NOR__IN3		(*(node->input[4]))

void dst_logic_nor_step(node_description *node)
{
	if(DST_LOGIC_NOR__ENABLE)
	{
		node->output = (DST_LOGIC_NOR__IN0 || DST_LOGIC_NOR__IN1 || DST_LOGIC_NOR__IN2 || DST_LOGIC_NOR__IN3) ? 0.0 : 1.0;
	}
	else
	{
		node->output=0.0;
	}
}

/************************************************************************
 *
 * DST_LOGIC_XOR - Logic XOR gate implementation
 *
 * input[0]    - Enable
 * input[1]    - input[0] value
 * input[2]    - input[1] value
 *
 ************************************************************************/
#define DST_LOGIC_XOR__ENABLE	(*(node->input[0]))
#define DST_LOGIC_XOR__IN0		(*(node->input[1]))
#define DST_LOGIC_XOR__IN1		(*(node->input[2]))

void dst_logic_xor_step(node_description *node)
{
	if(DST_LOGIC_XOR__ENABLE)
	{
		node->output=((DST_LOGIC_XOR__IN0 && !DST_LOGIC_XOR__IN1) || (!DST_LOGIC_XOR__IN0 && DST_LOGIC_XOR__IN1)) ? 1.0 : 0.0;
	}
	else
	{
		node->output=0.0;
	}
}

/************************************************************************
 *
 * DST_LOGIC_NXOR - Logic NXOR gate implementation
 *
 * input[0]    - Enable
 * input[1]    - input[0] value
 * input[2]    - input[1] value
 *
 ************************************************************************/
#define DST_LOGIC_XNOR__ENABLE	(*(node->input[0]))
#define DST_LOGIC_XNOR__IN0		(*(node->input[1]))
#define DST_LOGIC_XNOR__IN1		(*(node->input[2]))

void dst_logic_nxor_step(node_description *node)
{
	if(DST_LOGIC_XNOR__ENABLE)
	{
		node->output=((DST_LOGIC_XNOR__IN0 && !DST_LOGIC_XNOR__IN1) || (!DST_LOGIC_XNOR__IN0 && DST_LOGIC_XNOR__IN1)) ? 0.0 : 1.0;
	}
	else
	{
		node->output=0.0;
	}
}


/************************************************************************
 *
 * DST_LOGIC_DFF - Standard D-type flip-flop implementation
 *
 * input[0]    - enable
 * input[1]    - /Reset
 * input[2]    - /Set
 * input[3]    - clock
 * input[4]    - data
 *
 ************************************************************************/
#define DST_LOGIC_DFF__ENABLE	 (*(node->input[0]))
#define DST_LOGIC_DFF__RESET	!(*(node->input[1]))
#define DST_LOGIC_DFF__SET		!(*(node->input[2]))
#define DST_LOGIC_DFF__CLOCK	 (*(node->input[3]))
#define DST_LOGIC_DFF__DATA 	 (*(node->input[4]))

void dst_logic_dff_step(node_description *node)
{
	struct dst_flipflop_context *context = node->context;
	int clk = (int)DST_LOGIC_DFF__CLOCK;

	if (DST_LOGIC_DFF__ENABLE)
	{
		if (DST_LOGIC_DFF__RESET)
			node->output = 0;
		else if (DST_LOGIC_DFF__SET)
			node->output = 1;
		else if (!context->last_clk && clk)	/* low to high */
		{
			node->output = DST_LOGIC_DFF__DATA;
		}
	}
	else
	{
		node->output = 0;
	}
	context->last_clk = clk;
}

void dst_logic_ff_reset(node_description *node)
{
	struct dst_flipflop_context *context = node->context;
	context->last_clk = 0;
	node->output = 0;
}


/************************************************************************
 *
 * DST_LOGIC_JKFF - Standard JK-type flip-flop implementation
 *
 * input[0]    - enable
 * input[1]    - /Reset
 * input[2]    - /Set
 * input[3]    - clock
 * input[4]    - J
 * input[5]    - K
 *
 ************************************************************************/
#define DST_LOGIC_JKFF__ENABLE	 (*(node->input[0]))
#define DST_LOGIC_JKFF__RESET	!(*(node->input[1]))
#define DST_LOGIC_JKFF__SET		!(*(node->input[2]))
#define DST_LOGIC_JKFF__CLOCK	 (*(node->input[3]))
#define DST_LOGIC_JKFF__J 		 (*(node->input[4]))
#define DST_LOGIC_JKFF__K	 	 (*(node->input[5]))

void dst_logic_jkff_step(node_description *node)
{
	struct dst_flipflop_context *context = node->context;
	int clk = (int)DST_LOGIC_JKFF__CLOCK;
	int j = (int)DST_LOGIC_JKFF__J;
	int k = (int)DST_LOGIC_JKFF__K;

	if (DST_LOGIC_JKFF__ENABLE)
	{
		if (DST_LOGIC_JKFF__RESET)
			node->output = 0;
		else if (DST_LOGIC_JKFF__SET)
			node->output = 1;
		else if (context->last_clk && !clk)	/* high to low */
		{
			if (!j)
			{
				/* J=0, K=0 - Hold */
				if (k)
					/* J=0, K=1 - Reset */
					node->output = 0;
			}
			else
			{
				if (!k)
					/* J=1, K=0 - Set */
					node->output = 1;
				else
					/* J=1, K=1 - Toggle */
					node->output = !(int)node->output;
			}
		}
	}
	else
	{
		node->output=0;
	}
	context->last_clk = clk;
}


/************************************************************************
 *
 * DST_LOOKUP_TABLE  - Return value from lookup table
 *
 * input[0]    - Enable input value
 * input[1]    - Input 1
 * input[2]    - Table size
 *
 * Also passed address of the lookup table
 *
 * Feb 2007, D Renaud.
 ************************************************************************/
#define DST_LOOKUP_TABLE__ENABLE	(*(node->input[0]))
#define DST_LOOKUP_TABLE__IN		(*(node->input[1]))
#define DST_LOOKUP_TABLE__SIZE		(*(node->input[2]))

void dst_lookup_table_step(node_description *node)
{
	const double *table = node->custom;
	int	addr = DST_LOOKUP_TABLE__IN;

	if (!DST_LOOKUP_TABLE__ENABLE || addr < 0 || addr >= DST_LOOKUP_TABLE__SIZE)
		node->output = 0;
	else
		node->output = table[addr];
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
#define DST_MIXER__ENABLE		(*(node->input[0]))
#define DST_MIXER__IN(bit)		(*(node->input[bit + 1]))

void dst_mixer_step(node_description *node)
{
	const discrete_mixer_desc *info = node->custom;
	struct dst_mixer_context *context = node->context;

	double	v, vTemp, rTotal, rTemp, rTemp2 = 0;
	double	i = 0;		// total current of inputs
	int	bit, connected;

	if (DST_MIXER__ENABLE)
	{
		rTotal = context->rTotal;

		for(bit=0; bit < context->size; bit++)
		{
			rTemp = info->r[bit];
			connected = 1;
			vTemp = DST_MIXER__IN(bit);

			if (info->rNode[bit])
			{
				/* a node has the posibility of being disconnected from the circuit. */
				if (*context->rNode[bit] == 0)
					connected = 0;
				else
				{
					rTemp += *context->rNode[bit];
					rTotal += 1.0 / rTemp;
					if (info->c[bit] != 0)
					{
						switch (context->type & DISC_MIXER_TYPE_MASK)
						{
							case DISC_MIXER_IS_RESISTOR:
								/* is there an rF? */
								if (info->rF != 0)
								{
									rTemp2 = 1.0 / ((1.0 / rTemp) + (1.0 / info->rF));
									break;
								}
								/* else, fall through and just use the resistor value */
							case DISC_MIXER_IS_OP_AMP:
								rTemp2 = rTemp;
								break;
							case DISC_MIXER_IS_OP_AMP_WITH_RI:
								rTemp2 = rTemp + info->rI;
								break;
						}
						/* Re-calculate exponent if resistor is a node */
						context->exponent_rc[bit] = -1.0 / (rTemp2 * info->c[bit]  * discrete_current_context->sample_rate);
						context->exponent_rc[bit] = 1.0 - exp(context->exponent_rc[bit]);
					}
				}
			}

			if (connected)
			{
				if (info->c[bit] != 0)
				{
					/* do input high pass filtering if needed. */
					context->vCap[bit] += (vTemp - info->vRef - context->vCap[bit]) * context->exponent_rc[bit];
					vTemp -= context->vCap[bit];
				}
				i += (((context->type & DISC_MIXER_TYPE_MASK) == DISC_MIXER_IS_OP_AMP) ? info->vRef - vTemp : vTemp) / rTemp;
			}
		}

		if ((context->type & DISC_MIXER_TYPE_MASK) == DISC_MIXER_IS_OP_AMP_WITH_RI) i += info->vRef / info->rI;
		rTotal = 1.0 / rTotal;

		/* If resistor network or has rI then Millman is used.
         * If op-amp then summing formula is used. */
		v = i * (((context->type & DISC_MIXER_TYPE_MASK) == DISC_MIXER_IS_OP_AMP) ? info->rF : rTotal);

		if ((context->type & DISC_MIXER_TYPE_MASK) == DISC_MIXER_IS_OP_AMP_WITH_RI)
			v = info->vRef + (context->gain * (info->vRef - v));

		/* Do the low pass filtering for cF */
		if (info->cF != 0)
		{
			if (context->type & DISC_MIXER_HAS_R_NODE)
			{
				/* Re-calculate exponent if resistor nodes are used */
				context->exponent_cF = -1.0 / (rTotal * info->cF  * discrete_current_context->sample_rate);
				context->exponent_cF = 1.0 - exp(context->exponent_cF);
			}
			context->vCapF += (v -info->vRef - context->vCapF) * context->exponent_cF;
			v = context->vCapF;
		}

		/* Do the high pass filtering for cAmp */
		if (info->cAmp != 0)
		{
			context->vCapAmp += (v - context->vCapAmp) * context->exponent_cAmp;
			v -= context->vCapAmp;
		}
		node->output = v * info->gain;
	}
	else
	{
		node->output = 0;
	}
}

void dst_mixer_reset(node_description *node)
{
	const discrete_mixer_desc *info = node->custom;
	struct dst_mixer_context *context = node->context;
	node_description *r_node;

	int	bit;
	double	rTemp = 0;

	/* link to rNode outputs */
	for (bit = 0; bit < 8; bit ++)
	{
		r_node = discrete_find_node(NULL, info->rNode[bit]);
		if (r_node)
			context->rNode[bit] = &(r_node->output);
		else
			context->rNode[bit] = NULL;
	}

	context->size = node->active_inputs - 1;

	/*
     * THERE IS NO ERROR CHECKING!!!!!!!!!
     * If you pass a bad ladder table
     * then you deserve a crash.
     */

	context->type = ((info->type == DISC_MIXER_IS_OP_AMP) && info->rI) ? DISC_MIXER_IS_OP_AMP_WITH_RI : info->type;

	/*
     * Calculate the total of all resistors in parallel.
     * This is the combined resistance of the voltage sources.
     * Also calculate the exponents while we are here.
     */
	context->rTotal = 0;
	for(bit=0; bit < context->size; bit++)
	{
		if (info->rNode[bit])
			context->type = context->type | DISC_MIXER_HAS_R_NODE;

		if ((info->r[bit] != 0) && !info->rNode[bit] )
		{
			context->rTotal += 1.0 / info->r[bit];
		}

		context->vCap[bit] = 0;
		context->exponent_rc[bit] = 0;
		if ((info->c[bit] != 0)  && !info->rNode[bit])
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
			context->exponent_rc[bit] = -1.0 / (rTemp * info->c[bit]  * discrete_current_context->sample_rate);
			context->exponent_rc[bit] = 1.0 - exp(context->exponent_rc[bit]);
		}
	}

	if (info->rF != 0)
	{
		if (info->type == DISC_MIXER_IS_RESISTOR) context->rTotal += 1.0 / info->rF;
	}
	if (context->type == DISC_MIXER_IS_OP_AMP_WITH_RI) context->rTotal += 1.0 / info->rI;

	context->vCapF = 0;
	context->exponent_cF = 0;
	if (info->cF != 0)
	{
		/* Setup filter constants */
		context->exponent_cF = -1.0 / (((info->type == DISC_MIXER_IS_OP_AMP) ? info->rF : (1.0 / context->rTotal))* info->cF  * discrete_current_context->sample_rate);
		context->exponent_cF = 1.0 - exp(context->exponent_cF);
	}

	context->vCapAmp = 0;
	context->exponent_cAmp = 0;
	if (info->cAmp != 0)
	{
		/* Setup filter constants */
		/* We will use 100000 ohms as an average final stage impedance. */
		/* Your amp/speaker system will have more effect on incorrect filtering then any value used here. */
		context->exponent_cAmp = -1.0 / (100000 * info->cAmp  * discrete_current_context->sample_rate);
		context->exponent_cAmp = 1.0 - exp(context->exponent_cAmp);
	}

	if ((context->type & DISC_MIXER_TYPE_MASK) == DISC_MIXER_IS_OP_AMP_WITH_RI) context->gain = info->rF / info->rI;

	node->output = 0;
}


/************************************************************************
 *
 * DST_MULTIPLEX - 1 of x multiplexer/switch
 *
 * input[0]    - Enable input value
 * input[1]    - switch position
 * input[2]    - input[0]
 * input[3]    - input[1]
 * .....
 *
 * Dec 2004, D Renaud.
 ************************************************************************/
#define DST_MULTIPLEX__ENABLE		(*(node->input[0]))
#define DST_MULTIPLEX__ADDR			(*(node->input[1]))
#define DST_MULTIPLEX__INP(addr)	(*(node->input[2 + addr]))

void dst_multiplex_step(node_description *node)
{
	struct dst_size_context *context = node->context;
	int addr;

	if(DST_MULTIPLEX__ENABLE)
	{
		addr = DST_MULTIPLEX__ADDR;	// FP to INT
		if ((addr >= 0) && (addr < context->size))
		{
			node->output = DST_MULTIPLEX__INP(addr);
		}
		else
		{
			/* Bad address.  We will leave the output alone. */
			discrete_log("NODE_%02d - Address = %d. Out of bounds\n",node->node-NODE_00, addr);
		}
	}
	else
	{
		node->output=0;
	}
}

void dst_multiplex_reset(node_description *node)
{
	struct dst_size_context *context = node->context;

	context->size = node->active_inputs - 2;

	dst_multiplex_step(node);
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
#define DST_ONESHOT__RESET	(*(node->input[0]))
#define DST_ONESHOT__TRIG	(*(node->input[1]))
#define DST_ONESHOT__AMP	(*(node->input[2]))
#define DST_ONESHOT__WIDTH	(*(node->input[3]))
#define DST_ONESHOT__TYPE	(int)(*(node->input[4]))

void dst_oneshot_step(node_description *node)
{
	struct dst_oneshot_context *context = node->context;
	int trigger = (DST_ONESHOT__TRIG != 0);

	/* If the state is triggered we will need to countdown later */
	int doCount = context->state;

	if (DST_ONESHOT__RESET)
	{
		/* Hold in Reset */
		node->output = 0;
		context->state = 0;
	}
	else
	{
		/* are we at an edge? */
		if (trigger != context->lastTrig)
		{
			/* There has been a trigger edge */
			context->lastTrig = trigger;

			/* Is it the proper edge trigger */
			if ((DST_ONESHOT__TYPE & DISC_ONESHOT_REDGE) ? trigger : !trigger)
			{
				if (!context->state)
				{
					/* We have first trigger */
					context->state = 1;
					node->output = (DST_ONESHOT__TYPE & DISC_OUT_ACTIVE_LOW) ? 0 : DST_ONESHOT__AMP;
					context->countdown = DST_ONESHOT__WIDTH;
				}
				else
				{
					/* See if we retrigger */
					if (DST_ONESHOT__TYPE & DISC_ONESHOT_RETRIG)
					{
						/* Retrigger */
						context->countdown = DST_ONESHOT__WIDTH;
						doCount = 0;
					}
				}
			}
		}

		if (doCount)
		{
			context->countdown -= discrete_current_context->sample_time;
			if(context->countdown <= 0.0)
			{
				node->output = (DST_ONESHOT__TYPE & DISC_OUT_ACTIVE_LOW) ? DST_ONESHOT__AMP : 0;
				context->countdown = 0;
				context->state = 0;
			}
		}
	}
}


void dst_oneshot_reset(node_description *node)
{
	struct dst_oneshot_context *context = node->context;
	context->countdown = 0;
	context->state = 0;

 	context->lastTrig = 0;
 	node->output = (DST_ONESHOT__TYPE & DISC_OUT_ACTIVE_LOW) ? DST_ONESHOT__AMP : 0;
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
#define DST_RAMP__ENABLE	(*(node->input[0]))
#define DST_RAMP__DIR		(*(node->input[1]))
#define DST_RAMP__GRAD		(*(node->input[2]))
#define DST_RAMP__START		(*(node->input[3]))
#define DST_RAMP__END		(*(node->input[4]))
#define DST_RAMP__CLAMP		(*(node->input[5]))

void dst_ramp_step(node_description *node)
{
	struct dss_ramp_context *context = node->context;

	if(DST_RAMP__ENABLE)
	{
		if (!context->last_en)
		{
			context->last_en = 1;
			node->output = DST_RAMP__START;
		}
		if(context->dir ? DST_RAMP__DIR : !DST_RAMP__DIR) node->output+=context->step;
		else node->output-=context->step;
		/* Clamp to min/max */
		if(context->dir ? (node->output < DST_RAMP__START)
				: (node->output > DST_RAMP__START)) node->output=DST_RAMP__START;
		if(context->dir ? (node->output > DST_RAMP__END)
				: (node->output < DST_RAMP__END)) node->output=DST_RAMP__END;
	}
	else
	{
		context->last_en = 0;
		// Disabled so clamp to output
		node->output=DST_RAMP__CLAMP;
	}
}

void dst_ramp_reset(node_description *node)
{
	struct dss_ramp_context *context = node->context;

	node->output=DST_RAMP__CLAMP;
	context->step = DST_RAMP__GRAD / discrete_current_context->sample_rate;
	context->dir = ((DST_RAMP__END - DST_RAMP__START) == abs(DST_RAMP__END - DST_RAMP__START));
	context->last_en = 0;
}


/************************************************************************
 *
 * DST_SAMPHOLD - Sample & Hold Implementation
 *
 * input[0]    - Enable
 * input[1]    - input[0] value
 * input[2]    - clock node
 * input[3]    - clock type
 *
 ************************************************************************/
#define DST_SAMPHOLD__ENABLE	(*(node->input[0]))
#define DST_SAMPHOLD__IN0		(*(node->input[1]))
#define DST_SAMPHOLD__CLOCK		(*(node->input[2]))
#define DST_SAMPHOLD__TYPE		(*(node->input[3]))

void dst_samphold_step(node_description *node)
{
	struct dst_samphold_context *context = node->context;

	if(DST_SAMPHOLD__ENABLE)
	{
		switch(context->clocktype)
		{
			case DISC_SAMPHOLD_REDGE:
				/* Clock the whole time the input is rising */
				if(DST_SAMPHOLD__CLOCK > context->lastinput) node->output=DST_SAMPHOLD__IN0;
				break;
			case DISC_SAMPHOLD_FEDGE:
				/* Clock the whole time the input is falling */
				if(DST_SAMPHOLD__CLOCK < context->lastinput) node->output=DST_SAMPHOLD__IN0;
				break;
			case DISC_SAMPHOLD_HLATCH:
				/* Output follows input if clock != 0 */
				if(DST_SAMPHOLD__CLOCK) node->output=DST_SAMPHOLD__IN0;
				break;
			case DISC_SAMPHOLD_LLATCH:
				/* Output follows input if clock == 0 */
				if(DST_SAMPHOLD__CLOCK==0) node->output=DST_SAMPHOLD__IN0;
				break;
			default:
				discrete_log("dst_samphold_step - Invalid clocktype passed");
				break;
		}
	}
	else
	{
		node->output=0;
	}
	/* Save the last value */
	context->lastinput=DST_SAMPHOLD__CLOCK;
}

void dst_samphold_reset(node_description *node)
{
	struct dst_samphold_context *context = node->context;

	node->output=0;
	context->lastinput=-1;
	/* Only stored in here to speed up and save casting in the step function */
	context->clocktype=(int)DST_SAMPHOLD__TYPE;
	dst_samphold_step(node);
}


/************************************************************************
 *
 * DSS_SWITCH - Programmable 2 pole switch module with enable function
 *
 * input[0]    - Enable input value
 * input[1]    - switch position
 * input[2]    - input[0]
 * input[3]    - input[1]
 *
 ************************************************************************/
#define DSS_SWITCH__ENABLE	(*(node->input[0]))
#define DSS_SWITCH__SWITCH	(*(node->input[1]))
#define DSS_SWITCH__IN0		(*(node->input[2]))
#define DSS_SWITCH__IN1		(*(node->input[3]))

void dst_switch_step(node_description *node)
{
	if(DSS_SWITCH__ENABLE)
	{
		node->output=DSS_SWITCH__SWITCH ? DSS_SWITCH__IN1 : DSS_SWITCH__IN0;
	}
	else
	{
		node->output=0;
	}
}

/************************************************************************
 *
 * DSS_ASWITCH - Analog switch
 *
 * input[0]    - Enable input value
 * input[1]    - Control
 * input[2]    - Input
 * input[3]    - Threshold for enable
 *
 ************************************************************************/
#define DSS_ASWITCH__ENABLE		(*(node->input[0]))
#define DSS_ASWITCH__CTRL		(*(node->input[1]))
#define DSS_ASWITCH__IN			(*(node->input[2]))
#define DSS_ASWITCH__THRESHOLD	(*(node->input[3]))


void dst_aswitch_step(node_description *node)
{
	if(DSS_SWITCH__ENABLE)
	{
		node->output=DSS_ASWITCH__CTRL > DSS_ASWITCH__THRESHOLD ? DSS_ASWITCH__IN : 0;
	}
	else
	{
		node->output=0;
	}
}

/************************************************************************
 *
 * DST_TRANSFORM - Programmable math module with enable function
 *
 * input[0]    - Enable input value
 * input[1]    - Channel0 input value
 * input[2]    - Channel1 input value
 * input[3]    - Channel2 input value
 * input[4]    - Channel3 input value
 * input[5]    - Channel4 input value
 *
 ************************************************************************/
#define DST_TRANSFORM__ENABLE	(*(node->input[0]))
#define DST_TRANSFORM__IN0		(*(node->input[1]))
#define DST_TRANSFORM__IN1		(*(node->input[2]))
#define DST_TRANSFORM__IN2		(*(node->input[3]))
#define DST_TRANSFORM__IN3		(*(node->input[4]))
#define DST_TRANSFORM__IN4		(*(node->input[5]))

#define MAX_TRANS_STACK	16

double dst_transform_pop(double *stack,int *pointer)
{
	double value;
	//decrement THEN read
	if(*pointer>0) (*pointer)--;
	value=stack[*pointer];
	return value;
}

double dst_transform_push(double *stack,int *pointer,double value)
{
	//Store THEN increment
	if(*pointer<MAX_TRANS_STACK) stack[(*pointer)++]=value;
	return value;
}

void dst_transform_step(node_description *node)
{
	if(DST_TRANSFORM__ENABLE)
	{
		double trans_stack[MAX_TRANS_STACK];
		double result,number1,number2;
		int	trans_stack_ptr=0;

		const char *fPTR = node->custom;
		node->output=0;

		while(*fPTR!=0)
		{
			switch (*fPTR++)
			{
				case '*':
					number2=dst_transform_pop(trans_stack,&trans_stack_ptr);
					number1=dst_transform_pop(trans_stack,&trans_stack_ptr);
					result=number1*number2;
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					break;
				case '/':
					number2=dst_transform_pop(trans_stack,&trans_stack_ptr);
					number1=dst_transform_pop(trans_stack,&trans_stack_ptr);
					result=number1/number2;
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					break;
				case '+':
					number2=dst_transform_pop(trans_stack,&trans_stack_ptr);
					number1=dst_transform_pop(trans_stack,&trans_stack_ptr);
					result=number1+number2;
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					break;
				case '-':
					number2=dst_transform_pop(trans_stack,&trans_stack_ptr);
					number1=dst_transform_pop(trans_stack,&trans_stack_ptr);
					result=number1-number2;
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					break;
				case '0':
					dst_transform_push(trans_stack,&trans_stack_ptr,DST_TRANSFORM__IN0);
					break;
				case '1':
					dst_transform_push(trans_stack,&trans_stack_ptr,DST_TRANSFORM__IN1);
					break;
				case '2':
					dst_transform_push(trans_stack,&trans_stack_ptr,DST_TRANSFORM__IN2);
					break;
				case '3':
					dst_transform_push(trans_stack,&trans_stack_ptr,DST_TRANSFORM__IN3);
					break;
				case '4':
					dst_transform_push(trans_stack,&trans_stack_ptr,DST_TRANSFORM__IN4);
					break;
				case 'P':
					result=dst_transform_pop(trans_stack,&trans_stack_ptr);
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					break;
				case 'i':	// * -1
					number1=dst_transform_pop(trans_stack,&trans_stack_ptr);
					result=-number1;
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					break;
				case '!':	// Logical NOT of Last Value
					number1=dst_transform_pop(trans_stack,&trans_stack_ptr);
					result=!number1;
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					break;
				case '=':	// Logical =
					number2=dst_transform_pop(trans_stack,&trans_stack_ptr);
					number1=dst_transform_pop(trans_stack,&trans_stack_ptr);
					result=(int)number1 == (int)number2;
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					break;
				case '>':	// Logical >
					number2=dst_transform_pop(trans_stack,&trans_stack_ptr);
					number1=dst_transform_pop(trans_stack,&trans_stack_ptr);
					result=number1 > number2;
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					break;
				case '<':	// Logical <
					number2=dst_transform_pop(trans_stack,&trans_stack_ptr);
					number1=dst_transform_pop(trans_stack,&trans_stack_ptr);
					result=number1 < number2;
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					break;
				case '&':	// Bitwise AND
					number2=dst_transform_pop(trans_stack,&trans_stack_ptr);
					number1=dst_transform_pop(trans_stack,&trans_stack_ptr);
					result=(int)number1 & (int)number2;
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					break;
				case '|':	// Bitwise OR
					number2=dst_transform_pop(trans_stack,&trans_stack_ptr);
					number1=dst_transform_pop(trans_stack,&trans_stack_ptr);
					result=(int)number1 | (int)number2;
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					break;
				case '^':	// Bitwise XOR
					number2=dst_transform_pop(trans_stack,&trans_stack_ptr);
					number1=dst_transform_pop(trans_stack,&trans_stack_ptr);
					result=(int)number1 ^ (int)number2;
					dst_transform_push(trans_stack,&trans_stack_ptr,result);
					break;
				default:
					discrete_log("dst_transform_step - Invalid function type/variable passed");
					node->output = 0;
					break;
			}
		}
		node->output=dst_transform_pop(trans_stack,&trans_stack_ptr);
	}
	else
	{
		node->output=0;
	}
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
#define DST_OP_AMP__ENABLE	(*(node->input[0]))
#define DST_OP_AMP__INP0	(*(node->input[1]))
#define DST_OP_AMP__INP1	(*(node->input[2]))

void dst_op_amp_step(node_description *node)
{
	const discrete_op_amp_info *info = node->custom;
	struct dst_op_amp_context *context = node->context;

	double iPos = 0;
	double iNeg = 0;
	double i = 0;

	if (DST_OP_AMP__ENABLE)
	{
		switch (info->type)
		{
			case DISC_OP_AMP_IS_NORTON:
				/* work out neg pin current */
				if  (context->has_r1)
				{
					iNeg = (DST_OP_AMP__INP0 - OP_AMP_NORTON_VBE) / info->r1;
					if (iNeg < 0) iNeg = 0;
				}
				iNeg += context->iFixed;

				/* work out neg pin current */
				iPos = (DST_OP_AMP__INP1 - OP_AMP_NORTON_VBE) / info->r2;
				if (iPos < 0) iPos = 0;

				/* work out current across r4 */
				i = iPos - iNeg;

				if (context->has_cap)
				{
					if (context->has_r4)
					{
						/* voltage across r4 charging cap */
						i *= info->r4;
						/* exponential charge */
						context->vCap += (i - context->vCap) * context->exponent;
					}
					else
						/* linear charge */
						context->vCap += i / context->exponent;
					node->output = context->vCap;
				}
				else
					node->output = i * info->r4;

				/* clamp output */
				if (node->output > context->vMax) node->output = context->vMax;
				else if (node->output < info->vN) node->output = info->vN;
				context->vCap = node->output;
				break;

			default:
				node->output = 0;
		}
	}
	else
		node->output = 0;
}

void dst_op_amp_reset(node_description *node)
{
	const discrete_op_amp_info *info = node->custom;
	struct dst_op_amp_context *context = node->context;

	context->has_r1 = info->r1 > 0;
	context->has_r4 = info->r4 > 0;

	context->vMax = info->vP - OP_AMP_NORTON_VBE;

	context->vCap = 0;
	if (info->c > 0)
	{
		context->has_cap = 1;
		/* Setup filter constants */
		if (context->has_r4)
		{
			/* exponential charge */
			context->exponent = -1.0 / (info->r4 * info->c  * discrete_current_context->sample_rate);
			context->exponent = 1.0 - exp(context->exponent);
		}
		else
			/* linear charge */
			context->exponent = discrete_current_context->sample_rate * info->c;
	}

	if (info->r3 >= 0)
		context->iFixed = (info->vP - OP_AMP_NORTON_VBE) / info->r3;
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
#define DST_OP_AMP_1SHT__TRIGGER	(*(node->input[0]))

void dst_op_amp_1sht_step(node_description *node)
{
	const discrete_op_amp_1sht_info *info = node->custom;
	struct dst_op_amp_1sht_context *context = node->context;

	double iPos;
	double iNeg;
	double v;

	/* update trigger circuit */
	iPos = (DST_OP_AMP_1SHT__TRIGGER - context->vCap2) / info->r2;
	iPos += node->output / info->r5;
	context->vCap2 += (DST_OP_AMP_1SHT__TRIGGER - context->vCap2) * context->exponent2;

	/* calculate currents and output */
	iNeg = (context->vCap1 - OP_AMP_NORTON_VBE) / info->r3;
	if (iNeg < 0) iNeg = 0;
	iNeg += context->iFixed;

	if (iPos > iNeg) node->output = context->vMax;
	else node->output = info->vN;

	/* update c1 */
	/* rough value of voltage at anode of diode if discharging */
	v = node->output + 0.6;
	if (context->vCap1 > node->output)
	{
		/* discharge */
		if (context->vCap1 > v)
			/* immediate discharge through diode */
			context->vCap1 = v;
		else
			/* discharge through r4 */
			context->vCap1 += (node->output - context->vCap1) * context->exponent1d;
	}
	else
		/* charge */
		context->vCap1 += ((node->output - OP_AMP_NORTON_VBE) * context->r34ratio + OP_AMP_NORTON_VBE - context->vCap1) * context->exponent1c;
}

void dst_op_amp_1sht_reset(node_description *node)
{
	const discrete_op_amp_1sht_info *info = node->custom;
	struct dst_op_amp_1sht_context *context = node->context;

	context->exponent1c = -1.0 / ((1.0 / (1.0 / info->r3 + 1.0 / info->r4)) * info->c1  * discrete_current_context->sample_rate);
	context->exponent1c = 1.0 - exp(context->exponent1c);
	context->exponent1d = -1.0 / (info->r4 * info->c1  * discrete_current_context->sample_rate);
	context->exponent1d = 1.0 - exp(context->exponent1d);
	context->exponent2 = -1.0 / (info->r2 * info->c2  * discrete_current_context->sample_rate);
	context->exponent2 = 1.0 - exp(context->exponent2);
	context->iFixed = (info->vP - OP_AMP_NORTON_VBE) / info->r1;
	context->vCap1 = context->vCap2 = 0;
	context->vMax = info->vP - OP_AMP_NORTON_VBE;
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
#define DST_TVCA_OP_AMP__TRG0	(*(node->input[0]))
#define DST_TVCA_OP_AMP__TRG1	(*(node->input[1]))
#define DST_TVCA_OP_AMP__TRG2	(*(node->input[2]))
#define DST_TVCA_OP_AMP__INP0	(*(node->input[3]))
#define DST_TVCA_OP_AMP__INP1	(*(node->input[4]))

void dst_tvca_op_amp_step(node_description *node)
{
	const discrete_op_amp_tvca_info *info = node->custom;
	struct dst_tvca_op_amp_context *context = node->context;

	int		trig0, trig1, trig2, f3;
	double	i2 = 0;		// current through r2
	double	i3 = 0;		// current through r3
	double	iNeg = 0;	// current into - input
	double	iPos = 0;	// current into + input
	double	iOut = 0;	// current at output

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
	iNeg = context->iFixed + i2 + i3;

	/* Update the c1 cap voltage. */
	if (dst_trigger_function(trig0, trig1, trig2, info->f2))
	{
		/* F2 is not grounding the circuit so we charge the cap. */
		context->vCap1 += (context->vTrig[f3] - context->vCap1) * context->exponentC[f3];
	}
	else
	{
		/* F2 is at ground.  The diode blocks this so F2 and r5 are out of circuit.
         * So now the discharge rate is dependent upon F3.
         * If F3 is at ground then we discharge to 0V through r6.
         * If F3 is out of circuit then we discharge to OP_AMP_NORTON_VBE through r6+r7. */
 		context->vCap1 += ((f3 ? OP_AMP_NORTON_VBE : 0.0) - context->vCap1) * context->exponentD[f3];
	}

	/* Calculate c1 current going in to + input. */
	iPos = (context->vCap1 - OP_AMP_NORTON_VBE) / context->r67;
	if ((iPos < 0) || !f3) iPos = 0;

	/* Update the c2 cap voltage and current. */
	if (info->r9 != 0)
	{
		f3 = dst_trigger_function(trig0, trig1, trig2, info->f4);
		context->vCap2 += ((f3 ? context->vTrig2 : 0) - context->vCap2) * context->exponent2[f3];
		iPos += context->vCap2 / info->r9;
	}

	/* Update the c3 cap voltage and current. */
	if (info->r11 != 0)
	{
		f3 = dst_trigger_function(trig0, trig1, trig2, info->f5);
		context->vCap3 += ((f3 ? context->vTrig3 : 0) - context->vCap3) * context->exponent3[f3];
		iPos += context->vCap3 / info->r11;
	}


	/* Calculate output current. */
	iOut = iPos - iNeg;
	if (iOut < 0) iOut = 0;
	/* Convert to voltage for final output. */
	node->output = iOut * info->r4;
	/* Clip the output if needed. */
	if (node->output > context->vOutMax) node->output = context->vOutMax;
}

void dst_tvca_op_amp_reset(node_description *node)
{
	const discrete_op_amp_tvca_info *info = node->custom;
	struct dst_tvca_op_amp_context *context = node->context;

	context->r67 = info->r6 + info->r7;

	context->vOutMax = info->vP - OP_AMP_NORTON_VBE;
	/* This is probably overkill because R5 is usually much lower then r6 or r7,
     * but it is better to play it safe. */
	context->vTrig[0] = (info->v1 - 0.6) * (info->r6 / (info->r6 + info->r5));
	context->vTrig[1] = (info->v1 - 0.6 - OP_AMP_NORTON_VBE) * (context->r67 / (context->r67 + info->r5)) + OP_AMP_NORTON_VBE;
	context->iFixed = context->vOutMax / info->r1;

	context->vCap1 = 0;
	/* Charge rate thru r5 */
	/* There can be a different charge rates depending on function F3. */
	context->exponentC[0] = -1.0 / ((1.0 / (1.0 / info->r5 + 1.0 / info->r6)) * info->c1 * discrete_current_context->sample_rate);
	context->exponentC[0] = 1.0 - exp(context->exponentC[0]);
	context->exponentC[1] = -1.0 / ((1.0 / (1.0 / info->r5 + 1.0 / context->r67)) * info->c1 * discrete_current_context->sample_rate);
	context->exponentC[1] = 1.0 - exp(context->exponentC[1]);
	/* Discharge rate thru r6 + r7 */
	context->exponentD[1] = -1.0 / (context->r67 * info->c1 * discrete_current_context->sample_rate);
	context->exponentD[1] = 1.0 - exp(context->exponentD[1]);
	/* Discharge rate thru r6 */
	if (info->r6 != 0)
	{
		context->exponentD[0] = -1.0 / (info->r6 * info->c1 * discrete_current_context->sample_rate);
		context->exponentD[0] = 1.0 - exp(context->exponentD[0]);
	}
	context->vCap2 = 0;
	context->vTrig2 = (info->v2 - 0.6 - OP_AMP_NORTON_VBE) * (info->r9 / (info->r8 + info->r9));
	context->exponent2[0] = -1.0 / (info->r9 * info->c2 * discrete_current_context->sample_rate);
	context->exponent2[0] = 1.0 - exp(context->exponent2[0]);
	context->exponent2[1] = -1.0 / ((1.0 / (1.0 / info->r8 + 1.0 / info->r9)) * info->c2 * discrete_current_context->sample_rate);
	context->exponent2[1] = 1.0 - exp(context->exponent2[1]);
	context->vCap3 = 0;
	context->vTrig3 = (info->v3 - 0.6 - OP_AMP_NORTON_VBE) * (info->r11 / (info->r10 + info->r11));
	context->exponent3[0] = -1.0 / (info->r11 * info->c3 * discrete_current_context->sample_rate);
	context->exponent3[0] = 1.0 - exp(context->exponent3[0]);
	context->exponent3[1] = -1.0 / ((1.0 / (1.0 / info->r10 + 1.0 / info->r11)) * info->c3 * discrete_current_context->sample_rate);
	context->exponent3[1] = 1.0 - exp(context->exponent3[1]);

	dst_tvca_op_amp_step(node);
}
