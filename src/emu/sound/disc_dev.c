/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by Keith Wilkins (mame@dysfunction.demon.co.uk)
 *
 *  (c) K.Wilkins 2000
 *  (c) D.Renaud 2003-2004
 *
 ************************************************************************
 *
 * DSD_555_ASTBL         - NE555 Simulation - Astable mode
 * DSD_555_MSTBL         - NE555 Simulation - Monostable mode
 * DSD_555_CC            - NE555 Constant Current VCO
 * DSD_555_VCO1          - Op-Amp linear ramp based 555 VCO
 * DSD_566               - NE566 Simulation
 *
 ************************************************************************
 *
 * You will notice that the code for a lot of these routines are similar.
 * I tried to make a common charging routine, but there are too many
 * minor differences that affect each module.
 *
 ************************************************************************/

#define DEFAULT_555_CAP_BLEED	RES_M(10)

struct dsd_555_astbl_context
{
	int		error;
	int		use_ctrlv;
	int		output_type;
	int		output_is_ac;
	double	ac_shift;				// DC shift needed to make waveform ac
	int		flip_flop;				// 555 flip/flop output state
	double	x_init;
	double	cap_voltage;			// voltage on cap
	double	threshold;
	double	trigger;
	double	output_high_voltage;	// Logic 1 voltage level
	double	v555;
};

struct dsd_555_mstbl_context
{
	int		error;
	int		trig_is_logic;
	int		trig_discharges_cap;
	int		output_type;
	int		output_is_ac;
	double	ac_shift;				// DC shift needed to make waveform ac
	int		flip_flop;				// 555 flip/flop output state
	double	cap_voltage;			// voltage on cap
	double	threshold;
	double	trigger;
	double	output_high_voltage;	// Logic 1 voltage level
};

struct dsd_555_cc_context
{
	int				error;
	unsigned int	type;			// type of 555cc circuit
	int				output_type;
	int				output_is_ac;
	double			ac_shift;		// DC shift needed to make waveform ac
	int				flip_flop;		// 555 flip/flop output state
	double			x_init;
	double			cap_voltage;	// voltage on cap
	double			threshold;
	double			trigger;
	double			output_high_voltage;	// Logic 1 voltage level
};

struct dsd_555_vco1_context
{
	int		ctrlv_is_node;
	int		output_type;
	int		output_is_ac;
	double	ac_shift;				// DC shift needed to make waveform ac
	int		flip_flop;				// flip/flop output state
	double	output_high_voltage;	// 555 high voltage
	double	threshold;				// falling threshold
	double	trigger;				// rising threshold
	double	i_charge;				// charge current
	double	i_discharge;			// discharge current
	double	cap_voltage;			// current capacitor voltage
};

struct dsd_566_context
{
	int			error;
	unsigned int state[2];		// keeps track of excess flip_flop changes during the current step
	int			flip_flop;		// 566 flip/flop output state
	double		cap_voltage;	// voltage on cap
	double		vDiff;			// voltage difference between vPlus and vNeg
	double		vSqrLow;		// voltage for a squarewave at low
	double		vSqrHigh;		// voltage for a squarewave at high
	double		thresholdLow;	// falling threshold
	double		thresholdHigh;	// rising threshold
	double		triOffset;		// used to shift a triangle to AC
};



/* Test to see if basic 555 options are valid. */
static int test_555(double threshold, double trigger, double v555, int node)
{
	int error = 0;
	if (threshold > v555)
	{
		logerror("[Threshold > B+]");
		error = 1;
	}
	if (threshold <= trigger)
	{
		logerror("[Threshold <= Trigger]");
		error = 1;
	}
	if (trigger < 0)
	{
		logerror("[Trigger < 0]");
		error = 1;
	}
	if (v555 <= 0)
	{
		logerror("[B+ <= 0]");
		error = 1;
	}
	if (error)
		logerror(" - NODE_%d DISABLED!\n", node - NODE_00);
	return error;
}


/************************************************************************
 *
 * DSD_555_ASTBL -  - 555 Astable simulation
 *
 * input[0]    - Reset value
 * input[1]    - R1 value
 * input[2]    - R2 value
 * input[3]    - C value
 * input[4]    - Control Voltage value
 *
 * also passed discrete_555_desc structure
 *
 * Jan 2004, D Renaud.
 ************************************************************************/
#define DSD_555_ASTBL__RESET	(! *(node->input[0]))
#define DSD_555_ASTBL__R1		(*(node->input[1]))
#define DSD_555_ASTBL__R2		(*(node->input[2]))
#define DSD_555_ASTBL__C		(*(node->input[3]))
#define DSD_555_ASTBL__CTRLV	(*(node->input[4]))

