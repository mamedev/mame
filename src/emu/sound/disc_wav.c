/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by Keith Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *
 ************************************************************************
 *
 * DSS_COUNTER        - External clock Binary Counter
 * DSS_LFSR_NOISE     - Linear Feedback Shift Register Noise
 * DSS_NOISE          - Noise Source - Random source
 * DSS_NOTE           - Note/tone generator
 * DSS_OP_AMP_OSC     - Op Amp oscillator circuits
 * DSS_SAWTOOTHWAVE   - Sawtooth waveform generator
 * DSS_SCHMITT_OSC    - Schmitt Feedback Oscillator
 * DSS_SINEWAVE       - Sinewave generator source code
 * DSS_SQUAREWAVE     - Squarewave generator source code
 * DSS_SQUAREWFIX     - Squarewave generator - fixed frequency
 * DSS_SQUAREWAVE2    - Squarewave generator - by t_on/t_off
 * DSS_TRIANGLEWAVE   - Triangle waveform generator
 *
 ************************************************************************/

struct dss_adsr_context
{
	double phase;
};

struct dss_counter_context
{
	int		clock_type;
	int		out_type;
	int		is_7492;
	int		last;		/* Last clock state */
	int		count;		/* current count */
	double	t_left;		/* time unused during last sample in seconds */
};

#define DSS_INV_TAB_SIZE	500

struct dss_inverter_osc_context
{
	double	w;
	double  wc;
	double	v_cap;
	double  v_g2_old;
	double	rp;
	double  r1;
	double  r2;
	double  c;
	double	tf_a;
	double	tf_b;
	double  tf_tab[DSS_INV_TAB_SIZE];
};

struct dss_lfsr_context
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
	double	v_out_high;
	double	threshold_low;	/* falling threshold */
	double	threshold_high;	/* rising threshold */
	double	v_cap;			/* current capacitor voltage */
	double	r_total;		/* all input resistors in parallel */
	double	i_fixed;		/* fixed current at the input */
	double	temp1;			/* Multi purpose */
	double	temp2;			/* Multi purpose */
	double	temp3;			/* Multi purpose */
	double	is_linear_charge;
	double	charge_rc;
	double	charge_exp;
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


/************************************************************************
 *
 * DSS_COUNTER - External clock Binary Counter
 *
 * input0    - Enable input value
 * input1    - Reset input (active high)
 * input2    - Clock Input
 * input3    - Max count
 * input4    - Direction - 0=down, 1=up
 * input5    - Reset Value
 * input6    - Clock type
 *
 * Jan 2004, D Renaud.
 ************************************************************************/
#define DSS_COUNTER__ENABLE		DISCRETE_INPUT(0)
#define DSS_COUNTER__RESET		DISCRETE_INPUT(1)
#define DSS_COUNTER__CLOCK		DISCRETE_INPUT(2)
#define DSS_COUNTER__MAX		DISCRETE_INPUT(3)
#define DSS_COUNTER__DIR		DISCRETE_INPUT(4)
#define DSS_COUNTER__INIT		DISCRETE_INPUT(5)
#define DSS_COUNTER__CLOCK_TYPE	DISCRETE_INPUT(6)
#define DSS_7492__CLOCK_TYPE	 DSS_COUNTER__MAX

static const int disc_7492_count[6] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x06};

static DISCRETE_STEP(dss_counter)
{
	struct	dss_counter_context *context = (struct	dss_counter_context *)node->context;
	double	cycles;
	double	ds_clock;
	int		clock = 0, last_count, inc = 0;
	int		max;
	double	x_time = 0;

	if (context->is_7492)
		max = 5;
	else
		max = DSS_COUNTER__MAX;

	ds_clock = DSS_COUNTER__CLOCK;
	if (UNEXPECTED(context->clock_type == DISC_CLK_IS_FREQ))
	{
		/* We need to keep clocking the internal clock even if disabled. */
		cycles = (context->t_left + node->info->sample_time) * ds_clock;
		inc    = (int)cycles;
		context->t_left = (cycles - inc) / ds_clock;
		if (inc) x_time = context->t_left / node->info->sample_time;
	}
	else
	{
		clock  = (int)ds_clock;
		x_time = ds_clock - clock;
	}


	/* If reset enabled then set output to the reset value.  No x_time in reset. */
	if (UNEXPECTED(DSS_COUNTER__RESET))
	{
		context->count = DSS_COUNTER__INIT;
		node->output[0] = context->count;
		return;
	}

	/*
     * Only count if module is enabled.
     * This has the effect of holding the output at it's current value.
     */
	if (EXPECTED(DSS_COUNTER__ENABLE))
	{
		last_count = context->count;

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

		if (DSS_COUNTER__DIR)
			context->count = (context->count + inc) % (max + 1);
		else
			context->count = max - ((context->count + inc) % (max + 1));

		node->output[0] = context->is_7492 ? disc_7492_count[context->count] : context->count;

		if (EXPECTED(context->count != last_count))
		{
			/* the x_time is only output if the output changed. */
			switch (context->out_type)
			{
				case DISC_OUT_IS_ENERGY:
					if (x_time == 0) x_time = 1.0;
					node->output[0] = last_count;
					if (context->count > last_count)
						node->output[0] += (context->count - last_count) * x_time;
					else
						node->output[0] -= (last_count - context->count) * x_time;
					break;
				case DISC_OUT_HAS_XTIME:
					node->output[0] += x_time;
					break;
			}
		}
	}
	else
		node->output[0] = context->count;
}

static DISCRETE_RESET(dss_counter)
{
	struct dss_counter_context *context = (struct dss_counter_context *)node->context;

	if ((int)DSS_COUNTER__CLOCK_TYPE & DISC_COUNTER_IS_7492)
	{
		context->is_7492    = 1;
		context->clock_type = (int)DSS_7492__CLOCK_TYPE;
	}
	else
	{
		context->is_7492    = 0;
		context->clock_type = (int)DSS_COUNTER__CLOCK_TYPE;
	}
	context->out_type    = context->clock_type & DISC_OUT_MASK;
	context->clock_type &= DISC_CLK_MASK;

	context->t_left  = 0;
	context->last    = 0;
	context->count   = DSS_COUNTER__INIT; /* count starts at reset value */
	node->output[0]  = DSS_COUNTER__INIT;
}


/************************************************************************
 *
 * DSS_LFSR_NOISE - Usage of node_description values for LFSR noise gen
 *
 * input0    - Enable input value
 * input1    - Register reset
 * input2    - Clock Input
 * input3    - Amplitude input value
 * input4    - Input feed bit
 * input5    - Bias
 *
 * also passed dss_lfsr_context structure
 *
 ************************************************************************/
#define DSS_LFSR_NOISE__ENABLE	DISCRETE_INPUT(0)
#define DSS_LFSR_NOISE__RESET	DISCRETE_INPUT(1)
#define DSS_LFSR_NOISE__CLOCK	DISCRETE_INPUT(2)
#define DSS_LFSR_NOISE__AMP		DISCRETE_INPUT(3)
#define DSS_LFSR_NOISE__FEED	DISCRETE_INPUT(4)
#define DSS_LFSR_NOISE__BIAS	DISCRETE_INPUT(5)

INLINE int dss_lfsr_function(const discrete_info *disc_info, int myfunc, int in0, int in1, int bitmask)
{
	int retval;

	in0 &= bitmask;
	in1 &= bitmask;

	switch(myfunc)
	{
		case DISC_LFSR_XOR:
			retval = in0 ^ in1;
			break;
		case DISC_LFSR_OR:
			retval = in0 | in1;
			break;
		case DISC_LFSR_AND:
			retval = in0 & in1;
			break;
		case DISC_LFSR_XNOR:
			retval = in0 ^ in1;
			retval = retval ^ bitmask;	/* Invert output */
			break;
		case DISC_LFSR_NOR:
			retval = in0 | in1;
			retval = retval ^ bitmask;	/* Invert output */
			break;
		case DISC_LFSR_NAND:
			retval = in0 & in1;
			retval = retval ^ bitmask;	/* Invert output */
			break;
		case DISC_LFSR_IN0:
			retval = in0;
			break;
		case DISC_LFSR_IN1:
			retval = in1;
			break;
		case DISC_LFSR_NOT_IN0:
			retval = in0 ^ bitmask;
			break;
		case DISC_LFSR_NOT_IN1:
			retval = in1 ^ bitmask;
			break;
		case DISC_LFSR_REPLACE:
			retval = in0 & ~in1;
			retval = retval | in1;
			break;
		case DISC_LFSR_XOR_INV_IN0:
			retval = in0 ^ bitmask; /* invert in0 */
			retval = retval ^ in1;  /* xor in1 */
			break;
		case DISC_LFSR_XOR_INV_IN1:
			retval = in1 ^ bitmask; /* invert in1 */
			retval = retval ^ in0;  /* xor in0 */
			break;
		default:
			discrete_log(disc_info, "dss_lfsr_function - Invalid function type passed");
			retval=0;
			break;
	}
	return retval;
}

