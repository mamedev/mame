// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/************************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by K.Wilkins (mame@esplexo.co.uk)
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
#define DST_CRFILTER__IN        DISCRETE_INPUT(0)
#define DST_CRFILTER__R         DISCRETE_INPUT(1)
#define DST_CRFILTER__C         DISCRETE_INPUT(2)
#define DST_CRFILTER__VREF      DISCRETE_INPUT(3)

DISCRETE_STEP(dst_crfilter)
{
	if (UNEXPECTED(m_has_rc_nodes))
	{
		double rc = DST_CRFILTER__R * DST_CRFILTER__C;
		if (rc != m_rc)
		{
			m_rc = rc;
			m_exponent = RC_CHARGE_EXP(rc);
		}
	}

	double v_out = DST_CRFILTER__IN - m_vCap;
	double v_diff = v_out - DST_CRFILTER__VREF;
	set_output(0,  v_out);
	m_vCap += v_diff * m_exponent;
}

DISCRETE_RESET(dst_crfilter)
{
	m_has_rc_nodes = this->input_is_node() & 0x6;
	m_rc = DST_CRFILTER__R * DST_CRFILTER__C;
	m_exponent = RC_CHARGE_EXP(m_rc);
	m_vCap = 0;
	set_output(0,  DST_CRFILTER__IN);
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
#define DST_FILTER1__ENABLE DISCRETE_INPUT(0)
#define DST_FILTER1__IN     DISCRETE_INPUT(1)
#define DST_FILTER1__FREQ   DISCRETE_INPUT(2)
#define DST_FILTER1__TYPE   DISCRETE_INPUT(3)

static void calculate_filter1_coefficients(discrete_base_node *node, double fc, double type,
											struct discrete_filter_coeff &coeff)
{
	double den, w, two_over_T;

	/* calculate digital filter coefficents */
	/*w = 2.0*M_PI*fc; no pre-warping */
	w = node->sample_rate()*2.0*tan(M_PI*fc/node->sample_rate()); /* pre-warping */
	two_over_T = 2.0*node->sample_rate();

	den = w + two_over_T;
	coeff.a1 = (w - two_over_T)/den;
	if (type == DISC_FILTER_LOWPASS)
	{
		coeff.b0 = coeff.b1 = w/den;
	}
	else if (type == DISC_FILTER_HIGHPASS)
	{
		coeff.b0 = two_over_T/den;
		coeff.b1 = -(coeff.b0);
	}
	else
	{
		/* FIXME: reenable */
		//node->m_device->discrete_log("calculate_filter1_coefficients() - Invalid filter type for 1st order filter.");
	}
}

DISCRETE_STEP(dst_filter1)
{
	double gain = 1.0;
	double v_out;

	if (DST_FILTER1__ENABLE == 0.0)
	{
		gain = 0.0;
	}

	v_out = -m_fc.a1*m_fc.y1 + m_fc.b0*gain*DST_FILTER1__IN + m_fc.b1*m_fc.x1;

	m_fc.x1 = gain*DST_FILTER1__IN;
	m_fc.y1 = v_out;
	set_output(0, v_out);
}

DISCRETE_RESET(dst_filter1)
{
	calculate_filter1_coefficients(this, DST_FILTER1__FREQ, DST_FILTER1__TYPE, m_fc);
	set_output(0,  0);
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
#define DST_FILTER2__ENABLE DISCRETE_INPUT(0)
#define DST_FILTER2__IN     DISCRETE_INPUT(1)
#define DST_FILTER2__FREQ   DISCRETE_INPUT(2)
#define DST_FILTER2__DAMP   DISCRETE_INPUT(3)
#define DST_FILTER2__TYPE   DISCRETE_INPUT(4)

static void calculate_filter2_coefficients(discrete_base_node *node,
											double fc, double d, double type,
											struct discrete_filter_coeff &coeff)
{
	double w;   /* cutoff freq, in radians/sec */
	double w_squared;
	double den; /* temp variable */
	double two_over_T = 2 * node->sample_rate();
	double two_over_T_squared = two_over_T * two_over_T;

	/* calculate digital filter coefficents */
	/*w = 2.0*M_PI*fc; no pre-warping */
	w = node->sample_rate() * 2.0 * tan(M_PI * fc / node->sample_rate()); /* pre-warping */
	w_squared = w * w;

	den = two_over_T_squared + d*w*two_over_T + w_squared;

	coeff.a1 = 2.0 * (-two_over_T_squared + w_squared) / den;
	coeff.a2 = (two_over_T_squared - d * w * two_over_T + w_squared) / den;

	if (type == DISC_FILTER_LOWPASS)
	{
		coeff.b0 = coeff.b2 = w_squared/den;
		coeff.b1 = 2.0 * (coeff.b0);
	}
	else if (type == DISC_FILTER_BANDPASS)
	{
		coeff.b0 = d * w * two_over_T / den;
		coeff.b1 = 0.0;
		coeff.b2 = -(coeff.b0);
	}
	else if (type == DISC_FILTER_HIGHPASS)
	{
		coeff.b0 = coeff.b2 = two_over_T_squared / den;
		coeff.b1 = -2.0 * (coeff.b0);
	}
	else
	{
		/* FIXME: reenable */
		//node->device->discrete_log("calculate_filter2_coefficients() - Invalid filter type for 2nd order filter.");
	}
}

DISCRETE_STEP(dst_filter2)
{
	double gain = 1.0;
	double v_out;

	if (DST_FILTER2__ENABLE == 0.0)
	{
		gain = 0.0;
	}

	v_out = -m_fc.a1 * m_fc.y1 - m_fc.a2 * m_fc.y2 +
					m_fc.b0 * gain * DST_FILTER2__IN + m_fc.b1 * m_fc.x1 + m_fc.b2 * m_fc.x2;

	m_fc.x2 = m_fc.x1;
	m_fc.x1 = gain * DST_FILTER2__IN;
	m_fc.y2 = m_fc.y1;
	m_fc.y1 = v_out;
	set_output(0, v_out);
}

DISCRETE_RESET(dst_filter2)
{
	calculate_filter2_coefficients(this, DST_FILTER2__FREQ, DST_FILTER2__DAMP, DST_FILTER2__TYPE,
									m_fc);
	set_output(0,  0);
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
#define DST_OP_AMP_FILT__ENABLE DISCRETE_INPUT(0)
#define DST_OP_AMP_FILT__INP1   DISCRETE_INPUT(1)
#define DST_OP_AMP_FILT__INP2   DISCRETE_INPUT(2)
#define DST_OP_AMP_FILT__TYPE   DISCRETE_INPUT(3)

DISCRETE_STEP(dst_op_amp_filt)
{
	DISCRETE_DECLARE_INFO(discrete_op_amp_filt_info)
	double v_out = 0;

	double i, v = 0;

	if (DST_OP_AMP_FILT__ENABLE)
	{
		if (m_is_norton)
		{
			v = DST_OP_AMP_FILT__INP1 - OP_AMP_NORTON_VBE;
			if (v < 0) v = 0;
		}
		else
		{
			/* Millman the input voltages. */
			i  = m_iFixed;
			switch (m_type)
			{
				case DISC_OP_AMP_FILTER_IS_LOW_PASS_1_A:
					i += (DST_OP_AMP_FILT__INP1 - DST_OP_AMP_FILT__INP2) / info->r1;
					if (info->r2 != 0)
						i += (m_vP - DST_OP_AMP_FILT__INP2) / info->r2;
					if (info->r3 != 0)
						i += (m_vN - DST_OP_AMP_FILT__INP2) / info->r3;
					break;
				default:
					i += (DST_OP_AMP_FILT__INP1 - m_vRef) / info->r1;
					if (info->r2 != 0)
						i += (DST_OP_AMP_FILT__INP2 - m_vRef) / info->r2;
					break;
			}
			v = i * m_rTotal;
		}

		switch (m_type)
		{
			case DISC_OP_AMP_FILTER_IS_LOW_PASS_1:
				m_vC1 += (v - m_vC1) * m_exponentC1;
				v_out = m_vC1 * m_gain + info->vRef;
				break;

			case DISC_OP_AMP_FILTER_IS_LOW_PASS_1_A:
				m_vC1 += (v - m_vC1) * m_exponentC1;
				v_out = m_vC1 * m_gain + DST_OP_AMP_FILT__INP2;
				break;

			case DISC_OP_AMP_FILTER_IS_HIGH_PASS_1:
				v_out = (v - m_vC1) * m_gain + info->vRef;
				m_vC1 += (v - m_vC1) * m_exponentC1;
				break;

			case DISC_OP_AMP_FILTER_IS_BAND_PASS_1:
				v_out = (v - m_vC2);
				m_vC2 += (v - m_vC2) * m_exponentC2;
				m_vC1 += (v_out - m_vC1) * m_exponentC1;
				v_out = m_vC1 * m_gain + info->vRef;
				break;

			case DISC_OP_AMP_FILTER_IS_BAND_PASS_0 | DISC_OP_AMP_IS_NORTON:
				m_vC1 += (v - m_vC1) * m_exponentC1;
				m_vC2 += (m_vC1 - m_vC2) * m_exponentC2;
				v = m_vC2;
				v_out = v - m_vC3;
				m_vC3 += (v - m_vC3) * m_exponentC3;
				i = v_out / m_rTotal;
				v_out = (m_iFixed - i) * info->rF;
				break;

			case DISC_OP_AMP_FILTER_IS_HIGH_PASS_0 | DISC_OP_AMP_IS_NORTON:
				v_out = v - m_vC1;
				m_vC1 += (v - m_vC1) * m_exponentC1;
				i = v_out / m_rTotal;
				v_out = (m_iFixed - i) * info->rF;
				break;

			case DISC_OP_AMP_FILTER_IS_BAND_PASS_1M:
			case DISC_OP_AMP_FILTER_IS_BAND_PASS_1M | DISC_OP_AMP_IS_NORTON:
				v_out = -m_fc.a1 * m_fc.y1 - m_fc.a2 * m_fc.y2 +
								m_fc.b0 * v + m_fc.b1 * m_fc.x1 + m_fc.b2 * m_fc.x2 +
								m_vRef;
				m_fc.x2 = m_fc.x1;
				m_fc.x1 = v;
				m_fc.y2 = m_fc.y1;
				break;
		}

		/* Clip the output to the voltage rails.
		 * This way we get the original distortion in all it's glory.
		 */
		if (v_out > m_vP) v_out = m_vP;
		if (v_out < m_vN) v_out = m_vN;
		m_fc.y1 = v_out - m_vRef;
		set_output(0, v_out);
	}
	else
		set_output(0, 0);

}

DISCRETE_RESET(dst_op_amp_filt)
{
	DISCRETE_DECLARE_INFO(discrete_op_amp_filt_info)

	/* Convert the passed filter type into an int for easy use. */
	m_type = (int)DST_OP_AMP_FILT__TYPE & DISC_OP_AMP_FILTER_TYPE_MASK;
	m_is_norton = (int)DST_OP_AMP_FILT__TYPE & DISC_OP_AMP_IS_NORTON;

	if (m_is_norton)
	{
		m_vRef = 0;
		m_rTotal = info->r1;
		if (m_type == (DISC_OP_AMP_FILTER_IS_BAND_PASS_0 | DISC_OP_AMP_IS_NORTON))
			m_rTotal += info->r2 +  info->r3;

		/* Setup the current to the + input. */
		m_iFixed = (info->vP - OP_AMP_NORTON_VBE) / info->r4;

		/* Set the output max. */
		m_vP =  info->vP - OP_AMP_NORTON_VBE;
		m_vN =  info->vN;
	}
	else
	{
		m_vRef = info->vRef;
		/* Set the output max. */
		m_vP =  info->vP - OP_AMP_VP_RAIL_OFFSET;
		m_vN =  info->vN;

		/* Work out the input resistance.  It is all input and bias resistors in parallel. */
		m_rTotal  = 1.0 / info->r1;         /* There has to be an R1.  Otherwise the table is wrong. */
		if (info->r2 != 0) m_rTotal += 1.0 / info->r2;
		if (info->r3 != 0) m_rTotal += 1.0 / info->r3;
		m_rTotal = 1.0 / m_rTotal;

		m_iFixed = 0;

		m_rRatio = info->rF / (m_rTotal + info->rF);
		m_gain = -info->rF / m_rTotal;
	}

	switch (m_type)
	{
		case DISC_OP_AMP_FILTER_IS_LOW_PASS_1:
		case DISC_OP_AMP_FILTER_IS_LOW_PASS_1_A:
			m_exponentC1 = RC_CHARGE_EXP(info->rF * info->c1);
			m_exponentC2 =  0;
			break;
		case DISC_OP_AMP_FILTER_IS_HIGH_PASS_1:
			m_exponentC1 = RC_CHARGE_EXP(m_rTotal * info->c1);
			m_exponentC2 =  0;
			break;
		case DISC_OP_AMP_FILTER_IS_BAND_PASS_1:
			m_exponentC1 = RC_CHARGE_EXP(info->rF * info->c1);
			m_exponentC2 = RC_CHARGE_EXP(m_rTotal * info->c2);
			break;
		case DISC_OP_AMP_FILTER_IS_BAND_PASS_1M | DISC_OP_AMP_IS_NORTON:
			if (info->r2 == 0)
				m_rTotal = info->r1;
			else
				m_rTotal = RES_2_PARALLEL(info->r1, info->r2);
		case DISC_OP_AMP_FILTER_IS_BAND_PASS_1M:
		{
			double fc = 1.0 / (2 * M_PI * sqrt(m_rTotal * info->rF * info->c1 * info->c2));
			double d  = (info->c1 + info->c2) / sqrt(info->rF / m_rTotal * info->c1 * info->c2);
			double gain = -info->rF / m_rTotal * info->c2 / (info->c1 + info->c2);

			calculate_filter2_coefficients(this, fc, d, DISC_FILTER_BANDPASS, m_fc);
			m_fc.b0 *= gain;
			m_fc.b1 *= gain;
			m_fc.b2 *= gain;

			if (m_is_norton)
				m_vRef = (info->vP - OP_AMP_NORTON_VBE) / info->r3 * info->rF;
			else
				m_vRef = info->vRef;

			break;
		}
		case DISC_OP_AMP_FILTER_IS_BAND_PASS_0 | DISC_OP_AMP_IS_NORTON:
			m_exponentC1 = RC_CHARGE_EXP(RES_2_PARALLEL(info->r1, info->r2 + info->r3 + info->r4) * info->c1);
			m_exponentC2 = RC_CHARGE_EXP(RES_2_PARALLEL(info->r1 + info->r2, info->r3 + info->r4) * info->c2);
			m_exponentC3 = RC_CHARGE_EXP((info->r1 + info->r2 + info->r3 + info->r4) * info->c3);
			break;
		case DISC_OP_AMP_FILTER_IS_HIGH_PASS_0 | DISC_OP_AMP_IS_NORTON:
			m_exponentC1 = RC_CHARGE_EXP(info->r1 * info->c1);
			break;
	}

	/* At startup there is no charge on the caps and output is 0V in relation to vRef. */
	m_vC1 = 0;
	m_vC1b = 0;
	m_vC2 = 0;
	m_vC3 = 0;

	set_output(0,  info->vRef);
}


/************************************************************************
 *
 * DST_RC_CIRCUIT_1 - RC charge/discharge circuit
 *
 ************************************************************************/
#define DST_RC_CIRCUIT_1__IN0       DISCRETE_INPUT(0)
#define DST_RC_CIRCUIT_1__IN1       DISCRETE_INPUT(1)
#define DST_RC_CIRCUIT_1__R         DISCRETE_INPUT(2)
#define DST_RC_CIRCUIT_1__C         DISCRETE_INPUT(3)

#define CD4066_R_ON 270

DISCRETE_STEP( dst_rc_circuit_1 )
{
	if (DST_RC_CIRCUIT_1__IN0 == 0)
		if (DST_RC_CIRCUIT_1__IN1 == 0)
			/* cap is floating and does not change charge */
			/* output is pulled to ground */
			set_output(0,  0);
		else
		{
			/* cap is discharged */
			m_v_cap -= m_v_cap * m_exp_2;
			set_output(0,  m_v_cap * m_v_drop);
		}
	else
		if (DST_RC_CIRCUIT_1__IN1 == 0)
		{
			/* cap is charged */
			m_v_cap += (5.0 - m_v_cap) * m_exp_1;
			/* output is pulled to ground */
			set_output(0,  0);
		}
		else
		{
			/* cap is charged slightly less */
			m_v_cap += (m_v_charge_1_2 - m_v_cap) * m_exp_1_2;
			set_output(0,  m_v_cap * m_v_drop);
		}
}

DISCRETE_RESET( dst_rc_circuit_1 )
{
	/* the charging voltage across the cap based on in2*/
	m_v_drop =  RES_VOLTAGE_DIVIDER(CD4066_R_ON, CD4066_R_ON + DST_RC_CIRCUIT_1__R);
	m_v_charge_1_2 = 5.0 * m_v_drop;
	m_v_cap = 0;

	/* precalculate charging exponents */
	/* discharge cap - in1 = 0, in2 = 1*/
	m_exp_2 = RC_CHARGE_EXP((CD4066_R_ON + DST_RC_CIRCUIT_1__R) * DST_RC_CIRCUIT_1__C);
	/* charge cap - in1 = 1, in2 = 0 */
	m_exp_1 = RC_CHARGE_EXP(CD4066_R_ON * DST_RC_CIRCUIT_1__C);
	/* charge cap - in1 = 1, in2 = 1 */
	m_exp_1_2 = RC_CHARGE_EXP(RES_2_PARALLEL(CD4066_R_ON, CD4066_R_ON + DST_RC_CIRCUIT_1__R) * DST_RC_CIRCUIT_1__C);

	/* starts at 0 until cap starts charging */
	set_output(0,  0);
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
#define DST_RCDISC__ENABLE  DISCRETE_INPUT(0)
#define DST_RCDISC__IN      DISCRETE_INPUT(1)
#define DST_RCDISC__R       DISCRETE_INPUT(2)
#define DST_RCDISC__C       DISCRETE_INPUT(3)

DISCRETE_STEP(dst_rcdisc)
{
	switch (m_state)
	{
		case 0:     /* waiting for trigger  */
			if(DST_RCDISC__ENABLE)
			{
				m_state = 1;
				m_t = 0;
			}
			set_output(0,  0);
			break;

		case 1:
			if (DST_RCDISC__ENABLE)
			{
				set_output(0,  DST_RCDISC__IN * exp(m_t / m_exponent0));
				m_t += this->sample_time();
			} else
			{
				m_state = 0;
			}
	}
}

DISCRETE_RESET(dst_rcdisc)
{
	set_output(0,  0);

	m_state = 0;
	m_t = 0;
	m_exponent0=-1.0 * DST_RCDISC__R * DST_RCDISC__C;
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
#define DST_RCDISC2__ENABLE DISCRETE_INPUT(0)
#define DST_RCDISC2__IN0    DISCRETE_INPUT(1)
#define DST_RCDISC2__R0     DISCRETE_INPUT(2)
#define DST_RCDISC2__IN1    DISCRETE_INPUT(3)
#define DST_RCDISC2__R1     DISCRETE_INPUT(4)
#define DST_RCDISC2__C      DISCRETE_INPUT(5)

DISCRETE_STEP(dst_rcdisc2)
{
	double diff;

	/* Works differently to other as we are always on, no enable */
	/* exponential based in difference between input/output   */

	diff = ((DST_RCDISC2__ENABLE == 0) ? DST_RCDISC2__IN0 : DST_RCDISC2__IN1) - m_v_out;
	diff = diff - (diff * ((DST_RCDISC2__ENABLE == 0) ? m_exponent0 : m_exponent1));
	m_v_out += diff;
	set_output(0, m_v_out);
}

DISCRETE_RESET(dst_rcdisc2)
{
	m_v_out = 0;

	m_state = 0;
	m_t = 0;
	m_exponent0 = RC_DISCHARGE_EXP(DST_RCDISC2__R0 * DST_RCDISC2__C);
	m_exponent1 = RC_DISCHARGE_EXP(DST_RCDISC2__R1 * DST_RCDISC2__C);
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
#define DST_RCDISC3__ENABLE DISCRETE_INPUT(0)
#define DST_RCDISC3__IN     DISCRETE_INPUT(1)
#define DST_RCDISC3__R1     DISCRETE_INPUT(2)
#define DST_RCDISC3__R2     DISCRETE_INPUT(3)
#define DST_RCDISC3__C      DISCRETE_INPUT(4)
#define DST_RCDISC3__DJV    DISCRETE_INPUT(5)

DISCRETE_STEP(dst_rcdisc3)
{
	double diff;

	/* Exponential based in difference between input/output   */

	if(DST_RCDISC3__ENABLE)
	{
		diff = DST_RCDISC3__IN - m_v_out;
		if (m_v_diode > 0)
		{
			if (diff > 0)
			{
				diff = diff * m_exponent0;
			}
			else if (diff < -m_v_diode)
			{
				diff = diff * m_exponent1;
			}
			else
			{
				diff = diff * m_exponent0;
			}
		}
		else
		{
			if (diff < 0)
			{
				diff = diff * m_exponent0;
			}
			else if (diff > -m_v_diode)
			{
				diff = diff * m_exponent1;
			}
			else
			{
				diff = diff * m_exponent0;
			}
		}
		m_v_out += diff;
		set_output(0, m_v_out);
	}
	else
	{
		set_output(0, 0);
	}
}

DISCRETE_RESET(dst_rcdisc3)
{
	m_v_out = 0;

	m_state = 0;
	m_t = 0;
	m_v_diode = DST_RCDISC3__DJV;
	m_exponent0 = RC_CHARGE_EXP(DST_RCDISC3__R1 * DST_RCDISC3__C);
	m_exponent1 = RC_CHARGE_EXP(RES_2_PARALLEL(DST_RCDISC3__R1, DST_RCDISC3__R2) * DST_RCDISC3__C);
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
#define DST_RCDISC4__ENABLE DISCRETE_INPUT(0)
#define DST_RCDISC4__IN     DISCRETE_INPUT(1)
#define DST_RCDISC4__R1     DISCRETE_INPUT(2)
#define DST_RCDISC4__R2     DISCRETE_INPUT(3)
#define DST_RCDISC4__R3     DISCRETE_INPUT(4)
#define DST_RCDISC4__C1     DISCRETE_INPUT(5)
#define DST_RCDISC4__VP     DISCRETE_INPUT(6)
#define DST_RCDISC4__TYPE   DISCRETE_INPUT(7)

DISCRETE_STEP(dst_rcdisc4)
{
	int inp1 = (DST_RCDISC4__IN == 0) ? 0 : 1;
	double v_out = 0;

	if (DST_RCDISC4__ENABLE == 0)
	{
		set_output(0, 0);
		return;
	}

	switch (m_type)
	{
		case 1:
		case 3:
			m_vC1 += ((m_v[inp1] - m_vC1) * m_exp[inp1]);
			v_out = m_vC1;
			break;
	}

	/* clip output */
	if (v_out > m_max_out) v_out = m_max_out;
	if (v_out < 0) v_out = 0;
	set_output(0, v_out);
}

DISCRETE_RESET( dst_rcdisc4)
{
	double  v, i, r, rT;

	m_type = 0;
	/* some error checking. */
	if (DST_RCDISC4__R1 <= 0 || DST_RCDISC4__R2 <= 0 || DST_RCDISC4__C1 <= 0 || (DST_RCDISC4__R3 <= 0 &&  m_type == 1))
	{
		m_device->discrete_log("Invalid component values in NODE_%d.\n", this->index());
		return;
	}
	if (DST_RCDISC4__VP < 3)
	{
		m_device->discrete_log("vP must be >= 3V in NODE_%d.\n", this->index());
		return;
	}
	if (DST_RCDISC4__TYPE < 1 || DST_RCDISC4__TYPE > 3)
	{
		m_device->discrete_log("Invalid circuit type in NODE_%d.\n", this->index());
		return;
	}

	m_vC1 = 0;
	/* store type as integer */
	m_type = (int)DST_RCDISC4__TYPE;
	/* setup the maximum op-amp output. */
	m_max_out = DST_RCDISC4__VP - OP_AMP_VP_RAIL_OFFSET;

	switch (m_type)
	{
		case 1:
			/* We will simulate this as a voltage divider with 2 states depending
			 * on the input.  But we have to take the diodes into account.
			 */
			v = DST_RCDISC4__VP - .5;   /* diode drop */

			/* When the input is 1, both R1 & R3 are basically in parallel. */
			r  = RES_2_PARALLEL(DST_RCDISC4__R1, DST_RCDISC4__R3);
			rT = DST_RCDISC4__R2 + r;
			i  = v / rT;
			m_v[1] = i * r + .5;
			rT = RES_2_PARALLEL(DST_RCDISC4__R2, r);
			m_exp[1] = RC_CHARGE_EXP(rT * DST_RCDISC4__C1);

			/* When the input is 0, R1 is out of circuit. */
			rT = DST_RCDISC4__R2 + DST_RCDISC4__R3;
			i  = v / rT;
			m_v[0] = i * DST_RCDISC4__R3 + .5;
			rT = RES_2_PARALLEL(DST_RCDISC4__R2, DST_RCDISC4__R3);
			m_exp[0] = RC_CHARGE_EXP(rT * DST_RCDISC4__C1);
			break;

		case 3:
			/* We will simulate this as a voltage divider with 2 states depending
			 * on the input.  The 1k pullup is in parallel with the internal TTL
			 * resistance, so we will just use .5k in series with R1.
			 */
			r = 500.0 + DST_RCDISC4__R1;
			m_v[1] = RES_VOLTAGE_DIVIDER(r, DST_RCDISC4__R2) * (5.0 - 0.5);
			rT = RES_2_PARALLEL(r, DST_RCDISC4__R2);
			m_exp[1] = RC_CHARGE_EXP(rT * DST_RCDISC4__C1);

			/* When the input is 0, R1 is out of circuit. */
			m_v[0] = 0;
			m_exp[0] = RC_CHARGE_EXP(DST_RCDISC4__R2 * DST_RCDISC4__C1);
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
#define DST_RCDISC5__ENABLE DISCRETE_INPUT(0)
#define DST_RCDISC5__IN     DISCRETE_INPUT(1)
#define DST_RCDISC5__R      DISCRETE_INPUT(2)
#define DST_RCDISC5__C      DISCRETE_INPUT(3)

DISCRETE_STEP( dst_rcdisc5)
{
	double diff,u;

	/* Exponential based in difference between input/output   */

	u = DST_RCDISC5__IN - 0.7; /* Diode drop */
	if( u < 0)
		u = 0;

	diff = u - m_v_cap;

	if(DST_RCDISC5__ENABLE)
	{
		if(diff < 0)
			diff = diff * m_exponent0;

		m_v_cap += diff;
		set_output(0,  m_v_cap);
	}
	else
	{
		if(diff > 0)
			m_v_cap = u;

		set_output(0,  0);
	}
}

DISCRETE_RESET( dst_rcdisc5)
{
	set_output(0,  0);

	m_state = 0;
	m_t = 0;
	m_v_cap = 0;
	m_exponent0 = RC_CHARGE_EXP(DST_RCDISC5__R * DST_RCDISC5__C);
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
#define DST_RCDISC_MOD__IN1     DISCRETE_INPUT(0)
#define DST_RCDISC_MOD__IN2     DISCRETE_INPUT(1)
#define DST_RCDISC_MOD__R1      DISCRETE_INPUT(2)
#define DST_RCDISC_MOD__R2      DISCRETE_INPUT(3)
#define DST_RCDISC_MOD__R3      DISCRETE_INPUT(4)
#define DST_RCDISC_MOD__R4      DISCRETE_INPUT(5)
#define DST_RCDISC_MOD__C       DISCRETE_INPUT(6)
#define DST_RCDISC_MOD__VP      DISCRETE_INPUT(7)

DISCRETE_STEP(dst_rcdisc_mod)
{
	double  diff, v_cap, u, vD;
	int     mod_state, mod1_state, mod2_state;

	/* Exponential based in difference between input/output   */
	v_cap = m_v_cap;

	mod1_state = DST_RCDISC_MOD__IN1 > 0.5;
	mod2_state = DST_RCDISC_MOD__IN2 > 0.6;
	mod_state  = (mod2_state << 1) + mod1_state;

	u = mod1_state ? 0 : DST_RCDISC_MOD__VP;
	/* Clamp */
	diff = u - v_cap;
	vD = diff * m_vd_gain[mod_state];
	if (vD < -0.6)
	{
		diff  = u + 0.6 - v_cap;
		diff -= diff * m_exp_low[mod1_state];
		v_cap += diff;
		set_output(0,  mod2_state ? 0 : -0.6);
	}
	else
	{
		diff  -= diff * m_exp_high[mod_state];
		v_cap += diff;
		/* neglecting current through R3 drawn by next8 node */
		set_output(0,  mod2_state ? 0: (u - v_cap) * m_gain[mod1_state]);
	}
	m_v_cap = v_cap;
}

DISCRETE_RESET(dst_rcdisc_mod)
{
	double  rc[2], rc2[2];

	/* pre-calculate fixed values */
	/* DST_RCDISC_MOD__IN1 <= 0.5 */
	rc[0] = DST_RCDISC_MOD__R1 + DST_RCDISC_MOD__R2;
	if (rc[0] < 1) rc[0] = 1;
	m_exp_low[0]  = RC_DISCHARGE_EXP(DST_RCDISC_MOD__C * rc[0]);
	m_gain[0]     = RES_VOLTAGE_DIVIDER(rc[0], DST_RCDISC_MOD__R4);
	/* DST_RCDISC_MOD__IN1 > 0.5 */
	rc[1] = DST_RCDISC_MOD__R2;
	if (rc[1] < 1) rc[1] = 1;
	m_exp_low[1]  = RC_DISCHARGE_EXP(DST_RCDISC_MOD__C * rc[1]);
	m_gain[1]     = RES_VOLTAGE_DIVIDER(rc[1], DST_RCDISC_MOD__R4);
	/* DST_RCDISC_MOD__IN2 <= 0.6 */
	rc2[0] = DST_RCDISC_MOD__R4;
	/* DST_RCDISC_MOD__IN2 > 0.6 */
	rc2[1] = RES_2_PARALLEL(DST_RCDISC_MOD__R3, DST_RCDISC_MOD__R4);
	/* DST_RCDISC_MOD__IN1 <= 0.5 && DST_RCDISC_MOD__IN2 <= 0.6 */
	m_exp_high[0] = RC_DISCHARGE_EXP(DST_RCDISC_MOD__C * (rc[0] + rc2[0]));
	m_vd_gain[0]  = RES_VOLTAGE_DIVIDER(rc[0], rc2[0]);
	/* DST_RCDISC_MOD__IN1 > 0.5  && DST_RCDISC_MOD__IN2 <= 0.6 */
	m_exp_high[1] = RC_DISCHARGE_EXP(DST_RCDISC_MOD__C * (rc[1] + rc2[0]));
	m_vd_gain[1]  = RES_VOLTAGE_DIVIDER(rc[1], rc2[0]);
	/* DST_RCDISC_MOD__IN1 <= 0.5 && DST_RCDISC_MOD__IN2 > 0.6 */
	m_exp_high[2] = RC_DISCHARGE_EXP(DST_RCDISC_MOD__C * (rc[0] + rc2[1]));
	m_vd_gain[2]  = RES_VOLTAGE_DIVIDER(rc[0], rc2[1]);
	/* DST_RCDISC_MOD__IN1 > 0.5  && DST_RCDISC_MOD__IN2 > 0.6 */
	m_exp_high[3] = RC_DISCHARGE_EXP(DST_RCDISC_MOD__C * (rc[1] + rc2[1]));
	m_vd_gain[3]  = RES_VOLTAGE_DIVIDER(rc[1], rc2[1]);

	m_v_cap  = 0;
	set_output(0,  0);
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
#define DST_RCFILTER__VIN       DISCRETE_INPUT(0)
#define DST_RCFILTER__R         DISCRETE_INPUT(1)
#define DST_RCFILTER__C         DISCRETE_INPUT(2)
#define DST_RCFILTER__VREF      DISCRETE_INPUT(3)

DISCRETE_STEP(dst_rcfilter)
{
	if (EXPECTED(m_is_fast))
		m_v_out += ((DST_RCFILTER__VIN - m_v_out) * m_exponent);
	else
	{
		if (UNEXPECTED(m_has_rc_nodes))
		{
			double rc = DST_RCFILTER__R * DST_RCFILTER__C;
			if (rc != m_rc)
			{
				m_rc = rc;
				m_exponent = RC_CHARGE_EXP(rc);
			}
		}

		/************************************************************************/
		/* Next Value = PREV + (INPUT_VALUE - PREV)*(1-(EXP(-TIMEDELTA/RC)))    */
		/************************************************************************/

		m_vCap += ((DST_RCFILTER__VIN - m_v_out) * m_exponent);
		m_v_out = m_vCap + DST_RCFILTER__VREF;
	}
	set_output(0,  m_v_out);
}


DISCRETE_RESET(dst_rcfilter)
{
	m_has_rc_nodes = this->input_is_node() & 0x6;
	m_rc = DST_RCFILTER__R * DST_RCFILTER__C;
	m_exponent = RC_CHARGE_EXP(m_rc);
	m_vCap   = 0;
	m_v_out = 0;
	/* FIXME --> we really need another class here */
	if (!m_has_rc_nodes && DST_RCFILTER__VREF == 0)
		m_is_fast = 1;
	else
		m_is_fast = 0;
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
#define DST_RCFILTER_SW__ENABLE     DISCRETE_INPUT(0)
#define DST_RCFILTER_SW__VIN        DISCRETE_INPUT(1)
#define DST_RCFILTER_SW__SWITCH     DISCRETE_INPUT(2)
#define DST_RCFILTER_SW__R          DISCRETE_INPUT(3)
#define DST_RCFILTER_SW__C(x)       DISCRETE_INPUT(4+x)

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
DISCRETE_STEP(dst_rcfilter_sw)
{
	int i;
	int bits = (int)DST_RCFILTER_SW__SWITCH;
	double us = 0;
	double vIn = DST_RCFILTER_SW__VIN;
	double v_out;

	if (EXPECTED(DST_RCFILTER_SW__ENABLE))
	{
		switch (bits)
		{
		case 0:
			v_out = vIn;
			break;
		case 1:
			m_vCap[0] += (vIn - m_vCap[0]) * m_exp0;
			v_out = m_vCap[0] + (vIn - m_vCap[0]) * m_factor;
			break;
		case 2:
			m_vCap[1] += (vIn - m_vCap[1]) * m_exp1;
			v_out = m_vCap[1] + (vIn - m_vCap[1]) * m_factor;
			break;
		default:
			for (i = 0; i < 4; i++)
			{
				if (( bits & (1 << i)) != 0)
					us += m_vCap[i];
			}
			v_out = m_f1[bits] * vIn + m_f2[bits]  * us;
			for (i = 0; i < 4; i++)
			{
				if (( bits & (1 << i)) != 0)
					m_vCap[i] += (v_out - m_vCap[i]) * m_exp[i];
			}
		}
		set_output(0, v_out);
	}
	else
	{
		set_output(0, 0);
	}
}

DISCRETE_RESET(dst_rcfilter_sw)
{
	int i, bits;

	for (i = 0; i < 4; i++)
	{
		m_vCap[i] = 0;
		m_exp[i] = RC_CHARGE_EXP( CD4066_ON_RES * DST_RCFILTER_SW__C(i));
	}

	for (bits=0; bits < 15; bits++)
	{
		double rs = 0;

		for (i = 0; i < 4; i++)
		{
			if (( bits & (1 << i)) != 0)
				rs += DST_RCFILTER_SW__R;
		}
		m_f1[bits] = RES_VOLTAGE_DIVIDER(rs, CD4066_ON_RES);
		m_f2[bits] = DST_RCFILTER_SW__R / (CD4066_ON_RES + rs);
	}


	/* fast cases */
	m_exp0 = RC_CHARGE_EXP((CD4066_ON_RES + DST_RCFILTER_SW__R) * DST_RCFILTER_SW__C(0));
	m_exp1 = RC_CHARGE_EXP((CD4066_ON_RES + DST_RCFILTER_SW__R) * DST_RCFILTER_SW__C(1));
	m_factor = RES_VOLTAGE_DIVIDER(DST_RCFILTER_SW__R, CD4066_ON_RES);

	set_output(0,  0);
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
#define DST_RCINTEGRATE__IN1    DISCRETE_INPUT(0)
#define DST_RCINTEGRATE__R1     DISCRETE_INPUT(1)
#define DST_RCINTEGRATE__R2     DISCRETE_INPUT(2)
#define DST_RCINTEGRATE__R3     DISCRETE_INPUT(3)
#define DST_RCINTEGRATE__C      DISCRETE_INPUT(4)
#define DST_RCINTEGRATE__VP     DISCRETE_INPUT(5)
#define DST_RCINTEGRATE__TYPE   DISCRETE_INPUT(6)

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
#define IES     7e-15
#define ALPHAT  0.99
#define KT      0.026
#define EM_IC(x) (ALPHAT * IES * exp( (x) / KT - 1.0 ))

DISCRETE_STEP( dst_rcintegrate)
{
	double diff, u, iQ, iQc, iC, RG, vE;
	double vP;

	u  = DST_RCINTEGRATE__IN1;
	vP = DST_RCINTEGRATE__VP;

	if ( u - 0.7  < m_vCap * m_gain_r1_r2)
	{
		/* discharge .... */
		diff  = 0.0 - m_vCap;
		iC    = m_c_exp1 * diff; /* iC */
		diff -= diff * m_exp_exponent1;
		m_vCap += diff;
		iQ = 0;
		vE = m_vCap * m_gain_r1_r2;
		RG = vE / iC;
	}
	else
	{
		/* charging */
		diff  = (vP - m_vCE) * m_f - m_vCap;
		iC    = 0.0 - m_c_exp0 * diff; /* iC */
		diff -= diff * m_exp_exponent0;
		m_vCap += diff;
		iQ = iC + (iC * DST_RCINTEGRATE__R1 + m_vCap) / DST_RCINTEGRATE__R2;
		RG = (vP - m_vCE) / iQ;
		vE = (RG - DST_RCINTEGRATE__R3) / RG * (vP - m_vCE);
	}


	u = DST_RCINTEGRATE__IN1;
	if (u > 0.7 + vE)
	{
		vE = u - 0.7;
		//iQc = EM_IC(u - vE);
		iQc = m_EM_IC_0_7;
	}
	else
		iQc = EM_IC(u - vE);

	m_vCE = MIN(vP - 0.1, vP - RG * iQc);

	/* Avoid oscillations
	 * The method tends to largely overshoot - no wonder without
	 * iterative solution approximation
	 */

	m_vCE = MAX(m_vCE, 0.1 );
	m_vCE = 0.1 * m_vCE + 0.9 * (vP - vE - iQ * DST_RCINTEGRATE__R3);

	switch (m_type)
	{
		case DISC_RC_INTEGRATE_TYPE1:
			set_output(0,  m_vCap);
			break;
		case DISC_RC_INTEGRATE_TYPE2:
			set_output(0,  vE);
			break;
		case DISC_RC_INTEGRATE_TYPE3:
			set_output(0,  MAX(0, vP - iQ * DST_RCINTEGRATE__R3));
			break;
	}
}

DISCRETE_RESET(dst_rcintegrate)
{
	double r;
	double dt = this->sample_time();

	m_type = DST_RCINTEGRATE__TYPE;

	m_vCap = 0;
	m_vCE  = 0;

	/* pre-calculate fixed values */
	m_gain_r1_r2 = RES_VOLTAGE_DIVIDER(DST_RCINTEGRATE__R1, DST_RCINTEGRATE__R2);

	r = DST_RCINTEGRATE__R1 / DST_RCINTEGRATE__R2 * DST_RCINTEGRATE__R3 + DST_RCINTEGRATE__R1 + DST_RCINTEGRATE__R3;

	m_f = RES_VOLTAGE_DIVIDER(DST_RCINTEGRATE__R3, DST_RCINTEGRATE__R2);
	m_exponent0 = -1.0 * r * m_f * DST_RCINTEGRATE__C;
	m_exponent1 = -1.0 * (DST_RCINTEGRATE__R1 + DST_RCINTEGRATE__R2) * DST_RCINTEGRATE__C;
	m_exp_exponent0 = exp(dt / m_exponent0);
	m_exp_exponent1 = exp(dt / m_exponent1);
	m_c_exp0 =  DST_RCINTEGRATE__C / m_exponent0 * m_exp_exponent0;
	m_c_exp1 =  DST_RCINTEGRATE__C / m_exponent1 * m_exp_exponent1;

	m_EM_IC_0_7 = EM_IC(0.7);

	set_output(0,  0);
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
#define DST_SALLEN_KEY__ENABLE  DISCRETE_INPUT(0)
#define DST_SALLEN_KEY__INP0    DISCRETE_INPUT(1)
#define DST_SALLEN_KEY__TYPE    DISCRETE_INPUT(2)

DISCRETE_STEP(dst_sallen_key)
{
	double gain = 1.0;
	double v_out;

	if (DST_SALLEN_KEY__ENABLE == 0.0)
	{
		gain = 0.0;
	}

	v_out = -m_fc.a1 * m_fc.y1 - m_fc.a2 * m_fc.y2 +
					m_fc.b0 * gain * DST_SALLEN_KEY__INP0 + m_fc.b1 * m_fc.x1 + m_fc.b2 * m_fc.x2;

	m_fc.x2 = m_fc.x1;
	m_fc.x1 = gain * DST_SALLEN_KEY__INP0;
	m_fc.y2 = m_fc.y1;
	m_fc.y1 = v_out;
	set_output(0, v_out);
}

DISCRETE_RESET(dst_sallen_key)
{
	DISCRETE_DECLARE_INFO(discrete_op_amp_filt_info)

	double freq, q;

	switch ((int) DST_SALLEN_KEY__TYPE)
	{
		case DISC_SALLEN_KEY_LOW_PASS:
			freq = 1.0 / ( 2.0 * M_PI * sqrt(info->c1 * info->c2 * info->r1 * info->r2));
			q = sqrt(info->c1 * info->c2 * info->r1 * info->r2) / (info->c2 * (info->r1 + info->r2));
			break;
		default:
			fatalerror("Unknown sallen key filter type\n");
	}

	calculate_filter2_coefficients(this, freq, 1.0 / q, DISC_FILTER_LOWPASS, m_fc);
	set_output(0,  0);
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
#define DST_RCFILTERN__ENABLE       DISCRETE_INPUT(0)
#define DST_RCFILTERN__IN       DISCRETE_INPUT(1)
#define DST_RCFILTERN__R        DISCRETE_INPUT(2)
#define DST_RCFILTERN__C        DISCRETE_INPUT(3)

#if 0
DISCRETE_RESET(dst_rcfilterN)
{
#if 0
	double f=1.0/(2*M_PI* DST_RCFILTERN__R * DST_RCFILTERN__C);

/* !!!!!!!!!!!!!! CAN'T CHEAT LIKE THIS !!!!!!!!!!!!!!!! */
/* Put this stuff in a context */

	this->m_input[2] = f;
	this->m_input[3] = DISC_FILTER_LOWPASS;

	/* Use first order filter */
	dst_filter1_reset(node);
#endif
}
#endif

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
#define DST_RCDISCN__ENABLE DISCRETE_INPUT(0)
#define DST_RCDISCN__IN     DISCRETE_INPUT(1)
#define DST_RCDISCN__R      DISCRETE_INPUT(2)
#define DST_RCDISCN__C      DISCRETE_INPUT(3)

DISCRETE_RESET(dst_rcdiscN)
{
#if 0
	double f = 1.0 / (2 * M_PI * DST_RCDISCN__R * DST_RCDISCN__C);

/* !!!!!!!!!!!!!! CAN'T CHEAT LIKE THIS !!!!!!!!!!!!!!!! */
/* Put this stuff in a context */

	this->m_input[2] = f;
	this->m_input[3] = DISC_FILTER_LOWPASS;

	/* Use first order filter */
	dst_filter1_reset(node);
#endif
}

DISCRETE_STEP(dst_rcdiscN)
{
	double gain = 1.0;
	double v_out;

	if (DST_RCDISCN__ENABLE == 0.0)
	{
		gain = 0.0;
	}

	/* A rise in the input signal results in an instant charge, */
	/* else discharge through the RC to zero */
	if (gain* DST_RCDISCN__IN > m_x1)
		v_out = gain* DST_RCDISCN__IN;
	else
		v_out = -m_a1*m_y1;

	m_x1 = gain* DST_RCDISCN__IN;
	m_y1 = v_out;
	set_output(0, v_out);
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
#define DST_RCDISC2N__ENABLE    DISCRETE_INPUT(0)
#define DST_RCDISC2N__IN0       DISCRETE_INPUT(1)
#define DST_RCDISC2N__R0        DISCRETE_INPUT(2)
#define DST_RCDISC2N__IN1       DISCRETE_INPUT(3)
#define DST_RCDISC2N__R1        DISCRETE_INPUT(4)
#define DST_RCDISC2N__C         DISCRETE_INPUT(5)


DISCRETE_STEP(dst_rcdisc2N)
{
	double inp = ((DST_RCDISC2N__ENABLE == 0) ? DST_RCDISC2N__IN0 : DST_RCDISC2N__IN1);
	double v_out;

	if (DST_RCDISC2N__ENABLE == 0)
		v_out = -m_fc0.a1*m_y1 + m_fc0.b0*inp + m_fc0.b1 * m_x1;
	else
		v_out = -m_fc1.a1*m_y1 + m_fc1.b0*inp + m_fc1.b1*m_x1;

	m_x1 = inp;
	m_y1 = v_out;
	set_output(0, v_out);
}

DISCRETE_RESET(dst_rcdisc2N)
{
	double f1,f2;

	f1 = 1.0 / (2 * M_PI * DST_RCDISC2N__R0 * DST_RCDISC2N__C);
	f2 = 1.0 / (2 * M_PI * DST_RCDISC2N__R1 * DST_RCDISC2N__C);

	calculate_filter1_coefficients(this, f1, DISC_FILTER_LOWPASS, m_fc0);
	calculate_filter1_coefficients(this, f2, DISC_FILTER_LOWPASS, m_fc1);

	/* Initialize the object */
	set_output(0,  0);
}