void dsd_555_astbl_step(node_description *node)
{
	const discrete_555_desc *info = node->custom;
	struct dsd_555_astbl_context *context = node->context;

	int		count_f = 0;
	int		count_r = 0;
	double	dt;					// change in time
	double	xTime;				// time since change happened
	double	tRC = 0;			// RC time constant
	double	vC = context->cap_voltage;	// Current voltage on capacitor, before dt
	double	vCnext = 0;			// Voltage on capacitor, after dt

	if(DSD_555_ASTBL__RESET || context->error)
	{
		/* We are in RESET */
		/* If there was a fatal INIT error then we will also stay in RESET */
		node->output = 0;
		context->flip_flop = 1;
		context->cap_voltage = 0;
		return;
	}

	/* Check: if the Control Voltage node is connected. */
	if (context->use_ctrlv)
	{
		/* If CV is less then .25V, the circuit will oscillate way out of range.
         * So we will just ignore it when it happens. */
		if (DSD_555_ASTBL__CTRLV < .25) return;
		/* If it is a node then calculate thresholds based on Control Voltage */
		context->threshold = DSD_555_ASTBL__CTRLV;
		context->trigger = DSD_555_ASTBL__CTRLV / 2.0;
		/* Since the thresholds may have changed we need to update the FF */
		if (vC >= context->threshold)
		{
			context->flip_flop = 0;
			count_f++;
		}
		else
		if (vC <= context->trigger)
		{
			context->flip_flop = 1;
			count_r++;
		}
	}

	/* Calculate future capacitor voltage.
     * ref@ http://www.physics.rutgers.edu/ugrad/205/capacitance.html
     * The formulas from the ref pages have been modified to reflect that we are stepping the change.
     * dt = time of sample (1/sample frequency)
     * VC = Voltage across capacitor
     * VC' = Future voltage across capacitor
     * Vc = Voltage change
     * Vr = is the voltage across the resistor.  For charging it is Vcc - VC.  Discharging it is VC - 0.
     * R = R1+R2 (for charging)  R = R2 for discharging.
     * Vc = Vr*(1-exp(-dt/(R*C)))
     * VC' = VC + Vc (for charging) VC' = VC - Vc for discharging.
     *
     * We will also need to calculate the amount of time we overshoot the thresholds
     * dt = amount of time we overshot
     * Vc = voltage change overshoot
     * dt = R*C(log(1/(1-(Vc/Vr))))
     */

	dt = discrete_current_context->sample_time;
	xTime = context->x_init;

	/* Sometimes a switching network is used to setup the capacitance.
     * These may select no capacitor, causing oscillation to stop.
     */
	if (DSD_555_ASTBL__C == 0)
	{
		context->flip_flop = 1;
		/* The voltage goes high because the cap circuit is open. */
		vCnext = context->v555;
		vC = context->v555;
		context->cap_voltage = 0;
	}
	else
	{
		/* Keep looping until all toggling in time sample is used up. */
		do
		{
			if (context->flip_flop)
			{
				if (DSD_555_ASTBL__R1 == 0)
				{
					/* Oscillation disabled because there is no longer any charge resistor. */
					/* Bleed the cap due to circuit losses. */
					tRC = DEFAULT_555_CAP_BLEED * DSD_555_ASTBL__C;
					vCnext = vC - (vC * (1.0 - exp(-(dt / tRC))));
					dt = 0;
				}
				else
				{
					/* Charging */
					/* Use quick charge if specified. */
					tRC = (DSD_555_ASTBL__R1 + ((info->options & DISC_555_ASTABLE_HAS_FAST_CHARGE_DIODE) ? 0 : DSD_555_ASTBL__R2)) * DSD_555_ASTBL__C;
					vCnext = vC + ((context->v555 - vC) * (1.0 - exp(-(dt / tRC))));
					dt = 0;

					/* has it charged past upper limit? */
					if (vCnext > context->threshold)
					{
						/* calculate the overshoot time */
						dt = tRC * log(1.0 / (1.0 - ((vCnext - context->threshold) / (context->v555 - vC))));
						xTime = dt;
						vC = context->threshold;
						context->flip_flop = 0;
						count_f++;
					}
				}
			}
			else
			{
				/* Discharging */
				if(DSD_555_ASTBL__R2!=0)
				{
					tRC = DSD_555_ASTBL__R2 * DSD_555_ASTBL__C;
					vCnext = vC - (vC * (1 - exp(-(dt / tRC))));
					dt = 0;
				}
				else
				{
					vCnext = context->trigger;
					dt = 0;
				}

				/* has it discharged past lower limit? */
				if (vCnext < context->trigger)
				{
					/* calculate the overshoot time */
					dt = tRC * log(1.0 / (1.0 - ((context->trigger - vCnext) / vC)));
					xTime = dt;
					vC = context->trigger;
					context->flip_flop = 1;
					count_r++;
				}
			}
		} while(dt);

		context->cap_voltage = vCnext;
	}

	/* Convert last switch time to a ratio */
	xTime = xTime / discrete_current_context->sample_time;

	switch (context->output_type)
	{
		case DISC_555_OUT_SQW:
			node->output = context->flip_flop * context->output_high_voltage + context->ac_shift;
			break;
		case DISC_555_OUT_CAP:
			node->output = vCnext;
			/* Fake it to AC if needed */
			if (context->output_is_ac)
				node->output -= context->threshold * 3.0 /4.0;
			break;
		case DISC_555_OUT_ENERGY:
			node->output = context->output_high_voltage * (context->flip_flop ? xTime : (1 - xTime));
			node->output += context->ac_shift;
			break;
		case DISC_555_OUT_LOGIC_X:
			node->output = context->flip_flop + xTime;
			break;
		case DISC_555_OUT_COUNT_F_X:
			node->output = count_f ? count_f + xTime : count_f;
			break;
		case DISC_555_OUT_COUNT_R_X:
			node->output =  count_r ? count_r + xTime : count_r;
			break;
		case DISC_555_OUT_COUNT_F:
			node->output = count_f;
			break;
		case DISC_555_OUT_COUNT_R:
			node->output = count_r;
			break;
	}
}

void dsd_555_astbl_reset(node_description *node)
{
	const discrete_555_desc *info = node->custom;
	struct dsd_555_astbl_context *context = node->context;

	context->use_ctrlv = (node->input_is_node >> 4) & 1;
	context->output_type = info->options & DISC_555_OUT_MASK;

	/* Use the supplied values or set to defaults. */
	context->output_high_voltage = (info->v555high == DEFAULT_555_HIGH) ? info->v555 - 1.2 : info->v555high;
	if ((DSD_555_ASTBL__CTRLV != -1) && !context->use_ctrlv)
	{
		/* Setup based on supplied static value */
		context->threshold = DSD_555_ASTBL__CTRLV;
		context->trigger = DSD_555_ASTBL__CTRLV / 2.0;
	}
	else
	{
		/* use values passed in structure */
		context->threshold = (info->threshold555 == DEFAULT_555_THRESHOLD) ? info->v555 *2 /3 : info->threshold555;
		context->trigger =  (info->trigger555 == DEFAULT_555_TRIGGER) ? info->v555 /3 : info->trigger555;
	}

	context->output_is_ac = info->options & DISC_555_OUT_AC;
	/* Calculate DC shift needed to make squarewave waveform AC */
	context->ac_shift = context->output_is_ac ? -context->output_high_voltage / 2.0 : 0;

	context->error = test_555(context->threshold, context->trigger, info->v555, node->node);

	context->v555 = (info->options & DISC_555_ASTABLE_HAS_FAST_CHARGE_DIODE) ? info->v555 - 0.5: info->v555;
	context->flip_flop = 1;
	context->cap_voltage = 0;

	/* Used to adjust the ratio depending on if it is the extra percent or energy */
	context->x_init = 0;
	if (context->output_type == DISC_555_OUT_ENERGY)
		context->x_init = discrete_current_context->sample_time;

	/* Step to set the output */
	dsd_555_astbl_step(node);
}