/* reset prototype so that it can be used in init function */
static DISCRETE_RESET(dss_lfsr);

static DISCRETE_STEP(dss_lfsr)
{
	const  discrete_lfsr_desc *lfsr_desc = (const discrete_lfsr_desc *)node->custom;
	struct dss_lfsr_context   *context   = (struct dss_lfsr_context *)node->context;
	double cycles;
	int clock, inc = 0;
	int fb0, fb1, fbresult = 0, noise_feed;

	if (lfsr_desc->clock_type == DISC_CLK_IS_FREQ)
	{
		/* We need to keep clocking the internal clock even if disabled. */
		cycles = (context->t_left + node->info->sample_time) / context->t_clock;
		inc    = (int)cycles;
		context->t_left = (cycles - inc) * context->t_clock;
	}

	/* Reset everything if necessary */
	if(((DSS_LFSR_NOISE__RESET == 0) ? 0 : 1) == context->reset_on_high)
	{
		DISCRETE_RESET_CALL(dss_lfsr);
		return;
	}

	switch (lfsr_desc->clock_type)
	{
		case DISC_CLK_ON_F_EDGE:
		case DISC_CLK_ON_R_EDGE:
			/* See if the clock has toggled to the proper edge */
			clock = (DSS_LFSR_NOISE__CLOCK != 0);
			if (context->last != clock)
			{
				context->last = clock;
				if (lfsr_desc->clock_type == clock)
				{
					/* Toggled */
					inc = 1;
				}
			}
			break;

		case DISC_CLK_BY_COUNT:
			/* Clock number of times specified. */
			inc = (int)DSS_LFSR_NOISE__CLOCK;
			break;
	}

	if (inc > 0)
	{
		noise_feed = (DSS_LFSR_NOISE__FEED ? 0x01 : 0x00);
		for (clock = 0; clock < inc; clock++)
		{
			/* Fetch the last feedback result */
			fbresult = (context->lfsr_reg >> lfsr_desc->bitlength) & 0x01;

			/* Stage 2 feedback combine fbresultNew with infeed bit */
			fbresult = dss_lfsr_function(node->info, lfsr_desc->feedback_function1, fbresult, noise_feed, 0x01);

			/* Stage 3 first we setup where the bit is going to be shifted into */
			fbresult = fbresult * lfsr_desc->feedback_function2_mask;
			/* Then we left shift the register, */
			context->lfsr_reg = context->lfsr_reg << 1;
			/* Now move the fbresult into the shift register and mask it to the bitlength */
			context->lfsr_reg = dss_lfsr_function(node->info, lfsr_desc->feedback_function2, fbresult, context->lfsr_reg, (1 << lfsr_desc->bitlength) - 1 );

			/* Now get and store the new feedback result */
			/* Fetch the feedback bits */
			fb0 = (context->lfsr_reg >> lfsr_desc->feedback_bitsel0) & 0x01;
			fb1 = (context->lfsr_reg >> lfsr_desc->feedback_bitsel1) & 0x01;
			/* Now do the combo on them */
			fbresult = dss_lfsr_function(node->info, lfsr_desc->feedback_function0, fb0, fb1, 0x01);
			context->lfsr_reg = dss_lfsr_function(node->info, DISC_LFSR_REPLACE, context->lfsr_reg, fbresult << lfsr_desc->bitlength, (2 << lfsr_desc->bitlength) - 1);

		}
		/* Now select the output bit */
		if (context->out_is_f0)
			node->output[0] = fbresult & 0x01;
		else
			node->output[0] = (context->lfsr_reg >> lfsr_desc->output_bit) & 0x01;

		/* Final inversion if required */
		if (context->invert_output) node->output[0] = node->output[0] ? 0 : 1;

		/* Gain stage */
		node->output[0] = node->output[0] ? DSS_LFSR_NOISE__AMP / 2 : -DSS_LFSR_NOISE__AMP / 2;
		/* Bias input as required */
		node->output[0] = node->output[0] + DSS_LFSR_NOISE__BIAS;

		/* output the lfsr reg ?*/
		if (context->out_lfsr_reg)
			node->output[1] = (double) context->lfsr_reg;

	}
	if(!DSS_LFSR_NOISE__ENABLE)
	{
		node->output[0] = 0;
	}
}

static DISCRETE_RESET(dss_lfsr)
{
	const  discrete_lfsr_desc *lfsr_desc = (const  discrete_lfsr_desc *)node->custom;
	struct dss_lfsr_context   *context   = (struct dss_lfsr_context *)node->context;
	int    fb0 , fb1, fbresult;

	context->reset_on_high = (lfsr_desc->flags & DISC_LFSR_FLAG_RESET_TYPE_H) ? 1 : 0;
	context->invert_output = lfsr_desc->flags & DISC_LFSR_FLAG_OUT_INVERT;
	context->out_is_f0 = (lfsr_desc->flags & DISC_LFSR_FLAG_OUTPUT_F0) ? 1 : 0;
	context->out_lfsr_reg = (lfsr_desc->flags & DISC_LFSR_FLAG_OUTPUT_SR_SN1) ? 1 : 0;

	if ((lfsr_desc->clock_type < DISC_CLK_ON_F_EDGE) || (lfsr_desc->clock_type > DISC_CLK_IS_FREQ))
		discrete_log(node->info, "Invalid clock type passed in NODE_%d\n", NODE_BLOCKINDEX(node));

	context->last = (DSS_COUNTER__CLOCK != 0);
	if (lfsr_desc->clock_type == DISC_CLK_IS_FREQ) context->t_clock = 1.0 / DSS_LFSR_NOISE__CLOCK;
	context->t_left = 0;

	context->lfsr_reg = lfsr_desc->reset_value;

	/* Now get and store the new feedback result */
	/* Fetch the feedback bits */
	fb0 = (context->lfsr_reg >> lfsr_desc->feedback_bitsel0) & 0x01;
	fb1=(context->lfsr_reg >> lfsr_desc->feedback_bitsel1) & 0x01;
	/* Now do the combo on them */
	fbresult = dss_lfsr_function(node->info, lfsr_desc->feedback_function0, fb0, fb1, 0x01);
	context->lfsr_reg=dss_lfsr_function(node->info, DISC_LFSR_REPLACE, context->lfsr_reg, fbresult << lfsr_desc->bitlength, (2<< lfsr_desc->bitlength ) - 1);

	/* Now select and setup the output bit */
	node->output[0] = (context->lfsr_reg >> lfsr_desc->output_bit) & 0x01;

	/* Final inversion if required */
	if(lfsr_desc->flags & DISC_LFSR_FLAG_OUT_INVERT) node->output[0] = node->output[0] ? 0 : 1;

	/* Gain stage */
	node->output[0] = node->output[0] ? DSS_LFSR_NOISE__AMP / 2 : -DSS_LFSR_NOISE__AMP / 2;
	/* Bias input as required */
	node->output[0] = node->output[0] + DSS_LFSR_NOISE__BIAS;
}


/************************************************************************
 *
 * DSS_NOISE - Usage of node_description values for white nose generator
 *
 * input0    - Enable input value
 * input1    - Noise sample frequency
 * input2    - Amplitude input value
 * input3    - DC Bias value
 *
 ************************************************************************/
#define DSS_NOISE__ENABLE	DISCRETE_INPUT(0)
#define DSS_NOISE__FREQ		DISCRETE_INPUT(1)
#define DSS_NOISE__AMP		DISCRETE_INPUT(2)
#define DSS_NOISE__BIAS		DISCRETE_INPUT(3)

static DISCRETE_STEP(dss_noise)
{
	struct dss_noise_context *context = (struct dss_noise_context *)node->context;

	if(DSS_NOISE__ENABLE)
	{
		/* Only sample noise on rollover to next cycle */
		if(context->phase > (2.0 * M_PI))
		{
			/* GCC's rand returns a RAND_MAX value of 0x7fff */
			int newval = (mame_rand(node->info->device->machine) & 0x7fff) - 16384;

			/* make sure the peak to peak values are the amplitude */
			node->output[0] = DSS_NOISE__AMP / 2;
			if (newval > 0)
				node->output[0] *= ((double)newval / 16383);
			else
				node->output[0] *= ((double)newval / 16384);

			/* Add DC Bias component */
			node->output[0] += DSS_NOISE__BIAS;
		}
	}
	else
	{
		node->output[0] = 0;
	}

	/* Keep the new phasor in the 2Pi range.*/
	context->phase = fmod(context->phase, 2.0 * M_PI);

	/* The enable input only curtails output, phase rotation still occurs. */
	/* We allow the phase to exceed 2Pi here, so we can tell when to sample the noise. */
	context->phase += ((2.0 * M_PI * DSS_NOISE__FREQ) / node->info->sample_rate);
}


