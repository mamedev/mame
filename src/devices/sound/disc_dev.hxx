// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Derrick Renaud
/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by K.Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *  (c) Derrick Renaud 2003-2004
 *
 ************************************************************************
 *
 * DSD_555_ASTBL         - NE555 Simulation - Astable mode
 * DSD_555_MSTBL         - NE555 Simulation - Monostable mode
 * DSD_555_CC            - NE555 Constant Current VCO
 * DSD_555_VCO1          - Op-Amp linear ramp based 555 VCO
 * DSD_566               - NE566 Simulation
 * DSD_LS624             - 74LS624/629 Simulation
 *
 ************************************************************************
 *
 * You will notice that the code for a lot of these routines are similar.
 * I tried to make a common charging routine, but there are too many
 * minor differences that affect each module.
 *
 ************************************************************************/

#define DEFAULT_555_BLEED_R RES_M(10)

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
#define DSD_555_ASTBL__RESET    (! DISCRETE_INPUT(0))
#define DSD_555_ASTBL__R1       DISCRETE_INPUT(1)
#define DSD_555_ASTBL__R2       DISCRETE_INPUT(2)
#define DSD_555_ASTBL__C        DISCRETE_INPUT(3)
#define DSD_555_ASTBL__CTRLV    DISCRETE_INPUT(4)

/* bit mask of the above RC inputs */
#define DSD_555_ASTBL_RC_MASK   0x0e

/* charge/discharge constants */
#define DSD_555_ASTBL_T_RC_BLEED        (DEFAULT_555_BLEED_R * DSD_555_ASTBL__C)
/* Use quick charge if specified. */
#define DSD_555_ASTBL_T_RC_CHARGE       ((DSD_555_ASTBL__R1 + ((info->options & DISC_555_ASTABLE_HAS_FAST_CHARGE_DIODE) ? 0 : DSD_555_ASTBL__R2)) * DSD_555_ASTBL__C)
#define DSD_555_ASTBL_T_RC_DISCHARGE    (DSD_555_ASTBL__R2 * DSD_555_ASTBL__C)

DISCRETE_STEP(dsd_555_astbl)
{
	DISCRETE_DECLARE_INFO(discrete_555_desc)

	int     count_f = 0;
	int     count_r = 0;
	double  dt;                             /* change in time */
	double  x_time  = 0;                    /* time since change happened */
	double  v_cap   = m_cap_voltage;    /* Current voltage on capacitor, before dt */
	double  v_cap_next = 0;                 /* Voltage on capacitor, after dt */
	double  v_charge, exponent = 0;
	UINT8   flip_flop = m_flip_flop;
	UINT8   update_exponent = 0;
	double  v_out = 0.0;

	/* put commonly used stuff in local variables for speed */
	double  threshold = m_threshold;
	double  trigger   = m_trigger;

	if(DSD_555_ASTBL__RESET)
	{
		/* We are in RESET */
		set_output(0, 0);
		m_flip_flop   = 1;
		m_cap_voltage = 0;
		return;
	}

	/* Check: if the Control Voltage node is connected. */
	if (m_use_ctrlv)
	{
		/* If CV is less then .25V, the circuit will oscillate way out of range.
		 * So we will just ignore it when it happens. */
		if (DSD_555_ASTBL__CTRLV < .25) return;
		/* If it is a node then calculate thresholds based on Control Voltage */
		threshold = DSD_555_ASTBL__CTRLV;
		trigger   = DSD_555_ASTBL__CTRLV / 2.0;
		/* Since the thresholds may have changed we need to update the FF */
		if (v_cap >= threshold)
		{
			flip_flop = 0;
			count_f++;
		}
		else
		if (v_cap <= trigger)
		{
			flip_flop = 1;
			count_r++;
		}
	}

	/* get the v_charge and update each step if it is a node */
	if (m_v_charge_node != nullptr)
	{
		v_charge = *m_v_charge_node;
		if (info->options & DISC_555_ASTABLE_HAS_FAST_CHARGE_DIODE) v_charge -= 0.5;
	}
	else
		v_charge = m_v_charge;


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

	dt = this->sample_time();

	/* Sometimes a switching network is used to setup the capacitance.
	 * These may select no capacitor, causing oscillation to stop.
	 */
	if (DSD_555_ASTBL__C == 0)
	{
		flip_flop = 1;
		/* The voltage goes high because the cap circuit is open. */
		v_cap_next = v_charge;
		v_cap      = v_charge;
		m_cap_voltage = 0;
	}
	else
	{
		/* Update charge contstants and exponents if nodes changed */
		if (m_has_rc_nodes && (DSD_555_ASTBL__R1 != m_last_r1 || DSD_555_ASTBL__C != m_last_c || DSD_555_ASTBL__R2 != m_last_r2))
		{
			m_t_rc_bleed  = DSD_555_ASTBL_T_RC_BLEED;
			m_t_rc_charge = DSD_555_ASTBL_T_RC_CHARGE;
			m_t_rc_discharge = DSD_555_ASTBL_T_RC_DISCHARGE;
			m_exp_bleed  = RC_CHARGE_EXP(m_t_rc_bleed);
			m_exp_charge = RC_CHARGE_EXP(m_t_rc_charge);
			m_exp_discharge = RC_CHARGE_EXP(m_t_rc_discharge);
			m_last_r1 = DSD_555_ASTBL__R1;
			m_last_r2 = DSD_555_ASTBL__R2;
			m_last_c  = DSD_555_ASTBL__C;
		}
		/* Keep looping until all toggling in time sample is used up. */
		do
		{
			if (flip_flop)
			{
				if (DSD_555_ASTBL__R1 == 0)
				{
					/* Oscillation disabled because there is no longer any charge resistor. */
					/* Bleed the cap due to circuit losses. */
					if (update_exponent)
						exponent = RC_CHARGE_EXP_DT(m_t_rc_bleed, dt);
					else
						exponent = m_exp_bleed;
					v_cap_next = v_cap - (v_cap * exponent);
					dt = 0;
				}
				else
				{
					/* Charging */
					if (update_exponent)
						exponent = RC_CHARGE_EXP_DT(m_t_rc_charge, dt);
					else
						exponent = m_exp_charge;
					v_cap_next = v_cap + ((v_charge - v_cap) * exponent);
					dt = 0;

					/* has it charged past upper limit? */
					if (v_cap_next >= threshold)
					{
						/* calculate the overshoot time */
						dt     = m_t_rc_charge  * log(1.0 / (1.0 - ((v_cap_next - threshold) / (v_charge - v_cap))));
						x_time = dt;
						v_cap_next  = threshold;
						flip_flop = 0;
						count_f++;
						update_exponent = 1;
					}
				}
			}
			else
			{
				/* Discharging */
				if(DSD_555_ASTBL__R2 != 0)
				{
					if (update_exponent)
						exponent = RC_CHARGE_EXP_DT(m_t_rc_discharge, dt);
					else
						exponent = m_exp_discharge;
					v_cap_next = v_cap - (v_cap * exponent);
					dt = 0;
				}
				else
				{
					/* no discharge resistor so we immediately discharge */
					v_cap_next = trigger;
				}

				/* has it discharged past lower limit? */
				if (v_cap_next <= trigger)
				{
					/* calculate the overshoot time */
					if (v_cap_next < trigger)
						dt = m_t_rc_discharge  * log(1.0 / (1.0 - ((trigger - v_cap_next) / v_cap)));
					x_time = dt;
					v_cap_next  = trigger;
					flip_flop = 1;
					count_r++;
					update_exponent = 1;
				}
			}
			v_cap = v_cap_next;
		} while(dt);

		m_cap_voltage = v_cap;
	}

	/* Convert last switch time to a ratio */
	x_time = x_time / this->sample_time();

	switch (m_output_type)
	{
		case DISC_555_OUT_SQW:
			if (count_f + count_r >= 2)
				/* force at least 1 toggle */
				v_out =  m_flip_flop ? 0 : m_v_out_high;
			else
				v_out =  flip_flop * m_v_out_high;
			v_out += m_ac_shift;
			break;
		case DISC_555_OUT_CAP:
			v_out =  v_cap;
			/* Fake it to AC if needed */
			if (m_output_is_ac)
				v_out -= threshold * 3.0 /4.0;
			break;
		case DISC_555_OUT_ENERGY:
			if (x_time == 0) x_time = 1.0;
			v_out = m_v_out_high * (flip_flop ? x_time : (1.0 - x_time));
			v_out += m_ac_shift;
			break;
		case DISC_555_OUT_LOGIC_X:
			v_out =  flip_flop + x_time;
			break;
		case DISC_555_OUT_COUNT_F_X:
			v_out = count_f ? count_f + x_time : count_f;
			break;
		case DISC_555_OUT_COUNT_R_X:
			v_out =  count_r ? count_r + x_time : count_r;
			break;
		case DISC_555_OUT_COUNT_F:
			v_out =  count_f;
			break;
		case DISC_555_OUT_COUNT_R:
			v_out =  count_r;
			break;
	}
	set_output(0, v_out);
	m_flip_flop = flip_flop;
}