/************************************************************************
 *
 * DSD_555_MSTBL - 555 Monostable simulation
 *
 * input[0]    - Reset value
 * input[1]    - Trigger input
 * input[2]    - R2 value
 * input[3]    - C value
 *
 * also passed discrete_555_desc structure
 *
 * Oct 2004, D Renaud.
 ************************************************************************/
#define DSD_555_MSTBL__RESET	(! *(node->input[0]))
#define DSD_555_MSTBL__TRIGGER	(*(node->input[1]))
#define DSD_555_MSTBL__R		(*(node->input[2]))
#define DSD_555_MSTBL__C		(*(node->input[3]))

void dsd_555_mstbl_step(node_description *node)
{
	const discrete_555_desc *info = node->custom;
	struct dsd_555_mstbl_context *context = node->context;

	double vC;	// Current voltage on capacitor, before dt
	double vCnext = 0;	// Voltage on capacitor, after dt

	if(DSD_555_MSTBL__RESET || context->error)
	{
		/* We are in RESET */
		/* If there was a fatal INIT error then we will also stay in RESET */
		node->output = 0;
		context->flip_flop = 0;
		context->cap_voltage = 0;
	}
	else
	{
		int trigger;

		if (context->trig_is_logic)
			trigger = !DSD_555_MSTBL__TRIGGER;
		else
			trigger = DSD_555_MSTBL__TRIGGER < context->trigger;

		if (context->trig_discharges_cap && trigger)
			context->cap_voltage = 0;

		if (!context->flip_flop)
		{
			/* Wait for trigger */
			if (trigger)
				context->flip_flop = 1;
		}
		else
		{
			vC = context->cap_voltage;

			/* Sometimes a switching network is used to setup the capacitance.
             * These may select 'no' capacitor, causing oscillation to stop.
             */
			if (DSD_555_MSTBL__C == 0)
			{
				context->flip_flop = 0;
				/* The voltage goes high because the cap circuit is open. */
				vCnext = info->v555;
				vC = info->v555;
				context->cap_voltage = 0;
			}
			else
			{
				/* Charging */
				vCnext = vC + ((info->v555 - vC) * (1.0 - exp(-(discrete_current_context->sample_time / (DSD_555_MSTBL__R * DSD_555_MSTBL__C)))));

				/* Has it charged past upper limit? */
				/* If trigger is still enabled, then we keep charging,
                 * regardless of threshold. */
				if ((vCnext >= context->threshold) && !trigger)
				{
					vCnext = 0;
					vC = context->threshold;
					context->flip_flop = 0;
				}
			}

			context->cap_voltage = vCnext;

			switch (info->options & DISC_555_OUT_MASK)
			{
				case DISC_555_OUT_SQW:
					node->output = context->flip_flop * context->output_high_voltage;
					/* Fake it to AC if needed */
					if (context->output_is_ac)
						node->output -= context->output_high_voltage / 2.0;
					break;
				case DISC_555_OUT_CAP:
					node->output = vCnext;
					/* Fake it to AC if needed */
					if (context->output_is_ac)
						node->output -= context->threshold * 3.0 /4.0;
					break;
			}
		}
	}
}

void dsd_555_mstbl_reset(node_description *node)
{
	const discrete_555_desc *info = node->custom;
	struct dsd_555_mstbl_context *context = node->context;

	context->output_type = info->options & DISC_555_OUT_MASK;
	if ((context->output_type == DISC_555_OUT_COUNT_F) || (context->output_type == DISC_555_OUT_COUNT_R))
	{
		discrete_log("Invalid Output type in NODE_%d.\n", node->node - NODE_00);
		context->output_type = DISC_555_OUT_SQW;
	}

	/* Use the supplied values or set to defaults. */
	context->threshold = (info->threshold555 == DEFAULT_555_THRESHOLD) ? info->v555 *2 /3 : info->threshold555;
	context->trigger =  (info->trigger555 == DEFAULT_555_TRIGGER) ? info->v555 /3 : info->trigger555;
	context->output_high_voltage = (info->v555high == DEFAULT_555_HIGH) ? info->v555 - 1.2 : info->v555high;

	context->output_is_ac = info->options & DISC_555_OUT_AC;
	/* Calculate DC shift needed to make squarewave waveform AC */
	context->ac_shift = context->output_is_ac ? -context->output_high_voltage / 2.0 : 0;

	context->error = test_555(context->threshold, context->trigger, info->v555, node->node);

	context->trig_is_logic = (info->options & DISC_555_TRIGGER_IS_VOLTAGE) ? 0: 1;
	context->trig_discharges_cap = (info->options & DISC_555_TRIGGER_DISCHARGES_CAP) ? 1: 0;

	context->flip_flop = 0;
	context->cap_voltage = 0;

	node->output = 0;
}


/************************************************************************
 *
 * DSD_555_CC - Usage of node_description values
 *
 * input[0]    - Reset input value
 * input[1]    - Voltage input for Constant current source.
 * input[2]    - R value to set CC current.
 * input[3]    - C value
 * input[4]    - rBias value
 * input[5]    - rGnd value
 * input[6]    - rDischarge value
 *
 * also passed discrete_555_cc_desc structure
 *
 * Mar 2004, D Renaud.
 ************************************************************************/
#define DSD_555_CC__RESET	(! *(node->input[0]))
#define DSD_555_CC__VIN		(*(node->input[1]))
#define DSD_555_CC__R		(*(node->input[2]))
#define DSD_555_CC__C		(*(node->input[3]))
#define DSD_555_CC__RBIAS	(*(node->input[4]))
#define DSD_555_CC__RGND	(*(node->input[5]))
#define DSD_555_CC__RDIS	(*(node->input[6]))