static DISCRETE_RESET(dss_noise)
{
	struct dss_noise_context *context = (struct dss_noise_context *)node->context;

	context->phase=0;
	DISCRETE_STEP_CALL( dss_noise );
}


/************************************************************************
 *
 * DSS_NOTE - Note/tone generator
 *
 * input0    - Enable input value
 * input1    - Clock Input
 * input2    - data value
 * input3    - Max count 1
 * input4    - Max count 2
 * input5    - Clock type
 *
 * Mar 2004, D Renaud.
 ************************************************************************/
 #define DSS_NOTE__ENABLE		DISCRETE_INPUT(0)
 #define DSS_NOTE__CLOCK		DISCRETE_INPUT(1)
 #define DSS_NOTE__DATA			DISCRETE_INPUT(2)
 #define DSS_NOTE__MAX1			DISCRETE_INPUT(3)
 #define DSS_NOTE__MAX2			DISCRETE_INPUT(4)
 #define DSS_NOTE__CLOCK_TYPE	DISCRETE_INPUT(5)

static DISCRETE_STEP(dss_note)
{
	struct dss_note_context *context = (struct dss_note_context *)node->context;

	double	cycles;
	int		clock  = 0, last_count2, inc = 0;
	double	x_time = 0;

	if (context->clock_type == DISC_CLK_IS_FREQ)
	{
		/* We need to keep clocking the internal clock even if disabled. */
		cycles = (context->t_left + node->info->sample_time) / context->t_clock;
		inc    = (int)cycles;
		context->t_left = (cycles - inc) * context->t_clock;
		if (inc) x_time = context->t_left / node->info->sample_time;
	}
	else
	{
		/* Seperate clock info from x_time info. */
		clock = (int)DSS_NOTE__CLOCK;
		x_time = DSS_NOTE__CLOCK - clock;
	}

	if (DSS_NOTE__ENABLE)
	{
		last_count2 = context->count2;

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

		/* Count output as long as the data loaded is not already equal to max 1 count. */
		if (DSS_NOTE__DATA != DSS_NOTE__MAX1)
		{
			for (clock = 0; clock < inc; clock++)
			{
				context->count1++;
				if (context->count1 > context->max1)
				{
					/* Max 1 count reached.  Load Data into counter. */
					context->count1  = (int)DSS_NOTE__DATA;
					context->count2 += 1;
					if (context->count2 > context->max2) context->count2 = 0;
				}
			}
		}

		node->output[0] = context->count2;
		if (context->count2 != last_count2)
		{
			/* the x_time is only output if the output changed. */
			switch (context->out_type)
			{
				case DISC_OUT_IS_ENERGY:
					if (x_time == 0) x_time = 1.0;
					node->output[0] = last_count2;
					if (context->count2 > last_count2)
						node->output[0] += (context->count2 - last_count2) * x_time;
					else
						node->output[0] -= (last_count2 - context->count2) * x_time;
					break;
				case DISC_OUT_HAS_XTIME:
					node->output[0] += x_time;
					break;
			}
		}
	}
	else
		node->output[0] = 0;
}

static DISCRETE_RESET(dss_note)
{
	struct dss_note_context *context = (struct dss_note_context *)node->context;

	context->clock_type = (int)DSS_NOTE__CLOCK_TYPE & DISC_CLK_MASK;
	context->out_type   = (int)DSS_NOTE__CLOCK_TYPE & DISC_OUT_MASK;

	context->last    = (DSS_NOTE__CLOCK != 0);
	context->t_left  = 0;
	context->t_clock = 1.0 / DSS_NOTE__CLOCK;

	context->count1 = (int)DSS_NOTE__DATA;
	context->count2 = 0;
	context->max1   = (int)DSS_NOTE__MAX1;
	context->max2   = (int)DSS_NOTE__MAX2;
	node->output[0] = 0;
}

/************************************************************************
 *
 * DSS_OP_AMP_OSC - Op Amp Oscillators
 *
 * input0    - Enable input value
 * input1    - vMod1 (if needed)
 * input2    - vMod2 (if needed)
 *
 * also passed discrete_op_amp_osc_info structure
 *
 * Mar 2004, D Renaud.
 ************************************************************************/
#define DSS_OP_AMP_OSC__ENABLE	DISCRETE_INPUT(0)
#define DSS_OP_AMP_OSC__VMOD1	DISCRETE_INPUT(1)
#define DSS_OP_AMP_OSC__VMOD2	DISCRETE_INPUT(2)

/* The inputs on a norton op-amp are (info->vP - OP_AMP_NORTON_VBE) */
/* which is the same as the output high voltage.  We will define them */
/* the same to save a calculation step */
#define DSS_OP_AMP_OSC_NORTON_VP_IN		context->v_out_high

