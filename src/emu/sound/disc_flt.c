/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by Keith Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *
 ***********************************************************************
 *
 * DST_CRFILTER          - Simple CR filter & also highpass filter
 * DST_FILTER1           - Generic 1st order filter
 * DST_FILTER2           - Generic 2nd order filter
 * DST_OP_AMP_FILT       - Op Amp filter circuits
 * DST_RC_CIRCUIT_1      - RC charge/discharge circuit
 * DST_RCDISC            - Simple discharging RC
 * DST_RCDISC2           - Simple charge R1/C, discharge R0/C
 * DST_RCDISC3           - Simple charge R1/c, discharge R0*R1/(R0+R1)/C
 * DST_RCDISC4           - Various charge/discharge circuits
 * DST_RCDISC5           - Diode in series with R//C
 * DST_RCDISC_MOD        - RC triggered by logic and modulated
 * DST_RCFILTER          - Simple RC filter & also lowpass filter
 * DST_RCFILTER_SW       - Usage of node_description values for switchable RC filter
 * DST_RCINTEGRATE       - Two diode inputs, transistor and a R/C charge
 *                         discharge network
 * DST_SALLEN_KEY        - Sallen-Key filter circuit
 *
 ************************************************************************/

struct dss_filter1_context
{
	double x1;		/* x[k-1], previous input value */
	double y1;		/* y[k-1], previous output value */
	double a1;		/* digital filter coefficients, denominator */
	double b0, b1;		/* digital filter coefficients, numerator */
};

struct dss_filter2_context
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

/************************************************************************
 *
 * DST_CRFILTER - Usage of node_description values for CR filter
 *
 * input[0]    - Enable input value
 * input[1]    - input value
 * input[2]    - Resistor value (initialization only)
 * input[3]    - Capacitor Value (initialization only)
 * input[4]    - Voltage reference. Usually 0V.
 *
 ************************************************************************/
#define DST_CRFILTER__IN		DISCRETE_INPUT(0)
#define DST_CRFILTER__R			DISCRETE_INPUT(1)
#define DST_CRFILTER__C			DISCRETE_INPUT(2)
#define DST_CRFILTER__VREF		DISCRETE_INPUT(3)

static DISCRETE_STEP(dst_crfilter)
{
	struct dst_rcfilter_context *context = (struct dst_rcfilter_context *)node->context;

	if (UNEXPECTED(context->has_rc_nodes))
	{
		double rc = DST_CRFILTER__R * DST_CRFILTER__C;
		if (rc != context->rc)
		{
			context->rc = rc;
			context->exponent = RC_CHARGE_EXP(rc);
		}
	}

	node->output[0] = DST_CRFILTER__IN - context->vCap;
	//context->vCap += ((DST_CRFILTER__IN - context->vRef) - context->vCap) * context->exponent;
	context->vCap += (node->output[0] - DST_CRFILTER__VREF) * context->exponent;
}

static DISCRETE_RESET(dst_crfilter)
{
	struct dst_rcfilter_context *context = (struct dst_rcfilter_context *)node->context;

	context->has_rc_nodes = node->input_is_node & 0x6;
	context->rc = DST_CRFILTER__R * DST_CRFILTER__C;
	context->exponent = RC_CHARGE_EXP(context->rc);
	context->vCap = 0;
	node->output[0] = DST_CRFILTER__IN;
}


/************************************************************************
 *
 * DST_FILTER1 - Generic 1st order filter
 *
 * input[0]    - Enable input value
 * input[1]    - input value
 * input[2]    - Frequency value (initialization only)
 * input[3]    - Filter type (initialization only)
 *
 ************************************************************************/
#define DST_FILTER1__ENABLE	DISCRETE_INPUT(0)
#define DST_FILTER1__IN		DISCRETE_INPUT(1)
#define DST_FILTER1__FREQ	DISCRETE_INPUT(2)
#define DST_FILTER1__TYPE	DISCRETE_INPUT(3)

static void calculate_filter1_coefficients(const discrete_info *disc_info, double fc, double type,
										   double *a1, double *b0, double *b1)
{
	double den, w, two_over_T;

	/* calculate digital filter coefficents */
	/*w = 2.0*M_PI*fc; no pre-warping */
	w = disc_info->sample_rate*2.0*tan(M_PI*fc/disc_info->sample_rate); /* pre-warping */
	two_over_T = 2.0*disc_info->sample_rate;

	den = w + two_over_T;
	*a1 = (w - two_over_T)/den;
	if (type == DISC_FILTER_LOWPASS)
	{
		*b0 = *b1 = w/den;
	}
	else if (type == DISC_FILTER_HIGHPASS)
	{
		*b0 = two_over_T/den;
		*b1 = -(*b0);
	}
	else
	{
		discrete_log(disc_info, "calculate_filter1_coefficients() - Invalid filter type for 1st order filter.");
	}
}

static DISCRETE_STEP(dst_filter1)
{
	struct dss_filter1_context *context = (struct dss_filter1_context *)node->context;

	double gain = 1.0;

	if (DST_FILTER1__ENABLE == 0.0)
	{
		gain = 0.0;
	}

	node->output[0] = -context->a1*context->y1 + context->b0*gain*DST_FILTER1__IN + context->b1*context->x1;

	context->x1 = gain*DST_FILTER1__IN;
	context->y1 = node->output[0];
}

static DISCRETE_RESET(dst_filter1)
{
	struct dss_filter1_context *context = (struct dss_filter1_context *)node->context;

	calculate_filter1_coefficients(node->info, DST_FILTER1__FREQ, DST_FILTER1__TYPE, &context->a1, &context->b0, &context->b1);
	node->output[0] = 0;
}


/************************************************************************
 *
 * DST_FILTER2 - Generic 2nd order filter
 *
 * input[0]    - Enable input value
 * input[1]    - input value
 * input[2]    - Frequency value (initialization only)
 * input[3]    - Damping value (initialization only)
 * input[4]    - Filter type (initialization only)
 *
 ************************************************************************/
#define DST_FILTER2__ENABLE	DISCRETE_INPUT(0)
#define DST_FILTER2__IN		DISCRETE_INPUT(1)
#define DST_FILTER2__FREQ	DISCRETE_INPUT(2)
#define DST_FILTER2__DAMP	DISCRETE_INPUT(3)
#define DST_FILTER2__TYPE	DISCRETE_INPUT(4)

static void calculate_filter2_coefficients(const discrete_info *disc_info,
		                                   double fc, double d, double type,
										   double *a1, double *a2,
										   double *b0, double *b1, double *b2)
{
	double w;	/* cutoff freq, in radians/sec */
	double w_squared;
	double den;	/* temp variable */
	double two_over_T = 2 * disc_info->sample_rate;
	double two_over_T_squared = two_over_T * two_over_T;

	/* calculate digital filter coefficents */
	/*w = 2.0*M_PI*fc; no pre-warping */
	w = disc_info->sample_rate * 2.0 * tan(M_PI * fc / disc_info->sample_rate); /* pre-warping */
	w_squared = w * w;

	den = two_over_T_squared + d*w*two_over_T + w_squared;

	*a1 = 2.0 * (-two_over_T_squared + w_squared) / den;
	*a2 = (two_over_T_squared - d * w * two_over_T + w_squared) / den;

	if (type == DISC_FILTER_LOWPASS)
	{
		*b0 = *b2 = w_squared/den;
		*b1 = 2.0 * (*b0);
	}
	else if (type == DISC_FILTER_BANDPASS)
	{
		*b0 = d * w * two_over_T / den;
		*b1 = 0.0;
		*b2 = -(*b0);
	}
	else if (type == DISC_FILTER_HIGHPASS)
	{
		*b0 = *b2 = two_over_T_squared / den;
		*b1 = -2.0 * (*b0);
	}
	else
	{
		discrete_log(disc_info, "calculate_filter2_coefficients() - Invalid filter type for 2nd order filter.");
	}
}