void dsd_555_cc_step(node_description *node)
{
	const discrete_555_cc_desc *info = node->custom;
	struct dsd_555_cc_context *context = node->context;

	int		count_f = 0;
	int		count_r = 0;
	double	i;			// Charging current created by vIn
	double	rC = 0;		// Equivalent charging resistor
	double	rD = 0;		// Equivalent discharging resistor
	double	vi = 0;		// Equivalent voltage from current source
	double	vB = 0;		// Equivalent voltage from bias voltage
	double	v  = 0;		// Equivalent voltage total from current source and bias circuit if used
	double	dt;			// change in time
	double	xTime;	// time since change happened
	double	tRC;		// RC time constant
	double	vC;			// Current voltage on capacitor, before dt
	double	vCnext = 0;	// Voltage on capacitor, after dt
	double	viLimit;	// vIn and the junction voltage limit the max charging voltage from i
	double	rTemp;		// play thing


	if (DSD_555_CC__RESET || context->error)
	{
		/* We are in RESET */
		/* If there was a fatal INIT error then we will also stay in RESET */
		node->output = 0;
		context->flip_flop = 1;
		context->cap_voltage = 0;
		return;
	}

	dt = discrete_current_context->sample_time;	// Change in time
	xTime = context->x_init;
	vC = context->cap_voltage;	// Set to voltage before change
	viLimit = DSD_555_CC__VIN + info->vCCjunction;	// the max vC can be and still be charged by i
	/* Calculate charging current */
	i = (info->vCCsource - viLimit) / DSD_555_CC__R;
	if ( i < 0) i = 0;

	if (info->options & DISCRETE_555_CC_TO_CAP)
	{
		vi = i * DSD_555_CC__RDIS;
	}
	else
	switch (context->type)	// see dsd_555_cc_reset for descriptions
	{
		case 1:
			rD = DSD_555_CC__RDIS;
		case 0:
			break;
		case 3:
			rD = (DSD_555_CC__RDIS * DSD_555_CC__RGND) / (DSD_555_CC__RDIS + DSD_555_CC__RGND);
		case 2:
			rC = DSD_555_CC__RGND;
			vi = i * rC;
			break;
		case 4:
			rC = DSD_555_CC__RBIAS;
			vi = i * rC;
			vB = info->v555;
			break;
		case 5:
			rC = DSD_555_CC__RBIAS + DSD_555_CC__RDIS;
			vi = i * DSD_555_CC__RBIAS;
			vB = info->v555;
			rD = DSD_555_CC__RDIS;
			break;
		case 6:
			rC = (DSD_555_CC__RBIAS * DSD_555_CC__RGND) / (DSD_555_CC__RBIAS + DSD_555_CC__RGND);
			vi = i * rC;
			vB = info->v555 * (DSD_555_CC__RGND / (DSD_555_CC__RBIAS + DSD_555_CC__RGND));
			break;
		case 7:
			rTemp = DSD_555_CC__RBIAS + DSD_555_CC__RDIS;
			rC = (rTemp * DSD_555_CC__RGND) / (rTemp + DSD_555_CC__RGND);
			rTemp += DSD_555_CC__RGND;
			rTemp = DSD_555_CC__RGND / rTemp;	// now has voltage divider ratio, not resistance
			vi = i * DSD_555_CC__RBIAS * rTemp;
			vB = info->v555 * rTemp;
			rD = (DSD_555_CC__RGND * DSD_555_CC__RDIS) / (DSD_555_CC__RGND + DSD_555_CC__RDIS);
			break;
	}

	/* Keep looping until all toggling in time sample is used up. */
	do
	{
		if (context->type <= 1)
		{
			/* Standard constant current charge */
			if (context->flip_flop)
			{
				if (i == 0)
				{
					/* No charging current, so we have to discharge the cap
                     * due to cap and circuit losses.
                     */
					tRC = DEFAULT_555_CAP_BLEED * DSD_555_CC__C;
					vCnext = vC - (vC * (1.0 - exp(-(dt / tRC))));
					dt = 0;
				}
				else
				{
					/* Charging */
					/* iC=C*dv/dt  works out to dv=iC*dt/C */
					vCnext = vC + (i * dt / DSD_555_CC__C);
					/* Yes, if the cap voltage has reached the max voltage it can,
                     * and the 555 threshold has not been reached, then oscillation stops.
                     * This is the way the actual electronics works.
                     * This is why you never play with the pots after being factory adjusted
                     * to work in the proper range. */
					if (vCnext > viLimit) vCnext = viLimit;
					dt = 0;

					/* has it charged past upper limit? */
					if (vCnext >= context->threshold)
					{
						/* calculate the overshoot time */
						dt = DSD_555_CC__C * (vCnext - context->threshold) / i;
						xTime = dt;
						vC = context->threshold;
						context->flip_flop = 0;
						count_f++;
					}
				}
			}
			else if (DSD_555_CC__RDIS)
			{
				/* Discharging */
				tRC = DSD_555_CC__RDIS * DSD_555_CC__C;

				if (info->options & DISCRETE_555_CC_TO_CAP)
				{
					/* Asteroids - Special Case */
					/* Charging in discharge mode */
					/* If the cap voltage is past the current source charging limit
                     * then only the bias voltage will charge the cap. */
					v = (vC < viLimit) ? vi : viLimit;
					vCnext = vC + ((v - vC) * (1.0 - exp(-(dt / tRC))));
				}
				else
				{
					vCnext = vC - (vC * (1.0 - exp(-(dt / tRC))));
				}

				dt = 0;
				/* has it discharged past lower limit? */
				if (vCnext <= context->trigger)
				{
					dt = tRC * log(1.0 / (1.0 - ((context->trigger - vCnext) / vC)));
					xTime = dt;
					vC = context->trigger;
					context->flip_flop = 1;
					count_r++;
				}
			}
			else	// Immediate discharge. No change in dt.
			{
				vC = context->trigger;
				context->flip_flop = 1;
				count_r++;
			}
		}
		else
		{
			/* The constant current gets changed to a voltage due to a load resistor. */
			if (context->flip_flop)
			{
				if ((i == 0) && (DSD_555_CC__RBIAS == 0))
				{
					/* No charging current, so we have to discharge the cap
                     * due to rGnd.
                     */
					tRC = DSD_555_CC__RGND * DSD_555_CC__C;
					vCnext = vC - (vC * (1.0 - exp(-(dt / tRC))));
					dt = 0;
				}
				else
				{
					/* Charging */
					/* If the cap voltage is past the current source charging limit
                     * then only the bias voltage will charge the cap. */
					v = vB;
					if (vC < viLimit) v += vi;
					else if (context->type <= 3) v = viLimit;

					tRC = rC * DSD_555_CC__C;
					vCnext = vC + ((v - vC) * (1.0 - exp(-(dt / tRC))));
					dt = 0;

					/* has it charged past upper limit? */
					if (vCnext >= context->threshold)
					{
						/* calculate the overshoot time */
						dt = tRC * log(1.0 / (1.0 - ((vCnext - context->threshold) / (v - vC))));
						xTime = dt;
						vC = context->threshold;
						context->flip_flop = 0;
						count_f++;
					}
				}
			}
			else /* Discharging */
			if (rD)
			{
				tRC = rD * DSD_555_CC__C;
				vCnext = vC - (vC * (1.0 - exp(-(dt / tRC))));
				dt = 0;

				/* has it discharged past lower limit? */
				if (vCnext <= context->trigger)
				{
					/* calculate the overshoot time */
					dt = tRC * log(1.0 / (1.0 - ((context->trigger - vCnext) / vC)));
					xTime = dt;
					vC = context->trigger;
					context->flip_flop = 1;
					count_r++;
				}
			}
			else	// Immediate discharge. No change in dt.
			{
				vC = context->trigger;
				context->flip_flop = 1;
				count_r++;
			}
		}
	} while(dt);

	context->cap_voltage = vCnext;

	/* Convert last switch time to a ratio */
	xTime = xTime / discrete_current_context->sample_time;

	switch (context->output_type)
	{
		case DISC_555_OUT_SQW:
			if (count_r && (~context->type & 0x01))
			{
				/* There has been an immediate discharge, so keep low for 1 sample. */
				node->output = 0;
			}
			else
				node->output = context->flip_flop * context->output_high_voltage;
			/* Fake it to AC if needed */
			node->output += context->ac_shift;
			break;
		case DISC_555_OUT_CAP:
			node->output = vCnext + context->ac_shift;
			break;
		case DISC_555_OUT_ENERGY:
			node->output = context->output_high_voltage * (context->flip_flop ? xTime : (1 - xTime));
			node->output += context->ac_shift;
			break;
		case DISC_555_OUT_LOGIC_X:
			node->output = context->flip_flop + xTime;
			break;
		case DISC_555_OUT_COUNT_F_X:
			node->output = count_f + xTime;
			break;
		case DISC_555_OUT_COUNT_R_X:
			node->output = count_r + xTime;
			break;
		case DISC_555_OUT_COUNT_F:
			node->output = count_f;
			break;
		case DISC_555_OUT_COUNT_R:
			node->output = count_r;
			break;
	}
}

