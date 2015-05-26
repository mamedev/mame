// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *  Written by K.Wilkins (mame@esplexo.co.uk)
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
#define DSS_COUNTER__ENABLE     DISCRETE_INPUT(0)
#define DSS_COUNTER__RESET      DISCRETE_INPUT(1)
#define DSS_COUNTER__CLOCK      DISCRETE_INPUT(2)
#define DSS_COUNTER__MIN        DISCRETE_INPUT(3)
#define DSS_COUNTER__MAX        DISCRETE_INPUT(4)
#define DSS_COUNTER__DIR        DISCRETE_INPUT(5)
#define DSS_COUNTER__INIT       DISCRETE_INPUT(6)
#define DSS_COUNTER__CLOCK_TYPE DISCRETE_INPUT(7)
#define DSS_7492__CLOCK_TYPE     DSS_COUNTER__MIN

static const int disc_7492_count[6] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x06};

DISCRETE_STEP(dss_counter)
{
	double  cycles;
	double  ds_clock;
	int     clock = 0, inc = 0;
	UINT32  last_count = m_last_count;  /* it is different then output in 7492 */
	double  x_time = 0;
	UINT32  count = last_count;

	ds_clock = DSS_COUNTER__CLOCK;
	if (UNEXPECTED(m_clock_type == DISC_CLK_IS_FREQ))
	{
		/* We need to keep clocking the internal clock even if disabled. */
		cycles = (m_t_left + this->sample_time()) * ds_clock;
		inc    = (int)cycles;
		m_t_left = (cycles - inc) / ds_clock;
		if (inc) x_time = m_t_left / this->sample_time();
	}
	else
	{
		clock  = (int)ds_clock;
		/* x_time from input clock */
		x_time = ds_clock - clock;
	}


	/* If reset enabled then set output to the reset value.  No x_time in reset. */
	if (UNEXPECTED(DSS_COUNTER__RESET))
	{
		m_last_count = (int)DSS_COUNTER__INIT;
		set_output(0, (int)DSS_COUNTER__INIT);
		return;
	}

	/*
	 * Only count if module is enabled.
	 * This has the effect of holding the output at it's current value.
	 */
	if (EXPECTED(DSS_COUNTER__ENABLE))
	{
		double v_out;

		switch (m_clock_type)
		{
			case DISC_CLK_ON_F_EDGE:
			case DISC_CLK_ON_R_EDGE:
				/* See if the clock has toggled to the proper edge */
				clock = (clock != 0);
				if (m_last_clock != clock)
				{
					m_last_clock = clock;
					if (m_clock_type == clock)
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

		/* use loops because init is not always min or max */
		if (DSS_COUNTER__DIR)
		{
			count += inc;
			while (count > m_max)
			{
				count -= m_diff;
			}
		}
		else
		{
			count -= inc;
			while (count < m_min || count > (0xffffffff - inc))
			{
				count += m_diff;
			}
		}

		m_last_count = count;
		v_out = m_is_7492 ? disc_7492_count[count] : count;

		if (UNEXPECTED(count != last_count))
		{
			/* the x_time is only output if the output changed. */
			switch (m_out_type)
			{
				case DISC_OUT_HAS_XTIME:
					v_out += x_time;
					break;
				case DISC_OUT_IS_ENERGY:
					if (x_time == 0) x_time = 1.0;
					v_out = last_count;
					if (count > last_count)
						v_out += (count - last_count) * x_time;
					else
						v_out -= (last_count - count) * x_time;
					break;
			}
		}
		set_output(0, v_out);
	}
}

DISCRETE_RESET(dss_counter)
{
	if ((int)DSS_COUNTER__CLOCK_TYPE & DISC_COUNTER_IS_7492)
	{
		m_is_7492    = 1;
		m_clock_type = DSS_7492__CLOCK_TYPE;
		m_max = 5;
		m_min = 0;
		m_diff = 6;
	}
	else
	{
		m_is_7492    = 0;
		m_clock_type = DSS_COUNTER__CLOCK_TYPE;
		m_max = DSS_COUNTER__MAX;
		m_min = DSS_COUNTER__MIN;
		m_diff = m_max - m_min + 1;
	}


	if (!m_is_7492 && (DSS_COUNTER__MAX < DSS_COUNTER__MIN))
		fatalerror("MAX < MIN in NODE_%02d\n", this->index());

	m_out_type    = m_clock_type & DISC_OUT_MASK;
	m_clock_type &= DISC_CLK_MASK;

	m_t_left = 0;
	m_last_count = 0;
	m_last_clock = 0;
	set_output(0, DSS_COUNTER__INIT); /* count starts at reset value */
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
#define DSS_LFSR_NOISE__ENABLE  DISCRETE_INPUT(0)
#define DSS_LFSR_NOISE__RESET   DISCRETE_INPUT(1)
#define DSS_LFSR_NOISE__CLOCK   DISCRETE_INPUT(2)
#define DSS_LFSR_NOISE__AMP     DISCRETE_INPUT(3)
#define DSS_LFSR_NOISE__FEED    DISCRETE_INPUT(4)
#define DSS_LFSR_NOISE__BIAS    DISCRETE_INPUT(5)

INLINE int dss_lfsr_function(discrete_device *dev, int myfunc, int in0, int in1, int bitmask)
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
			retval = retval ^ bitmask;  /* Invert output */
			break;
		case DISC_LFSR_NOR:
			retval = in0 | in1;
			retval = retval ^ bitmask;  /* Invert output */
			break;
		case DISC_LFSR_NAND:
			retval = in0 & in1;
			retval = retval ^ bitmask;  /* Invert output */
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
			dev->discrete_log("dss_lfsr_function - Invalid function type passed");
			retval=0;
			break;
	}
	return retval;
}


DISCRETE_STEP(dss_lfsr_noise)
{
	DISCRETE_DECLARE_INFO(discrete_lfsr_desc)

	double cycles;
	int clock, inc = 0;
	int fb0, fb1, fbresult = 0, noise_feed;

	if (info->clock_type == DISC_CLK_IS_FREQ)
	{
		/* We need to keep clocking the internal clock even if disabled. */
		cycles = (m_t_left + this->sample_time()) / m_t_clock;
		inc    = (int)cycles;
		m_t_left = (cycles - inc) * m_t_clock;
	}

	/* Reset everything if necessary */
	if(((DSS_LFSR_NOISE__RESET == 0) ? 0 : 1) == m_reset_on_high)
	{
		this->reset();
		return;
	}

	switch (info->clock_type)
	{
		case DISC_CLK_ON_F_EDGE:
		case DISC_CLK_ON_R_EDGE:
			/* See if the clock has toggled to the proper edge */
			clock = (DSS_LFSR_NOISE__CLOCK != 0);
			if (m_last != clock)
			{
				m_last = clock;
				if (info->clock_type == clock)
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
		double v_out;

		noise_feed = (DSS_LFSR_NOISE__FEED ? 0x01 : 0x00);
		for (clock = 0; clock < inc; clock++)
		{
			/* Fetch the last feedback result */
			fbresult = (m_lfsr_reg >> info->bitlength) & 0x01;

			/* Stage 2 feedback combine fbresultNew with infeed bit */
			fbresult = dss_lfsr_function(m_device, info->feedback_function1, fbresult, noise_feed, 0x01);

			/* Stage 3 first we setup where the bit is going to be shifted into */
			fbresult = fbresult * info->feedback_function2_mask;
			/* Then we left shift the register, */
			m_lfsr_reg = m_lfsr_reg << 1;
			/* Now move the fbresult into the shift register and mask it to the bitlength */
			m_lfsr_reg = dss_lfsr_function(m_device, info->feedback_function2, fbresult, m_lfsr_reg, (1 << info->bitlength) - 1 );

			/* Now get and store the new feedback result */
			/* Fetch the feedback bits */
			fb0 = (m_lfsr_reg >> info->feedback_bitsel0) & 0x01;
			fb1 = (m_lfsr_reg >> info->feedback_bitsel1) & 0x01;
			/* Now do the combo on them */
			fbresult = dss_lfsr_function(m_device, info->feedback_function0, fb0, fb1, 0x01);
			m_lfsr_reg = dss_lfsr_function(m_device, DISC_LFSR_REPLACE, m_lfsr_reg, fbresult << info->bitlength, (2 << info->bitlength) - 1);

		}
		/* Now select the output bit */
		if (m_out_is_f0)
			v_out = fbresult & 0x01;
		else
			v_out = (m_lfsr_reg >> info->output_bit) & 0x01;

		/* Final inversion if required */
		if (m_invert_output) v_out = v_out ? 0 : 1;

		/* Gain stage */
		v_out = v_out ? DSS_LFSR_NOISE__AMP / 2 : -DSS_LFSR_NOISE__AMP / 2;
		/* Bias input as required */
		v_out = v_out + DSS_LFSR_NOISE__BIAS;

		set_output(0, v_out);

		/* output the lfsr reg ?*/
		if (m_out_lfsr_reg)
			set_output(1, (double) m_lfsr_reg);

	}
	if(!DSS_LFSR_NOISE__ENABLE)
	{
		set_output(0, 0);
	}
}

DISCRETE_RESET(dss_lfsr_noise)
{
	DISCRETE_DECLARE_INFO(discrete_lfsr_desc)

	int    fb0 , fb1, fbresult;
	double v_out;

	m_reset_on_high = (info->flags & DISC_LFSR_FLAG_RESET_TYPE_H) ? 1 : 0;
	m_invert_output = info->flags & DISC_LFSR_FLAG_OUT_INVERT;
	m_out_is_f0 = (info->flags & DISC_LFSR_FLAG_OUTPUT_F0) ? 1 : 0;
	m_out_lfsr_reg = (info->flags & DISC_LFSR_FLAG_OUTPUT_SR_SN1) ? 1 : 0;

	if ((info->clock_type < DISC_CLK_ON_F_EDGE) || (info->clock_type > DISC_CLK_IS_FREQ))
		m_device->discrete_log("Invalid clock type passed in NODE_%d\n", this->index());

	m_last = (DSS_COUNTER__CLOCK != 0);
	if (info->clock_type == DISC_CLK_IS_FREQ) m_t_clock = 1.0 / DSS_LFSR_NOISE__CLOCK;
	m_t_left = 0;

	m_lfsr_reg = info->reset_value;

	/* Now get and store the new feedback result */
	/* Fetch the feedback bits */
	fb0 = (m_lfsr_reg >> info->feedback_bitsel0) & 0x01;
	fb1=(m_lfsr_reg >> info->feedback_bitsel1) & 0x01;
	/* Now do the combo on them */
	fbresult = dss_lfsr_function(m_device, info->feedback_function0, fb0, fb1, 0x01);
	m_lfsr_reg=dss_lfsr_function(m_device, DISC_LFSR_REPLACE, m_lfsr_reg, fbresult << info->bitlength, (2<< info->bitlength ) - 1);

	/* Now select and setup the output bit */
	v_out = (m_lfsr_reg >> info->output_bit) & 0x01;

	/* Final inversion if required */
	if(info->flags & DISC_LFSR_FLAG_OUT_INVERT) v_out = v_out ? 0 : 1;

	/* Gain stage */
	v_out = v_out ? DSS_LFSR_NOISE__AMP / 2 : -DSS_LFSR_NOISE__AMP / 2;
	/* Bias input as required */
	v_out += DSS_LFSR_NOISE__BIAS;

	set_output(0, v_out);
	set_output(1, 0);
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
#define DSS_NOISE__ENABLE   DISCRETE_INPUT(0)
#define DSS_NOISE__FREQ     DISCRETE_INPUT(1)
#define DSS_NOISE__AMP      DISCRETE_INPUT(2)
#define DSS_NOISE__BIAS     DISCRETE_INPUT(3)

DISCRETE_STEP(dss_noise)
{
	double v_out;

	if(DSS_NOISE__ENABLE)
	{
		/* Only sample noise on rollover to next cycle */
		if(m_phase > (2.0 * M_PI))
		{
			/* GCC's rand returns a RAND_MAX value of 0x7fff */
			int newval = (m_device->machine().rand() & 0x7fff) - 16384;

			/* make sure the peak to peak values are the amplitude */
			v_out = DSS_NOISE__AMP / 2;
			if (newval > 0)
				v_out *= ((double)newval / 16383);
			else
				v_out *= ((double)newval / 16384);

			/* Add DC Bias component */
			v_out += DSS_NOISE__BIAS;
			set_output(0, v_out);
		}
	}
	else
	{
		set_output(0, 0);
	}

	/* Keep the new phasor in the 2Pi range.*/
	m_phase = fmod(m_phase, 2.0 * M_PI);

	/* The enable input only curtails output, phase rotation still occurs. */
	/* We allow the phase to exceed 2Pi here, so we can tell when to sample the noise. */
	m_phase += ((2.0 * M_PI * DSS_NOISE__FREQ) / this->sample_rate());
}


DISCRETE_RESET(dss_noise)
{
	m_phase=0;
	this->step();
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
	#define DSS_NOTE__ENABLE        DISCRETE_INPUT(0)
	#define DSS_NOTE__CLOCK     DISCRETE_INPUT(1)
	#define DSS_NOTE__DATA          DISCRETE_INPUT(2)
	#define DSS_NOTE__MAX1          DISCRETE_INPUT(3)
	#define DSS_NOTE__MAX2          DISCRETE_INPUT(4)
	#define DSS_NOTE__CLOCK_TYPE    DISCRETE_INPUT(5)

DISCRETE_STEP(dss_note)
{
	double  cycles;
	int     clock  = 0, last_count2, inc = 0;
	double  x_time = 0;
	double  v_out;

	if (m_clock_type == DISC_CLK_IS_FREQ)
	{
		/* We need to keep clocking the internal clock even if disabled. */
		cycles = (m_t_left + this->sample_time()) / m_t_clock;
		inc    = (int)cycles;
		m_t_left = (cycles - inc) * m_t_clock;
		if (inc) x_time = m_t_left / this->sample_time();
	}
	else
	{
		/* separate clock info from x_time info. */
		clock = (int)DSS_NOTE__CLOCK;
		x_time = DSS_NOTE__CLOCK - clock;
	}

	if (DSS_NOTE__ENABLE)
	{
		last_count2 = m_count2;

		switch (m_clock_type)
		{
			case DISC_CLK_ON_F_EDGE:
			case DISC_CLK_ON_R_EDGE:
				/* See if the clock has toggled to the proper edge */
				clock = (clock != 0);
				if (m_last != clock)
				{
					m_last = clock;
					if (m_clock_type == clock)
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
				m_count1++;
				if (m_count1 > m_max1)
				{
					/* Max 1 count reached.  Load Data into counter. */
					m_count1  = (int)DSS_NOTE__DATA;
					m_count2 += 1;
					if (m_count2 > m_max2) m_count2 = 0;
				}
			}
		}

		v_out = m_count2;
		if (m_count2 != last_count2)
		{
			/* the x_time is only output if the output changed. */
			switch (m_out_type)
			{
				case DISC_OUT_IS_ENERGY:
					if (x_time == 0) x_time = 1.0;
					v_out = last_count2;
					if (m_count2 > last_count2)
						v_out += (m_count2 - last_count2) * x_time;
					else
						v_out -= (last_count2 - m_count2) * x_time;
					break;
				case DISC_OUT_HAS_XTIME:
					v_out += x_time;
					break;
			}
		}
		set_output(0, v_out);
	}
	else
		set_output(0, 0);
}

DISCRETE_RESET(dss_note)
{
	m_clock_type = (int)DSS_NOTE__CLOCK_TYPE & DISC_CLK_MASK;
	m_out_type   = (int)DSS_NOTE__CLOCK_TYPE & DISC_OUT_MASK;

	m_last    = (DSS_NOTE__CLOCK != 0);
	m_t_left  = 0;
	m_t_clock = 1.0 / DSS_NOTE__CLOCK;

	m_count1 = (int)DSS_NOTE__DATA;
	m_count2 = 0;
	m_max1   = (int)DSS_NOTE__MAX1;
	m_max2   = (int)DSS_NOTE__MAX2;
	set_output(0, 0);
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
#define DSS_OP_AMP_OSC__ENABLE  DISCRETE_INPUT(0)
#define DSS_OP_AMP_OSC__VMOD1   DISCRETE_INPUT(1)
#define DSS_OP_AMP_OSC__VMOD2   DISCRETE_INPUT(2)

/* The inputs on a norton op-amp are (info->vP - OP_AMP_NORTON_VBE) */
/* which is the same as the output high voltage.  We will define them */
/* the same to save a calculation step */
#define DSS_OP_AMP_OSC_NORTON_VP_IN     m_v_out_high

DISCRETE_STEP(dss_op_amp_osc)
{
	DISCRETE_DECLARE_INFO(discrete_op_amp_osc_info)

	double i = 0;               /* Charging current created by vIn */
	double v = 0;           /* all input voltages mixed */
	double dt;              /* change in time */
	double v_cap;           /* Current voltage on capacitor, before dt */
	double v_cap_next = 0;  /* Voltage on capacitor, after dt */
	double charge[2]  = {0};
	double x_time  = 0;     /* time since change happened */
	double exponent;
	UINT8 force_charge = 0;
	UINT8 enable = DSS_OP_AMP_OSC__ENABLE;
	UINT8 update_exponent = 0;
	UINT8 flip_flop = m_flip_flop;
	int count_f = 0;
	int count_r = 0;

	double v_out = 0;

	dt = this->sample_time();   /* Change in time */
	v_cap = m_v_cap;    /* Set to voltage before change */

	/* work out the charge currents/voltages. */
	switch (m_type)
	{
		case DISC_OP_AMP_OSCILLATOR_VCO_1:
			/* Work out the charge rates. */
			/* i is not a current.  It is being used as a temp variable. */
			i = DSS_OP_AMP_OSC__VMOD1 * m_temp1;
			charge[0] = (DSS_OP_AMP_OSC__VMOD1 - i) / info->r1;
			charge[1] = (i - (DSS_OP_AMP_OSC__VMOD1 * m_temp2)) / m_temp3;
			break;

		case DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON:
		{
			/* resistors can be nodes, so everything needs updating */
			double i1, i2;
			/* add in enable current if using real enable */
			if (m_has_enable)
			{
				if (enable)
					i = m_i_enable;
				enable = 1;
			}
			/* Work out the charge rates. */
			charge[0] = DSS_OP_AMP_OSC_NORTON_VP_IN / *m_r[1-1] - i;
			charge[1] = (m_v_out_high - OP_AMP_NORTON_VBE) / *m_r[2-1] - charge[0];
			/* Work out the Inverting Schmitt thresholds. */
			i1 = DSS_OP_AMP_OSC_NORTON_VP_IN / *m_r[5-1];
			i2 = (0.0 - OP_AMP_NORTON_VBE) / *m_r[4-1];
			m_threshold_low  = (i1 + i2) * *m_r[3-1] + OP_AMP_NORTON_VBE;
			i2 = (m_v_out_high - OP_AMP_NORTON_VBE) / *m_r[4-1];
			m_threshold_high = (i1 + i2) * *m_r[3-1] + OP_AMP_NORTON_VBE;
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
				i  = m_i_fixed;
				i += DSS_OP_AMP_OSC__VMOD1 / info->r7;
				if (info->r8 != 0)
					i += DSS_OP_AMP_OSC__VMOD2 / info->r8;
				v  = i * m_r_total;
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
			charge[0] = i - m_temp1;
			charge[1] = m_temp2 - i;
			/* if the negative pin current is less then the positive pin current, */
			/* then the osc is disabled and the cap keeps charging */
			if (charge[0] < 0)
			{
				force_charge =  1;
				charge[0]  *= -1;
			}
			break;

		case DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON:
			/* start with fixed bias */
			charge[0] = m_i_fixed;
			/* add in enable current if using real enable */
			if (m_has_enable)
			{
				if (enable)
					charge[0] -= m_i_enable;
				enable = 1;
			}
			/* we need to mix any bias and all modulation voltages together. */
			v = DSS_OP_AMP_OSC__VMOD1 - OP_AMP_NORTON_VBE;
			if (v < 0) v = 0;
			charge[0] += v / info->r1;
			if (info->r6 != 0)
			{
				v = DSS_OP_AMP_OSC__VMOD2 - OP_AMP_NORTON_VBE;
				charge[0] += v / info->r6;
			}
			charge[1] = m_temp1 - charge[0];
			break;
	}

	if (!enable)
	{
		/* we will just output 0 for oscillators that have no real enable. */
		set_output(0, 0);
		return;
	}

	/* Keep looping until all toggling in time sample is used up. */
	do
	{
		if (m_is_linear_charge)
		{
			if ((flip_flop ^ m_flip_flop_xor) || force_charge)
			{
				/* Charging */
				/* iC=C*dv/dt  works out to dv=iC*dt/C */
				v_cap_next = v_cap + (charge[1] * dt / info->c);
				dt = 0;

				/* has it charged past upper limit? */
				if (v_cap_next > m_threshold_high)
				{
					flip_flop = m_flip_flop_xor;
					if (flip_flop)
						count_r++;
					else
						count_f++;
					if (force_charge)
					{
						/* we need to keep charging the cap to the max thereby disabling the circuit */
						if (v_cap_next > m_v_out_high)
							v_cap_next = m_v_out_high;
					}
					else
					{
						/* calculate the overshoot time */
						dt = info->c * (v_cap_next - m_threshold_high) / charge[1];
						x_time = dt;
						v_cap_next = m_threshold_high;
					}
				}
			}
			else
			{
				/* Discharging */
				v_cap_next = v_cap - (charge[0] * dt / info->c);
				dt     = 0;

				/* has it discharged past lower limit? */
				if (v_cap_next < m_threshold_low)
				{
					flip_flop = !m_flip_flop_xor;
					if (flip_flop)
						count_r++;
					else
						count_f++;
					/* calculate the overshoot time */
					dt = info->c * (m_threshold_low - v_cap_next) / charge[0];
					x_time = dt;
					v_cap_next = m_threshold_low;
				}
			}
		}
		else    /* non-linear charge */
		{
			if (update_exponent)
				exponent = RC_CHARGE_EXP_DT(m_charge_rc[flip_flop], dt);
			else
				exponent = m_charge_exp[flip_flop];

			v_cap_next = v_cap + ((m_charge_v[flip_flop] - v_cap) * exponent);
			dt = 0;

			if (flip_flop)
			{
				/* Has it charged past upper limit? */
				if (v_cap_next > m_threshold_high)
				{
					dt = m_charge_rc[1]  * log(1.0 / (1.0 - ((v_cap_next - m_threshold_high) / (m_v_out_high - v_cap))));
					x_time = dt;
					v_cap_next = m_threshold_high;
					flip_flop = 0;
					count_f++;
					update_exponent = 1;
				}
			}
			else
			{
				/* has it discharged past lower limit? */
				if (v_cap_next < m_threshold_low)
				{
					dt = m_charge_rc[0] * log(1.0 / (1.0 - ((m_threshold_low - v_cap_next) / v_cap)));
					x_time = dt;
					v_cap_next = m_threshold_low;
					flip_flop = 1;
					count_r++;
					update_exponent = 1;
				}
			}
		}
		v_cap = v_cap_next;
	} while(dt);
	if (v_cap > m_v_out_high)
		v_cap = m_v_out_high;
	if (v_cap < 0)
		v_cap = 0;
	m_v_cap = v_cap;

	x_time = dt / this->sample_time();

	switch (m_output_type)
	{
		case DISC_OP_AMP_OSCILLATOR_OUT_CAP:
			v_out = v_cap;
			break;
		case DISC_OP_AMP_OSCILLATOR_OUT_ENERGY:
			if (x_time == 0) x_time = 1.0;
			v_out = m_v_out_high * (flip_flop ? x_time : (1.0 - x_time));
			break;
		case DISC_OP_AMP_OSCILLATOR_OUT_SQW:
			if (count_f + count_r >= 2)
				/* force at least 1 toggle */
				v_out = m_flip_flop ? 0 : m_v_out_high;
			else
				v_out = flip_flop * m_v_out_high;
			break;
		case DISC_OP_AMP_OSCILLATOR_OUT_COUNT_F_X:
			v_out = count_f ? count_f + x_time : count_f;
			break;
		case DISC_OP_AMP_OSCILLATOR_OUT_COUNT_R_X:
			v_out =  count_r ? count_r + x_time : count_r;
			break;
		case DISC_OP_AMP_OSCILLATOR_OUT_LOGIC_X:
			v_out = m_flip_flop + x_time;
			break;
	}
	set_output(0, v_out);
	m_flip_flop = flip_flop;
}

#define DIODE_DROP  0.7

DISCRETE_RESET(dss_op_amp_osc)
{
	DISCRETE_DECLARE_INFO(discrete_op_amp_osc_info)

	const double *r_info_ptr;
	int loop;

	double i1 = 0;  /* inverting input current */
	double i2 = 0;  /* non-inverting input current */

	/* link to resistor static or node values */
	r_info_ptr    = &info->r1;
	for (loop = 0; loop < 8; loop ++)
	{
		m_r[loop] = m_device->node_output_ptr(*r_info_ptr);
		if (m_r[loop] == NULL)
			m_r[loop] = r_info_ptr;
		r_info_ptr++;
	}

	m_is_linear_charge = 1;
	m_output_type = info->type & DISC_OP_AMP_OSCILLATOR_OUT_MASK;
	m_type        = info->type & DISC_OP_AMP_OSCILLATOR_TYPE_MASK;
	m_charge_rc[0] = 0;
	m_charge_rc[1] = 0;
	m_charge_v[0] = 0;
	m_charge_v[1] = 0;
	m_i_fixed = 0;
	m_has_enable = 0;

	switch (m_type)
	{
		case DISC_OP_AMP_OSCILLATOR_VCO_1:
			/* The charge rates vary depending on vMod so they are not precalculated. */
			/* Charges while FlipFlop High */
			m_flip_flop_xor = 0;
			/* Work out the Non-inverting Schmitt thresholds. */
			m_temp1 = (info->vP / 2) / info->r4;
			m_temp2 = (info->vP - OP_AMP_VP_RAIL_OFFSET) / info->r3;
			m_temp3 = 1.0 / (1.0 / info->r3 + 1.0 / info->r4);
			m_threshold_low  =  m_temp1 * m_temp3;
			m_threshold_high = (m_temp1 + m_temp2) * m_temp3;
			/* There is no charge on the cap so the schmitt goes high at init. */
			m_flip_flop = 1;
			/* Setup some commonly used stuff */
			m_temp1 = info->r5 / (info->r2 + info->r5);         /* voltage ratio across r5 */
			m_temp2 = info->r6 / (info->r1 + info->r6);         /* voltage ratio across r6 */
			m_temp3 = 1.0 / (1.0 / info->r1 + 1.0 / info->r6);  /* input resistance when r6 switched in */
			break;

		case DISC_OP_AMP_OSCILLATOR_1 | DISC_OP_AMP_IS_NORTON:
			/* Charges while FlipFlop High */
			m_flip_flop_xor = 0;
			/* There is no charge on the cap so the schmitt inverter goes high at init. */
			m_flip_flop = 1;
			/* setup current if using real enable */
			if (info->r6 > 0)
			{
				m_has_enable = 1;
				m_i_enable = (info->vP - OP_AMP_NORTON_VBE) / (info->r6 + RES_K(1));
			}
			break;

		case DISC_OP_AMP_OSCILLATOR_2 | DISC_OP_AMP_IS_NORTON:
			m_is_linear_charge = 0;
			/* First calculate the parallel charge resistors and volatges. */
			/* We can cheat and just calcuate the charges in the working area. */
			/* The thresholds are well past the effect of the voltage drop */
			/* and the component tolerances far exceed the .5V charge difference */
			if (info->r1 != 0)
			{
				m_charge_rc[0] = 1.0 / info->r1;
				m_charge_rc[1] = 1.0 / info->r1;
				m_charge_v[1] = (info->vP - OP_AMP_NORTON_VBE) / info->r1;
			}
			if (info->r5 != 0)
			{
				m_charge_rc[0] += 1.0 / info->r5;
				m_charge_v[0] = DIODE_DROP / info->r5;
			}
			if (info->r6 != 0)
			{
				m_charge_rc[1] += 1.0 / info->r6;
				m_charge_v[1] += (info->vP - OP_AMP_NORTON_VBE - DIODE_DROP) / info->r6;
			}
			m_charge_rc[0] += 1.0 / info->r2;
			m_charge_rc[0] = 1.0 / m_charge_rc[0];
			m_charge_v[0] += OP_AMP_NORTON_VBE / info->r2;
			m_charge_v[0] *= m_charge_rc[0];
			m_charge_rc[1] += 1.0 / info->r2;
			m_charge_rc[1] = 1.0 / m_charge_rc[1];
			m_charge_v[1] += OP_AMP_NORTON_VBE / info->r2;
			m_charge_v[1] *= m_charge_rc[1];

			m_charge_rc[0] *= info->c;
			m_charge_rc[1] *= info->c;
			m_charge_exp[0] = RC_CHARGE_EXP(m_charge_rc[0]);
			m_charge_exp[1] = RC_CHARGE_EXP(m_charge_rc[1]);
			m_threshold_low  = (info->vP - OP_AMP_NORTON_VBE) / info->r4;
			m_threshold_high = m_threshold_low + (info->vP - 2 * OP_AMP_NORTON_VBE) / info->r3;;
			m_threshold_low  = m_threshold_low * info->r2 + OP_AMP_NORTON_VBE;
			m_threshold_high = m_threshold_high * info->r2 + OP_AMP_NORTON_VBE;

			/* There is no charge on the cap so the schmitt inverter goes high at init. */
			m_flip_flop = 1;
			break;

		case DISC_OP_AMP_OSCILLATOR_VCO_1 | DISC_OP_AMP_IS_NORTON:
			/* Charges while FlipFlop Low */
			m_flip_flop_xor = 1;
			/* There is no charge on the cap so the schmitt goes low at init. */
			m_flip_flop = 0;
			/* The charge rates vary depending on vMod so they are not precalculated. */
			/* But we can precalculate the fixed currents. */
			if (info->r6 != 0) m_i_fixed += info->vP / info->r6;
			m_i_fixed += OP_AMP_NORTON_VBE / info->r1;
			m_i_fixed += OP_AMP_NORTON_VBE / info->r2;
			/* Work out the input resistance to be used later to calculate the Millman voltage. */
			m_r_total = 1.0 / info->r1 + 1.0 / info->r2 + 1.0 / info->r7;
			if (info->r6) m_r_total += 1.0 / info->r6;
			if (info->r8) m_r_total += 1.0 / info->r8;
			m_r_total = 1.0 / m_r_total;
			/* Work out the Non-inverting Schmitt thresholds. */
			i1 = (info->vP - OP_AMP_NORTON_VBE) / info->r5;
			i2 = (info->vP - OP_AMP_NORTON_VBE - OP_AMP_NORTON_VBE) / info->r4;
			m_threshold_low = (i1 - i2) * info->r3 + OP_AMP_NORTON_VBE;
			i2 = (0.0 - OP_AMP_NORTON_VBE) / info->r4;
			m_threshold_high = (i1 - i2) * info->r3 + OP_AMP_NORTON_VBE;
			break;

		case DISC_OP_AMP_OSCILLATOR_VCO_2 | DISC_OP_AMP_IS_NORTON:
			/* Charges while FlipFlop High */
			m_flip_flop_xor = 0;
			/* There is no charge on the cap so the schmitt inverter goes high at init. */
			m_flip_flop = 1;
			/* Work out the charge rates. */
			m_temp1 = (info->vP - OP_AMP_NORTON_VBE) / info->r2;
			m_temp2 = (info->vP - OP_AMP_NORTON_VBE) * (1.0 / info->r2 + 1.0 / info->r6);
			/* Work out the Inverting Schmitt thresholds. */
			i1 = (info->vP - OP_AMP_NORTON_VBE) / info->r5;
			i2 = (0.0 - OP_AMP_NORTON_VBE) / info->r4;
			m_threshold_low = (i1 + i2) * info->r3 + OP_AMP_NORTON_VBE;
			i2 = (info->vP - OP_AMP_NORTON_VBE - OP_AMP_NORTON_VBE) / info->r4;
			m_threshold_high = (i1 + i2) * info->r3 + OP_AMP_NORTON_VBE;
			break;

		case DISC_OP_AMP_OSCILLATOR_VCO_3 | DISC_OP_AMP_IS_NORTON:
			/* Charges while FlipFlop High */
			m_flip_flop_xor = 0;
			/* There is no charge on the cap so the schmitt inverter goes high at init. */
			m_flip_flop = 1;
			/* setup current if using real enable */
			if (info->r8 > 0)
			{
				m_has_enable = 1;
				m_i_enable = (info->vP - OP_AMP_NORTON_VBE) / (info->r8 + RES_K(1));
			}
			/* Work out the charge rates. */
			/* The charge rates vary depending on vMod so they are not precalculated. */
			/* But we can precalculate the fixed currents. */
			if (info->r7 != 0) m_i_fixed = (info->vP - OP_AMP_NORTON_VBE) / info->r7;
			m_temp1 = (info->vP - OP_AMP_NORTON_VBE - OP_AMP_NORTON_VBE) / info->r2;
			/* Work out the Inverting Schmitt thresholds. */
			i1 = (info->vP - OP_AMP_NORTON_VBE) / info->r5;
			i2 = (0.0 - OP_AMP_NORTON_VBE) / info->r4;
			m_threshold_low = (i1 + i2) * info->r3 + OP_AMP_NORTON_VBE;
			i2 = (info->vP - OP_AMP_NORTON_VBE - OP_AMP_NORTON_VBE) / info->r4;
			m_threshold_high = (i1 + i2) * info->r3 + OP_AMP_NORTON_VBE;
			break;
	}

	m_v_out_high = info->vP - ((m_type & DISC_OP_AMP_IS_NORTON) ? OP_AMP_NORTON_VBE : OP_AMP_VP_RAIL_OFFSET);
	m_v_cap      = 0;

	this->step();
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
#define DSS_SAWTOOTHWAVE__ENABLE    DISCRETE_INPUT(0)
#define DSS_SAWTOOTHWAVE__FREQ      DISCRETE_INPUT(1)
#define DSS_SAWTOOTHWAVE__AMP       DISCRETE_INPUT(2)
#define DSS_SAWTOOTHWAVE__BIAS      DISCRETE_INPUT(3)
#define DSS_SAWTOOTHWAVE__GRAD      DISCRETE_INPUT(4)
#define DSS_SAWTOOTHWAVE__PHASE     DISCRETE_INPUT(5)

DISCRETE_STEP(dss_sawtoothwave)
{
	double v_out;

	if(DSS_SAWTOOTHWAVE__ENABLE)
	{
		v_out = (m_type == 0) ? m_phase * (DSS_SAWTOOTHWAVE__AMP / (2.0 * M_PI)) : DSS_SAWTOOTHWAVE__AMP - (m_phase * (DSS_SAWTOOTHWAVE__AMP / (2.0 * M_PI)));
		v_out -= DSS_SAWTOOTHWAVE__AMP / 2.0;
		/* Add DC Bias component */
		v_out = v_out + DSS_SAWTOOTHWAVE__BIAS;
	}
	else
	{
		v_out = 0;
	}
	set_output(0, v_out);

	/* Work out the phase step based on phase/freq & sample rate */
	/* The enable input only curtails output, phase rotation     */
	/* still occurs                                              */
	/*     phase step = 2Pi/(output period/sample period)        */
	/*                    boils out to                           */
	/*     phase step = (2Pi*output freq)/sample freq)           */
	/* Also keep the new phasor in the 2Pi range.                */
	m_phase = fmod((m_phase + ((2.0 * M_PI * DSS_SAWTOOTHWAVE__FREQ) / this->sample_rate())), 2.0 * M_PI);
}

DISCRETE_RESET(dss_sawtoothwave)
{
	double start;

	/* Establish starting phase, convert from degrees to radians */
	start = (DSS_SAWTOOTHWAVE__PHASE / 360.0) * (2.0 * M_PI);
	/* Make sure its always mod 2Pi */
	m_phase = fmod(start, 2.0 * M_PI);

	/* Invert gradient depending on sawtooth type /|/|/|/|/| or |\|\|\|\|\ */
	m_type = (DSS_SAWTOOTHWAVE__GRAD) ? 1 : 0;

	/* Step the node to set the output */
	this->step();
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
#define DSS_SCHMITT_OSC__ENABLE (int)DISCRETE_INPUT(0)
#define DSS_SCHMITT_OSC__VIN    DISCRETE_INPUT(1)
#define DSS_SCHMITT_OSC__AMP    DISCRETE_INPUT(2)

DISCRETE_STEP(dss_schmitt_osc)
{
	DISCRETE_DECLARE_INFO(discrete_schmitt_osc_desc)

	double supply, v_cap, new_vCap, t, exponent;
	double v_out = 0;

	/* We will always oscillate.  The enable just affects the output. */
	v_cap    = m_v_cap;
	exponent = m_exponent;

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
		supply   = m_input_is_voltage ? m_ration_in * DSS_SCHMITT_OSC__VIN : (DSS_SCHMITT_OSC__VIN ? m_ration_in * info->vGate : 0);
		supply  += (m_state ? m_ratio_feedback : 0);
		new_vCap = v_cap + ((supply - v_cap) * exponent);
		if (m_state)
		{
			/* Charging */
			/* has it charged past upper limit? */
			if (new_vCap > info->trshRise)
			{
				/* calculate the overshoot time */
				t = m_rc * log(1.0 / (1.0 - ((new_vCap - info->trshRise) / (info->vGate - v_cap))));
				/* calculate new exponent because of reduced time */
				exponent = RC_CHARGE_EXP_DT(m_rc, t);
				v_cap    = new_vCap = info->trshRise;
				m_state = 0;
			}
		}
		else
		{
			/* Discharging */
			/* has it discharged past lower limit? */
			if (new_vCap < info->trshFall)
			{
				/* calculate the overshoot time */
				t = m_rc * log(1.0 / (1.0 - ((info->trshFall - new_vCap) / v_cap)));
				/* calculate new exponent because of reduced time */
				exponent = RC_CHARGE_EXP_DT(m_rc, t);
				v_cap    = new_vCap = info->trshFall;
				m_state = 1;
			}
		}
	} while(t);

	m_v_cap = new_vCap;

	switch (m_enable_type)
	{
		case DISC_SCHMITT_OSC_ENAB_IS_AND:
			v_out = DSS_SCHMITT_OSC__ENABLE && m_state;
			break;
		case DISC_SCHMITT_OSC_ENAB_IS_NAND:
			v_out = !(DSS_SCHMITT_OSC__ENABLE && m_state);
			break;
		case DISC_SCHMITT_OSC_ENAB_IS_OR:
			v_out = DSS_SCHMITT_OSC__ENABLE || m_state;
			break;
		case DISC_SCHMITT_OSC_ENAB_IS_NOR:
			v_out = !(DSS_SCHMITT_OSC__ENABLE || m_state);
			break;
	}
	v_out *= DSS_SCHMITT_OSC__AMP;
	set_output(0, v_out);
}

DISCRETE_RESET(dss_schmitt_osc)
{
	DISCRETE_DECLARE_INFO(discrete_schmitt_osc_desc)

	double rSource;

	m_enable_type      =  info->options & DISC_SCHMITT_OSC_ENAB_MASK;
	m_input_is_voltage = (info->options & DISC_SCHMITT_OSC_IN_IS_VOLTAGE) ? 1 : 0;

	/* The 2 resistors make a voltage divider, so their ratios add together
	 * to make the charging voltage. */
	m_ration_in      = info->rFeedback / (info->rIn + info->rFeedback);
	m_ratio_feedback = info->rIn / (info->rIn + info->rFeedback) * info->vGate;

	/* The voltage source resistance works out to the 2 resistors in parallel.
	 * So use this for the RC charge constant. */
	rSource     = 1.0 / ((1.0 / info->rIn) + (1.0 / info->rFeedback));
	m_rc = rSource * info->c;
	m_exponent = RC_CHARGE_EXP(m_rc);

	/* Cap is at 0V on power up.  Causing output to be high. */
	m_v_cap = 0;
	m_state = 1;

	set_output(0, info->options ? 0 : DSS_SCHMITT_OSC__AMP);
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
#define DSS_SINEWAVE__ENABLE    DISCRETE_INPUT(0)
#define DSS_SINEWAVE__FREQ      DISCRETE_INPUT(1)
#define DSS_SINEWAVE__AMPL      DISCRETE_INPUT(2)
#define DSS_SINEWAVE__BIAS      DISCRETE_INPUT(3)
#define DSS_SINEWAVE__PHASE     DISCRETE_INPUT(4)

DISCRETE_STEP(dss_sinewave)
{
	/* Set the output */
	if(DSS_SINEWAVE__ENABLE)
	{
		set_output(0, (DSS_SINEWAVE__AMPL / 2.0) * sin(m_phase) + DSS_SINEWAVE__BIAS);
		/* Add DC Bias component */
	}
	else
	{
		set_output(0, 0);
	}

	/* Work out the phase step based on phase/freq & sample rate */
	/* The enable input only curtails output, phase rotation     */
	/* still occurs                                              */
	/*     phase step = 2Pi/(output period/sample period)        */
	/*                    boils out to                           */
	/*     phase step = (2Pi*output freq)/sample freq)           */
	/* Also keep the new phasor in the 2Pi range.                */
	m_phase=fmod((m_phase + ((2.0 * M_PI * DSS_SINEWAVE__FREQ) / this->sample_rate())), 2.0 * M_PI);
}

DISCRETE_RESET(dss_sinewave)
{
	double start;

	/* Establish starting phase, convert from degrees to radians */
	start = (DSS_SINEWAVE__PHASE / 360.0) * (2.0 * M_PI);
	/* Make sure its always mod 2Pi */
	m_phase = fmod(start, 2.0 * M_PI);
	/* Step the output to make it correct */
	this->step();
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
#define DSS_SQUAREWAVE__ENABLE  DISCRETE_INPUT(0)
#define DSS_SQUAREWAVE__FREQ    DISCRETE_INPUT(1)
#define DSS_SQUAREWAVE__AMP     DISCRETE_INPUT(2)
#define DSS_SQUAREWAVE__DUTY    DISCRETE_INPUT(3)
#define DSS_SQUAREWAVE__BIAS    DISCRETE_INPUT(4)
#define DSS_SQUAREWAVE__PHASE   DISCRETE_INPUT(5)

DISCRETE_STEP(dss_squarewave)
{
	/* Establish trigger phase from duty */
	m_trigger=((100-DSS_SQUAREWAVE__DUTY)/100)*(2.0*M_PI);

	/* Set the output */
	if(DSS_SQUAREWAVE__ENABLE)
	{
		if(m_phase>m_trigger)
			set_output(0, DSS_SQUAREWAVE__AMP / 2.0 + DSS_SQUAREWAVE__BIAS);
		else
			set_output(0, - DSS_SQUAREWAVE__AMP / 2.0 + DSS_SQUAREWAVE__BIAS);
		/* Add DC Bias component */
	}
	else
	{
		set_output(0, 0);
	}

	/* Work out the phase step based on phase/freq & sample rate */
	/* The enable input only curtails output, phase rotation     */
	/* still occurs                                              */
	/*     phase step = 2Pi/(output period/sample period)        */
	/*                    boils out to                           */
	/*     phase step = (2Pi*output freq)/sample freq)           */
	/* Also keep the new phasor in the 2Pi range.                */
	m_phase=fmod(m_phase + ((2.0 * M_PI * DSS_SQUAREWAVE__FREQ) / this->sample_rate()), 2.0 * M_PI);
}

DISCRETE_RESET(dss_squarewave)
{
	double start;

	/* Establish starting phase, convert from degrees to radians */
	start = (DSS_SQUAREWAVE__PHASE / 360.0) * (2.0 * M_PI);
	/* Make sure its always mod 2Pi */
	m_phase = fmod(start, 2.0 * M_PI);

	/* Step the output */
	this->step();
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
#define DSS_SQUAREWFIX__ENABLE  DISCRETE_INPUT(0)
#define DSS_SQUAREWFIX__FREQ    DISCRETE_INPUT(1)
#define DSS_SQUAREWFIX__AMP     DISCRETE_INPUT(2)
#define DSS_SQUAREWFIX__DUTY    DISCRETE_INPUT(3)
#define DSS_SQUAREWFIX__BIAS    DISCRETE_INPUT(4)
#define DSS_SQUAREWFIX__PHASE   DISCRETE_INPUT(5)

DISCRETE_STEP(dss_squarewfix)
{
	m_t_left -= m_sample_step;

	/* The enable input only curtails output, phase rotation still occurs */
	while (m_t_left <= 0)
	{
		m_flip_flop = m_flip_flop ? 0 : 1;
		m_t_left   += m_flip_flop ? m_t_on : m_t_off;
	}

	if(DSS_SQUAREWFIX__ENABLE)
	{
		/* Add gain and DC Bias component */

		m_t_off  = 1.0 / DSS_SQUAREWFIX__FREQ;  /* cycle time */
		m_t_on   = m_t_off * (DSS_SQUAREWFIX__DUTY / 100.0);
		m_t_off -= m_t_on;

		set_output(0, (m_flip_flop ? DSS_SQUAREWFIX__AMP / 2.0 : -(DSS_SQUAREWFIX__AMP / 2.0)) + DSS_SQUAREWFIX__BIAS);
	}
	else
	{
		set_output(0, 0);
	}
}

DISCRETE_RESET(dss_squarewfix)
{
	m_sample_step = 1.0 / this->sample_rate();
	m_flip_flop   = 1;

	/* Do the intial time shift and convert freq to off/on times */
	m_t_off   = 1.0 / DSS_SQUAREWFIX__FREQ; /* cycle time */
	m_t_left  = DSS_SQUAREWFIX__PHASE / 360.0;  /* convert start phase to % */
	m_t_left  = m_t_left - (int)m_t_left;   /* keep % between 0 & 1 */
	m_t_left  = (m_t_left < 0) ? 1.0 + m_t_left : m_t_left; /* if - then flip to + phase */
	m_t_left *= m_t_off;
	m_t_on    = m_t_off * (DSS_SQUAREWFIX__DUTY / 100.0);
	m_t_off  -= m_t_on;

	m_t_left = -m_t_left;

	/* toggle output and work out intial time shift */
	while (m_t_left <= 0)
	{
		m_flip_flop = m_flip_flop ? 0 : 1;
		m_t_left   += m_flip_flop ? m_t_on : m_t_off;
	}

	/* Step the output */
	this->step();
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
#define DSS_SQUAREWAVE2__ENABLE DISCRETE_INPUT(0)
#define DSS_SQUAREWAVE2__AMP    DISCRETE_INPUT(1)
#define DSS_SQUAREWAVE2__T_OFF  DISCRETE_INPUT(2)
#define DSS_SQUAREWAVE2__T_ON   DISCRETE_INPUT(3)
#define DSS_SQUAREWAVE2__BIAS   DISCRETE_INPUT(4)
#define DSS_SQUAREWAVE2__SHIFT  DISCRETE_INPUT(5)

DISCRETE_STEP(dss_squarewave2)
{
	double newphase;

	if(DSS_SQUAREWAVE2__ENABLE)
	{
		/* Establish trigger phase from time periods */
		m_trigger = (DSS_SQUAREWAVE2__T_OFF / (DSS_SQUAREWAVE2__T_OFF + DSS_SQUAREWAVE2__T_ON)) * (2.0 * M_PI);

		/* Work out the phase step based on phase/freq & sample rate */
		/* The enable input only curtails output, phase rotation     */
		/* still occurs                                              */

		/*     phase step = 2Pi/(output period/sample period)        */
		/*                    boils out to                           */
		/*     phase step = 2Pi/(output period*sample freq)          */
		newphase = m_phase + ((2.0 * M_PI) / ((DSS_SQUAREWAVE2__T_OFF + DSS_SQUAREWAVE2__T_ON) * this->sample_rate()));
		/* Keep the new phasor in the 2Pi range.*/
		m_phase = fmod(newphase, 2.0 * M_PI);

		/* Add DC Bias component */
		if(m_phase>m_trigger)
			set_output(0, DSS_SQUAREWAVE2__AMP / 2.0  + DSS_SQUAREWAVE2__BIAS);
		else
			set_output(0, -DSS_SQUAREWAVE2__AMP / 2.0 + DSS_SQUAREWAVE2__BIAS);
	}
	else
	{
		set_output(0, 0);
	}
}

DISCRETE_RESET(dss_squarewave2)
{
	double start;

	/* Establish starting phase, convert from degrees to radians */
	/* Only valid if we have set the on/off time                 */
	if((DSS_SQUAREWAVE2__T_OFF + DSS_SQUAREWAVE2__T_ON) != 0.0)
		start = (DSS_SQUAREWAVE2__SHIFT / (DSS_SQUAREWAVE2__T_OFF + DSS_SQUAREWAVE2__T_ON)) * (2.0 * M_PI);
	else
		start = 0.0;
	/* Make sure its always mod 2Pi */
	m_phase = fmod(start, 2.0 * M_PI);

	/* Step the output */
	this->step();
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

/*
 * Taken from the transfer characteristerics diagram in CD4049UB datasheet (TI)
 * There is no default trigger point and vI-vO is a continuous function
 */

inline double DISCRETE_CLASS_FUNC(dss_inverter_osc, tftab)(double x)
{
	DISCRETE_DECLARE_INFO(description)

	x = x / info->vB;
	if (x > 0)
		return info->vB * exp(-mc_tf_a * pow(x, mc_tf_b));
	else
		return info->vB;
}

inline double DISCRETE_CLASS_FUNC(dss_inverter_osc, tf)(double x)
{
	DISCRETE_DECLARE_INFO(description)

	if (x < 0.0)
		return info->vB;
	else if (x <= info->vB)
		return mc_tf_tab[(int)((double)(DSS_INV_TAB_SIZE - 1) * x / info->vB)];
	else
		return mc_tf_tab[DSS_INV_TAB_SIZE - 1];
}

DISCRETE_STEP(dss_inverter_osc)
{
	DISCRETE_DECLARE_INFO(description)
	double diff, vG1, vG2, vG3, vI;
	double vMix, rMix;
	int clamped;
	double v_out;

	/* Get new state */
	vI = mc_v_cap + mc_v_g2_old;
	switch (info->options & TYPE_MASK)
	{
		case IS_TYPE1:
		case IS_TYPE3:
			vG1 = this->tf(vI);
			vG2 = this->tf(vG1);
			vG3 = this->tf(vG2);
			break;
		case IS_TYPE2:
			vG1 = 0;
			vG3 = this->tf(vI);
			vG2 = this->tf(vG3);
			break;
		case IS_TYPE4:
			vI  = MIN(I_ENABLE(), vI + 0.7);
			vG1 = 0;
			vG3 = this->tf(vI);
			vG2 = this->tf(vG3);
			break;
		case IS_TYPE5:
			vI  = MAX(I_ENABLE(), vI - 0.7);
			vG1 = 0;
			vG3 = this->tf(vI);
			vG2 = this->tf(vG3);
			break;
		default:
			fatalerror("DISCRETE_INVERTER_OSC - Wrong type on NODE_%02d\n", this->index());
	}

	clamped = 0;
	if (info->clamp >= 0.0)
	{
		if (vI < -info->clamp)
		{
			vI = -info->clamp;
			clamped = 1;
		}
		else if (vI > info->vB+info->clamp)
		{
			vI = info->vB + info->clamp;
			clamped = 1;
		}
	}

	switch (info->options & TYPE_MASK)
	{
		case IS_TYPE1:
		case IS_TYPE2:
		case IS_TYPE3:
			if (clamped)
			{
				double ratio = mc_rp / (mc_rp + mc_r1);
				diff = vG3 * (ratio)
						- (mc_v_cap + vG2)
						+ vI * (1.0 - ratio);
				diff = diff - diff * mc_wc;
			}
			else
			{
				diff = vG3 - (mc_v_cap + vG2);
				diff = diff - diff * mc_w;
			}
			break;
		case IS_TYPE4:
			/*  FIXME handle r2 = 0  */
			rMix = (mc_r1 * mc_r2) / (mc_r1 + mc_r2);
			vMix = rMix* ((vG3 - vG2) / mc_r1 + (I_MOD() -vG2) / mc_r2);
			if (vMix < (vI-vG2-0.7))
			{
				rMix = 1.0 / rMix + 1.0 / mc_rp;
				rMix = 1.0 / rMix;
				vMix = rMix* ( (vG3-vG2) / mc_r1 + (I_MOD() - vG2) / mc_r2
						+ (vI - 0.7 - vG2) / mc_rp);
			}
			diff = vMix - mc_v_cap;
			diff = diff - diff * exp(-this->sample_time() / (mc_c * rMix));
			break;
		case IS_TYPE5:
			/*  FIXME handle r2 = 0  */
			rMix = (mc_r1 * mc_r2) / (mc_r1 + mc_r2);
			vMix = rMix* ((vG3 - vG2) / mc_r1 + (I_MOD() - vG2) / mc_r2);
			if (vMix > (vI -vG2 + 0.7))
			{
				rMix = 1.0 / rMix + 1.0 / mc_rp;
				rMix = 1.0 / rMix;
				vMix = rMix * ( (vG3 - vG2) / mc_r1 + (I_MOD() - vG2) / mc_r2
						+ (vI + 0.7 - vG2) / mc_rp);
			}
			diff = vMix - mc_v_cap;
			diff = diff - diff * exp(-this->sample_time()/(mc_c * rMix));
			break;
		default:
			fatalerror("DISCRETE_INVERTER_OSC - Wrong type on NODE_%02d\n", this->index());
	}

	mc_v_cap   += diff;
	mc_v_g2_old = vG2;

	if ((info->options & TYPE_MASK) == IS_TYPE3)
		v_out = vG1;
	else
		v_out = vG3;

	if (info->options & OUT_IS_LOGIC)
		v_out = (v_out > info->vInFall);

	set_output(0, v_out);
}

DISCRETE_RESET(dss_inverter_osc)
{
	DISCRETE_DECLARE_INFO(description)

	int i;

	/* exponent */
	mc_w  = exp(-this->sample_time() / (I_RC() * I_C()));
	mc_wc = exp(-this->sample_time() / ((I_RC() * I_RP()) / (I_RP() + I_RC()) * I_C()));
	set_output(0, 0);
	mc_v_cap    = 0;
	mc_v_g2_old = 0;
	mc_rp   = I_RP();
	mc_r1   = I_RC();
	mc_r2   = I_R2();
	mc_c    = I_C();
	mc_tf_b = (log(0.0 - log(info->vOutLow/info->vB)) - log(0.0 - log((info->vOutHigh/info->vB))) ) / log(info->vInRise / info->vInFall);
	mc_tf_a = log(0.0 - log(info->vOutLow/info->vB)) - mc_tf_b * log(info->vInRise/info->vB);
	mc_tf_a = exp(mc_tf_a);

	for (i = 0; i < DSS_INV_TAB_SIZE; i++)
	{
		mc_tf_tab[i] = this->tftab((double)i / (double)(DSS_INV_TAB_SIZE - 1) * info->vB);
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
#define DSS_TRIANGLEWAVE__ENABLE    DISCRETE_INPUT(0)
#define DSS_TRIANGLEWAVE__FREQ      DISCRETE_INPUT(1)
#define DSS_TRIANGLEWAVE__AMP       DISCRETE_INPUT(2)
#define DSS_TRIANGLEWAVE__BIAS      DISCRETE_INPUT(3)
#define DSS_TRIANGLEWAVE__PHASE     DISCRETE_INPUT(4)

DISCRETE_STEP(dss_trianglewave)
{
	if(DSS_TRIANGLEWAVE__ENABLE)
	{
		double v_out = m_phase < M_PI ? (DSS_TRIANGLEWAVE__AMP * (m_phase / (M_PI / 2.0) - 1.0)) / 2.0 :
									(DSS_TRIANGLEWAVE__AMP * (3.0 - m_phase / (M_PI / 2.0))) / 2.0 ;

		/* Add DC Bias component */
		v_out  += DSS_TRIANGLEWAVE__BIAS;
		set_output(0, v_out);
	}
	else
	{
		set_output(0, 0);
	}

	/* Work out the phase step based on phase/freq & sample rate */
	/* The enable input only curtails output, phase rotation     */
	/* still occurs                                              */
	/*     phase step = 2Pi/(output period/sample period)        */
	/*                    boils out to                           */
	/*     phase step = (2Pi*output freq)/sample freq)           */
	/* Also keep the new phasor in the 2Pi range.                */
	m_phase=fmod((m_phase + ((2.0 * M_PI * DSS_TRIANGLEWAVE__FREQ) / this->sample_rate())), 2.0 * M_PI);
}

DISCRETE_RESET(dss_trianglewave)
{
	double start;

	/* Establish starting phase, convert from degrees to radians */
	start = (DSS_TRIANGLEWAVE__PHASE / 360.0) * (2.0 * M_PI);
	/* Make sure its always mod 2Pi */
	m_phase=fmod(start, 2.0 * M_PI);

	/* Step to set the output */
	this->step();
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
#define DSS_ADSR__ENABLE    DISCRETE_INPUT(0)

DISCRETE_STEP(dss_adsrenv)
{
	if(DSS_ADSR__ENABLE)
	{
		set_output(0, 0);
	}
	else
	{
		set_output(0, 0);
	}
}


DISCRETE_RESET(dss_adsrenv)
{
	this->step();
}