static DISCRETE_STEP(dst_filter2)
{
	struct dss_filter2_context *context = (struct dss_filter2_context *)node->context;

	double gain = 1.0;

	if (DST_FILTER2__ENABLE == 0.0)
	{
		gain = 0.0;
	}

	node->output[0] = -context->a1 * context->y1 - context->a2 * context->y2 +
					context->b0 * gain * DST_FILTER2__IN + context->b1 * context->x1 + context->b2 * context->x2;

	context->x2 = context->x1;
	context->x1 = gain * DST_FILTER2__IN;
	context->y2 = context->y1;
	context->y1 = node->output[0];
}

static DISCRETE_RESET(dst_filter2)
{
	struct dss_filter2_context *context = (struct dss_filter2_context *)node->context;

	calculate_filter2_coefficients(node->info, DST_FILTER2__FREQ, DST_FILTER2__DAMP, DST_FILTER2__TYPE,
								   &context->a1, &context->a2,
								   &context->b0, &context->b1, &context->b2);
	node->output[0] = 0;
}


/************************************************************************
 *
 * DST_OP_AMP_FILT - Op Amp filter circuit RC filter
 *
 * input[0]    - Enable input value
 * input[1]    - IN0 node
 * input[2]    - IN1 node
 * input[3]    - Filter Type
 *
 * also passed discrete_op_amp_filt_info structure
 *
 * Mar 2004, D Renaud.
 ************************************************************************/
#define DST_OP_AMP_FILT__ENABLE	DISCRETE_INPUT(0)
#define DST_OP_AMP_FILT__INP1	DISCRETE_INPUT(1)
#define DST_OP_AMP_FILT__INP2	DISCRETE_INPUT(2)
#define DST_OP_AMP_FILT__TYPE	DISCRETE_INPUT(3)

static DISCRETE_STEP(dst_op_amp_filt)
{
	const  discrete_op_amp_filt_info *info    = (const  discrete_op_amp_filt_info *)node->custom;
	struct dst_op_amp_filt_context   *context = (struct dst_op_amp_filt_context *)node->context;

	double i, v = 0;

	if (DST_OP_AMP_FILT__ENABLE)
	{
		if (context->is_norton)
		{
			v = DST_OP_AMP_FILT__INP1 - OP_AMP_NORTON_VBE;
			if (v < 0) v = 0;
		}
		else
		{
			/* Millman the input voltages. */
			i  = context->iFixed;
			switch (context->type)
			{
				case DISC_OP_AMP_FILTER_IS_LOW_PASS_1_A:
					i += (DST_OP_AMP_FILT__INP1 - DST_OP_AMP_FILT__INP2) / info->r1;
					if (info->r2 != 0)
						i += (context->vP - DST_OP_AMP_FILT__INP2) / info->r2;
					if (info->r3 != 0)
						i += (context->vN - DST_OP_AMP_FILT__INP2) / info->r3;
					break;
				default:
					i += (DST_OP_AMP_FILT__INP1 - context->vRef) / info->r1;
					if (info->r2 != 0)
						i += (DST_OP_AMP_FILT__INP2 - context->vRef) / info->r2;
					break;
			}
			v = i * context->rTotal;
		}

		switch (context->type)
		{
			case DISC_OP_AMP_FILTER_IS_LOW_PASS_1:
				context->vC1 += (v - context->vC1) * context->exponentC1;
				node->output[0] = context->vC1 * context->gain + info->vRef;
				break;

			case DISC_OP_AMP_FILTER_IS_LOW_PASS_1_A:
				context->vC1 += (v - context->vC1) * context->exponentC1;
				node->output[0] = context->vC1 * context->gain + DST_OP_AMP_FILT__INP2;
				break;

			case DISC_OP_AMP_FILTER_IS_HIGH_PASS_1:
				node->output[0] = (v - context->vC1) * context->gain + info->vRef;
				context->vC1 += (v - context->vC1) * context->exponentC1;
				break;

			case DISC_OP_AMP_FILTER_IS_BAND_PASS_1:
				node->output[0] = (v - context->vC2);
				context->vC2 += (v - context->vC2) * context->exponentC2;
				context->vC1 += (node->output[0] - context->vC1) * context->exponentC1;
				node->output[0] = context->vC1 * context->gain + info->vRef;
				break;

			case DISC_OP_AMP_FILTER_IS_BAND_PASS_0 | DISC_OP_AMP_IS_NORTON:
				context->vC1 += (v - context->vC1) * context->exponentC1;
				context->vC2 += (context->vC1 - context->vC2) * context->exponentC2;
				v = context->vC2;
				node->output[0] = v - context->vC3;
				context->vC3 += (v - context->vC3) * context->exponentC3;
				i = node->output[0] / context->rTotal;
				node->output[0] = (context->iFixed - i) * info->rF;
				break;

			case DISC_OP_AMP_FILTER_IS_HIGH_PASS_0 | DISC_OP_AMP_IS_NORTON:
				node->output[0] = v - context->vC1;
				context->vC1 += (v - context->vC1) * context->exponentC1;
				i = node->output[0] / context->rTotal;
				node->output[0] = (context->iFixed - i) * info->rF;
				break;

			case DISC_OP_AMP_FILTER_IS_BAND_PASS_1M:
			case DISC_OP_AMP_FILTER_IS_BAND_PASS_1M | DISC_OP_AMP_IS_NORTON:
				node->output[0] = -context->a1 * context->y1 - context->a2 * context->y2 +
								context->b0 * v + context->b1 * context->x1 + context->b2 * context->x2 +
								context->vRef;
				context->x2 = context->x1;
				context->x1 = v;
				context->y2 = context->y1;
				break;
		}

		/* Clip the output to the voltage rails.
         * This way we get the original distortion in all it's glory.
         */
		if (node->output[0] > context->vP) node->output[0] = context->vP;
		if (node->output[0] < context->vN) node->output[0] = context->vN;
		context->y1 = node->output[0] - context->vRef;
	}
	else
		node->output[0] = 0;

}