void dsd_555_cc_reset(node_description *node)
{
	const discrete_555_cc_desc *info = node->custom;
	struct dsd_555_cc_context *context = node->context;

	context->flip_flop=1;
	context->cap_voltage = 0;

	context->output_type = info->options & DISC_555_OUT_MASK;

	/* Used to adjust the ratio depending on if it is the extra percent or energy */
	context->x_init = 0;
	if (context->output_type == DISC_555_OUT_ENERGY)
		context->x_init = discrete_current_context->sample_time;

	/* Use the supplied values or set to defaults. */
	context->threshold = (info->threshold555 == DEFAULT_555_THRESHOLD) ? info->v555 *2 /3 : info->threshold555;
	context->trigger =  (info->trigger555 == DEFAULT_555_TRIGGER) ? info->v555 /3 : info->trigger555;
	context->output_high_voltage = (info->v555high == DEFAULT_555_HIGH) ? info->v555 - 1.2 : info->v555high;

	context->output_is_ac = info->options & DISC_555_OUT_AC;
	/* Calculate DC shift needed to make squarewave waveform AC */
	context->ac_shift = context->output_is_ac ? -context->output_high_voltage / 2.0 : 0;

	context->error = test_555(context->threshold, context->trigger, info->v555, node->node);

	/* There are 8 different types of basic oscillators
     * depending on the resistors used.  We will determine
     * the type of circuit at reset, because the ciruit type
     * is constant. */
	context->type = (DSD_555_CC__RDIS > 0) | ((DSD_555_CC__RGND  > 0) << 1) | ((DSD_555_CC__RBIAS  > 0) << 2);
	/*
     * TYPES:
     * Note: These are equivalent circuits shown without the 555 circuitry.
     *       See the schematic in src\sound\discrete.h for full hookup info.
     *
     * DISCRETE_555_CC_TO_DISCHARGE_PIN
     * When the CC source is connected to the discharge pin, it allows the
     * circuit to charge when the 555 is in charge mode.  But when in discharge
     * mode, the CC source is grounded, disabling it's effect.
     *
     * [0]
     * No resistors.  Straight constant current charge of capacitor.
     * When there is not any charge current, the cap will bleed off.
     * Once the lower threshold(trigger) is reached, the output will
     * go high but the cap will continue to discharge due to losses.
     *   .------+---> cap_voltage      CHARGING:
     *   |      |                 dv (change in voltage) compared to dt (change in time in seconds).
     * .---.   ---                dv = i * dt / C; where i is current in amps and C is capacitance in farads.
     * | i |   --- C              cap_voltage = cap_voltage + dv
     * '---'    |
     *   |      |               DISCHARGING:
     *  gnd    gnd                instantaneous
     *
     * [1]
     * Same as type 1 but with rDischarge.  rDischarge has no effect on the charge rate because
     * of the constant current source i.
     * When there is not any charge current, the cap will bleed off.
     * Once the lower threshold(trigger) is reached, the output will
     * go high but the cap will continue to discharge due to losses.
     *   .----ZZZ-----+---> cap_voltage      CHARGING:
     *   | rDischarge |                 dv (change in voltage) compared to dt (change in time in seconds).
     * .---.         ---                dv = i * dt / C; where i is current in amps and C is capacitance in farads.
     * | i |         --- C              cap_voltage = cap_voltage + dv
     * '---'          |
     *   |            |               DISCHARGING:
     *  gnd          gnd                thru rDischarge
     *
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * !!!!! IMPORTANT NOTE ABOUT TYPES 3 - 7 !!!!!
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     *
     * From here on in all the circuits have either an rBias or rGnd resistor.
     * This converts the constant current into a voltage source.
     * So all the remaining circuit types will be converted to this circuit.
     * When discharging, rBias is out of the equation because the 555 is grounding the circuit
     * after that point.
     *
     * .------------.     Rc                  Rc is the equivilent circuit resistance.
     * |     v      |----ZZZZ---+---> cap_voltage    v  is the equivilent circuit voltage.
     * |            |           |
     * '------------'          ---            Then the standard RC charging formula applies.
     *       |                 --- C
     *       |                  |             NOTE: All the following types are converted to Rc and v values.
     *      gnd                gnd
     *
     * [2]
     * When there is not any charge current, the cap will bleed off.
     * Once the lower threshold(trigger) is reached, the output will
     * go high but the cap will continue to discharge due to rGnd.
     *   .-------+------+------> cap_voltage         CHARGING:
     *   |       |      |                       v = vi = i * rGnd
     * .---.    ---     Z                       Rc = rGnd
     * | i |    --- C   Z rGnd
     * '---'     |      |                     DISCHARGING:
     *   |       |      |                       instantaneous
     *  gnd     gnd    gnd
     *
     * [3]
     * When there is not any charge current, the cap will bleed off.
     * Once the lower threshold(trigger) is reached, the output will
     * go high but the cap will continue to discharge due to rGnd.
     *   .----ZZZ-----+------+------> cap_voltage    CHARGING:
     *   | rDischarge |      |                  v = vi = i * rGnd
     * .---.         ---     Z                  Rc = rGnd
     * | i |         --- C   Z rGnd
     * '---'          |      |                DISCHARGING:
     *   |            |      |                  thru rDischarge || rGnd  ( || means in parallel)
     *  gnd          gnd    gnd
     *
     * [4]
     *     .---ZZZ---+------------+-------------> cap_voltage      CHARGING:
     *     |  rBias  |            |                           Rc = rBias
     * .-------.   .---.         ---                          vi = i * rBias
     * | vBias |   | i |         --- C                        v = vBias + vi
     * '-------'   '---'          |
     *     |         |            |                         DISCHARGING:
     *    gnd       gnd          gnd                          instantaneous
     *
     * [5]
     *     .---ZZZ---+----ZZZ-----+-------------> cap_voltage      CHARGING:
     *     |  rBias  | rDischarge |                           Rc = rBias + rDischarge
     * .-------.   .---.         ---                          vi = i * rBias
     * | vBias |   | i |         --- C                        v = vBias + vi
     * '-------'   '---'          |
     *     |         |            |                         DISCHARGING:
     *    gnd       gnd          gnd                          thru rDischarge
     *
     * [6]
     *     .---ZZZ---+------------+------+------> cap_voltage      CHARGING:
     *     |  rBias  |            |      |                    Rc = rBias || rGnd
     * .-------.   .---.         ---     Z                    vi = i * Rc
     * | vBias |   | i |         --- C   Z rGnd               v = vBias * (rGnd / (rBias + rGnd)) + vi
     * '-------'   '---'          |      |
     *     |         |            |      |                  DISCHARGING:
     *    gnd       gnd          gnd    gnd                   instantaneous
     *
     * [7]
     *     .---ZZZ---+----ZZZ-----+------+------> cap_voltage      CHARGING:
     *     |  rBias  | rDischarge |      |                    Rc = (rBias + rDischarge) || rGnd
     * .-------.   .---.         ---     Z                    vi = i * rBias * (rGnd / (rBias + rDischarge + rGnd))
     * | vBias |   | i |         --- C   Z rGnd               v = vBias * (rGnd / (rBias + rDischarge + rGnd)) + vi
     * '-------'   '---'          |      |
     *     |         |            |      |                  DISCHARGING:
     *    gnd       gnd          gnd    gnd                   thru rDischarge || rGnd
     */

    /*
     * DISCRETE_555_CC_TO_CAP
     *
     * When the CC source is connected to the capacitor, it allows the
     * current to charge the cap while it is in discharge mode, slowing the
     * discharge.  So in charge mode it charges linearly from the constant
     * current cource.  But when in discharge mode it behaves like circuit
     * type 2 above.
     *   .-------+------+------> cap_voltage         CHARGING:
     *   |       |      |                       dv = i * dt / C
     * .---.    ---     Z                       cap_voltage = cap_voltage + dv
     * | i |    --- C   Z rDischarge
     * '---'     |      |                     DISCHARGING:
     *   |       |      |                       v = vi = i * rGnd
     *  gnd     gnd   discharge                 Rc = rDischarge
     */

	/* Step to set the output */
	dsd_555_cc_step(node);
}