static DISCRETE_STEP(dss_op_amp_osc)
{
	const  discrete_op_amp_osc_info *info    = (const  discrete_op_amp_osc_info *)node->custom;
	struct dss_op_amp_osc_context   *context = (struct dss_op_amp_osc_context *)node->context;


	double i;				/* Charging current created by vIn */
	double v = 0;			/* all input voltages mixed */
	double dt;				/* change in time */
	double v_cap;			/* Current voltage on capacitor, before dt */
	double v_cap_next = 0;	/* Voltage on capacitor, after dt */
	double charge[2]  = {0};
	double x_time  = 0;		/* time since change happened */
	double exponent;
	UINT8 force_charge = 0;
	UINT8 enable = DSS_OP_AMP_OSC__ENABLE;
	UINT8 update_exponent = 0;
	UINT8 flip_flop = context->flip_flop;
	int count_f = 0;
	int count_r = 0;

	dt = node->info->sample_time;	/* Change in time */
	v_cap = context->v_cap;	/* Set to voltage before change */

	/* work out the charge currents/voltages. */
	switch (context->type)
	{
		case DISC_OP_AMP_OSCILLATOR_VCO_1:
			/* Work out the charge rates. */
			/* i is not a current.  It is being used as a temp variable. */
			i = DSS_OP_AMP_OSC__VMOD1 * context->temp1;
			charge[0] = (DSS_OP_AMP_OSC__VMOD1 - i) / info->r1;
			charge[1] = (i - (DSS_OP_AMP_OSC__VMOD1 * context->temp2)) / context->temp3;
			break;

		case DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON:
		{
			/* resistors can be nodes, so everything needs updating */
			double i1, i2;
			/* Work out the charge rates. */
			charge[0] = DSS_OP_AMP_OSC_NORTON_VP_IN / *context->r1;
			charge[1] = (context->v_out_high - OP_AMP_NORTON_VBE) / *context->r2 - charge[0];
			/* Work out the Inverting Schmitt thresholds. */
			i1 = DSS_OP_AMP_OSC_NORTON_VP_IN / *context->r5;
			i2 = (0.0 - OP_AMP_NORTON_VBE) / *context->r4;
			context->threshold_low  = (i1 + i2) * *context->r3 + OP_AMP_NORTON_VBE;
			i2 = (context->v_out_high - OP_AMP_NORTON_VBE) / *context->r4;
			context->threshold_high = (i1 + i2) * *context->r3 + OP_AMP_NORTON_VBE;
			break;
		}

		case DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON:
			/* Millman the input voltages. */
			if (info->r7 == 0)
			{
				/* No r7 means that the modulation circuit is fed directly into the circuit. */
				v = DSS_OP_AMP_OSC__VMOD1;
			}
			else
			{
				/* we need to mix any bias and all modulation voltages together. */
				i  = context->i_fixed;
				i += DSS_OP_AMP_OSC__VMOD1 / info->r7;
				if (info->r8 != 0)
					i += DSS_OP_AMP_OSC__VMOD2 / info->r8;
				v  = i * context->r_total;
			}

			/* Work out the charge rates. */
			v -= OP_AMP_NORTON_VBE;
			charge[0] = v / info->r1;
			charge[1] = v / info->r2 - charge[0];

			/* use the real enable circuit */
			force_charge = !enable;
			enable = 1;
			break;

		case DISC_OP_AMP_OSCILLATOR_VCO_2 | DISC_OP_AMP_IS_NORTON:
			/* Work out the charge rates. */
			i = DSS_OP_AMP_OSC__VMOD1 / info->r1;
			charge[0] = i - context->temp1;
			charge[1] = context->temp2 - i;
			/* if the negative pin current is less then the positive pin current, */
			/* then the osc is disabled and the cap keeps charging */
			if (charge[0] < 0)
			{
				force_charge =  1;
				charge[0]  *= -1;
			}
			break;

		case DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON:
			/* we need to mix any bias and all modulation voltages together. */
			charge[0] = context->i_fixed;
			v = DSS_OP_AMP_OSC__VMOD1 - OP_AMP_NORTON_VBE;
			if (v < 0) v = 0;
			charge[0] += v / info->r1;
			if (info->r6 != 0)
			{
				v = DSS_OP_AMP_OSC__VMOD2 - OP_AMP_NORTON_VBE;
				charge[0] += v / info->r6;
			}
			charge[1] = context->temp1 - charge[0];
			break;
	}

	if (!enable)
	{
		/* we will just output 0 for oscillators that have no real enable. */
		node->output[0] = 0;
		return;
	}

	/* Keep looping until all toggling in time sample is used up. */
	do
	{
		if (context->is_linear_charge)
		{
			if ((flip_flop ^ context->flip_flop_xor) || force_charge)
			{
				/* Charging */
				/* iC=C*dv/dt  works out to dv=iC*dt/C */
				v_cap_next = v_cap + (charge[1] * dt / info->c);
				dt = 0;

				/* has it charged past upper limit? */
				if (v_cap_next > context->threshold_high)
				{
					flip_flop = context->flip_flop_xor;
					if (flip_flop)
						count_r++;
					else
						count_f++;
					if (force_charge)
					{
						/* we need to keep charging the cap to the max thereby disabling the circuit */
						if (v_cap_next > context->v_out_high)
							v_cap_next = context->v_out_high;
					}
					else
					{
						/* calculate the overshoot time */
						dt = info->c * (v_cap_next - context->threshold_high) / charge[1];
						x_time = dt;
						v_cap_next = context->threshold_high;
					}
				}
			}
			else
			{
				/* Discharging */
				v_cap_next = v_cap - (charge[0] * dt / info->c);
				dt     = 0;

				/* has it discharged past lower limit? */
				if (v_cap_next < context->threshold_low)
				{
					flip_flop = !context->flip_flop_xor;
					if (flip_flop)
						count_r++;
					else
						count_f++;
					/* calculate the overshoot time */
					dt = info->c * (context->threshold_low - v_cap_next) / charge[0];
					x_time = dt;
					v_cap_next = context->threshold_low;
				}
			}
		}
		else	/* non-linear charge */
		{
			if (update_exponent)
				exponent = RC_CHARGE_EXP_DT(context->charge_rc, dt);
			else
				exponent = context->charge_exp;

			if (flip_flop)
			{
				/* Charging */
				v_cap_next = v_cap + ((context->v_out_high - v_cap) * exponent);
				dt = 0;

				/* Has it charged past upper limit? */
				if (v_cap_next > context->threshold_high)
				{
					dt = context->charge_rc  * log(1.0 / (1.0 - ((v_cap_next - context->threshold_high) / (context->v_out_high - v_cap))));
					x_time = dt;
					v_cap_next = 0;
					v_cap_next = context->threshold_high;
					flip_flop = 0;
					count_f++;
					update_exponent = 1;
				}
			}
			else
			{
				/* Discharging */
				v_cap_next = v_cap - (v_cap * exponent);
				dt = 0;

				/* has it discharged past lower limit? */
				if (v_cap_next < context->threshold_low)
				{
					dt = context->charge_rc * log(1.0 / (1.0 - ((context->threshold_low - v_cap_next) / v_cap)));
					x_time = dt;
					v_cap_next = context->threshold_low;
					flip_flop = 1;
					count_r++;
					update_exponent = 1;
				}
			}
		}
		v_cap = v_cap_next;
	} while(dt);
	context->v_cap = v_cap;

	x_time = dt / node->info->sample_time;

	switch (context->output_type)
	{
		case DISC_OP_AMP_OSCILLATOR_OUT_CAP:
			node->output[0] = v_cap;
			break;
		case DISC_OP_AMP_OSCILLATOR_OUT_ENERGY:
			if (x_time == 0) x_time = 1.0;
			node->output[0]  = context->v_out_high * (flip_flop ? x_time : (1.0 - x_time));
			break;
		case DISC_OP_AMP_OSCILLATOR_OUT_SQW:
			if (count_f + count_r >= 2)
				/* force at least 1 toggle */
				node->output[0] = context->flip_flop ? 0 : context->v_out_high;
			else
				node->output[0] = flip_flop * context->v_out_high;
			break;
		case DISC_OP_AMP_OSCILLATOR_OUT_COUNT_F_X:
			node->output[0] = count_f ? count_f + x_time : count_f;
			break;
		case DISC_OP_AMP_OSCILLATOR_OUT_COUNT_R_X:
			node->output[0] =  count_r ? count_r + x_time : count_r;
			break;
		case DISC_OP_AMP_OSCILLATOR_OUT_LOGIC_X:
			node->output[0] = context->flip_flop + x_time;
			break;
	}
	context->flip_flop = flip_flop;
}

static DISCRETE_RESET(dss_op_amp_osc)
{
	const discrete_op_amp_osc_info *info = (const discrete_op_amp_osc_info *)node->custom;
	struct dss_op_amp_osc_context *context = (struct dss_op_amp_osc_context *)node->context;
	const double *r_info_ptr;
	const double **r_context_ptr;
	int loop;
	node_description *r_node;

	double i1 = 0;	/* inverting input current */
	double i2 = 0;	/* non-inverting input current */

	/* link to resistor static or node values */
	r_info_ptr    = &info->r1;
	r_context_ptr = &context->r1;
	for (loop = 0; loop < 8; loop ++)
	{
		if IS_VALUE_A_NODE(*r_info_ptr)
		{
			r_node = discrete_find_node(node->info, *r_info_ptr);
			*r_context_ptr = &(r_node->output[NODE_CHILD_NODE_NUM((int)*r_info_ptr)]);
		}
		else
			*r_context_ptr = r_info_ptr;
		r_info_ptr++;
		r_context_ptr++;
	}

	context->is_linear_charge = 1;
	context->output_type = info->type & DISC_OP_AMP_OSCILLATOR_OUT_MASK;
	context->type        = info->type & DISC_OP_AMP_OSCILLATOR_TYPE_MASK;

	switch (context->type)
	{
		case DISC_OP_AMP_OSCILLATOR_VCO_1:
			/* The charge rates vary depending on vMod so they are not precalculated. */
			/* Charges while FlipFlop High */
			context->flip_flop_xor = 0;
			/* Work out the Non-inverting Schmitt thresholds. */
			context->temp1 = (info->vP / 2) / info->r4;
			context->temp2 = (info->vP - OP_AMP_VP_RAIL_OFFSET) / info->r3;
			context->temp3 = 1.0 / (1.0 / info->r3 + 1.0 / info->r4);
			context->threshold_low  =  context->temp1 * context->temp3;
			context->threshold_high = (context->temp1 + context->temp2) * context->temp3;
			/* There is no charge on the cap so the schmitt goes high at init. */
			context->flip_flop = 1;
			/* Setup some commonly used stuff */
			context->temp1 = info->r5 / (info->r2 + info->r5);			/* voltage ratio across r5 */
			context->temp2 = info->r6 / (info->r1 + info->r6);			/* voltage ratio across r6 */
			context->temp3 = 1.0 / (1.0 / info->r1 + 1.0 / info->r6);	/* input resistance when r6 switched in */
			break;

		case DISC_OP_AMP_OSCILLATOR_2 | DISC_OP_AMP_IS_NORTON:
			context->is_linear_charge = 0;
			context->charge_rc = info->r1 * info->c;
			context->charge_exp = RC_CHARGE_EXP(context->charge_rc);
			context->threshold_low  = (info->vP - OP_AMP_NORTON_VBE) / info->r4;
			context->threshold_high = context->threshold_low + (info->vP - OP_AMP_VP_RAIL_OFFSET - OP_AMP_VP_RAIL_OFFSET) / info->r3;;
			context->threshold_low  = context->threshold_low * info->r2 + OP_AMP_NORTON_VBE;
			context->threshold_high = context->threshold_high * info->r2 + OP_AMP_NORTON_VBE;
			/* fall through */

		case DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON:
			/* Charges while FlipFlop High */
			context->flip_flop_xor = 0;
			/* There is no charge on the cap so the schmitt inverter goes high at init. */
			context->flip_flop = 1;
			break;

		case DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON:
			/* Charges while FlipFlop Low */
			context->flip_flop_xor = 1;
			/* There is no charge on the cap so the schmitt goes low at init. */
			context->flip_flop = 0;
			/* The charge rates vary depending on vMod so they are not precalculated. */
			/* But we can precalculate the fixed currents. */
			context->i_fixed = 0;
			if (info->r6 != 0) context->i_fixed += info->vP / info->r6;
			context->i_fixed += OP_AMP_NORTON_VBE / info->r1;
			context->i_fixed += OP_AMP_NORTON_VBE / info->r2;
			/* Work out the input resistance to be used later to calculate the Millman voltage. */
			context->r_total = 1.0 / info->r1 + 1.0 / info->r2 + 1.0 / info->r7;
			if (info->r6) context->r_total += 1.0 / info->r6;
			if (info->r8) context->r_total += 1.0 / info->r8;
			context->r_total = 1.0 / context->r_total;
			/* Work out the Non-inverting Schmitt thresholds. */
			i1 = (info->vP - OP_AMP_NORTON_VBE) / info->r5;
			i2 = (info->vP - OP_AMP_NORTON_VBE - OP_AMP_NORTON_VBE) / info->r4;
			context->threshold_low = (i1 - i2) * info->r3 + OP_AMP_NORTON_VBE;
			i2 = (0.0 - OP_AMP_NORTON_VBE) / info->r4;
			context->threshold_high = (i1 - i2) * info->r3 + OP_AMP_NORTON_VBE;
			break;

		case DISC_OP_AMP_OSCILLATOR_VCO_2 | DISC_OP_AMP_IS_NORTON:
			/* Charges while FlipFlop High */
			context->flip_flop_xor = 0;
			/* There is no charge on the cap so the schmitt inverter goes high at init. */
			context->flip_flop = 1;
			/* Work out the charge rates. */
			context->temp1 = (info->vP - OP_AMP_NORTON_VBE) / info->r2;
			context->temp2 = (info->vP - OP_AMP_NORTON_VBE) * (1.0 / info->r2 + 1.0 / info->r6);
			/* Work out the Inverting Schmitt thresholds. */
			i1 = (info->vP - OP_AMP_NORTON_VBE) / info->r5;
			i2 = (0.0 - OP_AMP_NORTON_VBE) / info->r4;
			context->threshold_low = (i1 + i2) * info->r3 + OP_AMP_NORTON_VBE;
			i2 = (info->vP - OP_AMP_NORTON_VBE - OP_AMP_NORTON_VBE) / info->r4;
			context->threshold_high = (i1 + i2) * info->r3 + OP_AMP_NORTON_VBE;
			break;

		case DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON:
			/* Charges while FlipFlop High */
			context->flip_flop_xor = 0;
			/* There is no charge on the cap so the schmitt inverter goes high at init. */
			context->flip_flop = 1;
			/* Work out the charge rates. */
			/* The charge rates vary depending on vMod so they are not precalculated. */
			/* But we can precalculate the fixed currents. */
			if (info->r7 != 0) context->i_fixed = (info->vP - OP_AMP_NORTON_VBE) / info->r7;
			context->temp1 = (info->vP - OP_AMP_NORTON_VBE - OP_AMP_NORTON_VBE) / info->r2;
			/* Work out the Inverting Schmitt thresholds. */
			i1 = (info->vP - OP_AMP_NORTON_VBE) / info->r5;
			i2 = (0.0 - OP_AMP_NORTON_VBE) / info->r4;
			context->threshold_low = (i1 + i2) * info->r3 + OP_AMP_NORTON_VBE;
			i2 = (info->vP - OP_AMP_NORTON_VBE - OP_AMP_NORTON_VBE) / info->r4;
			context->threshold_high = (i1 + i2) * info->r3 + OP_AMP_NORTON_VBE;
			break;
	}

	context->v_out_high = info->vP - ((context->type & DISC_OP_AMP_IS_NORTON) ? OP_AMP_NORTON_VBE : OP_AMP_VP_RAIL_OFFSET);
	context->v_cap      = 0;

	DISCRETE_STEP_CALL(dss_op_amp_osc);
}