static DISCRETE_RESET(dst_op_amp_filt)
{
	const  discrete_op_amp_filt_info *info    = (const  discrete_op_amp_filt_info *)node->custom;
	struct dst_op_amp_filt_context   *context = (struct dst_op_amp_filt_context *)node->context;

	/* Convert the passed filter type into an int for easy use. */
	context->type = (int)DST_OP_AMP_FILT__TYPE & DISC_OP_AMP_FILTER_TYPE_MASK;
	context->is_norton = (int)DST_OP_AMP_FILT__TYPE & DISC_OP_AMP_IS_NORTON;

	if (context->is_norton)
	{
		context->vRef = 0;
		context->rTotal = info->r1;
		if (context->type == (DISC_OP_AMP_FILTER_IS_BAND_PASS_0 | DISC_OP_AMP_IS_NORTON))
			context->rTotal += info->r2 +  info->r3;

		/* Setup the current to the + input. */
		context->iFixed = (info->vP - OP_AMP_NORTON_VBE) / info->r4;

		/* Set the output max. */
		context->vP =  info->vP - OP_AMP_NORTON_VBE;
		context->vN =  info->vN;
	}
	else
	{
		context->vRef = info->vRef;
		/* Set the output max. */
		context->vP =  info->vP - OP_AMP_VP_RAIL_OFFSET;
		context->vN =  info->vN;

		/* Work out the input resistance.  It is all input and bias resistors in parallel. */
		context->rTotal  = 1.0 / info->r1;			/* There has to be an R1.  Otherwise the table is wrong. */
		if (info->r2 != 0) context->rTotal += 1.0 / info->r2;
		if (info->r3 != 0) context->rTotal += 1.0 / info->r3;
		context->rTotal = 1.0 / context->rTotal;

		context->iFixed = 0;

		context->rRatio = info->rF / (context->rTotal + info->rF);
		context->gain = -info->rF / context->rTotal;
	}

	switch (context->type)
	{
		case DISC_OP_AMP_FILTER_IS_LOW_PASS_1:
		case DISC_OP_AMP_FILTER_IS_LOW_PASS_1_A:
			context->exponentC1 = RC_CHARGE_EXP(info->rF * info->c1);
			context->exponentC2 =  0;
			break;
		case DISC_OP_AMP_FILTER_IS_HIGH_PASS_1:
			context->exponentC1 = RC_CHARGE_EXP(context->rTotal * info->c1);
			context->exponentC2 =  0;
			break;
		case DISC_OP_AMP_FILTER_IS_BAND_PASS_1:
			context->exponentC1 = RC_CHARGE_EXP(info->rF * info->c1);
			context->exponentC2 = RC_CHARGE_EXP(context->rTotal * info->c2);
			break;
		case DISC_OP_AMP_FILTER_IS_BAND_PASS_1M | DISC_OP_AMP_IS_NORTON:
			if (info->r2 == 0)
				context->rTotal = info->r1;
			else
				context->rTotal = RES_2_PARALLEL(info->r1, info->r2);
		case DISC_OP_AMP_FILTER_IS_BAND_PASS_1M:
		{
			double fc = 1.0 / (2 * M_PI * sqrt(context->rTotal * info->rF * info->c1 * info->c2));
			double d  = (info->c1 + info->c2) / sqrt(info->rF / context->rTotal * info->c1 * info->c2);
			double gain = -info->rF / context->rTotal * info->c2 / (info->c1 + info->c2);

			calculate_filter2_coefficients(node->info, fc, d, DISC_FILTER_BANDPASS,
										   &context->a1, &context->a2,
										   &context->b0, &context->b1, &context->b2);
			context->b0 *= gain;
			context->b1 *= gain;
			context->b2 *= gain;

			if (context->is_norton)
				context->vRef = (info->vP - OP_AMP_NORTON_VBE) / info->r3 * info->rF;
			else
				context->vRef = info->vRef;

			break;
		}
		case DISC_OP_AMP_FILTER_IS_BAND_PASS_0 | DISC_OP_AMP_IS_NORTON:
			context->exponentC1 = RC_CHARGE_EXP(RES_2_PARALLEL(info->r1, info->r2 + info->r3 + info->r4) * info->c1);
			context->exponentC2 = RC_CHARGE_EXP(RES_2_PARALLEL(info->r1 + info->r2, info->r3 + info->r4) * info->c2);
			context->exponentC3 = RC_CHARGE_EXP((info->r1 + info->r2 + info->r3 + info->r4) * info->c3);
			break;
		case DISC_OP_AMP_FILTER_IS_HIGH_PASS_0 | DISC_OP_AMP_IS_NORTON:
			context->exponentC1 = RC_CHARGE_EXP(info->r1 * info->c1);
			break;
	}

	/* At startup there is no charge on the caps and output is 0V in relation to vRef. */
	context->vC1 = 0;
	context->vC1b = 0;
	context->vC2 = 0;
	context->vC3 = 0;

	node->output[0] = info->vRef;
}


/************************************************************************
 *
 * DST_RC_CIRCUIT_1 - RC charge/discharge circuit
 *
 ************************************************************************/
#define DST_RC_CIRCUIT_1__IN0		DISCRETE_INPUT(0)
#define DST_RC_CIRCUIT_1__IN1		DISCRETE_INPUT(1)
#define DST_RC_CIRCUIT_1__R			DISCRETE_INPUT(2)
#define DST_RC_CIRCUIT_1__C			DISCRETE_INPUT(3)

#define CD4066_R_ON	270

static DISCRETE_STEP( dst_rc_circuit_1 )
{
	struct dst_rc_circuit_1_context *context = (struct dst_rc_circuit_1_context *)node->context;

	if (DST_RC_CIRCUIT_1__IN0 == 0)
		if (DST_RC_CIRCUIT_1__IN1 == 0)
			/* cap is floating and does not change charge */
			/* output is pulled to ground */
			node->output[0] = 0;
		else
		{
			/* cap is discharged */
			context->v_cap -= context->v_cap * context->exp_2;
			node->output[0] = context->v_cap * context->v_drop;
		}
	else
		if (DST_RC_CIRCUIT_1__IN1 == 0)
		{
			/* cap is charged */
			context->v_cap += (5.0 - context->v_cap) * context->exp_1;
			/* output is pulled to ground */
			node->output[0] = 0;
		}
		else
		{
			/* cap is charged slightly less */
			context->v_cap += (context->v_charge_1_2 - context->v_cap) * context->exp_1_2;
			node->output[0] = context->v_cap * context->v_drop;
		}
}

static DISCRETE_RESET( dst_rc_circuit_1 )
{
	struct dst_rc_circuit_1_context *context = (struct dst_rc_circuit_1_context *)node->context;

	/* the charging voltage across the cap based on in2*/
	context->v_drop =  RES_VOLTAGE_DIVIDER(CD4066_R_ON, CD4066_R_ON + DST_RC_CIRCUIT_1__R);
	context->v_charge_1_2 = 5.0 * context->v_drop;
	context->v_cap = 0;

	/* precalculate charging exponents */
	/* discharge cap - in1 = 0, in2 = 1*/
	context->exp_2 = RC_CHARGE_EXP((CD4066_R_ON + DST_RC_CIRCUIT_1__R) * DST_RC_CIRCUIT_1__C);
	/* charge cap - in1 = 1, in2 = 0 */
	context->exp_1 = RC_CHARGE_EXP(CD4066_R_ON * DST_RC_CIRCUIT_1__C);
	/* charge cap - in1 = 1, in2 = 1 */
	context->exp_1_2 = RC_CHARGE_EXP(RES_2_PARALLEL(CD4066_R_ON, CD4066_R_ON + DST_RC_CIRCUIT_1__R) * DST_RC_CIRCUIT_1__C);

	/* starts at 0 until cap starts charging */
	node->output[0] = 0;
}

/************************************************************************
 *
 * DST_RCDISC -   Usage of node_description values for RC discharge
 *                (inverse slope of DST_RCFILTER)
 *
 * input[0]    - Enable input value
 * input[1]    - input value
 * input[2]    - Resistor value (initialization only)
 * input[3]    - Capacitor Value (initialization only)
 *
 ************************************************************************/
#define DST_RCDISC__ENABLE	DISCRETE_INPUT(0)
#define DST_RCDISC__IN		DISCRETE_INPUT(1)
#define DST_RCDISC__R		DISCRETE_INPUT(2)
#define DST_RCDISC__C		DISCRETE_INPUT(3)