/************************************************************************
 *
 * DSD_555_VCO1 - Usage of node_description values
 *
 * input[0]    - Reset input value
 * input[1]    - Modulation Voltage (Vin1)
 * input[2]    - Control Voltage (Vin2)
 *
 * also passed discrete_5555_vco1_desc structure
 *
 * Apr 2006, D Renaud.
 ************************************************************************/
#define DSD_555_VCO1__RESET	(*(node->input[0]))	// reset active low
#define DSD_555_VCO1__VIN1	(*(node->input[1]))
#define DSD_555_VCO1__VIN2	(*(node->input[2]))

void dsd_555_vco1_step(node_description *node)
{
	const discrete_555_vco1_desc *info = node->custom;
	struct dsd_555_vco1_context *context = node->context;

	int		count_f = 0;
	int		count_r = 0;
	double	dt;			// change in time
	double	xTime = 0;	// time since change happened
	double	vC;			// Current voltage on capacitor, before dt
	double	vCnext = 0;	// Voltage on capacitor, after dt

	dt = discrete_current_context->sample_time;	// Change in time
	vC = context->cap_voltage;

	/* Check: if the Control Voltage node is connected. */
	if (context->ctrlv_is_node && DSD_555_VCO1__RESET)	// reset active low
	{
		/* If CV is less then .25V, the circuit will oscillate way out of range.
         * So we will just ignore it when it happens. */
		if (DSD_555_VCO1__VIN2 < .25) return;
		/* If it is a node then calculate thresholds based on Control Voltage */
		context->threshold = DSD_555_VCO1__VIN2;
		context->trigger = DSD_555_VCO1__VIN2 / 2.0;
		/* Since the thresholds may have changed we need to update the FF */
		if (vC >= context->threshold)
		{
			context->flip_flop = 0;
			count_f++;
		}
		else
		if (vC <= context->trigger)
		{
			context->flip_flop = 1;
			count_r++;
		}
	}

	/* Keep looping until all toggling in time sample is used up. */
	do
	{
		if (context->flip_flop)
		{
			// if we are in reset then toggle f/f and discharge
			if (!DSD_555_VCO1__RESET)	// reset active low
			{
				context->flip_flop = 0;
				count_f++;
			}
			else
			{
				/* Charging */
				/* iC=C*dv/dt  works out to dv=iC*dt/C */
				vCnext = vC + (context->i_charge * dt / info->c);
				dt = 0;

				/* has it charged past upper limit? */
				if (vCnext >= context->threshold)
				{
					if (vCnext > context->threshold)
					{
						/* calculate the overshoot time */
						dt = info->c * (vCnext - context->threshold) / context->i_charge;
					}
					vC = context->threshold;
					context->flip_flop = 0;
					count_f++;
					xTime = dt;
				}
			}
		}
		else
		{
			/* Discharging */
			/* iC=C*dv/dt  works out to dv=iC*dt/C */
			vCnext = vC - (context->i_discharge * dt / info->c);

			// if we are in reset, then the cap can discharge to 0
			if (!DSD_555_VCO1__RESET)	// reset active low
			{
				if (vCnext < 0) vCnext = 0;
				dt = 0;
			}
			else
			{
				// if we are out of reset and the cap voltage is less then
				// the lower threshold, toggle f/f and start charging
				if (vC <= context->trigger)
				{
					context->flip_flop = 1;
					count_r++;
				}
				else
				{
					dt = 0;
					/* has it discharged past lower limit? */
					if (vCnext <= context->trigger)
					{
						if (vCnext < context->trigger)
						{
							/* calculate the overshoot time */
							dt = info->c * (vCnext - context->trigger) / context->i_discharge;
						}
						vC = context->trigger;
						context->flip_flop = 1;
						count_r++;
						xTime = dt;
					}
				}
			}
		}
	} while(dt);

	context->cap_voltage = vCnext;

	/* Convert last switch time to a ratio */
	xTime = xTime / discrete_current_context->sample_time;

	switch (context->output_type)
	{
		case DISC_555_OUT_SQW:
			node->output = context->flip_flop * context->output_high_voltage + context->ac_shift;
			break;
		case DISC_555_OUT_CAP:
			node->output = vCnext;
			/* Fake it to AC if needed */
			if (context->output_is_ac)
				node->output -= context->threshold * 3.0 /4.0;
			break;
		case DISC_555_OUT_ENERGY:
			node->output = context->output_high_voltage * (context->flip_flop ? xTime : (1 - xTime));
			node->output += context->ac_shift;
			break;
		case DISC_555_OUT_LOGIC_X:
			node->output = context->flip_flop + xTime;
			break;
		case DISC_555_OUT_COUNT_F_X:
			node->output = count_f ? count_f + xTime : count_f;
			break;
		case DISC_555_OUT_COUNT_R_X:
			node->output =  count_r ? count_r + xTime : count_r;
			break;
		case DISC_555_OUT_COUNT_F:
			node->output = count_f;
			break;
		case DISC_555_OUT_COUNT_R:
			node->output = count_r;
			break;
	}
}