DISCRETE_RESET(dsd_555_astbl)
{
	DISCRETE_DECLARE_INFO(discrete_555_desc)

	m_use_ctrlv   = (this->input_is_node() >> 4) & 1;
	m_output_type = info->options & DISC_555_OUT_MASK;

	/* Use the defaults or supplied values. */
	m_v_out_high = (info->v_out_high == DEFAULT_555_HIGH) ? info->v_pos - 1.2 : info->v_out_high;

	/* setup v_charge or node */
	m_v_charge_node = m_device->node_output_ptr(info->v_charge);
	if (m_v_charge_node == nullptr)
	{
		m_v_charge   = (info->v_charge == DEFAULT_555_CHARGE) ? info->v_pos : info->v_charge;

		if (info->options & DISC_555_ASTABLE_HAS_FAST_CHARGE_DIODE) m_v_charge -= 0.5;
	}

	if ((DSD_555_ASTBL__CTRLV != -1) && !m_use_ctrlv)
	{
		/* Setup based on supplied Control Voltage static value */
		m_threshold = DSD_555_ASTBL__CTRLV;
		m_trigger   = DSD_555_ASTBL__CTRLV / 2.0;
	}
	else
	{
		/* Setup based on v_pos power source */
		m_threshold = info->v_pos * 2.0 / 3.0;
		m_trigger   = info->v_pos / 3.0;
	}

	/* optimization if none of the values are nodes */
	m_has_rc_nodes = 0;
	if (this->input_is_node() & DSD_555_ASTBL_RC_MASK)
		m_has_rc_nodes = 1;
	else
	{
		m_t_rc_bleed  = DSD_555_ASTBL_T_RC_BLEED;
		m_exp_bleed   = RC_CHARGE_EXP(m_t_rc_bleed);
		m_t_rc_charge = DSD_555_ASTBL_T_RC_CHARGE;
		m_exp_charge  = RC_CHARGE_EXP(m_t_rc_charge);
		m_t_rc_discharge = DSD_555_ASTBL_T_RC_DISCHARGE;
		m_exp_discharge  = RC_CHARGE_EXP(m_t_rc_discharge);
	}

	m_output_is_ac = info->options & DISC_555_OUT_AC;
	/* Calculate DC shift needed to make squarewave waveform AC */
	m_ac_shift = m_output_is_ac ? -m_v_out_high / 2.0 : 0;

	m_flip_flop = 1;
	m_cap_voltage = 0;

	/* Step to set the output */
	this->step();
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
#define DSD_555_MSTBL__RESET    (! DISCRETE_INPUT(0))
#define DSD_555_MSTBL__TRIGGER  DISCRETE_INPUT(1)
#define DSD_555_MSTBL__R        DISCRETE_INPUT(2)
#define DSD_555_MSTBL__C        DISCRETE_INPUT(3)

/* bit mask of the above RC inputs */
#define DSD_555_MSTBL_RC_MASK   0x0c

DISCRETE_STEP(dsd_555_mstbl)
{
	DISCRETE_DECLARE_INFO(discrete_555_desc)

	double v_cap;           /* Current voltage on capacitor, before dt */
	double x_time = 0;      /* time since change happened */
	double dt, exponent;
	double out = 0;
	int trigger = 0;
	int trigger_type;
	int update_exponent = m_has_rc_nodes;
	int flip_flop;

	if(UNEXPECTED(DSD_555_MSTBL__RESET))
	{
		/* We are in RESET */
		set_output(0, 0);
		m_flip_flop  = 0;
		m_cap_voltage = 0;
		return;
	}

	dt = this->sample_time();
	flip_flop = m_flip_flop;
	trigger_type = info->options;
	v_cap = m_cap_voltage;

	switch (trigger_type & DSD_555_TRIGGER_TYPE_MASK)
	{
		case DISC_555_TRIGGER_IS_LOGIC:
			trigger = ((int)DSD_555_MSTBL__TRIGGER) ? 0 : 1;
			if (UNEXPECTED(trigger))
				x_time = 1.0 - DSD_555_MSTBL__TRIGGER;
			break;
		case DISC_555_TRIGGER_IS_VOLTAGE:
			trigger = (int)(DSD_555_MSTBL__TRIGGER < m_trigger);
			break;
		case DISC_555_TRIGGER_IS_COUNT:
			trigger = (int)DSD_555_MSTBL__TRIGGER;
			if (UNEXPECTED(trigger))
				x_time = DSD_555_MSTBL__TRIGGER - trigger;
			break;
	}

	if (UNEXPECTED(trigger && !flip_flop && x_time != 0))
	{
		/* adjust sample to after trigger */
		update_exponent = 1;
		dt *= x_time;
	}
	x_time = 0;

	if ((trigger_type & DISC_555_TRIGGER_DISCHARGES_CAP) && trigger)
		m_cap_voltage = 0;

	/* Wait for trigger */
	if (UNEXPECTED(!flip_flop && trigger))
	{
		flip_flop = 1;
		m_flip_flop = 1;
	}

	if (flip_flop)
	{
		/* Sometimes a switching network is used to setup the capacitance.
		 * These may select 'no' capacitor, causing oscillation to stop.
		 */
		if (UNEXPECTED(DSD_555_MSTBL__C == 0))
		{
			/* The trigger voltage goes high because the cap circuit is open.
			 * and the cap discharges */
			v_cap = info->v_pos;    /* needed for cap output type */
			m_cap_voltage = 0;

			if (!trigger)
			{
				flip_flop = 0;
				m_flip_flop = 0;
			}
		}
		else
		{
			/* Charging */
			double v_diff = m_v_charge - v_cap;

			if (UNEXPECTED(update_exponent))
				exponent = RC_CHARGE_EXP_DT(DSD_555_MSTBL__R * DSD_555_MSTBL__C, dt);
			else
				exponent = m_exp_charge;
			v_cap += v_diff * exponent;

			/* Has it charged past upper limit? */
			/* If trigger is still enabled, then we keep charging,
			 * regardless of threshold. */
			if (UNEXPECTED((v_cap >= m_threshold) && !trigger))
			{
				dt = DSD_555_MSTBL__R * DSD_555_MSTBL__C  * log(1.0 / (1.0 - ((v_cap - m_threshold) / v_diff)));
				x_time = 1.0 - dt / this->sample_time();
				v_cap  = 0;
				flip_flop = 0;
				m_flip_flop = 0;
			}
			m_cap_voltage = v_cap;
		}
	}

	switch (m_output_type)
	{
		case DISC_555_OUT_SQW:
			out = flip_flop * m_v_out_high - m_ac_shift;
			break;
		case DISC_555_OUT_CAP:
			if (x_time > 0)
				out = v_cap * x_time;
			else
				out = v_cap;

			out -= m_ac_shift;
			break;
		case DISC_555_OUT_ENERGY:
			if (x_time > 0)
				out = m_v_out_high * x_time;
			else if (flip_flop)
				out = m_v_out_high;
			else
				out = 0;

			out -= m_ac_shift;
			break;
	}
	set_output(0,  out);
}

DISCRETE_RESET(dsd_555_mstbl)
{
	DISCRETE_DECLARE_INFO(discrete_555_desc)

	m_output_type = info->options & DISC_555_OUT_MASK;
	if ((m_output_type == DISC_555_OUT_COUNT_F) || (m_output_type == DISC_555_OUT_COUNT_R))
	{
		m_device->discrete_log("Invalid Output type in NODE_%d.\n", this->index());
		m_output_type = DISC_555_OUT_SQW;
	}

	/* Use the defaults or supplied values. */
	m_v_out_high = (info->v_out_high == DEFAULT_555_HIGH) ? info->v_pos - 1.2 : info->v_out_high;
	m_v_charge   = (info->v_charge   == DEFAULT_555_CHARGE) ? info->v_pos : info->v_charge;

	/* Setup based on v_pos power source */
	m_threshold = info->v_pos * 2.0 / 3.0;
	m_trigger   = info->v_pos / 3.0;

	/* Calculate DC shift needed to make waveform AC */
	if (info->options & DISC_555_OUT_AC)
	{
		if (m_output_type == DISC_555_OUT_CAP)
			m_ac_shift = m_threshold * 3.0 /4.0;
		else
			m_ac_shift = m_v_out_high / 2.0;
	}
	else
		m_ac_shift = 0;

	m_trig_is_logic       = (info->options & DISC_555_TRIGGER_IS_VOLTAGE) ? 0: 1;
	m_trig_discharges_cap = (info->options & DISC_555_TRIGGER_DISCHARGES_CAP) ? 1: 0;

	m_flip_flop   = 0;
	m_cap_voltage = 0;

	/* optimization if none of the values are nodes */
	m_has_rc_nodes = 0;
	if (this->input_is_node() & DSD_555_MSTBL_RC_MASK)
		m_has_rc_nodes = 1;
	else
		m_exp_charge = RC_CHARGE_EXP(DSD_555_MSTBL__R * DSD_555_MSTBL__C);

	set_output(0,  0);
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
#define DSD_555_CC__RESET   (! DISCRETE_INPUT(0))
#define DSD_555_CC__VIN     DISCRETE_INPUT(1)
#define DSD_555_CC__R       DISCRETE_INPUT(2)
#define DSD_555_CC__C       DISCRETE_INPUT(3)
#define DSD_555_CC__RBIAS   DISCRETE_INPUT(4)
#define DSD_555_CC__RGND    DISCRETE_INPUT(5)
#define DSD_555_CC__RDIS    DISCRETE_INPUT(6)

/* bit mask of the above RC inputs not including DSD_555_CC__R */
#define DSD_555_CC_RC_MASK  0x78

/* charge/discharge constants */
#define DSD_555_CC_T_RC_BLEED           (DEFAULT_555_BLEED_R * DSD_555_CC__C)
#define DSD_555_CC_T_RC_DISCHARGE_01    (DSD_555_CC__RDIS * DSD_555_CC__C)
#define DSD_555_CC_T_RC_DISCHARGE_NO_I  (DSD_555_CC__RGND * DSD_555_CC__C)
#define DSD_555_CC_T_RC_CHARGE          (r_charge * DSD_555_CC__C)
#define DSD_555_CC_T_RC_DISCHARGE       (r_discharge * DSD_555_CC__C)


DISCRETE_STEP(dsd_555_cc)
{
	DISCRETE_DECLARE_INFO(discrete_555_cc_desc)

	int     count_f  = 0;
	int     count_r  = 0;
	double  i;                  /* Charging current created by vIn */
	double  r_charge = 0;       /* Equivalent charging resistor */
	double  r_discharge = 0;    /* Equivalent discharging resistor */
	double  vi     = 0;         /* Equivalent voltage from current source */
	double  v_bias = 0;         /* Equivalent voltage from bias voltage */
	double  v      = 0;         /* Equivalent voltage total from current source and bias circuit if used */
	double  dt;                 /* change in time */
	double  x_time = 0;         /* time since change happened */
	double  t_rc ;              /* RC time constant */
	double  v_cap;              /* Current voltage on capacitor, before dt */
	double  v_cap_next = 0;     /* Voltage on capacitor, after dt */
	double  v_vcharge_limit;    /* vIn and the junction voltage limit the max charging voltage from i */
	double  r_temp;             /* play thing */
	double  exponent;
	UINT8   update_exponent, update_t_rc;
	UINT8   flip_flop = m_flip_flop;

	double v_out = 0;


	if (UNEXPECTED(DSD_555_CC__RESET))
	{
		/* We are in RESET */
		set_output(0, 0);
		m_flip_flop   = 1;
		m_cap_voltage = 0;
		return;
	}

	dt    = this->sample_time();    /* Change in time */
	v_cap = m_cap_voltage;  /* Set to voltage before change */
	v_vcharge_limit = DSD_555_CC__VIN + info->v_cc_junction;    /* the max v_cap can be and still be charged by i */
	/* Calculate charging current */
	i = (m_v_cc_source - v_vcharge_limit) / DSD_555_CC__R;
	if ( i < 0) i = 0;

	if (info->options & DISCRETE_555_CC_TO_CAP)
	{
		vi = i * DSD_555_CC__RDIS;
	}
	else
	{
		switch (m_type) /* see dsd_555_cc_reset for descriptions */
		{
			case 1:
				r_discharge = DSD_555_CC__RDIS;
			case 0:
				break;
			case 3:
				r_discharge = RES_2_PARALLEL(DSD_555_CC__RDIS, DSD_555_CC__RGND);
			case 2:
				r_charge = DSD_555_CC__RGND;
				vi       = i * r_charge;
				break;
			case 4:
				r_charge = DSD_555_CC__RBIAS;
				vi       = i * r_charge;
				v_bias   = info->v_pos;
				break;
			case 5:
				r_charge = DSD_555_CC__RBIAS + DSD_555_CC__RDIS;
				vi      = i * DSD_555_CC__RBIAS;
				v_bias  = info->v_pos;
				r_discharge = DSD_555_CC__RDIS;
				break;
			case 6:
				r_charge = RES_2_PARALLEL(DSD_555_CC__RBIAS, DSD_555_CC__RGND);
				vi      = i * r_charge;
				v_bias  = info->v_pos * RES_VOLTAGE_DIVIDER(DSD_555_CC__RGND, DSD_555_CC__RBIAS);
				break;
			case 7:
				r_temp   = DSD_555_CC__RBIAS + DSD_555_CC__RDIS;
				r_charge = RES_2_PARALLEL(r_temp, DSD_555_CC__RGND);
				r_temp  += DSD_555_CC__RGND;
				r_temp   = DSD_555_CC__RGND / r_temp;   /* now has voltage divider ratio, not resistance */
				vi      = i * DSD_555_CC__RBIAS * r_temp;
				v_bias  = info->v_pos * r_temp;
				r_discharge = RES_2_PARALLEL(DSD_555_CC__RGND, DSD_555_CC__RDIS);
				break;
		}
	}

	/* Keep looping until all toggling in time sample is used up. */
	update_t_rc = m_has_rc_nodes;
	update_exponent = update_t_rc;
	do
	{
		if (m_type <= 1)
		{
			/* Standard constant current charge */
			if (flip_flop)
			{
				if (i == 0)
				{
					/* No charging current, so we have to discharge the cap
					 * due to cap and circuit losses.
					 */
					if (update_exponent)
					{
						t_rc     = DSD_555_CC_T_RC_BLEED;
						exponent = RC_CHARGE_EXP_DT(t_rc, dt);
					}
					else
						exponent = m_exp_bleed;
					v_cap_next = v_cap - (v_cap * exponent);
					dt = 0;
				}
				else
				{
					/* Charging */
					/* iC=C*dv/dt  works out to dv=iC*dt/C */
					v_cap_next = v_cap + (i * dt / DSD_555_CC__C);
					/* Yes, if the cap voltage has reached the max voltage it can,
					 * and the 555 threshold has not been reached, then oscillation stops.
					 * This is the way the actual electronics works.
					 * This is why you never play with the pots after being factory adjusted
					 * to work in the proper range. */
					if (v_cap_next > v_vcharge_limit) v_cap_next = v_vcharge_limit;
					dt = 0;

					/* has it charged past upper limit? */
					if (v_cap_next >= m_threshold)
					{
						/* calculate the overshoot time */
						dt     = DSD_555_CC__C * (v_cap_next - m_threshold) / i;
						x_time = dt;
						v_cap_next = m_threshold;
						flip_flop = 0;
						count_f++;
						update_exponent = 1;
					}
				}
			}
			else if (DSD_555_CC__RDIS != 0)
			{
				/* Discharging */
				if (update_t_rc)
					t_rc = DSD_555_CC_T_RC_DISCHARGE_01;
				else
					t_rc = m_t_rc_discharge_01;
				if (update_exponent)
					exponent = RC_CHARGE_EXP_DT(t_rc, dt);
				else
					exponent = m_exp_discharge_01;

				if (info->options & DISCRETE_555_CC_TO_CAP)
				{
					/* Asteroids - Special Case */
					/* Charging in discharge mode */
					/* If the cap voltage is past the current source charging limit
					 * then only the bias voltage will charge the cap. */
					v          = (v_cap < v_vcharge_limit) ? vi : v_vcharge_limit;
					v_cap_next = v_cap + ((v - v_cap) * exponent);
				}
				else
				{
					v_cap_next = v_cap - (v_cap * exponent);
				}

				dt = 0;
				/* has it discharged past lower limit? */
				if (v_cap_next <= m_trigger)
				{
					dt     = t_rc  * log(1.0 / (1.0 - ((m_trigger - v_cap_next) / v_cap)));
					x_time = dt;
					v_cap_next  = m_trigger;
					flip_flop = 1;
					count_r++;
					update_exponent = 1;
				}
			}
			else    /* Immediate discharge. No change in dt. */
			{
				x_time = dt;
				v_cap_next = m_trigger;
				flip_flop = 1;
				count_r++;
			}
		}
		else
		{
			/* The constant current gets changed to a voltage due to a load resistor. */
			if (flip_flop)
			{
				if ((i == 0) && (DSD_555_CC__RBIAS == 0))
				{
					/* No charging current, so we have to discharge the cap
					 * due to rGnd.
					 */
					if (update_t_rc)
						t_rc = DSD_555_CC_T_RC_DISCHARGE_NO_I;
					else
						t_rc = m_t_rc_discharge_no_i;
					if (update_exponent)
						exponent = RC_CHARGE_EXP_DT(t_rc, dt);
					else
						exponent = m_exp_discharge_no_i;

					v_cap_next = v_cap - (v_cap * exponent);
					dt = 0;
				}
				else
				{
					/* Charging */
					/* If the cap voltage is past the current source charging limit
					 * then only the bias voltage will charge the cap. */
					v = v_bias;
					if (v_cap < v_vcharge_limit) v += vi;
					else if (m_type <= 3) v = v_vcharge_limit;

					if (update_t_rc)
						t_rc = DSD_555_CC_T_RC_CHARGE;
					else
						t_rc = m_t_rc_charge;
					if (update_exponent)
						exponent = RC_CHARGE_EXP_DT(t_rc, dt);
					else
						exponent = m_exp_charge;

					v_cap_next = v_cap + ((v - v_cap) * exponent);
					dt         = 0;

					/* has it charged past upper limit? */
					if (v_cap_next >= m_threshold)
					{
						/* calculate the overshoot time */
						dt     = t_rc  * log(1.0 / (1.0 - ((v_cap_next - m_threshold) / (v - v_cap))));
						x_time = dt;
						v_cap_next = m_threshold;
						flip_flop = 0;
						count_f++;
						update_exponent = 1;
					}
				}
			}
			else /* Discharging */
			if (r_discharge)
			{
				if (update_t_rc)
					t_rc = DSD_555_CC_T_RC_DISCHARGE;
				else
					t_rc = m_t_rc_discharge;
				if (update_exponent)
					exponent = RC_CHARGE_EXP_DT(t_rc, dt);
				else
					exponent = m_exp_discharge;

				v_cap_next = v_cap - (v_cap * exponent);
				dt = 0;

				/* has it discharged past lower limit? */
				if (v_cap_next <= m_trigger)
				{
					/* calculate the overshoot time */
					dt     = t_rc  * log(1.0 / (1.0 - ((m_trigger - v_cap_next) / v_cap)));
					x_time = dt;
					v_cap_next = m_trigger;
					flip_flop = 1;
					count_r++;
					update_exponent = 1;
				}
			}
			else    /* Immediate discharge. No change in dt. */
			{
				x_time = dt;
				v_cap_next = m_trigger;
				flip_flop = 1;
				count_r++;
			}
		}
		v_cap = v_cap_next;
	} while(dt);

	m_cap_voltage = v_cap;

	/* Convert last switch time to a ratio */
	x_time = x_time / this->sample_time();

	switch (m_output_type)
	{
		case DISC_555_OUT_SQW:
			if (count_f + count_r >= 2)
				/* force at least 1 toggle */
				v_out =  m_flip_flop ? 0 : m_v_out_high;
			else
				v_out = flip_flop * m_v_out_high;
			/* Fake it to AC if needed */
			v_out += m_ac_shift;
			break;
		case DISC_555_OUT_CAP:
			v_out = v_cap + m_ac_shift;
			break;
		case DISC_555_OUT_ENERGY:
			if (x_time == 0) x_time = 1.0;
			v_out = m_v_out_high * (flip_flop ? x_time : (1.0 - x_time));
			v_out += m_ac_shift;
			break;
		case DISC_555_OUT_LOGIC_X:
			v_out = flip_flop + x_time;
			break;
		case DISC_555_OUT_COUNT_F_X:
			v_out = count_f ? count_f + x_time : count_f;
			break;
		case DISC_555_OUT_COUNT_R_X:
			v_out = count_r ? count_r + x_time : count_r;
			break;
		case DISC_555_OUT_COUNT_F:
			v_out = count_f;
			break;
		case DISC_555_OUT_COUNT_R:
			v_out = count_r;
			break;
	}
	set_output(0, v_out);
	m_flip_flop = flip_flop;
}

DISCRETE_RESET(dsd_555_cc)
{
	DISCRETE_DECLARE_INFO(discrete_555_cc_desc)

	double  r_temp, r_discharge = 0, r_charge = 0;

	m_flip_flop   = 1;
	m_cap_voltage = 0;

	m_output_type = info->options & DISC_555_OUT_MASK;

	/* Use the defaults or supplied values. */
	m_v_out_high  = (info->v_out_high  == DEFAULT_555_HIGH) ? info->v_pos - 1.2 : info->v_out_high;
	m_v_cc_source = (info->v_cc_source == DEFAULT_555_CC_SOURCE) ? info->v_pos : info->v_cc_source;

	/* Setup based on v_pos power source */
	m_threshold = info->v_pos * 2.0 / 3.0;
	m_trigger   = info->v_pos / 3.0;

	m_output_is_ac = info->options & DISC_555_OUT_AC;
	/* Calculate DC shift needed to make squarewave waveform AC */
	m_ac_shift     = m_output_is_ac ? -m_v_out_high / 2.0 : 0;

	/* There are 8 different types of basic oscillators
	 * depending on the resistors used.  We will determine
	 * the type of circuit at reset, because the ciruit type
	 * is constant.  See Below.
	 */
	m_type = (DSD_555_CC__RDIS > 0) | ((DSD_555_CC__RGND  > 0) << 1) | ((DSD_555_CC__RBIAS  > 0) << 2);

	/* optimization if none of the values are nodes */
	m_has_rc_nodes = 0;
	if (this->input_is_node() & DSD_555_CC_RC_MASK)
		m_has_rc_nodes = 1;
	else
	{
		switch (m_type) /* see dsd_555_cc_reset for descriptions */
		{
			case 1:
				r_discharge = DSD_555_CC__RDIS;
			case 0:
				break;
			case 3:
				r_discharge = RES_2_PARALLEL(DSD_555_CC__RDIS, DSD_555_CC__RGND);
			case 2:
				r_charge = DSD_555_CC__RGND;
				break;
			case 4:
				r_charge = DSD_555_CC__RBIAS;
				break;
			case 5:
				r_charge = DSD_555_CC__RBIAS + DSD_555_CC__RDIS;
				r_discharge = DSD_555_CC__RDIS;
				break;
			case 6:
				r_charge = RES_2_PARALLEL(DSD_555_CC__RBIAS, DSD_555_CC__RGND);
				break;
			case 7:
				r_temp   = DSD_555_CC__RBIAS + DSD_555_CC__RDIS;
				r_charge = RES_2_PARALLEL(r_temp, DSD_555_CC__RGND);
				r_discharge = RES_2_PARALLEL(DSD_555_CC__RGND, DSD_555_CC__RDIS);
				break;
		}

		m_exp_bleed  = RC_CHARGE_EXP(DSD_555_CC_T_RC_BLEED);
		m_t_rc_discharge_01 = DSD_555_CC_T_RC_DISCHARGE_01;
		m_exp_discharge_01  = RC_CHARGE_EXP(m_t_rc_discharge_01);
		m_t_rc_discharge_no_i = DSD_555_CC_T_RC_DISCHARGE_NO_I;
		m_exp_discharge_no_i  = RC_CHARGE_EXP(m_t_rc_discharge_no_i);
		m_t_rc_charge = DSD_555_CC_T_RC_CHARGE;
		m_exp_charge  = RC_CHARGE_EXP(m_t_rc_charge);
		m_t_rc_discharge = DSD_555_CC_T_RC_DISCHARGE;
		m_exp_discharge  = RC_CHARGE_EXP(m_t_rc_discharge);
	}

	/* Step to set the output */
	this->step();

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
	 *  gnd          gnd                through rDischarge
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
	 *   |            |      |                  through rDischarge || rGnd  ( || means in parallel)
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
	 *    gnd       gnd          gnd                          through rDischarge
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
	 *    gnd       gnd          gnd    gnd                   through rDischarge || rGnd
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
#define DSD_555_VCO1__RESET DISCRETE_INPUT(0)   /* reset active low */
#define DSD_555_VCO1__VIN1  DISCRETE_INPUT(1)
#define DSD_555_VCO1__VIN2  DISCRETE_INPUT(2)

DISCRETE_STEP(dsd_555_vco1)
{
	DISCRETE_DECLARE_INFO(discrete_555_vco1_desc)

	int     count_f = 0;
	int     count_r = 0;
	double  dt;             /* change in time */
	double  x_time  = 0;    /* time since change happened */
	double  v_cap;          /* Current voltage on capacitor, before dt */
	double  v_cap_next = 0; /* Voltage on capacitor, after dt */

	double  v_out = 0;

	dt    = this->sample_time();    /* Change in time */
	v_cap = m_cap_voltage;

	/* Check: if the Control Voltage node is connected. */
	if (m_ctrlv_is_node && DSD_555_VCO1__RESET) /* reset active low */
	{
		/* If CV is less then .25V, the circuit will oscillate way out of range.
		 * So we will just ignore it when it happens. */
		if (DSD_555_VCO1__VIN2 < .25) return;
		/* If it is a node then calculate thresholds based on Control Voltage */
		m_threshold = DSD_555_VCO1__VIN2;
		m_trigger   = DSD_555_VCO1__VIN2 / 2.0;
		/* Since the thresholds may have changed we need to update the FF */
		if (v_cap >= m_threshold)
		{
			x_time = dt;
			m_flip_flop = 0;
			count_f++;
		}
		else
		if (v_cap <= m_trigger)
		{
			x_time = dt;
			m_flip_flop = 1;
			count_r++;
		}
	}

	/* Keep looping until all toggling in time sample is used up. */
	do
	{
		if (m_flip_flop)
		{
			/* if we are in reset then toggle f/f and discharge */
			if (!DSD_555_VCO1__RESET)   /* reset active low */
			{
				m_flip_flop = 0;
				count_f++;
			}
			else
			{
				/* Charging */
				/* iC=C*dv/dt  works out to dv=iC*dt/C */
				v_cap_next = v_cap + (m_i_charge * dt / info->c);
				dt         = 0;

				/* has it charged past upper limit? */
				if (v_cap_next >= m_threshold)
				{
					/* calculate the overshoot time */
					dt     = info->c * (v_cap_next - m_threshold) / m_i_charge;
					v_cap  = m_threshold;
					x_time = dt;
					m_flip_flop = 0;
					count_f++;
				}
			}
		}
		else
		{
			/* Discharging */
			/* iC=C*dv/dt  works out to dv=iC*dt/C */
			v_cap_next = v_cap - (m_i_discharge * dt / info->c);

			/* if we are in reset, then the cap can discharge to 0 */
			if (!DSD_555_VCO1__RESET)   /* reset active low */
			{
				if (v_cap_next < 0) v_cap_next = 0;
				dt = 0;
			}
			else
			{
				/* if we are out of reset and the cap voltage is less then
				 * the lower threshold, toggle f/f and start charging */
				if (v_cap <= m_trigger)
				{
					if (m_flip_flop == 0)
					{
						/* don't need to track x_time here */
						m_flip_flop = 1;
						count_r++;
					}
				}
				else
				{
					dt = 0;
					/* has it discharged past lower limit? */
					if (v_cap_next <= m_trigger)
					{
						/* calculate the overshoot time */
						dt     = info->c * (v_cap_next - m_trigger) / m_i_discharge;
						v_cap  = m_trigger;
						x_time = dt;
						m_flip_flop = 1;
						count_r++;
					}
				}
			}
		}
	} while(dt);

	m_cap_voltage = v_cap_next;

	/* Convert last switch time to a ratio.  No x_time in reset. */
	x_time = x_time / this->sample_time();
	if (!DSD_555_VCO1__RESET) x_time = 0;

	switch (m_output_type)
	{
		case DISC_555_OUT_SQW:
			v_out = m_flip_flop * m_v_out_high + m_ac_shift;
			break;
		case DISC_555_OUT_CAP:
			v_out = v_cap_next;
			/* Fake it to AC if needed */
			if (m_output_is_ac)
				v_out -= m_threshold * 3.0 /4.0;
			break;
		case DISC_555_OUT_ENERGY:
			if (x_time == 0) x_time = 1.0;
			v_out =  m_v_out_high * (m_flip_flop ? x_time : (1.0 - x_time));
			v_out += m_ac_shift;
			break;
		case DISC_555_OUT_LOGIC_X:
			v_out = m_flip_flop + x_time;
			break;
		case DISC_555_OUT_COUNT_F_X:
			v_out = count_f ? count_f + x_time : count_f;
			break;
		case DISC_555_OUT_COUNT_R_X:
			v_out = count_r ? count_r + x_time : count_r;
			break;
		case DISC_555_OUT_COUNT_F:
			v_out = count_f;
			break;
		case DISC_555_OUT_COUNT_R:
			v_out = count_r;
			break;
	}
	set_output(0, v_out);
}

DISCRETE_RESET(dsd_555_vco1)
{
	DISCRETE_DECLARE_INFO(discrete_555_vco1_desc)

	double v_ratio_r3, v_ratio_r4_1, r_in_1;

	m_output_type  = info->options & DISC_555_OUT_MASK;
	m_output_is_ac = info->options & DISC_555_OUT_AC;

	/* Setup op-amp parameters */

	/* The voltage at op-amp +in is always a fixed ratio of the modulation voltage. */
	v_ratio_r3 = info->r3 / (info->r2 + info->r3);          /* +in voltage */
	/* The voltage at op-amp -in is 1 of 2 fixed ratios of the modulation voltage,
	 * based on the 555 Flip-Flop state. */
	/* If the FF is 0, then only R1 is connected allowing the full modulation volatge to pass. */
	/* v_ratio_r4_0 = 1 */
	/* If the FF is 1, then R1 & R4 make a voltage divider similar to R2 & R3 */
	v_ratio_r4_1 = info->r4 / (info->r1 + info->r4);        /* -in voltage */
	/* the input resistance to the op amp depends on the FF state */
	/* r_in_0 = info->r1 when FF = 0 */
	r_in_1 = 1.0 / (1.0 / info->r1 + 1.0 / info->r4);   /* input resistance when r4 switched in */

	/* Now that we know the voltages entering the op amp and the resistance for the
	 * FF states, we can predetermine the ratios for the charge/discharge currents. */
	m_i_discharge = (1 - v_ratio_r3) / info->r1;
	m_i_charge    = (v_ratio_r3 - v_ratio_r4_1) / r_in_1;

	/* the cap starts off discharged */
	m_cap_voltage = 0;

	/* Setup 555 parameters */

	/* There is no charge on the cap so the 555 goes high at init. */
	m_flip_flop     = 1;
	m_ctrlv_is_node = (this->input_is_node() >> 2) & 1;
	m_v_out_high    = (info->v_out_high == DEFAULT_555_HIGH) ? info->v_pos - 1.2 : info->v_out_high;

	/* Calculate 555 thresholds.
	 * If the Control Voltage is a node, then the thresholds will be calculated each step.
	 * If the Control Voltage is a fixed voltage, then the thresholds will be calculated
	 * from that.  Otherwise we will use thresholds based on v_pos. */
	if (!m_ctrlv_is_node && (DSD_555_VCO1__VIN2 != -1))
	{
		/* Setup based on supplied Control Voltage static value */
		m_threshold = DSD_555_VCO1__VIN2;
		m_trigger   = DSD_555_VCO1__VIN2 / 2.0;
	}
	else
	{
		/* Setup based on v_pos power source */
		m_threshold = info->v_pos * 2.0 / 3.0;
		m_trigger   = info->v_pos / 3.0;
	}

	/* Calculate DC shift needed to make squarewave waveform AC */
	m_ac_shift = m_output_is_ac ? -m_v_out_high / 2.0 : 0;
}


/************************************************************************
 *
 * DSD_566 - Usage of node_description values
 *
 * Mar 2004, D Renaud. updated Sept 2009
 *
 * The data sheets for this are no where near correct.
 * This simulation is based on the internal schematic and testing of
 * a real Signetics IC.
 *
 * The 566 is a constant current based VCO.  If you change R, that affects
 * the charge/discharge rate.  A constant current source will charge the
 * cap linearly.  Of course due to the transistors there will be some
 * non-linear areas at the ends of the Vmod range.  As the Vmod voltage
 * drops from Vcharge, the frequency generated increases.
 *
 * The Triangle (pin 4) output is just a buffered version of the cap
 * charge.  It is about 1.35 higher then the cap voltage.
 * The Square (pin 3) output starts low as the cap voltages rises.
 * Once a threshold is reached, the cap starts to discharge, and the
 * Square output goes high.  The Square high output is about 1V less then
 * B+.  Unloaded it is .75V less.  With a 4.7k pull-down resistor, it
 * is 1.06V less.  So I will simulate at 1V less. The Square low voltage
 * is non-linear so I will use a table.  The cap toggle thresholds vary
 * depending on B+, so they will be simulated with a table.
 *
 * The data sheets show Vmod should be no less then 3/4*B+.  In reality
 * you can go to close to 1/2*B+ before you lose linearity.  Below 1/2,
 * oscillation stops.  When Vmod is 0V to 0.1V less then B+, it also
 * loses linearity, and stops oscillating when >= B+.  This is because
 * there is no voltage difference to create a current source.
 *
 * The current source is dependant on the voltage difference between B+
 * and Vmod.  Due to transistor action, it is not 100%, but this formula
 * gives a good approximation:
 * I = ((B+ - Vmod - 0.1) * 0.95) / R
 * You can test the current VS modulation function by using 10k for R
 * and replace C with a 10k resistor.  Then you can monitor the voltage
 * on pin 7 to work out the current.  I=V/R.  It will start to oscillate
 * when in the cap threshold range.
 *
 * When Vmod drops below the stable range, the current source no longer
 * functions properly.  Technically this is out of the range specified
 * for the IC.  Of course old games used this range anyways, so we need
 * to know how the real IC behaves.  When Vmod drops below the stable range,
 * the charge current is stops dropping instead of increasing, while the
 * discharge current still functions.  This means the frequency generated
 * starts to drop as the voltage lowers, instead of the normal increase
 * in frequency.
 *
 ************************************************************************/
#define DSD_566__VMOD       DISCRETE_INPUT(0)
#define DSD_566__R          DISCRETE_INPUT(1)
#define DSD_566__C          DISCRETE_INPUT(2)
#define DSD_566__VPOS       DISCRETE_INPUT(3)
#define DSD_566__VNEG       DISCRETE_INPUT(4)
#define DSD_566__VCHARGE    DISCRETE_INPUT(5)
#define DSD_566__OPTIONS    DISCRETE_INPUT(6)


static const struct
{
	double  c_high[6];
	double  c_low[6];
	double  sqr_low[6];
	double  osc_stable[6];
	double  osc_stop[6];
} ne566 =
{
	/* 10      10.5      11      11.5      12     13     14     15             B+ */
	{3.364, /*3.784,*/ 4.259, /*4.552,*/ 4.888, 5.384, 5.896, 6.416},       /* c_high */
	{1.940, /*2.100,*/ 2.276, /*2.404,*/ 2.580, 2.880, 3.180, 3.488},       /* c_low */
	{4.352, /*4.144,*/ 4.080, /*4.260,*/ 4.500, 4.960, 5.456, 5.940},       /* sqr_low */
	{4.885, /*5.316,*/ 5.772, /*6.075,*/ 6.335, 6.912, 7.492, 7.945},       /* osc_stable */
	{4.495, /*4.895,*/ 5.343, /*5.703,*/ 5.997, 6.507, 7.016, 7.518}        /* osc_stop */
};

DISCRETE_STEP(dsd_566)
{
	double  i = 0;          /* Charging current created by vIn */
	double  i_rise;         /* non-linear rise charge current */
	double  dt;             /* change in time */
	double  x_time = 0;
	double  v_cap;          /* Current voltage on capacitor, before dt */
	int     count_f = 0, count_r = 0;

	double  v_out = 0.0;

	dt    = this->sample_time();    /* Change in time */
	v_cap = m_cap_voltage;  /* Set to voltage before change */

	/* Calculate charging current if it is in range */
	if (EXPECTED(DSD_566__VMOD > m_v_osc_stop))
	{
		double v_charge = DSD_566__VCHARGE - DSD_566__VMOD - 0.1;
		if (v_charge > 0)
		{
			i = (v_charge * .95) / DSD_566__R;
			if (DSD_566__VMOD < m_v_osc_stable)
			{
				/* no where near correct calculation of non linear range */
				i_rise = ((DSD_566__VCHARGE - m_v_osc_stable - 0.1) * .95) / DSD_566__R;
				i_rise *= 1.0 - (m_v_osc_stable - DSD_566__VMOD) / (m_v_osc_stable - m_v_osc_stop);
			}
			else
				i_rise = i;
		}
		else
			return;
	}
	else return;

	/* Keep looping until all toggling in this time sample is used up. */
	do
	{
		if (m_flip_flop)
		{
			/* Discharging */
			v_cap -= i * dt / DSD_566__C;
			dt     = 0;

			/* has it discharged past lower limit? */
			if (UNEXPECTED(v_cap < m_threshold_low))
			{
				/* calculate the overshoot time */
				dt = DSD_566__C * (m_threshold_low - v_cap) / i;
				v_cap = m_threshold_low;
				m_flip_flop = 0;
				count_f++;
				x_time = dt;
			}
		}
		else
		{
			/* Charging */
			/* iC=C*dv/dt  works out to dv=iC*dt/C */
			v_cap += i_rise * dt / DSD_566__C;
			dt     = 0;
			/* Yes, if the cap voltage has reached the max voltage it can,
			 * and the 566 threshold has not been reached, then oscillation stops.
			 * This is the way the actual electronics works.
			 * This is why you never play with the pots after being factory adjusted
			 * to work in the proper range. */
			if (UNEXPECTED(v_cap > DSD_566__VMOD)) v_cap = DSD_566__VMOD;

			/* has it charged past upper limit? */
			if (UNEXPECTED(v_cap > m_threshold_high))
			{
				/* calculate the overshoot time */
				dt = DSD_566__C * (v_cap - m_threshold_high) / i;
				v_cap = m_threshold_high;
				m_flip_flop = 1;
				count_r++;
				x_time = dt;
			}
		}
	} while(dt);

	m_cap_voltage = v_cap;

	/* Convert last switch time to a ratio */
	x_time /= this->sample_time();

	switch (m_out_type)
	{
		case DISC_566_OUT_SQUARE:
			v_out = m_flip_flop ? m_v_sqr_high : m_v_sqr_low;
			if (m_fake_ac)
				v_out += m_ac_shift;
			break;
		case DISC_566_OUT_ENERGY:
			if (x_time == 0) x_time = 1.0;
			v_out = m_v_sqr_low + m_v_sqr_diff * (m_flip_flop ? x_time : (1.0 - x_time));
			if (m_fake_ac)
				v_out += m_ac_shift;
			break;
		case DISC_566_OUT_LOGIC:
			v_out = m_flip_flop;
			break;
		case DISC_566_OUT_TRIANGLE:
			v_out = v_cap;
			if (m_fake_ac)
				v_out += m_ac_shift;
			break;
		case DISC_566_OUT_COUNT_F_X:
			v_out = count_f ? count_f + x_time : count_f;
			break;
		case DISC_566_OUT_COUNT_R_X:
			v_out = count_r ? count_r + x_time : count_r;
			break;
		case DISC_566_OUT_COUNT_F:
			v_out = count_f;
			break;
		case DISC_566_OUT_COUNT_R:
			v_out = count_r;
			break;
	}
	set_output(0, v_out);
}

DISCRETE_RESET(dsd_566)
{
	int     v_int;
	double  v_float;

	m_out_type = (int)DSD_566__OPTIONS & DISC_566_OUT_MASK;
	m_fake_ac =  (int)DSD_566__OPTIONS & DISC_566_OUT_AC;

	if (DSD_566__VNEG >= DSD_566__VPOS)
		fatalerror("[v_neg >= v_pos] in NODE_%d!\n", this->index());

	v_float = DSD_566__VPOS - DSD_566__VNEG;
	v_int = (int)v_float;
	if ( v_float < 10 || v_float > 15 )
		fatalerror("v_neg and/or v_pos out of range in NODE_%d\n", this->index());
	if ( v_float != v_int )
		/* fatal for now. */
		fatalerror("Power should be integer in NODE_%d\n", this->index());

	m_flip_flop   = 0;
	m_cap_voltage = 0;

	v_int -= 10;
	m_threshold_high = ne566.c_high[v_int] + DSD_566__VNEG;
	m_threshold_low  = ne566.c_low[v_int] + DSD_566__VNEG;
	m_v_sqr_high     = DSD_566__VPOS - 1;
	m_v_sqr_low      = ne566.sqr_low[v_int] + DSD_566__VNEG;
	m_v_sqr_diff     = m_v_sqr_high - m_v_sqr_low;
	m_v_osc_stable  = ne566.osc_stable[v_int] + DSD_566__VNEG;
	m_v_osc_stop        = ne566.osc_stop[v_int] + DSD_566__VNEG;

	m_ac_shift = 0;
	if (m_fake_ac)
	{
		if (m_out_type == DISC_566_OUT_TRIANGLE)
			m_ac_shift = (m_threshold_high - m_threshold_low) / 2 - m_threshold_high;
		else
			m_ac_shift = m_v_sqr_diff / 2 - m_v_sqr_high;
	}

	/* Step the output */
	this->step();
}


/************************************************************************
 *
 * DSD_LS624 - Usage of node_description values
 *
 * Dec 2007, Couriersud based on data sheet
 * Oct 2009, complete re-write based on IC testing
 ************************************************************************/
#define DSD_LS624__ENABLE       DISCRETE_INPUT(0)
#define DSD_LS624__VMOD         DISCRETE_INPUT(1)
#define DSD_LS624__VRNG         DISCRETE_INPUT(2)
#define DSD_LS624__C            DISCRETE_INPUT(3)
#define DSD_LS624__R_FREQ_IN    DISCRETE_INPUT(4)
#define DSD_LS624__C_FREQ_IN    DISCRETE_INPUT(5)
#define DSD_LS624__R_RNG_IN     DISCRETE_INPUT(6)
#define DSD_LS624__OUTTYPE      DISCRETE_INPUT(7)

#define LS624_R_EXT         600.0       /* as specified in data sheet */
#define LS624_OUT_HIGH      4.5         /* measured */
#define LS624_IN_R      RES_K(90)   /* measured & 70K + 20k per data sheet */

/*
 * The 74LS624 series are constant current based VCOs.  The Freq Control voltage
 * modulates the current source.  The current is created from Rext, which is
 * internally fixed at 600 ohms for all devices except the 74LS628 which has
 * external connections.  The current source linearly discharges the cap voltage.
 * The cap starts with 0V charge across it.  One side is connected to a fixed voltage
 * bias circuit.  The other side is charged negatively from the current source until
 * a certain low threshold is reached.  Once this threshold is reached, the output
 * toggles state and the pins on the cap reverse in respect to the charge/bias hookup.
 * This starts the one side of the cap to be at bias, and the other side of the cap is
 * now at bias + the charge on the cap which is bias - threshold.
 * Y = 0;  CX1 = bias;    CX2 = charge
 * Y = 1;  CX1 = charge;  CX2 = bias
 * The Range voltage adjusts the threshold voltage.  The higher the Range voltage,
 * the lower the threshold voltage, the longer the cap can charge, the lower the frequency.
 *
 * In a perfect world it would work like this:
 * The current is based on the mysterious Rext mentioned in the data sheet.
 * I = (VfreqControl  * 20k/90k) / Rext
 * where Rext = 600 ohms or external Rext on a 74LS628
 * The Freq Control has an input impedance of approximately 90k, so any input resistance
 * connected to the Freq Control pin works as a voltage divider.
 * I = (VfreqControl * 20k/(90k + RfreqControlIn)) / Rext
 * That gives us a change in voltage on the cap of
 * dV = I / sampleRate / C_inFarads
 *
 * Unfortunately the chip does not behave linearly do to internal interactions,
 * so I have just worked out the formula (using zunzun.com) of FreqControl and
 * range to frequency out for a fixed cap value of 0.1uf.  Other cap values can just
 * scale from that.  From the freq, we calculate the time of 1/2 cycle using 1/Freq/2.
 * Then just use that to toggle a waveform.
 */


DISCRETE_STEP(dsd_ls624)
{
	double  x_time = 0;
	double  freq, t1;
	double  v_freq_2, v_freq_3, v_freq_4;
	double  t_used = m_t_used;
	double  dt = this->sample_time();;
	double  v_freq = DSD_LS624__VMOD;
	double  v_rng = DSD_LS624__VRNG;
	int     count_f = 0, count_r = 0;

	/* coefficients */
	const double k1 = 1.9904769024796283E+03;
	const double k2 = 1.2070059213983407E+03;
	const double k3 = 1.3266985579561108E+03;
	const double k4 = -1.5500979825922698E+02;
	const double k5 = 2.8184536266938172E+00;
	const double k6 = -2.3503421582744556E+02;
	const double k7 = -3.3836786704527788E+02;
	const double k8 = -1.3569136703258670E+02;
	const double k9 = 2.9914575453819188E+00;
	const double k10 = 1.6855569086173170E+00;

	if (UNEXPECTED(DSD_LS624__ENABLE == 0))
		return;

	/* scale due to input resistance */
	v_freq *= m_v_freq_scale;
	v_rng *= m_v_rng_scale;

	/* apply cap if needed */
	if (m_has_freq_in_cap)
	{
		m_v_cap_freq_in += (v_freq - m_v_cap_freq_in) * m_exponent;
		v_freq = m_v_cap_freq_in;
	}

	/* Polyfunctional3D_model created by zunzun.com using sum of squared absolute error */
	v_freq_2 = v_freq * v_freq;
	v_freq_3 = v_freq_2 * v_freq;
	v_freq_4 = v_freq_3 * v_freq;
	freq = k1;
	freq += k2 * v_freq;
	freq += k3 * v_freq_2;
	freq += k4 * v_freq_3;
	freq += k5 * v_freq_4;
	freq += k6 * v_rng;
	freq += k7 * v_rng * v_freq;
	freq += k8 * v_rng * v_freq_2;
	freq += k9 * v_rng * v_freq_3;
	freq += k10 * v_rng * v_freq_4;

	freq *= CAP_U(0.1) / DSD_LS624__C;

	t1 = 0.5 / freq ;
	t_used += this->sample_time();
	do
	{
		dt = 0;
		if (t_used > t1)
		{
			/* calculate the overshoot time */
			t_used -= t1;
			m_flip_flop ^= 1;
			if (m_flip_flop)
				count_r++;
			else
				count_f++;
			/* fix up any frequency increase change errors */
			while(t_used > this->sample_time())
				t_used -= this->sample_time();
			x_time = t_used;
			dt = t_used;
		}
	}while(dt);

	m_t_used = t_used;

	/* Convert last switch time to a ratio */
	x_time = x_time / this->sample_time();

	switch (m_out_type)
	{
		case DISC_LS624_OUT_LOGIC_X:
			set_output(0,  m_flip_flop  + x_time);
			break;
		case DISC_LS624_OUT_COUNT_F_X:
			set_output(0,  count_f ? count_f + x_time : count_f);
			break;
		case DISC_LS624_OUT_COUNT_R_X:
			set_output(0,   count_r ? count_r + x_time : count_r);
			break;
		case DISC_LS624_OUT_COUNT_F:
			set_output(0,  count_f);
			break;
		case DISC_LS624_OUT_COUNT_R:
			set_output(0,  count_r);
			break;
		case DISC_LS624_OUT_ENERGY:
			if (x_time == 0) x_time = 1.0;
			set_output(0,  LS624_OUT_HIGH * (m_flip_flop ? x_time : (1.0 - x_time)));
			break;
		case DISC_LS624_OUT_LOGIC:
				set_output(0,  m_flip_flop);
			break;
		case DISC_LS624_OUT_SQUARE:
			set_output(0,  m_flip_flop ? LS624_OUT_HIGH : 0);
			break;
	}
}

DISCRETE_RESET(dsd_ls624)
{
	m_out_type = (int)DSD_LS624__OUTTYPE;

	m_flip_flop = 0;
	m_t_used = 0;
	m_v_freq_scale = LS624_IN_R / (DSD_LS624__R_FREQ_IN + LS624_IN_R);
	m_v_rng_scale = LS624_IN_R / (DSD_LS624__R_RNG_IN + LS624_IN_R);
	if (DSD_LS624__C_FREQ_IN > 0)
	{
		m_has_freq_in_cap = 1;
		m_exponent = RC_CHARGE_EXP(RES_2_PARALLEL(DSD_LS624__R_FREQ_IN, LS624_IN_R) * DSD_LS624__C_FREQ_IN);
		m_v_cap_freq_in = 0;
	}
	else
		m_has_freq_in_cap = 0;

	set_output(0,  0);
}