static DISCRETE_STEP(dst_rcdisc)
{
	struct dst_rcdisc_context *context = (struct dst_rcdisc_context *)node->context;

	switch (context->state)
	{
		case 0:     /* waiting for trigger  */
			if(DST_RCDISC__ENABLE)
			{
				context->state = 1;
				context->t = 0;
			}
			node->output[0] = 0;
			break;

		case 1:
			if (DST_RCDISC__ENABLE)
			{
				node->output[0] = DST_RCDISC__IN * exp(context->t / context->exponent0);
				context->t += node->info->sample_time;
			} else
			{
				context->state = 0;
			}
	}
}

static DISCRETE_RESET(dst_rcdisc)
{
	struct dst_rcdisc_context *context = (struct dst_rcdisc_context *)node->context;

	node->output[0] = 0;

	context->state = 0;
	context->t = 0;
	context->exponent0=-1.0 * DST_RCDISC__R * DST_RCDISC__C;
}


/************************************************************************
 *
 * DST_RCDISC2 -  Usage of node_description values for RC discharge
 *                Has switchable charge resistor/input
 *
 * input[0]    - Switch input value
 * input[1]    - input[0] value
 * input[2]    - Resistor0 value (initialization only)
 * input[3]    - input[1] value
 * input[4]    - Resistor1 value (initialization only)
 * input[5]    - Capacitor Value (initialization only)
 *
 ************************************************************************/
#define DST_RCDISC2__ENABLE	DISCRETE_INPUT(0)
#define DST_RCDISC2__IN0	DISCRETE_INPUT(1)
#define DST_RCDISC2__R0		DISCRETE_INPUT(2)
#define DST_RCDISC2__IN1	DISCRETE_INPUT(3)
#define DST_RCDISC2__R1		DISCRETE_INPUT(4)
#define DST_RCDISC2__C		DISCRETE_INPUT(5)

static DISCRETE_STEP(dst_rcdisc2)
{
	struct dst_rcdisc_context *context = (struct dst_rcdisc_context *)node->context;

	double diff;

	/* Works differently to other as we are always on, no enable */
	/* exponential based in difference between input/output   */

	diff = ((DST_RCDISC2__ENABLE == 0) ? DST_RCDISC2__IN0 : DST_RCDISC2__IN1) - node->output[0];
	diff = diff - (diff * ((DST_RCDISC2__ENABLE == 0) ? context->exponent0 : context->exponent1));
	node->output[0] += diff;
}

static DISCRETE_RESET(dst_rcdisc2)
{
	struct dst_rcdisc_context *context = (struct dst_rcdisc_context *)node->context;

	node->output[0] = 0;

	context->state = 0;
	context->t = 0;
	context->exponent0 = RC_DISCHARGE_EXP(DST_RCDISC2__R0 * DST_RCDISC2__C);
	context->exponent1 = RC_DISCHARGE_EXP(DST_RCDISC2__R1 * DST_RCDISC2__C);
}

/************************************************************************
 *
 * DST_RCDISC3 -  Usage of node_description values for RC discharge
 *
 *
 * input[0]    - Enable
 * input[1]    - input  value
 * input[2]    - Resistor0 value (initialization only)
 * input[4]    - Resistor1 value (initialization only)
 * input[5]    - Capacitor Value (initialization only)
 * input[6]    - Diode Junction voltage (initialization only)
 *
 ************************************************************************/
#define DST_RCDISC3__ENABLE	DISCRETE_INPUT(0)
#define DST_RCDISC3__IN		DISCRETE_INPUT(1)
#define DST_RCDISC3__R1		DISCRETE_INPUT(2)
#define DST_RCDISC3__R2		DISCRETE_INPUT(3)
#define DST_RCDISC3__C		DISCRETE_INPUT(4)
#define DST_RCDISC3__DJV	DISCRETE_INPUT(5)

static DISCRETE_STEP(dst_rcdisc3)
{
	struct dst_rcdisc_context *context = (struct dst_rcdisc_context *)node->context;

	double diff;

	/* Exponential based in difference between input/output   */

	if(DST_RCDISC3__ENABLE)
	{
		diff = DST_RCDISC3__IN - node->output[0];
		if (context->v_diode > 0)
		{
			if (diff > 0)
			{
				diff = diff * context->exponent0;
			}
			else if (diff < -context->v_diode)
			{
				diff = diff * context->exponent1;
			}
			else
			{
				diff = diff * context->exponent0;
			}
		}
		else
		{
			if (diff < 0)
			{
				diff = diff * context->exponent0;
			}
			else if (diff > -context->v_diode)
			{
				diff = diff * context->exponent1;
			}
			else
			{
				diff = diff * context->exponent0;
			}
		}
		node->output[0] += diff;
	}
	else
	{
		node->output[0] = 0;
	}
}

static DISCRETE_RESET(dst_rcdisc3)
{
	struct dst_rcdisc_context *context = (struct dst_rcdisc_context *)node->context;

	node->output[0] = 0;

	context->state = 0;
	context->t = 0;
	context->v_diode = DST_RCDISC3__DJV;
	context->exponent0 = RC_CHARGE_EXP(DST_RCDISC3__R1 * DST_RCDISC3__C);
	context->exponent1 = RC_CHARGE_EXP(RES_2_PARALLEL(DST_RCDISC3__R1, DST_RCDISC3__R2) * DST_RCDISC3__C);
}


/************************************************************************
 *
 * DST_RCDISC4 -  Various charge/discharge circuits
 *
 * input[0]    - Enable input value
 * input[1]    - input value
 * input[2]    - R1 Resistor value (initialization only)
 * input[2]    - R2 Resistor value (initialization only)
 * input[4]    - C1 Capacitor Value (initialization only)
 * input[4]    - vP power source (initialization only)
 * input[4]    - circuit type (initialization only)
 *
 ************************************************************************/
#define DST_RCDISC4__ENABLE	DISCRETE_INPUT(0)
#define DST_RCDISC4__IN		DISCRETE_INPUT(1)
#define DST_RCDISC4__R1		DISCRETE_INPUT(2)
#define DST_RCDISC4__R2		DISCRETE_INPUT(3)
#define DST_RCDISC4__R3		DISCRETE_INPUT(4)
#define DST_RCDISC4__C1		DISCRETE_INPUT(5)
#define DST_RCDISC4__VP		DISCRETE_INPUT(6)
#define DST_RCDISC4__TYPE	DISCRETE_INPUT(7)

static DISCRETE_STEP(dst_rcdisc4)
{
	struct dst_rcdisc4_context *context = (struct dst_rcdisc4_context *)node->context;

	int inp1 = (DST_RCDISC4__IN == 0) ? 0 : 1;

	if (DST_RCDISC4__ENABLE == 0)
	{
		node->output[0] = 0;
		return;
	}

	switch (context->type)
	{
		case 1:
		case 3:
			context->vC1 += ((context->v[inp1] - context->vC1) * context->exp[inp1]);
			node->output[0] = context->vC1;
			break;
	}

	/* clip output */
	if (node->output[0] > context->max_out) node->output[0] = context->max_out;
	if (node->output[0] < 0) node->output[0] = 0;
}