void dsd_555_vco1_reset(node_description *node)
{
	const discrete_555_vco1_desc *info = node->custom;
	struct dsd_555_vco1_context *context = node->context;

	double v_ratio_r3, v_ratio_r4_1, r_in_1;

	context->output_type = info->options & DISC_555_OUT_MASK;
	context->output_is_ac = info->options & DISC_555_OUT_AC;

	/* Setup op-amp parameters */

	/* The voltage at op-amp +in is always a fixed ratio of the modulation voltage. */
	v_ratio_r3 = info->r3 / (info->r2 + info->r3);			// +in voltage
	/* The voltage at op-amp -in is 1 of 2 fixed ratios of the modulation voltage,
     * based on the 555 Flip-Flop state. */
	/* If the FF is 0, then only R1 is connected allowing the full modulation volatge to pass. */
	/* v_ratio_r4_0 = 1 */
	/* If the FF is 1, then R1 & R4 make a voltage divider similar to R2 & R3 */
	v_ratio_r4_1 = info->r4 / (info->r1 + info->r4);		// -in voltage
	/* the input resistance to the op amp depends on the FF state */
	/* r_in_0 = info->r1 when FF = 0 */
	r_in_1 = 1.0 / (1.0 / info->r1 + 1.0 / info->r4);	// input resistance when r4 switched in

	/* Now that we know the voltages entering the op amp and the resistance for the
     * FF states, we can predetermine the ratios for the charge/discharge currents. */
	 context->i_discharge = (1 - v_ratio_r3) / info->r1;
	 context->i_charge = (v_ratio_r3 - v_ratio_r4_1) / r_in_1;

	/* the cap starts off discharged */
	context->cap_voltage = 0;

	/* Setup 555 parameters */

	/* There is no charge on the cap so the 555 goes high at init. */
	context->flip_flop = 1;
	context->ctrlv_is_node = (node->input_is_node >> 2) & 1;
	context->output_high_voltage = (info->v555high == DEFAULT_555_HIGH) ? info->v555 - 1.2 : info->v555high;

	/* Calculate 555 thresholds.
     * If the Control Voltage is a node, then the thresholds will be calculated each step.
     * If the Control Voltage is a fixed voltage, then the thresholds will be calculated
     * from that.  Otherwise we will use the thresholds specified in the setup info. */
	if (!context->ctrlv_is_node && (DSD_555_VCO1__VIN2 != -1))
	{
		/* Setup based on supplied static value */
		context->threshold = DSD_555_VCO1__VIN2;
		context->trigger = DSD_555_VCO1__VIN2 / 2.0;
	}
	else
	{
		/* use values passed in structure */
		context->threshold = (info->threshold555 == DEFAULT_555_THRESHOLD) ? info->v555 * 2 /3 : info->threshold555;
		context->trigger =  (info->trigger555 == DEFAULT_555_TRIGGER) ? info->v555 /3 : info->trigger555;
	}

	/* Calculate DC shift needed to make squarewave waveform AC */
	context->ac_shift = context->output_is_ac ? -context->output_high_voltage / 2.0 : 0;
}