/************************************************************************
 *
 * DSS_SAWTOOTHWAVE - Usage of node_description values for step function
 *
 * input0    - Enable input value
 * input1    - Frequency input value
 * input2    - Amplitde input value
 * input3    - DC Bias Value
 * input4    - Gradient
 * input5    - Initial Phase
 *
 ************************************************************************/
#define DSS_SAWTOOTHWAVE__ENABLE	DISCRETE_INPUT(0)
#define DSS_SAWTOOTHWAVE__FREQ		DISCRETE_INPUT(1)
#define DSS_SAWTOOTHWAVE__AMP		DISCRETE_INPUT(2)
#define DSS_SAWTOOTHWAVE__BIAS		DISCRETE_INPUT(3)
#define DSS_SAWTOOTHWAVE__GRAD		DISCRETE_INPUT(4)
#define DSS_SAWTOOTHWAVE__PHASE		DISCRETE_INPUT(5)

static DISCRETE_STEP(dss_sawtoothwave)
{
	struct dss_sawtoothwave_context *context = (struct dss_sawtoothwave_context *)node->context;

	if(DSS_SAWTOOTHWAVE__ENABLE)
	{
		node->output[0]  = (context->type == 0) ? context->phase * (DSS_SAWTOOTHWAVE__AMP / (2.0 * M_PI)) : DSS_SAWTOOTHWAVE__AMP - (context->phase * (DSS_SAWTOOTHWAVE__AMP / (2.0 * M_PI)));
		node->output[0] -= DSS_SAWTOOTHWAVE__AMP / 2.0;
		/* Add DC Bias component */
		node->output[0] = node->output[0] + DSS_SAWTOOTHWAVE__BIAS;
	}
	else
	{
		node->output[0] = 0;
	}

	/* Work out the phase step based on phase/freq & sample rate */
	/* The enable input only curtails output, phase rotation     */
	/* still occurs                                              */
	/*     phase step = 2Pi/(output period/sample period)        */
	/*                    boils out to                           */
	/*     phase step = (2Pi*output freq)/sample freq)           */
	/* Also keep the new phasor in the 2Pi range.                */
	context->phase = fmod((context->phase + ((2.0 * M_PI * DSS_SAWTOOTHWAVE__FREQ) / node->info->sample_rate)), 2.0 * M_PI);
}

static DISCRETE_RESET(dss_sawtoothwave)
{
	struct dss_sawtoothwave_context *context = (struct dss_sawtoothwave_context *)node->context;
	double start;

	/* Establish starting phase, convert from degrees to radians */
	start = (DSS_SAWTOOTHWAVE__PHASE / 360.0) * (2.0 * M_PI);
	/* Make sure its always mod 2Pi */
	context->phase=fmod(start, 2.0 * M_PI);

	/* Invert gradient depending on sawtooth type /|/|/|/|/| or |\|\|\|\|\ */
	context->type=(DSS_SAWTOOTHWAVE__GRAD) ? 1 : 0;

	/* Step the node to set the output */
	DISCRETE_STEP_CALL(dss_sawtoothwave);
}


/************************************************************************
 *
 * DSS_SCHMITT_OSC - Schmitt feedback oscillator
 *
 * input0    - Enable input value
 * input1    - Vin
 * input2    - Amplitude
 *
 * also passed discrete_schmitt_osc_disc structure
 *
 * Mar 2004, D Renaud.
 ************************************************************************/
#define DSS_SCHMITT_OSC__ENABLE	(int)DISCRETE_INPUT(0)
#define DSS_SCHMITT_OSC__VIN	DISCRETE_INPUT(1)
#define DSS_SCHMITT_OSC__AMP	DISCRETE_INPUT(2)