static DISCRETE_RESET( dst_rcdisc4)
{
	struct dst_rcdisc4_context *context = (struct dst_rcdisc4_context *)node->context;

	double	v, i, r, rT;

	context->type = 0;
	/* some error checking. */
	if (DST_RCDISC4__R1 <= 0 || DST_RCDISC4__R2 <= 0 || DST_RCDISC4__C1 <= 0 || (DST_RCDISC4__R3 <= 0 &&  context->type == 1))
	{
		discrete_log(node->info, "Invalid component values in NODE_%d.\n", NODE_BLOCKINDEX(node));
		return;
	}
	if (DST_RCDISC4__VP < 3)
	{
		discrete_log(node->info, "vP must be >= 3V in NODE_%d.\n", NODE_BLOCKINDEX(node));
		return;
	}
	if (DST_RCDISC4__TYPE < 1 || DST_RCDISC4__TYPE > 3)
	{
		discrete_log(node->info, "Invalid circuit type in NODE_%d.\n", NODE_BLOCKINDEX(node));
		return;
	}

	context->vC1 = 0;
	/* store type as integer */
	context->type = (int)DST_RCDISC4__TYPE;
	/* setup the maximum op-amp output. */
	context->max_out = DST_RCDISC4__VP - OP_AMP_VP_RAIL_OFFSET;

	switch (context->type)
	{
		case 1:
			/* We will simulate this as a voltage divider with 2 states depending
             * on the input.  But we have to take the diodes into account.
             */
			v = DST_RCDISC4__VP - .5;	/* diode drop */

			/* When the input is 1, both R1 & R3 are basically in parallel. */
			r  = RES_2_PARALLEL(DST_RCDISC4__R1, DST_RCDISC4__R3);
			rT = DST_RCDISC4__R2 + r;
			i  = v / rT;
			context->v[1] = i * r + .5;
			rT = RES_2_PARALLEL(DST_RCDISC4__R2, r);
			context->exp[1] = RC_CHARGE_EXP(rT * DST_RCDISC4__C1);

			/* When the input is 0, R1 is out of circuit. */
			rT = DST_RCDISC4__R2 + DST_RCDISC4__R3;
			i  = v / rT;
			context->v[0] = i * DST_RCDISC4__R3 + .5;
			rT = RES_2_PARALLEL(DST_RCDISC4__R2, DST_RCDISC4__R3);
			context->exp[0] = RC_CHARGE_EXP(rT * DST_RCDISC4__C1);
			break;

		case 3:
			/* We will simulate this as a voltage divider with 2 states depending
             * on the input.  The 1k pullup is in parallel with the internal TTL
             * resistance, so we will just use .5k in series with R1.
             */
			r = 500.0 + DST_RCDISC4__R1;
			context->v[1] = RES_VOLTAGE_DIVIDER(r, DST_RCDISC4__R2) * (5.0 - 0.5);
			rT = RES_2_PARALLEL(r, DST_RCDISC4__R2);
			context->exp[1] = RC_CHARGE_EXP(rT * DST_RCDISC4__C1);

			/* When the input is 0, R1 is out of circuit. */
			context->v[0] = 0;
			context->exp[0] = RC_CHARGE_EXP(DST_RCDISC4__R2 * DST_RCDISC4__C1);
			break;
	}
}

/************************************************************************
 *
 * DST_RCDISC5 -  Diode in series with R//C
 *
 * input[0]    - Enable input value
 * input[1]    - input value
 * input[2]    - Resistor value (initialization only)
 * input[3]    - Capacitor Value (initialization only)
 *
 ************************************************************************/
#define DST_RCDISC5__ENABLE	DISCRETE_INPUT(0)
#define DST_RCDISC5__IN		DISCRETE_INPUT(1)
#define DST_RCDISC5__R		DISCRETE_INPUT(2)
#define DST_RCDISC5__C		DISCRETE_INPUT(3)

static DISCRETE_STEP( dst_rcdisc5)
{
	struct dst_rcdisc_context *context = (struct dst_rcdisc_context *)node->context;

	double diff,u;

	/* Exponential based in difference between input/output   */

    u = DST_RCDISC5__IN - 0.7; /* Diode drop */
	if( u < 0)
		u = 0;

	diff = u - context->v_cap;

	if(DST_RCDISC5__ENABLE)
	{
		if(diff < 0)
			diff = diff * context->exponent0;

		context->v_cap += diff;
		node->output[0] = context->v_cap;
	}
	else
	{
		if(diff > 0)
			context->v_cap = u;

		node->output[0] = 0;
	}
}

static DISCRETE_RESET( dst_rcdisc5)
{
	struct dst_rcdisc_context *context = (struct dst_rcdisc_context *)node->context;

	node->output[0] = 0;

	context->state = 0;
	context->t = 0;
	context->v_cap = 0;
	context->exponent0 = RC_CHARGE_EXP(DST_RCDISC5__R * DST_RCDISC5__C);
}


/************************************************************************
 *
 * DST_RCDISC_MOD -  RC triggered by logic and modulated
 *
 * input[0]    - Enable input value
 * input[1]    - input value 1
 * input[2]    - input value 2
 * input[3]    - Resistor 1 value (initialization only)
 * input[4]    - Resistor 2 value (initialization only)
 * input[5]    - Resistor 3 value (initialization only)
 * input[6]    - Resistor 4 value (initialization only)
 * input[7]    - Capacitor Value (initialization only)
 * input[8]    - Voltage Value (initialization only)
 *
 ************************************************************************/
#define DST_RCDISC_MOD__IN1		DISCRETE_INPUT(0)
#define DST_RCDISC_MOD__IN2		DISCRETE_INPUT(1)
#define DST_RCDISC_MOD__R1		DISCRETE_INPUT(2)
#define DST_RCDISC_MOD__R2		DISCRETE_INPUT(3)
#define DST_RCDISC_MOD__R3		DISCRETE_INPUT(4)
#define DST_RCDISC_MOD__R4		DISCRETE_INPUT(5)
#define DST_RCDISC_MOD__C		DISCRETE_INPUT(6)
#define DST_RCDISC_MOD__VP		DISCRETE_INPUT(7)

static DISCRETE_STEP(dst_rcdisc_mod)
{
	struct dst_rcdisc_mod_context *context = (struct dst_rcdisc_mod_context *)node->context;

	double	diff, v_cap, u, vD;
	int		mod_state, mod1_state, mod2_state;

	/* Exponential based in difference between input/output   */
	v_cap = context->v_cap;

	mod1_state = DST_RCDISC_MOD__IN1 > 0.5;
	mod2_state = DST_RCDISC_MOD__IN2 > 0.6;
	mod_state  = (mod2_state << 1) + mod1_state;

	u = mod1_state ? 0 : DST_RCDISC_MOD__VP;
	/* Clamp */
	diff = u - v_cap;
	vD = diff * context->vd_gain[mod_state];
	if (vD < -0.6)
	{
		diff  = u + 0.6 - v_cap;
		diff -= diff * context->exp_low[mod1_state];
		v_cap += diff;
		node->output[0] = mod2_state ? 0 : -0.6;
	}
	else
	{
		diff  -= diff * context->exp_high[mod_state];
		v_cap += diff;
		/* neglecting current through R3 drawn by next8 node */
		node->output[0] = mod2_state ? 0: (u - v_cap) * context->gain[mod1_state];
	}
	context->v_cap = v_cap;
}