/************************************************************************
 *
 * DSD_566 - Usage of node_description values
 *
 * input[0]    - Enable input value
 * input[1]    - Modulation Voltage
 * input[2]    - R value
 * input[3]    - C value
 *
 * also passed discrete_566_desc structure
 *
 * Mar 2004, D Renaud.
 ************************************************************************/
#define DSD_566__ENABLE	(*(node->input[0]))
#define DSD_566__VMOD	(*(node->input[1]))
#define DSD_566__R		(*(node->input[2]))
#define DSD_566__C		(*(node->input[3]))

void dsd_566_step(node_description *node)
{
	const discrete_566_desc *info = node->custom;
	struct dsd_566_context *context = node->context;

	double i;	// Charging current created by vIn
	double dt;	// change in time
	double vC;	// Current voltage on capacitor, before dt
	double vCnext = 0;	// Voltage on capacitor, after dt

	if (DSD_566__ENABLE && !context->error)
	{
		dt = discrete_current_context->sample_time;	// Change in time
		vC = context->cap_voltage;	// Set to voltage before change
		/* Calculate charging current */
		i = (context->vDiff - DSD_566__VMOD) / DSD_566__R;

		/* Keep looping until all toggling in time sample is used up. */
		do
		{
			if (context->flip_flop)
			{
				/* Discharging */
				vCnext = vC - (i * dt / DSD_566__C);
				dt = 0;

				/* has it discharged past lower limit? */
				if (vCnext <= context->thresholdLow)
				{
					if (vCnext < context->thresholdLow)
					{
						/* calculate the overshoot time */
						dt = DSD_566__C * (context->thresholdLow - vCnext) / i;
					}
					vC = context->thresholdLow;
					context->flip_flop = 0;
					/*
                     * If the sampling rate is too low and the desired frequency is too high
                     * then we will start getting too many outputs that can't catch up.  We will
                     * limit this to 3.  The output is already incorrect because of the low sampling,
                     * but at least this way it can recover.
                     */
					context->state[0] = (context->state[0] + 1) & 0x03;
				}
			}
			else
			{
				/* Charging */
				/* iC=C*dv/dt  works out to dv=iC*dt/C */
				vCnext = vC + (i * dt / DSD_566__C);
				dt = 0;
				/* Yes, if the cap voltage has reached the max voltage it can,
                 * and the 566 threshold has not been reached, then oscillation stops.
                 * This is the way the actual electronics works.
                 * This is why you never play with the pots after being factory adjusted
                 * to work in the proper range. */
				if (vCnext > DSD_566__VMOD) vCnext = DSD_566__VMOD;

				/* has it charged past upper limit? */
				if (vCnext >= context->thresholdHigh)
				{
					if (vCnext > context->thresholdHigh)
					{
						/* calculate the overshoot time */
						dt = DSD_566__C * (vCnext - context->thresholdHigh) / i;
					}
					vC = context->thresholdHigh;
					context->flip_flop = 1;
					context->state[1] = (context->state[1] + 1) & 0x03;
				}
			}
		} while(dt);

		context->cap_voltage = vCnext;

		switch (info->options & DISC_566_OUT_MASK)
		{
			case DISC_566_OUT_SQUARE:
			case DISC_566_OUT_LOGIC:
				/* use up any output states */
				if (node->output && context->state[0])
				{
					node->output = 0;
					context->state[0]--;
				}
				else if (!node->output && context->state[1])
				{
					node->output = 1;
					context->state[1]--;
				}
				else
				{
					node->output = context->flip_flop;
				}
				if ((info->options & DISC_566_OUT_MASK) != DISC_566_OUT_LOGIC)
					node->output = context->flip_flop ? context->vSqrHigh : context->vSqrLow;
				break;
			case DISC_566_OUT_TRIANGLE:
				/* we can ignore any unused states when
                 * outputting the cap voltage */
				node->output = vCnext;
				if (info->options & DISC_566_OUT_AC)
					node->output -= context->triOffset;
				break;
		}
	}
	else
		node->output = 0;
}

void dsd_566_reset(node_description *node)
{
	const discrete_566_desc *info = node->custom;
	struct dsd_566_context *context = node->context;

	double	temp;

	context->error = 0;
	if (info->vNeg >= info->vPlus)
	{
		logerror("[vNeg >= vPlus] - NODE_%d DISABLED!\n", node->node - NODE_00);
		context->error = 1;
	}

	context->vDiff = info->vPlus - info->vNeg;
	context->flip_flop = 0;
	context->cap_voltage = 0;
	context->state[0] = 0;
	context->state[1] = 0;

	/* The data sheets are crap on this IC.  I will have to get my hands on a chip
     * to make real measurements.  For now this should work fine for 12V. */
	context->thresholdHigh = context->vDiff / 2 + info->vNeg;
	context->thresholdLow = context->thresholdHigh - (0.2 * context->vDiff);
	context->vSqrHigh = info->vPlus - 0.6;
	context->vSqrLow = context->thresholdHigh;

	if (info->options & DISC_566_OUT_AC)
	{
		temp = (context->vSqrHigh - context->vSqrLow) / 2;
		context->vSqrHigh = temp;
		context->vSqrLow = -temp;
		context->triOffset = context->thresholdHigh - (0.1 * context->vDiff);
	}

	/* Step the output */
	dsd_566_step(node);
}