static DISCRETE_STEP(dss_schmitt_osc)
{
	const  discrete_schmitt_osc_desc *info    = (const  discrete_schmitt_osc_desc *)node->custom;
	struct dss_schmitt_osc_context   *context = (struct dss_schmitt_osc_context *)node->context;

	double supply, v_cap, new_vCap, t, exponent;

	/* We will always oscillate.  The enable just affects the output. */
	v_cap    = context->v_cap;
	exponent = context->exponent;

	/* Keep looping until all toggling in time sample is used up. */
	do
	{
		t = 0;
		/* The charging voltage to the cap is the sum of the input voltage and the gate
         * output voltage in the ratios determined by their resistors in a divider network.
         * The input voltage is selectable as straight voltage in or logic level that will
         * use vGate as its voltage.  Note that ration_in is just the ratio of the total
         * voltage and needs to be multipled by the input voltage.  ratio_feedback has
         * already been multiplied by vGate to save time because that voltage never changes. */
		supply   = context->input_is_voltage ? context->ration_in * DSS_SCHMITT_OSC__VIN : (DSS_SCHMITT_OSC__VIN ? context->ration_in * info->vGate : 0);
		supply  += (context->state ? context->ratio_feedback : 0);
		new_vCap = v_cap + ((supply - v_cap) * exponent);
		if (context->state)
		{
			/* Charging */
			/* has it charged past upper limit? */
			if (new_vCap > info->trshRise)
			{
				/* calculate the overshoot time */
				t = context->rc * log(1.0 / (1.0 - ((new_vCap - info->trshRise) / (info->vGate - v_cap))));
				/* calculate new exponent because of reduced time */
				exponent = RC_CHARGE_EXP_DT(context->rc, t);
				v_cap    = new_vCap = info->trshRise;
				context->state = 0;
			}
		}
		else
		{
			/* Discharging */
			/* has it discharged past lower limit? */
			if (new_vCap < info->trshFall)
			{
				/* calculate the overshoot time */
				t = context->rc * log(1.0 / (1.0 - ((info->trshFall - new_vCap) / v_cap)));
				/* calculate new exponent because of reduced time */
				exponent = RC_CHARGE_EXP_DT(context->rc, t);
				v_cap    = new_vCap = info->trshFall;
				context->state = 1;
			}
		}
	} while(t);

	context->v_cap = new_vCap;

	switch (context->enable_type)
	{
		case DISC_SCHMITT_OSC_ENAB_IS_AND:
			node->output[0] = DSS_SCHMITT_OSC__ENABLE && context->state;
			break;
		case DISC_SCHMITT_OSC_ENAB_IS_NAND:
			node->output[0] = !(DSS_SCHMITT_OSC__ENABLE && context->state);
			break;
		case DISC_SCHMITT_OSC_ENAB_IS_OR:
			node->output[0] = DSS_SCHMITT_OSC__ENABLE || context->state;
			break;
		case DISC_SCHMITT_OSC_ENAB_IS_NOR:
			node->output[0] = !(DSS_SCHMITT_OSC__ENABLE || context->state);
			break;
	}
	node->output[0] *= DSS_SCHMITT_OSC__AMP;
}

static DISCRETE_RESET(dss_schmitt_osc)
{
	const  discrete_schmitt_osc_desc *info    = (const  discrete_schmitt_osc_desc *)node->custom;
	struct dss_schmitt_osc_context   *context = (struct dss_schmitt_osc_context *)node->context;
	double rSource;

	context->enable_type      =  info->options & DISC_SCHMITT_OSC_ENAB_MASK;
	context->input_is_voltage = (info->options & DISC_SCHMITT_OSC_IN_IS_VOLTAGE) ? 1 : 0;

	/* The 2 resistors make a voltage divider, so their ratios add together
     * to make the charging voltage. */
	context->ration_in      = info->rFeedback / (info->rIn + info->rFeedback);
	context->ratio_feedback = info->rIn / (info->rIn + info->rFeedback) * info->vGate;

	/* The voltage source resistance works out to the 2 resistors in parallel.
     * So use this for the RC charge constant. */
	rSource     = 1.0 / ((1.0 / info->rIn) + (1.0 / info->rFeedback));
	context->rc = rSource * info->c;
	context->exponent = RC_CHARGE_EXP(context->rc);

	/* Cap is at 0V on power up.  Causing output to be high. */
	context->v_cap = 0;
	context->state = 1;

	node->output[0] = info->options ? 0 : DSS_SCHMITT_OSC__AMP;
}


/************************************************************************
 *
 * DSS_SINEWAVE - Usage of node_description values for step function
 *
 * input0    - Enable input value
 * input1    - Frequency input value
 * input2    - Amplitude input value
 * input3    - DC Bias
 * input4    - Starting phase
 *
 ************************************************************************/
#define DSS_SINEWAVE__ENABLE	DISCRETE_INPUT(0)
#define DSS_SINEWAVE__FREQ		DISCRETE_INPUT(1)
#define DSS_SINEWAVE__AMPL		DISCRETE_INPUT(2)
#define DSS_SINEWAVE__BIAS		DISCRETE_INPUT(3)
#define DSS_SINEWAVE__PHASE		DISCRETE_INPUT(4)

static DISCRETE_STEP(dss_sinewave)
{
	struct dss_sinewave_context *context = (struct dss_sinewave_context *)node->context;

	/* Set the output */
	if(DSS_SINEWAVE__ENABLE)
	{
		node->output[0] = (DSS_SINEWAVE__AMPL / 2.0) * sin(context->phase);
		/* Add DC Bias component */
		node->output[0] = node->output[0] + DSS_SINEWAVE__BIAS;
	}
	else
	{
		node->output[0] = 0;
	}

	/* Work out the phase step based on phase/freq & sample rate */
	/* The enable input only curtails output, phase rotation     */
	/* still occurs                                              */
	/*     phase step = 2Pi/(output period/sample period)        */
	/*                    boils out to                           */
	/*     phase step = (2Pi*output freq)/sample freq)           */
	/* Also keep the new phasor in the 2Pi range.                */
	context->phase=fmod((context->phase + ((2.0 * M_PI * DSS_SINEWAVE__FREQ) / node->info->sample_rate)), 2.0 * M_PI);
}

static DISCRETE_RESET(dss_sinewave)
{
	struct dss_sinewave_context *context = (struct dss_sinewave_context *)node->context;
	double start;

	/* Establish starting phase, convert from degrees to radians */
	start = (DSS_SINEWAVE__PHASE / 360.0) * (2.0 * M_PI);
	/* Make sure its always mod 2Pi */
	context->phase = fmod(start, 2.0 * M_PI);
	/* Step the output to make it correct */
	DISCRETE_STEP_CALL(dss_sinewave);
}


/************************************************************************
 *
 * DSS_SQUAREWAVE - Usage of node_description values for step function
 *
 * input0    - Enable input value
 * input1    - Frequency input value
 * input2    - Amplitude input value
 * input3    - Duty Cycle
 * input4    - DC Bias level
 * input5    - Start Phase
 *
 ************************************************************************/
#define DSS_SQUAREWAVE__ENABLE	DISCRETE_INPUT(0)
#define DSS_SQUAREWAVE__FREQ	DISCRETE_INPUT(1)
#define DSS_SQUAREWAVE__AMP		DISCRETE_INPUT(2)
#define DSS_SQUAREWAVE__DUTY	DISCRETE_INPUT(3)
#define DSS_SQUAREWAVE__BIAS	DISCRETE_INPUT(4)
#define DSS_SQUAREWAVE__PHASE	DISCRETE_INPUT(5)

static DISCRETE_STEP(dss_squarewave)
{
	struct dss_squarewave_context *context = (struct dss_squarewave_context *)node->context;

	/* Establish trigger phase from duty */
	context->trigger=((100-DSS_SQUAREWAVE__DUTY)/100)*(2.0*M_PI);

	/* Set the output */
	if(DSS_SQUAREWAVE__ENABLE)
	{
		if(context->phase>context->trigger)
			node->output[0] = DSS_SQUAREWAVE__AMP /2.0;
		else
			node->output[0] =- DSS_SQUAREWAVE__AMP / 2.0;

		/* Add DC Bias component */
		node->output[0] = node->output[0] + DSS_SQUAREWAVE__BIAS;
	}
	else
	{
		node->output[0] = 0;
	}

	/* Work out the phase step based on phase/freq & sample rate */
	/* The enable input only curtails output, phase rotation     */
	/* still occurs                                              */
	/*     phase step = 2Pi/(output period/sample period)        */
	/*                    boils out to                           */
	/*     phase step = (2Pi*output freq)/sample freq)           */
	/* Also keep the new phasor in the 2Pi range.                */
	context->phase=fmod(context->phase + ((2.0 * M_PI * DSS_SQUAREWAVE__FREQ) / node->info->sample_rate), 2.0 * M_PI);
}

static DISCRETE_RESET(dss_squarewave)
{
	struct dss_squarewave_context *context = (struct dss_squarewave_context *)node->context;
	double start;

	/* Establish starting phase, convert from degrees to radians */
	start = (DSS_SQUAREWAVE__PHASE / 360.0) * (2.0 * M_PI);
	/* Make sure its always mod 2Pi */
	context->phase = fmod(start, 2.0 * M_PI);

	/* Step the output */
	DISCRETE_STEP_CALL(dss_squarewave);
}

/************************************************************************
 *
 * DSS_SQUAREWFIX - Usage of node_description values for step function
 *
 * input0    - Enable input value
 * input1    - Frequency input value
 * input2    - Amplitude input value
 * input3    - Duty Cycle
 * input4    - DC Bias level
 * input5    - Start Phase
 *
 ************************************************************************/
#define DSS_SQUAREWFIX__ENABLE	DISCRETE_INPUT(0)
#define DSS_SQUAREWFIX__FREQ	DISCRETE_INPUT(1)
#define DSS_SQUAREWFIX__AMP		DISCRETE_INPUT(2)
#define DSS_SQUAREWFIX__DUTY	DISCRETE_INPUT(3)
#define DSS_SQUAREWFIX__BIAS	DISCRETE_INPUT(4)
#define DSS_SQUAREWFIX__PHASE	DISCRETE_INPUT(5)