static DISCRETE_RESET(dst_rcdisc_mod)
{
	struct dst_rcdisc_mod_context *context = (struct dst_rcdisc_mod_context *)node->context;

	double	rc[2], rc2[2];

	/* pre-calculate fixed values */
	/* DST_RCDISC_MOD__IN1 <= 0.5 */
	rc[0] = DST_RCDISC_MOD__R1 + DST_RCDISC_MOD__R2;
	if (rc[0] < 1) rc[0] = 1;
	context->exp_low[0]  = RC_DISCHARGE_EXP(DST_RCDISC_MOD__C * rc[0]);
	context->gain[0]     = RES_VOLTAGE_DIVIDER(rc[0], DST_RCDISC_MOD__R4);
	/* DST_RCDISC_MOD__IN1 > 0.5 */
	rc[1] = DST_RCDISC_MOD__R2;
	if (rc[1] < 1) rc[1] = 1;
	context->exp_low[1]  = RC_DISCHARGE_EXP(DST_RCDISC_MOD__C * rc[1]);
	context->gain[1]     = RES_VOLTAGE_DIVIDER(rc[1], DST_RCDISC_MOD__R4);
	/* DST_RCDISC_MOD__IN2 <= 0.6 */
	rc2[0] = DST_RCDISC_MOD__R4;
	/* DST_RCDISC_MOD__IN2 > 0.6 */
	rc2[1] = RES_2_PARALLEL(DST_RCDISC_MOD__R3, DST_RCDISC_MOD__R4);
	/* DST_RCDISC_MOD__IN1 <= 0.5 && DST_RCDISC_MOD__IN2 <= 0.6 */
	context->exp_high[0] = RC_DISCHARGE_EXP(DST_RCDISC_MOD__C * (rc[0] + rc2[0]));
	context->vd_gain[0]  = RES_VOLTAGE_DIVIDER(rc[0], rc2[0]);
	/* DST_RCDISC_MOD__IN1 > 0.5  && DST_RCDISC_MOD__IN2 <= 0.6 */
	context->exp_high[1] = RC_DISCHARGE_EXP(DST_RCDISC_MOD__C * (rc[1] + rc2[0]));
	context->vd_gain[1]  = RES_VOLTAGE_DIVIDER(rc[1], rc2[0]);
	/* DST_RCDISC_MOD__IN1 <= 0.5 && DST_RCDISC_MOD__IN2 > 0.6 */
	context->exp_high[2] = RC_DISCHARGE_EXP(DST_RCDISC_MOD__C * (rc[0] + rc2[1]));
	context->vd_gain[2]  = RES_VOLTAGE_DIVIDER(rc[0], rc2[1]);
	/* DST_RCDISC_MOD__IN1 > 0.5  && DST_RCDISC_MOD__IN2 > 0.6 */
	context->exp_high[3] = RC_DISCHARGE_EXP(DST_RCDISC_MOD__C * (rc[1] + rc2[1]));
	context->vd_gain[3]  = RES_VOLTAGE_DIVIDER(rc[1], rc2[1]);

	context->v_cap  = 0;
	node->output[0] = 0;
}

/************************************************************************
 *
 * DST_RCFILTER - Usage of node_description values for RC filter
 *
 * input[0]    - Enable input value
 * input[1]    - input value
 * input[2]    - Resistor value (initialization only)
 * input[3]    - Capacitor Value (initialization only)
 * input[4]    - Voltage reference. Usually 0V.
 *
 ************************************************************************/
#define DST_RCFILTER__VIN		DISCRETE_INPUT(0)
#define DST_RCFILTER__R			DISCRETE_INPUT(1)
#define DST_RCFILTER__C			DISCRETE_INPUT(2)
#define DST_RCFILTER__VREF		DISCRETE_INPUT(3)

static DISCRETE_STEP(dst_rcfilter)
{
	struct dst_rcfilter_context *context = (struct dst_rcfilter_context *)node->context;

	if (UNEXPECTED(context->has_rc_nodes))
	{
		double rc = DST_RCFILTER__R * DST_RCFILTER__C;
		if (rc != context->rc)
		{
			context->rc = rc;
			context->exponent = RC_CHARGE_EXP(rc);
		}
	}

	/************************************************************************/
	/* Next Value = PREV + (INPUT_VALUE - PREV)*(1-(EXP(-TIMEDELTA/RC)))    */
	/************************************************************************/

	context->vCap += ((DST_RCFILTER__VIN - node->output[0]) * context->exponent);
	node->output[0] = context->vCap + DST_RCFILTER__VREF;
}

static DISCRETE_STEP(dst_rcfilter_fast)
{
	struct dst_rcfilter_context *context = (struct dst_rcfilter_context *)node->context;
	node->output[0] += ((DST_RCFILTER__VIN - node->output[0]) * context->exponent);
}

static DISCRETE_RESET(dst_rcfilter)
{
	struct dst_rcfilter_context *context = (struct dst_rcfilter_context *)node->context;

	context->has_rc_nodes = node->input_is_node & 0x6;
	context->rc = DST_RCFILTER__R * DST_RCFILTER__C;
	context->exponent = RC_CHARGE_EXP(context->rc);
	context->vCap   = 0;
	node->output[0] = 0;
	if (!context->has_rc_nodes && DST_RCFILTER__VREF == 0)
		node->step = DISCRETE_STEP_NAME(dst_rcfilter_fast);
}

/************************************************************************
 *
 * DST_RCFILTER_SW - Usage of node_description values for switchable RC filter
 *
 * input[0]    - Enable input value
 * input[1]    - input value
 * input[2]    - Resistor value (initialization only)
 * input[3]    - Capacitor Value (initialization only)
 * input[4]    - Voltage reference. Usually 0V.
 *
 ************************************************************************/
#define DST_RCFILTER_SW__ENABLE		DISCRETE_INPUT(0)
#define DST_RCFILTER_SW__VIN		DISCRETE_INPUT(1)
#define DST_RCFILTER_SW__SWITCH		DISCRETE_INPUT(2)
#define DST_RCFILTER_SW__R			DISCRETE_INPUT(3)
#define DST_RCFILTER_SW__C(x)		DISCRETE_INPUT(4+x)

/* 74HC4066 : 15
 * 74VHC4066 : 15
 * UTC4066 : 270 @ 5VCC, 80 @ 15VCC
 * CD4066BC : 270 (Fairchild)
 *
 * The choice below makes scramble sound about "right". For future error reports,
 * we need the exact type of switch and at which voltage (5, 12?) it is operated.
 */
#define CD4066_ON_RES (40)

// FIXME: This needs optimization !
static DISCRETE_STEP(dst_rcfilter_sw)
{
	struct dst_rcfilter_sw_context *context = (struct dst_rcfilter_sw_context *)node->context;

	int i;
	int bits = (int)DST_RCFILTER_SW__SWITCH;
	double us = 0;

	if (DST_RCFILTER_SW__ENABLE)
	{
		switch (bits)
		{
		case 0:
			node->output[0] = DST_RCFILTER_SW__VIN;
			break;
		case 1:
			context->vCap[0] += (DST_RCFILTER_SW__VIN - context->vCap[0]) * context->exp0;
			node->output[0] = context->vCap[0] + (DST_RCFILTER_SW__VIN - context->vCap[0]) * context->factor;
			break;
		case 2:
			context->vCap[1] += (DST_RCFILTER_SW__VIN - context->vCap[1]) * context->exp1;
			node->output[0] = context->vCap[1] + (DST_RCFILTER_SW__VIN - context->vCap[1]) * context->factor;
			break;
		default:
			for (i = 0; i < 4; i++)
			{
				if (( bits & (1 << i)) != 0)
					us += context->vCap[i];
			}
			node->output[0] = context->f1[bits] * DST_RCFILTER_SW__VIN + context->f2[bits]  * us;
			for (i = 0; i < 4; i++)
			{
				if (( bits & (1 << i)) != 0)
					context->vCap[i] += (node->output[0] - context->vCap[i]) * context->exp[i];
			}
		}
	}
	else
	{
		node->output[0] = 0;
	}
}