static DISCRETE_STEP(dss_squarewfix)
{
	struct dss_squarewfix_context *context = (struct dss_squarewfix_context *)node->context;

	context->t_left -= context->sample_step;

	/* The enable input only curtails output, phase rotation still occurs */
	while (context->t_left <= 0)
	{
		context->flip_flop = context->flip_flop ? 0 : 1;
		context->t_left   += context->flip_flop ? context->t_on : context->t_off;
	}

	if(DSS_SQUAREWFIX__ENABLE)
	{
		/* Add gain and DC Bias component */

		context->t_off  = 1.0 / DSS_SQUAREWFIX__FREQ;	/* cycle time */
		context->t_on   = context->t_off * (DSS_SQUAREWFIX__DUTY / 100.0);
		context->t_off -= context->t_on;

		node->output[0] = (context->flip_flop ? DSS_SQUAREWFIX__AMP / 2.0 : -(DSS_SQUAREWFIX__AMP / 2.0)) + DSS_SQUAREWFIX__BIAS;
	}
	else
	{
		node->output[0]=0;
	}
}

static DISCRETE_RESET(dss_squarewfix)
{
	struct dss_squarewfix_context *context = (struct dss_squarewfix_context *)node->context;

	context->sample_step = 1.0 / node->info->sample_rate;
	context->flip_flop   = 1;

	/* Do the intial time shift and convert freq to off/on times */
	context->t_off   = 1.0 / DSS_SQUAREWFIX__FREQ;	/* cycle time */
	context->t_left  = DSS_SQUAREWFIX__PHASE / 360.0;	/* convert start phase to % */
	context->t_left  = context->t_left - (int)context->t_left;	/* keep % between 0 & 1 */
	context->t_left  = (context->t_left < 0) ? 1.0 + context->t_left : context->t_left;	/* if - then flip to + phase */
	context->t_left *= context->t_off;
	context->t_on    = context->t_off * (DSS_SQUAREWFIX__DUTY / 100.0);
	context->t_off  -= context->t_on;

	context->t_left = -context->t_left;

	/* toggle output and work out intial time shift */
	while (context->t_left <= 0)
	{
		context->flip_flop = context->flip_flop ? 0 : 1;
		context->t_left   += context->flip_flop ? context->t_on : context->t_off;
	}

	/* Step the output */
	DISCRETE_STEP_CALL(dss_squarewfix);
}


/************************************************************************
 *
 * DSS_SQUAREWAVE2 - Usage of node_description values
 *
 * input0    - Enable input value
 * input1    - Amplitude input value
 * input2    - OFF Time
 * input3    - ON Time
 * input4    - DC Bias level
 * input5    - Initial Time Shift
 *
 ************************************************************************/
#define DSS_SQUAREWAVE2__ENABLE	DISCRETE_INPUT(0)
#define DSS_SQUAREWAVE2__AMP	DISCRETE_INPUT(1)
#define DSS_SQUAREWAVE2__T_OFF	DISCRETE_INPUT(2)
#define DSS_SQUAREWAVE2__T_ON	DISCRETE_INPUT(3)
#define DSS_SQUAREWAVE2__BIAS	DISCRETE_INPUT(4)
#define DSS_SQUAREWAVE2__SHIFT	DISCRETE_INPUT(5)

static DISCRETE_STEP(dss_squarewave2)
{
	struct dss_squarewave_context *context = (struct dss_squarewave_context *)node->context;
	double newphase;

	if(DSS_SQUAREWAVE2__ENABLE)
	{
		/* Establish trigger phase from time periods */
		context->trigger = (DSS_SQUAREWAVE2__T_OFF / (DSS_SQUAREWAVE2__T_OFF + DSS_SQUAREWAVE2__T_ON)) * (2.0 * M_PI);

		/* Work out the phase step based on phase/freq & sample rate */
		/* The enable input only curtails output, phase rotation     */
		/* still occurs                                              */

		/*     phase step = 2Pi/(output period/sample period)        */
		/*                    boils out to                           */
		/*     phase step = 2Pi/(output period*sample freq)          */
		newphase = context->phase + ((2.0 * M_PI) / ((DSS_SQUAREWAVE2__T_OFF + DSS_SQUAREWAVE2__T_ON) * node->info->sample_rate));
		/* Keep the new phasor in the 2Pi range.*/
		context->phase = fmod(newphase, 2.0 * M_PI);

		if(context->phase>context->trigger)
			node->output[0] = DSS_SQUAREWAVE2__AMP / 2.0;
		else
			node->output[0] = -DSS_SQUAREWAVE2__AMP / 2.0;

		/* Add DC Bias component */
		node->output[0] = node->output[0] + DSS_SQUAREWAVE2__BIAS;
	}
	else
	{
		node->output[0] = 0;
	}
}

static DISCRETE_RESET(dss_squarewave2)
{
	struct dss_squarewave_context *context = (struct dss_squarewave_context *)node->context;
	double start;

	/* Establish starting phase, convert from degrees to radians */
	/* Only valid if we have set the on/off time                 */
	if((DSS_SQUAREWAVE2__T_OFF + DSS_SQUAREWAVE2__T_ON) != 0.0)
		start = (DSS_SQUAREWAVE2__SHIFT / (DSS_SQUAREWAVE2__T_OFF + DSS_SQUAREWAVE2__T_ON)) * (2.0 * M_PI);
	else
		start = 0.0;
	/* Make sure its always mod 2Pi */
	context->phase = fmod(start, 2.0 * M_PI);

	/* Step the output */
	DISCRETE_STEP_CALL(dss_squarewave2);
}

/************************************************************************
 *
 * DSS_INVERTER_OSC - Usage of node_description values
 *
 * input0    - Enable input value
 * input1    - RC Resistor
 * input2    - RP Resistor
 * input3    - C Capacitor
 * input4    - Desc
 *
 ************************************************************************/
#define DSS_INVERTER_OSC__ENABLE	DISCRETE_INPUT(0)
#define DSS_INVERTER_OSC__MOD		DISCRETE_INPUT(1)
#define DSS_INVERTER_OSC__RC		DISCRETE_INPUT(2)
#define DSS_INVERTER_OSC__RP		DISCRETE_INPUT(3)
#define DSS_INVERTER_OSC__C			DISCRETE_INPUT(4)
#define DSS_INVERTER_OSC__R2		DISCRETE_INPUT(5)

INLINE double dss_inverter_tftab(const node_description *node, double x)
{
	const  discrete_inverter_osc_desc *info    = (const  discrete_inverter_osc_desc *)node->custom;
	struct dss_inverter_osc_context   *context = (struct dss_inverter_osc_context *)node->context;

	x = x / info->vB;
	if (x > 0)
	 	return info->vB * exp(-context->tf_a * pow(x, context->tf_b));
	else
		return info->vB;
}

INLINE double dss_inverter_tf(const node_description *node, double x)
{
	const  discrete_inverter_osc_desc *info    = (const  discrete_inverter_osc_desc *)node->custom;
	struct dss_inverter_osc_context   *context = (struct dss_inverter_osc_context *)node->context;

	if (x < 0.0)
		return info->vB;
	else if (x <= info->vB)
		return context->tf_tab[(int)((double)(DSS_INV_TAB_SIZE - 1) * x / info->vB)];
	else
		return context->tf_tab[DSS_INV_TAB_SIZE - 1];
}

static DISCRETE_STEP(dss_inverter_osc)
{
	struct dss_inverter_osc_context   *context = (struct dss_inverter_osc_context *)node->context;
	const  discrete_inverter_osc_desc *info = (const  discrete_inverter_osc_desc *)node->custom;

	double diff, vG1, vG2, vG3, vI;
	double vMix, rMix;

	/* Get new state */
	vI = context->v_cap + context->v_g2_old;
	switch (info->options & DISC_OSC_INVERTER_TYPE_MASK)
	{
		case DISC_OSC_INVERTER_IS_TYPE1:
		case DISC_OSC_INVERTER_IS_TYPE3:
			vG1 = dss_inverter_tf(node, vI);
			vG2 = dss_inverter_tf(node, vG1);
			vG3 = dss_inverter_tf(node, vG2);
			break;
		case DISC_OSC_INVERTER_IS_TYPE2:
			vG1 = 0;
			vG3 = dss_inverter_tf(node, vI);
			vG2 = dss_inverter_tf(node, vG3);
			break;
		case DISC_OSC_INVERTER_IS_TYPE4:
			vI  = MIN(DSS_INVERTER_OSC__ENABLE, vI + 0.7);
			vG1 = 0;
			vG3 = dss_inverter_tf(node, vI);
			vG2 = dss_inverter_tf(node, vG3);
			break;
		case DISC_OSC_INVERTER_IS_TYPE5:
			vI  = MAX(DSS_INVERTER_OSC__ENABLE, vI - 0.7);
			vG1 = 0;
			vG3 = dss_inverter_tf(node,vI);
			vG2 = dss_inverter_tf(node,vG3);
			break;
		default:
			fatalerror("DISCRETE_INVERTER_OSC - Wrong type on NODE_%02d", NODE_BLOCKINDEX(node));
	}
	switch (info->options & DISC_OSC_INVERTER_TYPE_MASK)
	{
		case DISC_OSC_INVERTER_IS_TYPE1:
		case DISC_OSC_INVERTER_IS_TYPE2:
		case DISC_OSC_INVERTER_IS_TYPE3:
			if ((info->clamp >= 0.0) && ((vI< - info->clamp) || (vI> info->vB+info->clamp)))
			{
				vI = MAX(vI, (- info->clamp));
				vI = MIN(vI, info->vB + info->clamp);
				diff = vG3 * (context->rp / (context->rp + context->r1))
				     - (context->v_cap + vG2)
				     + vI*(context->r1 / (context->rp + context->r1));
				diff = diff - diff * context->wc;
			}
			else
			{
				diff = vG3 - (context->v_cap + vG2);
				diff = diff - diff * context->w;
			}
			break;
		case DISC_OSC_INVERTER_IS_TYPE4:
			if ((info->clamp >= 0.0) && ((vI< - info->clamp) || (vI> info->vB+info->clamp)))
			{
				vI = MAX(vI, (- info->clamp));
				vI = MIN(vI, info->vB + info->clamp);
			}
			/*  FIXME handle r2 = 0  */
			rMix = (context->r1 * context->r2) / (context->r1 + context->r2);
			vMix = rMix* ((vG3 - vG2) / context->r1 + (DSS_INVERTER_OSC__MOD-vG2) / context->r2);
			if (vMix < (vI-vG2-0.7))
			{
				rMix = 1.0 / rMix + 1.0 / context->rp;
				rMix = 1.0 / rMix;
				vMix = rMix* ( (vG3-vG2) / context->r1 + (DSS_INVERTER_OSC__MOD-vG2) / context->r2 + (vI-0.7-vG2)/context->rp);
			}
			diff = vMix - context->v_cap;
			diff = diff - diff * exp(-node->info->sample_time / (context->c * rMix));
			break;
		case DISC_OSC_INVERTER_IS_TYPE5:
			if ((info->clamp >= 0.0) && ((vI< - info->clamp) || (vI> info->vB+info->clamp)))
			{
				vI = MAX(vI, (- info->clamp));
				vI = MIN(vI, info->vB + info->clamp);
			}
			/*  FIXME handle r2 = 0  */
			rMix = (context->r1 * context->r2) / (context->r1 + context->r2);
			vMix = rMix* ((vG3 - vG2) / context->r1 + (DSS_INVERTER_OSC__MOD-vG2) / context->r2);
			if (vMix > (vI -vG2 +0.7))
			{
				rMix = 1.0 / rMix + 1.0/context->rp;
				rMix = 1.0 / rMix;
				vMix = rMix* ( (vG3 - vG2) / context->r1 + (DSS_INVERTER_OSC__MOD-vG2) / context->r2 + (vI+0.7-vG2)/context->rp);
			}
			diff = vMix - context->v_cap;
			diff = diff - diff * exp(-node->info->sample_time/(context->c * rMix));
			break;
		default:
			fatalerror("DISCRETE_INVERTER_OSC - Wrong type on NODE_%02d", NODE_BLOCKINDEX(node));
	}
	context->v_cap   += diff;
	context->v_g2_old = vG2;
	if ((info->options & DISC_OSC_INVERTER_TYPE_MASK) == DISC_OSC_INVERTER_IS_TYPE3)
		node->output[0] = vG1;
	else
		node->output[0] = vG3;
	if (info->options & DISC_OSC_INVERTER_OUT_IS_LOGIC)
		node->output[0] = (node->output[0] > info->vInFall);
}

static DISCRETE_RESET(dss_inverter_osc)
{
	struct dss_inverter_osc_context   *context = (struct dss_inverter_osc_context *)node->context;
	const  discrete_inverter_osc_desc *info    = (const  discrete_inverter_osc_desc *)node->custom;

	int i;

	/* exponent */
	context->w  = exp(-node->info->sample_time / (DSS_INVERTER_OSC__RC * DSS_INVERTER_OSC__C));
	context->wc = exp(-node->info->sample_time / ((DSS_INVERTER_OSC__RC * DSS_INVERTER_OSC__RP) / (DSS_INVERTER_OSC__RP + DSS_INVERTER_OSC__RC) * DSS_INVERTER_OSC__C));
	node->output[0]   = 0;
	context->v_cap    = 0;
	context->v_g2_old = 0;
	context->rp   = DSS_INVERTER_OSC__RP;
	context->r1   = DSS_INVERTER_OSC__RC;
	context->r2   = DSS_INVERTER_OSC__R2;
	context->c    = DSS_INVERTER_OSC__C;
	context->tf_b = (log(0.0 - log(info->vOutLow/info->vB)) - log(0.0 - log((info->vOutHigh/info->vB))) ) / log(info->vInRise / info->vInFall);
	context->tf_a = log(0.0 - log(info->vOutLow/info->vB)) - context->tf_b * log(info->vInRise/info->vB);
	context->tf_a = exp(context->tf_a);

	for (i = 0; i < DSS_INV_TAB_SIZE; i++)
	{
		context->tf_tab[i] = dss_inverter_tftab(node, (double)i / (double)(DSS_INV_TAB_SIZE - 1) * info->vB);
	}
}

/************************************************************************
 *
 * DSS_TRIANGLEWAVE - Usage of node_description values for step function
 *
 * input0    - Enable input value
 * input1    - Frequency input value
 * input2    - Amplitde input value
 * input3    - DC Bias value
 * input4    - Initial Phase
 *
 ************************************************************************/
#define DSS_TRIANGLEWAVE__ENABLE	DISCRETE_INPUT(0)
#define DSS_TRIANGLEWAVE__FREQ		DISCRETE_INPUT(1)
#define DSS_TRIANGLEWAVE__AMP		DISCRETE_INPUT(2)
#define DSS_TRIANGLEWAVE__BIAS		DISCRETE_INPUT(3)
#define DSS_TRIANGLEWAVE__PHASE		DISCRETE_INPUT(4)

static DISCRETE_STEP(dss_trianglewave)
{
	struct dss_trianglewave_context *context = (struct dss_trianglewave_context *)node->context;

	if(DSS_TRIANGLEWAVE__ENABLE)
	{
		node->output[0] = context->phase < M_PI ? (DSS_TRIANGLEWAVE__AMP * (context->phase / (M_PI / 2.0) - 1.0)) / 2.0 :
									(DSS_TRIANGLEWAVE__AMP * (3.0 - context->phase / (M_PI / 2.0))) / 2.0 ;

		/* Add DC Bias component */
		node->output[0] = node->output[0] + DSS_TRIANGLEWAVE__BIAS;
	}
	else
	{
		node->output[0] = 0;
	}

	/* Work out the phase step based on phase/freq & sample rate */
	/* The enable input only curtails output, phase rotation     */
	/* still occurs                                              */
	/*     phase step = 2Pi/(output period/sample period)        */
	/*                    boils out to                           */
	/*     phase step = (2Pi*output freq)/sample freq)           */
	/* Also keep the new phasor in the 2Pi range.                */
	context->phase=fmod((context->phase + ((2.0 * M_PI * DSS_TRIANGLEWAVE__FREQ) / node->info->sample_rate)), 2.0 * M_PI);
}

static DISCRETE_RESET(dss_trianglewave)
{
	struct dss_trianglewave_context *context = (struct dss_trianglewave_context *)node->context;
	double start;

	/* Establish starting phase, convert from degrees to radians */
	start = (DSS_TRIANGLEWAVE__PHASE / 360.0) * (2.0 * M_PI);
	/* Make sure its always mod 2Pi */
	context->phase=fmod(start, 2.0 * M_PI);

	/* Step to set the output */
	DISCRETE_STEP_CALL(dss_trianglewave);
}


/************************************************************************
 *
 * DSS_ADSR - Attack Decay Sustain Release
 *
 * input0    - Enable input value
 * input1    - Trigger value
 * input2    - gain scaling factor
 *
 ************************************************************************/
#define DSS_ADSR__ENABLE	DISCRETE_INPUT(0)

static DISCRETE_STEP(dss_adsrenv)
{
/*  struct dss_adsr_context *context = node->context; */

	if(DSS_ADSR__ENABLE)
	{
		node->output[0] = 0;
	}
	else
	{
		node->output[0] = 0;
	}
}


static DISCRETE_RESET(dss_adsrenv)
{
	DISCRETE_STEP_CALL(dss_adsrenv);
}