static DISCRETE_RESET(dst_rcfilter_sw)
{
	struct dst_rcfilter_sw_context *context = (struct dst_rcfilter_sw_context *)node->context;

	int i, bits;

	for (i = 0; i < 4; i++)
	{
		context->vCap[i] = 0;
		context->exp[i] = RC_CHARGE_EXP( CD4066_ON_RES * DST_RCFILTER_SW__C(i));
	}

	for (bits=0; bits < 15; bits++)
	{
		double rs = 0;

		for (i = 0; i < 4; i++)
		{
			if (( bits & (1 << i)) != 0)
				rs += DST_RCFILTER_SW__R;
		}
		context->f1[bits] = RES_VOLTAGE_DIVIDER(rs, CD4066_ON_RES);
		context->f2[bits] = DST_RCFILTER_SW__R / (CD4066_ON_RES + rs);
	}


	/* fast cases */
	context->exp0 = RC_CHARGE_EXP((CD4066_ON_RES + DST_RCFILTER_SW__R) * DST_RCFILTER_SW__C(0));
	context->exp1 = RC_CHARGE_EXP((CD4066_ON_RES + DST_RCFILTER_SW__R) * DST_RCFILTER_SW__C(1));
	context->factor = RES_VOLTAGE_DIVIDER(DST_RCFILTER_SW__R, CD4066_ON_RES);

	node->output[0] = 0;
}


/************************************************************************
 *
 * DST_RCINTEGRATE - Two diode inputs, transistor and a R/C charge
 *                   discharge network
 *
 * input[0]    - Enable input value
 * input[1]    - input value 1
 * input[2]    - input value 2
 * input[3]    - Resistor 1 value (initialization only)
 * input[4]    - Resistor 2 value (initialization only)
 * input[5]    - Capacitor Value (initialization only)
 *
 ************************************************************************/
#define DST_RCINTEGRATE__IN1	DISCRETE_INPUT(0)
#define DST_RCINTEGRATE__R1		DISCRETE_INPUT(1)
#define DST_RCINTEGRATE__R2		DISCRETE_INPUT(2)
#define DST_RCINTEGRATE__R3		DISCRETE_INPUT(3)
#define DST_RCINTEGRATE__C		DISCRETE_INPUT(4)
#define DST_RCINTEGRATE__VP		DISCRETE_INPUT(5)
#define DST_RCINTEGRATE__TYPE	DISCRETE_INPUT(6)

/* Ebers-Moll large signal model
 * Couriersud:
 * The implementation avoids all iterative approaches in order not to burn cycles
 * We will calculate Ic from vBE and use this as an indication where to go.
 * The implementation may oscillate if you change the weighting factors at the
 * end.
 *
 * This implementation is not perfect, but does it's job in dkong'
 */

/* reverse saturation current */
#define IES		7e-15
#define ALPHAT	0.99
#define KT		0.026
#define EM_IC(x) (ALPHAT * IES * exp( (x) / KT - 1.0 ))

static DISCRETE_STEP( dst_rcintegrate)
{
	struct dst_rcintegrate_context *context = (struct dst_rcintegrate_context *)node->context;

	double diff, u, iQ, iQc, iC, RG, vE;
	double vP;

	u  = DST_RCINTEGRATE__IN1;
	vP = DST_RCINTEGRATE__VP;

	if ( u - 0.7  < context->vCap * context->gain_r1_r2)
	{
		/* discharge .... */
		diff  = 0.0 - context->vCap;
		iC    = context->c_exp1 * diff; /* iC */
		diff -= diff * context->exp_exponent1;
		context->vCap += diff;
		iQ = 0;
		vE = context->vCap * context->gain_r1_r2;
		RG = vE / iC;
	}
	else
	{
		/* charging */
		diff  = (vP - context->vCE) * context->f - context->vCap;
		iC    = 0.0 - context->c_exp0 * diff; /* iC */
		diff -= diff * context->exp_exponent0;
		context->vCap += diff;
		iQ = iC + (iC * DST_RCINTEGRATE__R1 + context->vCap) / DST_RCINTEGRATE__R2;
		RG = (vP - context->vCE) / iQ;
		vE = (RG - DST_RCINTEGRATE__R3) / RG * (vP - context->vCE);
	}


	u = DST_RCINTEGRATE__IN1;
	if (u > 0.7 + vE)
		vE = u - 0.7;
	iQc = EM_IC(u - vE);
	context->vCE = MIN(vP - 0.1, vP - RG * iQc);

	/* Avoid oscillations
     * The method tends to largely overshoot - no wonder without
     * iterative solution approximation
     */

	context->vCE = MAX(context->vCE, 0.1 );
	context->vCE = 0.1 * context->vCE + 0.9 * (vP - vE - iQ * DST_RCINTEGRATE__R3);

	switch (context->type)
	{
		case DISC_RC_INTEGRATE_TYPE1:
			node->output[0] = context->vCap;
			break;
		case DISC_RC_INTEGRATE_TYPE2:
			node->output[0] = vE;
			break;
		case DISC_RC_INTEGRATE_TYPE3:
			node->output[0] = MAX(0, vP - iQ * DST_RCINTEGRATE__R3);
			break;
	}
}
static DISCRETE_RESET(dst_rcintegrate)
{
	struct dst_rcintegrate_context *context = (struct dst_rcintegrate_context *)node->context;

	double r;
	double dt = node->info->sample_time;

	context->type = DST_RCINTEGRATE__TYPE;

	context->vCap = 0;
	context->vCE  = 0;

	/* pre-calculate fixed values */
	context->gain_r1_r2 = RES_VOLTAGE_DIVIDER(DST_RCINTEGRATE__R1, DST_RCINTEGRATE__R2);

	r = DST_RCINTEGRATE__R1 / DST_RCINTEGRATE__R2 * DST_RCINTEGRATE__R3 + DST_RCINTEGRATE__R1 + DST_RCINTEGRATE__R3;

	context->f = RES_VOLTAGE_DIVIDER(DST_RCINTEGRATE__R3, DST_RCINTEGRATE__R2);
	context->exponent0 = -1.0 * r * context->f * DST_RCINTEGRATE__C;
	context->exponent1 = -1.0 * (DST_RCINTEGRATE__R1 + DST_RCINTEGRATE__R2) * DST_RCINTEGRATE__C;
	context->exp_exponent0 = exp(dt / context->exponent0);
	context->exp_exponent1 = exp(dt / context->exponent1);
	context->c_exp0 =  DST_RCINTEGRATE__C / context->exponent0 * context->exp_exponent0;
	context->c_exp1 =  DST_RCINTEGRATE__C / context->exponent1 * context->exp_exponent1;

	node->output[0] = 0;
}

/************************************************************************
 *
 * DST_SALLEN_KEY - Sallen-Key filter circuit
 *
 * input[0]    - Enable input value
 * input[1]    - IN0 node
 * input[3]    - Filter Type
 *
 * also passed discrete_op_amp_filt_info structure
 *
 * 2008, couriersud
 ************************************************************************/
#define DST_SALLEN_KEY__ENABLE	DISCRETE_INPUT(0)
#define DST_SALLEN_KEY__INP0	DISCRETE_INPUT(1)
#define DST_SALLEN_KEY__TYPE	DISCRETE_INPUT(2)

static DISCRETE_STEP(dst_sallen_key)
{
	struct dss_filter2_context *context = (struct dss_filter2_context *)node->context;

	double gain = 1.0;

	if (DST_SALLEN_KEY__ENABLE == 0.0)
	{
		gain = 0.0;
	}

	node->output[0] = -context->a1 * context->y1 - context->a2 * context->y2 +
					context->b0 * gain * DST_SALLEN_KEY__INP0 + context->b1 * context->x1 + context->b2 * context->x2;

	context->x2 = context->x1;
	context->x1 = gain * DST_SALLEN_KEY__INP0;
	context->y2 = context->y1;
	context->y1 = node->output[0];
}

static DISCRETE_RESET(dst_sallen_key)
{
	struct dss_filter2_context       *context = (struct dss_filter2_context *)node->context;
	const  discrete_op_amp_filt_info *info    = (const  discrete_op_amp_filt_info *)node->custom;

	double freq, q;

	switch ((int) DST_SALLEN_KEY__TYPE)
	{
		case DISC_SALLEN_KEY_LOW_PASS:
		    freq = 1.0 / ( 2.0 * M_PI * sqrt(info->c1 * info->c2 * info->r1 * info->r2));
		    q = sqrt(info->c1 * info->c2 * info->r1 * info->r2) / (info->c2 * (info->r1 + info->r2));
		    break;
		default:
			fatalerror("Unknown sallen key filter type");
	}

	calculate_filter2_coefficients(node->info, freq, 1.0 / q, DISC_FILTER_LOWPASS,
								   &context->a1, &context->a2,
								   &context->b0, &context->b1, &context->b2);
	node->output[0] = 0;
}


/* !!!!!!!!!!! NEW FILTERS for testing !!!!!!!!!!!!!!!!!!!!! */


/************************************************************************
 *
 * DST_RCFILTERN - Usage of node_description values for RC filter
 *
 * input[0]    - Enable input value
 * input[1]    - input value
 * input[2]    - Resistor value (initialization only)
 * input[3]    - Capacitor Value (initialization only)
 *
 ************************************************************************/
#define DST_RCFILTERN__ENABLE		DISCRETE_INPUT(0)
#define DST_RCFILTERN__IN		DISCRETE_INPUT(1)
#define DST_RCFILTERN__R		DISCRETE_INPUT(2)
#define DST_RCFILTERN__C		DISCRETE_INPUT(3)

static DISCRETE_RESET(dst_rcfilterN)
{
#if 0
	double f=1.0/(2*M_PI* DST_RCFILTERN__R * DST_RCFILTERN__C);

/* !!!!!!!!!!!!!! CAN'T CHEAT LIKE THIS !!!!!!!!!!!!!!!! */
/* Put this stuff in a context */

	node->input[2] = f;
	node->input[3] = DISC_FILTER_LOWPASS;

	/* Use first order filter */
	dst_filter1_reset(node);
#endif
}


/************************************************************************
 *
 * DST_RCDISCN -   Usage of node_description values for RC discharge
 *                (inverse slope of DST_RCFILTER)
 *
 * input[0]    - Enable input value
 * input[1]    - input value
 * input[2]    - Resistor value (initialization only)
 * input[3]    - Capacitor Value (initialization only)
 *
 ************************************************************************/
#define DST_RCDISCN__ENABLE	DISCRETE_INPUT(0)
#define DST_RCDISCN__IN		DISCRETE_INPUT(1)
#define DST_RCDISCN__R		DISCRETE_INPUT(2)
#define DST_RCDISCN__C		DISCRETE_INPUT(3)

static DISCRETE_RESET(dst_rcdiscN)
{
#if 0
	double f = 1.0 / (2 * M_PI * DST_RCDISCN__R * DST_RCDISCN__C);

/* !!!!!!!!!!!!!! CAN'T CHEAT LIKE THIS !!!!!!!!!!!!!!!! */
/* Put this stuff in a context */

	node->input[2] = f;
	node->input[3] = DISC_FILTER_LOWPASS;

	/* Use first order filter */
	dst_filter1_reset(node);
#endif
}

static DISCRETE_STEP(dst_rcdiscN)
{
	struct dss_filter1_context *context = (struct dss_filter1_context *)node->context;

	double gain = 1.0;

	if (DST_RCDISCN__ENABLE == 0.0)
	{
		gain = 0.0;
	}

	/* A rise in the input signal results in an instant charge, */
	/* else discharge through the RC to zero */
	if (gain* DST_RCDISCN__IN > context->x1)
		node->output[0] = gain* DST_RCDISCN__IN;
	else
		node->output[0] = -context->a1*context->y1;

	context->x1 = gain* DST_RCDISCN__IN;
	context->y1 = node->output[0];
}


/************************************************************************
 *
 * DST_RCDISC2N -  Usage of node_description values for RC discharge
 *                Has switchable charge resistor/input
 *
 * input[0]    - Switch input value
 * input[1]    - input[0] value
 * input[2]    - Resistor0 value (initialization only)
 * input[3]    - input[1] value
 * input[4]    - Resistor1 value (initialization only)
 * input[5]    - Capacitor Value (initialization only)
 *
 ************************************************************************/
#define DST_RCDISC2N__ENABLE		DISCRETE_INPUT(0)
#define DST_RCDISC2N__IN0		DISCRETE_INPUT(1)
#define DST_RCDISC2N__R0		DISCRETE_INPUT(2)
#define DST_RCDISC2N__IN1		DISCRETE_INPUT(3)
#define DST_RCDISC2N__R1		DISCRETE_INPUT(4)
#define DST_RCDISC2N__C			DISCRETE_INPUT(5)

struct dss_rcdisc2_context
{
	double x1;			/* x[k-1], last input value */
	double y1;			/* y[k-1], last output value */
	double a1_0, b0_0, b1_0;	/* digital filter coefficients, filter #1 */
	double a1_1, b0_1, b1_1;	/* digital filter coefficients, filter #2 */
};

static DISCRETE_STEP(dst_rcdisc2N)
{
	struct dss_rcdisc2_context *context = (struct dss_rcdisc2_context *)node->context;

	double input = ((DST_RCDISC2N__ENABLE == 0) ? DST_RCDISC2N__IN0 : DST_RCDISC2N__IN1);

	if (DST_RCDISC2N__ENABLE == 0)
		node->output[0] = -context->a1_0*context->y1 + context->b0_0*input + context->b1_0*context->x1;
	else
		node->output[0] = -context->a1_1*context->y1 + context->b0_1*input + context->b1_1*context->x1;

	context->x1 = input;
	context->y1 = node->output[0];
}

static DISCRETE_RESET(dst_rcdisc2N)
{
	struct dss_rcdisc2_context *context = (struct dss_rcdisc2_context *)node->context;
	double f1,f2;

	f1 = 1.0 / (2 * M_PI * DST_RCDISC2N__R0 * DST_RCDISC2N__C);
	f2 = 1.0 / (2 * M_PI * DST_RCDISC2N__R1 * DST_RCDISC2N__C);

	calculate_filter1_coefficients(node->info, f1, DISC_FILTER_LOWPASS, &context->a1_0, &context->b0_0, &context->b1_0);
	calculate_filter1_coefficients(node->info, f2, DISC_FILTER_LOWPASS, &context->a1_1, &context->b0_1, &context->b1_1);

	/* Initialize the object */
	node->output[0] = 0;
}
