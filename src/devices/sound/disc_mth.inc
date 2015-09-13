// license:BSD-3-Clause
// copyright-holders:K.Wilkins
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
#define DST_ADDER__ENABLE   DISCRETE_INPUT(0)
#define DST_ADDER__IN0      DISCRETE_INPUT(1)
#define DST_ADDER__IN1      DISCRETE_INPUT(2)
#define DST_ADDER__IN2      DISCRETE_INPUT(3)
#define DST_ADDER__IN3      DISCRETE_INPUT(4)

DISCRETE_STEP(dst_adder)
{
	if(DST_ADDER__ENABLE)
	{
		set_output(0,  DST_ADDER__IN0 + DST_ADDER__IN1 + DST_ADDER__IN2 + DST_ADDER__IN3);
	}
	else
	{
		set_output(0, 0);
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
#define DST_COMP_ADDER__SELECT  DISCRETE_INPUT(0)

DISCRETE_STEP(dst_comp_adder)
{
	int select;

	select = (int)DST_COMP_ADDER__SELECT;
	assert(select < 256);
	set_output(0,  m_total[select]);
}

DISCRETE_RESET(dst_comp_adder)
{
	DISCRETE_DECLARE_INFO(discrete_comp_adder_table)

	int i, bit;
	int bit_length = info->length;

	assert(bit_length <= 8);

	/* pre-calculate all possible values to speed up step routine */
	for(i = 0; i < 256; i++)
	{
		switch (info->type)
		{
			case DISC_COMP_P_CAPACITOR:
				m_total[i] = info->cDefault;
				for(bit = 0; bit < bit_length; bit++)
				{
					if (i & (1 << bit))
						m_total[i] += info->c[bit];
				}
				break;
			case DISC_COMP_P_RESISTOR:
				m_total[i] = (info->cDefault != 0) ? 1.0 / info->cDefault : 0;
				for(bit = 0; bit < bit_length; bit++)
				{
					if ((i & (1 << bit)) && (info->c[bit] != 0))
						m_total[i] += 1.0 / info->c[bit];
				}
				if (m_total[i] != 0)
					m_total[i] = 1.0 / m_total[i];
				break;
		}
	}
	set_output(0,  m_total[0]);
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
#define DST_CLAMP__IN       DISCRETE_INPUT(0)
#define DST_CLAMP__MIN      DISCRETE_INPUT(1)
#define DST_CLAMP__MAX      DISCRETE_INPUT(2)

DISCRETE_STEP(dst_clamp)
{
	if (DST_CLAMP__IN < DST_CLAMP__MIN)
		set_output(0,  DST_CLAMP__MIN);
	else if (DST_CLAMP__IN > DST_CLAMP__MAX)
		set_output(0,  DST_CLAMP__MAX);
	else
		set_output(0, DST_CLAMP__IN);
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
#define DST_DAC_R1__DATA        DISCRETE_INPUT(0)
#define DST_DAC_R1__VON         DISCRETE_INPUT(1)

DISCRETE_STEP(dst_dac_r1)
{
	int     data = (int)DST_DAC_R1__DATA;
	double  v = m_v_step[data];
	double  x_time = DST_DAC_R1__DATA - data;
	double  last_v = m_last_v;

	m_last_v = v;

	if (x_time > 0)
		v = x_time * (v - last_v) + last_v;

	/* Filter if needed, else just output voltage */
	if (m_has_c_filter)
	{
		double v_diff = v - m_v_out;
		/* optimization - if charged close enough to voltage */
		if (fabs(v_diff) < 0.000001)
			m_v_out = v;
		else
		{
			m_v_out += v_diff * m_exponent;
		}
	}
	else
		m_v_out = v;

	set_output(0, m_v_out);
}

DISCRETE_RESET(dst_dac_r1)
{
	DISCRETE_DECLARE_INFO(discrete_dac_r1_ladder)

	int bit;
	int ladderLength = info->ladderLength;
	int total_steps = 1 << ladderLength;
	double r_total = 0;
	double i_bias;
	double v_on = DST_DAC_R1__VON;

	m_last_v = 0;

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
		m_device->discrete_log("dst_dac_r1_reset - Ladder length too small");
	}
	if (ladderLength > DISC_LADDER_MAXRES )
	{
		m_device->discrete_log("dst_dac_r1_reset - Ladder length exceeds DISC_LADDER_MAXRES");
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

	m_v_out = 0;

	if (info->cFilter > 0)
	{
		m_has_c_filter = 1;
		/* Setup filter constant */
		m_exponent = RC_CHARGE_EXP(r_total * info->cFilter);
	}
	else
		m_has_c_filter = 0;

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
		m_v_step[i] = i_total * r_total;
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
#define DST_DIODE_MIX_INP_OFFSET    0
#define DST_DIODE_MIX__INP(addr)    DISCRETE_INPUT(DST_DIODE_MIX_INP_OFFSET + addr)

DISCRETE_STEP(dst_diode_mix)
{
	double  val, max = 0;
	int     addr;

	for (addr = 0; addr < m_size; addr++)
	{
		val = DST_DIODE_MIX__INP(addr) - m_v_junction[addr];
		if (val > max) max = val;
	}
	if (max < 0) max = 0;
	set_output(0,  max);
}

DISCRETE_RESET(dst_diode_mix)
{
	DISCRETE_DECLARE_INFO(double)

	int     addr;

	m_size = this->active_inputs() - DST_DIODE_MIX_INP_OFFSET;
	assert(m_size <= 8);

	for (addr = 0; addr < m_size; addr++)
	{
		if (info == NULL)
		{
			/* setup default junction voltage */
			m_v_junction[addr] = 0.5;
		}
		else
		{
			/* use supplied junction voltage */
			m_v_junction[addr] = *info++;
		}
	}
	this->step();
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
#define DST_DIVIDE__ENABLE  DISCRETE_INPUT(0)
#define DST_DIVIDE__IN      DISCRETE_INPUT(1)
#define DST_DIVIDE__DIV     DISCRETE_INPUT(2)

DISCRETE_STEP(dst_divide)
{
	if(DST_DIVIDE__ENABLE)
	{
		if(DST_DIVIDE__DIV == 0)
		{
			set_output(0, DBL_MAX); /* Max out but don't break */
			m_device->discrete_log("dst_divider_step() - Divide by Zero attempted in NODE_%02d.\n",this->index());
		}
		else
		{
				set_output(0, DST_DIVIDE__IN / DST_DIVIDE__DIV);
		}
	}
	else
	{
		set_output(0, 0);
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
#define DST_GAIN__IN        DISCRETE_INPUT(0)
#define DST_GAIN__GAIN      DISCRETE_INPUT(1)
#define DST_GAIN__OFFSET    DISCRETE_INPUT(2)

DISCRETE_STEP(dst_gain)
{
		set_output(0, DST_GAIN__IN * DST_GAIN__GAIN + DST_GAIN__OFFSET);
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
#define DST_INTEGRATE__TRG0 DISCRETE_INPUT(0)
#define DST_INTEGRATE__TRG1 DISCRETE_INPUT(1)

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
	DISCRETE_DECLARE_INFO(discrete_integrate_info)

	int     trig0, trig1;
	double  i_neg = 0;  /* current into - input */
	double  i_pos = 0;  /* current into + input */

	switch (info->type)
	{
		case DISC_INTEGRATE_OP_AMP_1:
			if (DST_INTEGRATE__TRG0 != 0)
			{
				/* This forces the cap to completely charge,
				 * and the output to go to it's max value.
				 */
				m_v_out = m_v_max_out;
				set_output(0, m_v_out);
				return;
			}
			m_v_out -= m_change;
			break;

		case DISC_INTEGRATE_OP_AMP_1 | DISC_OP_AMP_IS_NORTON:
			i_neg = m_v_max_in / info->r1;
			i_pos = (DST_INTEGRATE__TRG0 - OP_AMP_NORTON_VBE) / info->r2;
			if (i_pos < 0) i_pos = 0;
			m_v_out += (i_pos - i_neg) / this->sample_rate() / info->c;
			break;

		case DISC_INTEGRATE_OP_AMP_2 | DISC_OP_AMP_IS_NORTON:
			trig0  = (int)DST_INTEGRATE__TRG0;
			trig1  = (int)DST_INTEGRATE__TRG1;
			i_neg  = dst_trigger_function(trig0, trig1, 0, info->f0) ? m_v_max_in_d / info->r1 : 0;
			i_pos  = dst_trigger_function(trig0, trig1, 0, info->f1) ? m_v_max_in / info->r2 : 0;
			i_pos += dst_trigger_function(trig0, trig1, 0, info->f2) ? m_v_max_in_d / info->r3 : 0;
			m_v_out += (i_pos - i_neg) / this->sample_rate() / info->c;
			break;
	}

	/* Clip the output. */
	if (m_v_out < 0) m_v_out = 0;
	if (m_v_out > m_v_max_out) m_v_out = m_v_max_out;

	set_output(0, m_v_out);
}

DISCRETE_RESET(dst_integrate)
{
	DISCRETE_DECLARE_INFO(discrete_integrate_info)

	double  i, v;

	if (info->type & DISC_OP_AMP_IS_NORTON)
	{
		m_v_max_out  = info->vP - OP_AMP_NORTON_VBE;
		m_v_max_in   = info->v1 - OP_AMP_NORTON_VBE;
		m_v_max_in_d = m_v_max_in - OP_AMP_NORTON_VBE;
	}
	else
	{
		m_v_max_out =  info->vP - OP_AMP_VP_RAIL_OFFSET;

		v = info->v1 * info->r3 / (info->r2 + info->r3);    /* vRef */
		v = info->v1 - v;   /* actual charging voltage */
		i = v / info->r1;
		m_change = i / this->sample_rate() / info->c;
	}
	m_v_out = 0;
	set_output(0, m_v_out);
}


/************************************************************************
 *
 * DST_LOGIC_INV - Logic invertor gate implementation
 *
 * input[0]    - Enable
 * input[1]    - input[0] value
 *
 ************************************************************************/
#define DST_LOGIC_INV__IN       DISCRETE_INPUT(0)

DISCRETE_STEP(dst_logic_inv)
{
	set_output(0,  DST_LOGIC_INV__IN ? 0.0 : 1.0);
}

/************************************************************************
 *
 * DST_BITS_DECODE - Decode Bits from input node
 *
 ************************************************************************/
#define DST_BITS_DECODE__IN     DISCRETE_INPUT(0)
#define DST_BITS_DECODE__FROM   DISCRETE_INPUT(1)
#define DST_BITS_DECODE__TO     DISCRETE_INPUT(2)
#define DST_BITS_DECODE__VOUT   DISCRETE_INPUT(3)

DISCRETE_STEP(dst_bits_decode)
{
	int new_val = DST_BITS_DECODE__IN;
	int last_val = m_last_val;
	int last_had_x_time = m_last_had_x_time;

	if (last_val != new_val || last_had_x_time)
	{
		int i, new_bit, last_bit, last_bit_had_x_time, bit_changed;
		double x_time = DST_BITS_DECODE__IN - new_val;
		int from = m_from;
		int count = m_count;
		int decode_x_time = m_decode_x_time;
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
			set_output(i, out);
			if (has_x_time && bit_changed)
				/* set */
				m_last_had_x_time |= 1 << (i + from);
			else
				/* clear */
				m_last_had_x_time &= ~(1 << (i + from));
		}
		m_last_val = new_val;
	}
}

DISCRETE_RESET(dst_bits_decode)
{
	m_from = DST_BITS_DECODE__FROM;
	m_count = DST_BITS_DECODE__TO - m_from + 1;
	if (DST_BITS_DECODE__VOUT == 0)
		m_decode_x_time = 1;
	else
		m_decode_x_time = 0;
	m_last_had_x_time = 0;

	this->step();
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
#define DST_LOGIC_AND__IN0      DISCRETE_INPUT(0)
#define DST_LOGIC_AND__IN1      DISCRETE_INPUT(1)
#define DST_LOGIC_AND__IN2      DISCRETE_INPUT(2)
#define DST_LOGIC_AND__IN3      DISCRETE_INPUT(3)

DISCRETE_STEP(dst_logic_and)
{
	set_output(0,  (DST_LOGIC_AND__IN0 && DST_LOGIC_AND__IN1 && DST_LOGIC_AND__IN2 && DST_LOGIC_AND__IN3)? 1.0 : 0.0);
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
#define DST_LOGIC_NAND__IN0     DISCRETE_INPUT(0)
#define DST_LOGIC_NAND__IN1     DISCRETE_INPUT(1)
#define DST_LOGIC_NAND__IN2     DISCRETE_INPUT(2)
#define DST_LOGIC_NAND__IN3     DISCRETE_INPUT(3)

DISCRETE_STEP(dst_logic_nand)
{
	set_output(0, (DST_LOGIC_NAND__IN0 && DST_LOGIC_NAND__IN1 && DST_LOGIC_NAND__IN2 && DST_LOGIC_NAND__IN3)? 0.0 : 1.0);
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
#define DST_LOGIC_OR__IN0       DISCRETE_INPUT(0)
#define DST_LOGIC_OR__IN1       DISCRETE_INPUT(1)
#define DST_LOGIC_OR__IN2       DISCRETE_INPUT(2)
#define DST_LOGIC_OR__IN3       DISCRETE_INPUT(3)

DISCRETE_STEP(dst_logic_or)
{
	set_output(0,  (DST_LOGIC_OR__IN0 || DST_LOGIC_OR__IN1 || DST_LOGIC_OR__IN2 || DST_LOGIC_OR__IN3) ? 1.0 : 0.0);
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
#define DST_LOGIC_NOR__IN0      DISCRETE_INPUT(0)
#define DST_LOGIC_NOR__IN1      DISCRETE_INPUT(1)
#define DST_LOGIC_NOR__IN2      DISCRETE_INPUT(2)
#define DST_LOGIC_NOR__IN3      DISCRETE_INPUT(3)

DISCRETE_STEP(dst_logic_nor)
{
	set_output(0,  (DST_LOGIC_NOR__IN0 || DST_LOGIC_NOR__IN1 || DST_LOGIC_NOR__IN2 || DST_LOGIC_NOR__IN3) ? 0.0 : 1.0);
}

/************************************************************************
 *
 * DST_LOGIC_XOR - Logic XOR gate implementation
 *
 * input[0]    - input[0] value
 * input[1]    - input[1] value
 *
 ************************************************************************/
#define DST_LOGIC_XOR__IN0      DISCRETE_INPUT(0)
#define DST_LOGIC_XOR__IN1      DISCRETE_INPUT(1)

DISCRETE_STEP(dst_logic_xor)
{
	set_output(0,  ((DST_LOGIC_XOR__IN0 && !DST_LOGIC_XOR__IN1) || (!DST_LOGIC_XOR__IN0 && DST_LOGIC_XOR__IN1)) ? 1.0 : 0.0);
}

/************************************************************************
 *
 * DST_LOGIC_NXOR - Logic NXOR gate implementation
 *
 * input[0]    - input[0] value
 * input[1]    - input[1] value
 *
 ************************************************************************/
#define DST_LOGIC_XNOR__IN0     DISCRETE_INPUT(0)
#define DST_LOGIC_XNOR__IN1     DISCRETE_INPUT(1)

DISCRETE_STEP(dst_logic_nxor)
{
	set_output(0,  ((DST_LOGIC_XNOR__IN0 && !DST_LOGIC_XNOR__IN1) || (!DST_LOGIC_XNOR__IN0 && DST_LOGIC_XNOR__IN1)) ? 0.0 : 1.0);
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
#define DST_LOGIC_DFF__RESET    !DISCRETE_INPUT(0)
#define DST_LOGIC_DFF__SET      !DISCRETE_INPUT(1)
#define DST_LOGIC_DFF__CLOCK     DISCRETE_INPUT(2)
#define DST_LOGIC_DFF__DATA      DISCRETE_INPUT(3)

DISCRETE_STEP(dst_logic_dff)
{
	int clk = (int)DST_LOGIC_DFF__CLOCK;

	if (DST_LOGIC_DFF__RESET)
		set_output(0,  0);
	else if (DST_LOGIC_DFF__SET)
		set_output(0,  1);
	else if (!m_last_clk && clk)    /* low to high */
		set_output(0,  DST_LOGIC_DFF__DATA);
	m_last_clk = clk;
}

DISCRETE_RESET(dst_logic_dff)
{
	m_last_clk = 0;
	set_output(0,  0);
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
#define DST_LOGIC_JKFF__RESET   !DISCRETE_INPUT(0)
#define DST_LOGIC_JKFF__SET     !DISCRETE_INPUT(1)
#define DST_LOGIC_JKFF__CLOCK    DISCRETE_INPUT(2)
#define DST_LOGIC_JKFF__J        DISCRETE_INPUT(3)
#define DST_LOGIC_JKFF__K        DISCRETE_INPUT(4)

DISCRETE_STEP(dst_logic_jkff)
{
	int clk = (int)DST_LOGIC_JKFF__CLOCK;
	int j   = (int)DST_LOGIC_JKFF__J;
	int k   = (int)DST_LOGIC_JKFF__K;

	if (DST_LOGIC_JKFF__RESET)
		m_v_out = 0;
	else if (DST_LOGIC_JKFF__SET)
		m_v_out = 1;
	else if (m_last_clk && !clk)    /* high to low */
	{
		if (!j)
		{
			/* J=0, K=0 - Hold */
			if (k)
				/* J=0, K=1 - Reset */
				m_v_out = 0;
		}
		else
		{
			if (!k)
				/* J=1, K=0 - Set */
				m_v_out = 1;
			else
				/* J=1, K=1 - Toggle */
				m_v_out = !(int)m_v_out;
		}
	}
	m_last_clk = clk;
	set_output(0, m_v_out);
}

DISCRETE_RESET(dst_logic_jkff)
{
	m_last_clk = 0;
	m_v_out = 0;
	set_output(0, m_v_out);
}

/************************************************************************
 *
 * DST_LOGIC_SHIFT - Shift Register implementation
 *
 ************************************************************************/
#define DST_LOGIC_SHIFT__IN         DISCRETE_INPUT(0)
#define DST_LOGIC_SHIFT__RESET      DISCRETE_INPUT(1)
#define DST_LOGIC_SHIFT__CLK        DISCRETE_INPUT(2)
#define DST_LOGIC_SHIFT__SIZE       DISCRETE_INPUT(3)
#define DST_LOGIC_SHIFT__OPTIONS    DISCRETE_INPUT(4)

DISCRETE_STEP(dst_logic_shift)
{
	double  cycles;
	double  ds_clock;
	int     clock = 0, inc = 0;

	int input_bit = (DST_LOGIC_SHIFT__IN != 0) ? 1 : 0;
	ds_clock = DST_LOGIC_SHIFT__CLK;
	if (m_clock_type == DISC_CLK_IS_FREQ)
	{
		/* We need to keep clocking the internal clock even if in reset. */
		cycles = (m_t_left + this->sample_time()) * ds_clock;
		inc    = (int)cycles;
		m_t_left = (cycles - inc) / ds_clock;
	}
	else
	{
		clock  = (int)ds_clock;
	}

	/* If reset enabled then set output to the reset value.  No x_time in reset. */
	if(((DST_LOGIC_SHIFT__RESET == 0) ? 0 : 1) == m_reset_on_high)
	{
		m_shift_data = 0;
		set_output(0,  0);
		return;
	}

	/* increment clock */
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

	if (inc > 0)
	{
		if (m_shift_r)
		{
			m_shift_data >>= 1;
			m_shift_data |= input_bit << ((int)DST_LOGIC_SHIFT__SIZE - 1);
			inc--;
			m_shift_data >>= inc;
		}
		else
		{
			m_shift_data <<= 1;
			m_shift_data |= input_bit;
			inc--;
			m_shift_data <<= inc;
		}
		m_shift_data &= m_bit_mask;
	}

	set_output(0,  m_shift_data);
}

DISCRETE_RESET(dst_logic_shift)
{
	m_bit_mask = (1 << (int)DST_LOGIC_SHIFT__SIZE) - 1;
	m_clock_type = (int)DST_LOGIC_SHIFT__OPTIONS & DISC_CLK_MASK;
	m_reset_on_high = ((int)DST_LOGIC_SHIFT__OPTIONS & DISC_LOGIC_SHIFT__RESET_H) ? 1 : 0;
	m_shift_r = ((int)DST_LOGIC_SHIFT__OPTIONS & DISC_LOGIC_SHIFT__RIGHT)  ? 1 : 0;

	m_t_left  = 0;
	m_last = 0;
	m_shift_data   = 0;
	set_output(0, 0);
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
#define DST_LOOKUP_TABLE__IN        DISCRETE_INPUT(0)
#define DST_LOOKUP_TABLE__SIZE      DISCRETE_INPUT(1)

DISCRETE_STEP(dst_lookup_table)
{
	DISCRETE_DECLARE_INFO(double)

	int addr = DST_LOOKUP_TABLE__IN;

	if (addr < 0 || addr >= DST_LOOKUP_TABLE__SIZE)
		set_output(0,  0);
	else
		set_output(0,  info[addr]);
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
 * brief explanation.
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
#define DST_MIXER__ENABLE       DISCRETE_INPUT(0)
#define DST_MIXER__IN(bit)      DISCRETE_INPUT(bit + 1)

DISCRETE_STEP(dst_mixer)
{
	DISCRETE_DECLARE_INFO(discrete_mixer_desc)

	double  v, vTemp, r_total, rTemp, rTemp2 = 0;
	double  i = 0;      /* total current of inputs */
	int     bit, connected;

	/* put commonly used stuff in local variables for speed */
	int     r_node_bit_flag = m_r_node_bit_flag;
	int     c_bit_flag = m_c_bit_flag;
	int     bit_mask = 1;
	int     has_rF = (info->rF != 0);
	int     type = m_type;
	double  v_ref = info->vRef;
	double  rI = info->rI;

	if (EXPECTED(DST_MIXER__ENABLE))
	{
		r_total = m_r_total;

		if (UNEXPECTED(m_r_node_bit_flag != 0))
		{
			/* loop and do any high pass filtering for connected caps */
			/* but first see if there is an r_node for the current path */
			/* if so, then the exponents need to be re-calculated */
			for (bit = 0; bit < m_size; bit++)
			{
				rTemp     = info->r[bit];
				connected = 1;
				vTemp     = DST_MIXER__IN(bit);

				/* is there a resistor? */
				if (r_node_bit_flag & bit_mask)
				{
					/* a node has the possibility of being disconnected from the circuit. */
					if (*m_r_node[bit] == 0)
						connected = 0;
					else
					{
						/* value currently holds resistance */
						rTemp   += *m_r_node[bit];
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
							if (*m_r_node[bit] != m_r_last[bit])
							{
								m_exponent_rc[bit] =  RC_CHARGE_EXP(rTemp2 * info->c[bit]);
								m_r_last[bit] = *m_r_node[bit];
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
						m_v_cap[bit] += (vTemp - v_ref - m_v_cap[bit]) * m_exponent_rc[bit];
						vTemp -= m_v_cap[bit];
					}
					i += ((type == DISC_MIXER_IS_OP_AMP) ? v_ref - vTemp : vTemp) / rTemp;
				}
			bit_mask = bit_mask << 1;
			}
		}
		else if (UNEXPECTED(c_bit_flag != 0))
		{
			/* no r_nodes, so just do high pass filtering */
			for (bit = 0; bit < m_size; bit++)
			{
				vTemp = DST_MIXER__IN(bit);

				if (c_bit_flag & (1 << bit))
				{
					/* do input high pass filtering if needed. */
					m_v_cap[bit] += (vTemp - v_ref - m_v_cap[bit]) * m_exponent_rc[bit];
					vTemp -= m_v_cap[bit];
				}
				i += ((type == DISC_MIXER_IS_OP_AMP) ? v_ref - vTemp : vTemp) / info->r[bit];
			}
		}
		else
		{
			/* no r_nodes or c_nodes, mixing only */
			if (UNEXPECTED(type == DISC_MIXER_IS_OP_AMP))
			{
				for (bit = 0; bit < m_size; bit++)
					i += ( v_ref - DST_MIXER__IN(bit) ) / info->r[bit];
			}
			else
			{
				for (bit = 0; bit < m_size; bit++)
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
			v = v_ref + (m_gain * (v_ref - v));

		/* Do the low pass filtering for cF */
		if (EXPECTED(info->cF != 0))
		{
			if (UNEXPECTED(r_node_bit_flag != 0))
			{
				/* Re-calculate exponent if resistor nodes are used */
				m_exponent_c_f =  RC_CHARGE_EXP(r_total * info->cF);
			}
			m_v_cap_f += (v - v_ref - m_v_cap_f) * m_exponent_c_f;
			v = m_v_cap_f;
		}

		/* Do the high pass filtering for cAmp */
		if (EXPECTED(info->cAmp != 0))
		{
			m_v_cap_amp += (v - m_v_cap_amp) * m_exponent_c_amp;
			v -= m_v_cap_amp;
		}
		set_output(0,  v * info->gain);
	}
	else
	{
		set_output(0,  0);
	}
}


DISCRETE_RESET(dst_mixer)
{
	DISCRETE_DECLARE_INFO(discrete_mixer_desc)

	int     bit;
	double  rTemp = 0;

	/* link to r_node outputs */
	m_r_node_bit_flag = 0;
	for (bit = 0; bit < 8; bit++)
	{
		m_r_node[bit] = m_device->node_output_ptr(info->r_node[bit]);
		if (m_r_node[bit] != NULL)
		{
			m_r_node_bit_flag |= 1 << bit;
		}

		/* flag any caps */
		if (info->c[bit] != 0)
			m_c_bit_flag |= 1 << bit;
	}

	m_size = this->active_inputs() - 1;

	/*
	 * THERE IS NO ERROR CHECKING!!!!!!!!!
	 * If you pass a bad ladder table
	 * then you deserve a crash.
	 */

	m_type = info->type;
	if ((info->type == DISC_MIXER_IS_OP_AMP) && (info->rI != 0))
		m_type = DISC_MIXER_IS_OP_AMP_WITH_RI;

	/*
	 * Calculate the total of all resistors in parallel.
	 * This is the combined resistance of the voltage sources.
	 * Also calculate the exponents while we are here.
	 */
	m_r_total = 0;
	for(bit = 0; bit < m_size; bit++)
	{
		if ((info->r[bit] != 0) && !info->r_node[bit] )
		{
			m_r_total += 1.0 / info->r[bit];
		}

		m_v_cap[bit]       = 0;
		m_exponent_rc[bit] = 0;
		if ((info->c[bit] != 0)  && !info->r_node[bit])
		{
			switch (m_type)
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
			m_exponent_rc[bit] = RC_CHARGE_EXP(rTemp * info->c[bit]);
		}
	}

	if (info->rF != 0)
	{
		if (m_type == DISC_MIXER_IS_RESISTOR) m_r_total += 1.0 / info->rF;
	}
	if (m_type == DISC_MIXER_IS_OP_AMP_WITH_RI) m_r_total += 1.0 / info->rI;

	m_v_cap_f      = 0;
	m_exponent_c_f = 0;
	if (info->cF != 0)
	{
		/* Setup filter constants */
		m_exponent_c_f = RC_CHARGE_EXP(((info->type == DISC_MIXER_IS_OP_AMP) ? info->rF : (1.0 / m_r_total)) * info->cF);
	}

	m_v_cap_amp      = 0;
	m_exponent_c_amp = 0;
	if (info->cAmp != 0)
	{
		/* Setup filter constants */
		/* We will use 100k ohms as an average final stage impedance. */
		/* Your amp/speaker system will have more effect on incorrect filtering then any value used here. */
		m_exponent_c_amp = RC_CHARGE_EXP(RES_K(100) * info->cAmp);
	}

	if (m_type == DISC_MIXER_IS_OP_AMP_WITH_RI) m_gain = info->rF / info->rI;

	set_output(0,  0);
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
#define DST_MULTIPLEX__ADDR         DISCRETE_INPUT(0)
#define DST_MULTIPLEX__INP(addr)    DISCRETE_INPUT(1 + addr)

DISCRETE_STEP(dst_multiplex)
{
	int addr;

	addr = DST_MULTIPLEX__ADDR; /* FP to INT */
	if ((addr >= 0) && (addr < m_size))
	{
		set_output(0,  DST_MULTIPLEX__INP(addr));
	}
	else
	{
		/* Bad address.  We will leave the output alone. */
		m_device->discrete_log("NODE_%02d - Address = %d. Out of bounds\n", this->index(), addr);
	}
}

DISCRETE_RESET(dst_multiplex)
{
	m_size = this->active_inputs() - 1;

	this->step();
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
#define DST_ONESHOT__RESET  DISCRETE_INPUT(0)
#define DST_ONESHOT__TRIG   DISCRETE_INPUT(1)
#define DST_ONESHOT__AMP    DISCRETE_INPUT(2)
#define DST_ONESHOT__WIDTH  DISCRETE_INPUT(3)
#define DST_ONESHOT__TYPE   (int)DISCRETE_INPUT(4)

DISCRETE_STEP(dst_oneshot)
{
	int trigger = (DST_ONESHOT__TRIG != 0);

	/* If the state is triggered we will need to countdown later */
	int do_count = m_state;

	if (UNEXPECTED(DST_ONESHOT__RESET))
	{
		/* Hold in Reset */
		set_output(0, 0);
		m_state  = 0;
	}
	else
	{
		/* are we at an edge? */
		if (UNEXPECTED(trigger != m_last_trig))
		{
			/* There has been a trigger edge */
			m_last_trig = trigger;

			/* Is it the proper edge trigger */
			if ((m_type & DISC_ONESHOT_REDGE) ? trigger : !trigger)
			{
				if (!m_state)
				{
					/* We have first trigger */
					m_state     = 1;
					set_output(0, (m_type & DISC_OUT_ACTIVE_LOW) ? 0 : DST_ONESHOT__AMP);
					m_countdown = DST_ONESHOT__WIDTH;
				}
				else
				{
					/* See if we retrigger */
					if (m_type & DISC_ONESHOT_RETRIG)
					{
						/* Retrigger */
						m_countdown = DST_ONESHOT__WIDTH;
						do_count = 0;
					}
				}
			}
		}

		if (UNEXPECTED(do_count))
		{
			m_countdown -= this->sample_time();
			if(m_countdown <= 0.0)
			{
				set_output(0, (m_type & DISC_OUT_ACTIVE_LOW) ? DST_ONESHOT__AMP : 0);
				m_countdown = 0;
				m_state     = 0;
			}
		}
	}
}


DISCRETE_RESET(dst_oneshot)
{
	m_countdown = 0;
	m_state     = 0;

	m_last_trig = 0;
	m_type = DST_ONESHOT__TYPE;

	set_output(0,  (m_type & DISC_OUT_ACTIVE_LOW) ? DST_ONESHOT__AMP : 0);
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
#define DST_RAMP__ENABLE    DISCRETE_INPUT(0)
#define DST_RAMP__DIR       DISCRETE_INPUT(1)
#define DST_RAMP__GRAD      DISCRETE_INPUT(2)
#define DST_RAMP__START     DISCRETE_INPUT(3)
#define DST_RAMP__END       DISCRETE_INPUT(4)
#define DST_RAMP__CLAMP     DISCRETE_INPUT(5)

DISCRETE_STEP(dst_ramp)
{
	if(DST_RAMP__ENABLE)
	{
		if (!m_last_en)
		{
			m_last_en = 1;
			m_v_out = DST_RAMP__START;
		}
		if(m_dir ? DST_RAMP__DIR : !DST_RAMP__DIR) m_v_out += m_step;
		else m_v_out -= m_step;
		/* Clamp to min/max */
		if(m_dir ? (m_v_out < DST_RAMP__START)
				: (m_v_out > DST_RAMP__START)) m_v_out = DST_RAMP__START;
		if(m_dir ? (m_v_out > DST_RAMP__END)
				: (m_v_out < DST_RAMP__END)) m_v_out = DST_RAMP__END;
	}
	else
	{
		m_last_en = 0;
		/* Disabled so clamp to output */
		m_v_out = DST_RAMP__CLAMP;
	}

	set_output(0, m_v_out);
}

DISCRETE_RESET(dst_ramp)
{
	m_v_out = DST_RAMP__CLAMP;
	m_step    = DST_RAMP__GRAD / this->sample_rate();
	m_dir     = ((DST_RAMP__END - DST_RAMP__START) == fabs(DST_RAMP__END - DST_RAMP__START));
	m_last_en = 0;
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
#define DST_SAMPHOLD__IN0       DISCRETE_INPUT(0)
#define DST_SAMPHOLD__CLOCK     DISCRETE_INPUT(1)
#define DST_SAMPHOLD__TYPE      DISCRETE_INPUT(2)

DISCRETE_STEP(dst_samphold)
{
	switch(m_clocktype)
	{
		case DISC_SAMPHOLD_REDGE:
			/* Clock the whole time the input is rising */
			if (DST_SAMPHOLD__CLOCK > m_last_input) set_output(0,  DST_SAMPHOLD__IN0);
			break;
		case DISC_SAMPHOLD_FEDGE:
			/* Clock the whole time the input is falling */
			if(DST_SAMPHOLD__CLOCK < m_last_input) set_output(0,  DST_SAMPHOLD__IN0);
			break;
		case DISC_SAMPHOLD_HLATCH:
			/* Output follows input if clock != 0 */
			if( DST_SAMPHOLD__CLOCK) set_output(0,  DST_SAMPHOLD__IN0);
			break;
		case DISC_SAMPHOLD_LLATCH:
			/* Output follows input if clock == 0 */
			if (DST_SAMPHOLD__CLOCK == 0) set_output(0,  DST_SAMPHOLD__IN0);
			break;
		default:
			m_device->discrete_log("dst_samphold_step - Invalid clocktype passed");
			break;
	}
	/* Save the last value */
	m_last_input = DST_SAMPHOLD__CLOCK;
}

DISCRETE_RESET(dst_samphold)
{
	set_output(0, 0);
	m_last_input = -1;
	/* Only stored in here to speed up and save casting in the step function */
	m_clocktype = (int)DST_SAMPHOLD__TYPE;
	this->step();
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
#define DST_SWITCH__ENABLE  DISCRETE_INPUT(0)
#define DST_SWITCH__SWITCH  DISCRETE_INPUT(1)
#define DST_SWITCH__IN0     DISCRETE_INPUT(2)
#define DST_SWITCH__IN1     DISCRETE_INPUT(3)

DISCRETE_STEP(dst_switch)
{
	if(DST_SWITCH__ENABLE)
	{
		set_output(0,  DST_SWITCH__SWITCH ? DST_SWITCH__IN1 : DST_SWITCH__IN0);
	}
	else
	{
		set_output(0,  0);
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
#define DST_ASWITCH__CTRL       DISCRETE_INPUT(0)
#define DST_ASWITCH__IN         DISCRETE_INPUT(1)
#define DST_ASWITCH__THRESHOLD  DISCRETE_INPUT(2)


DISCRETE_STEP(dst_aswitch)
{
	set_output(0,  DST_ASWITCH__CTRL > DST_ASWITCH__THRESHOLD ? DST_ASWITCH__IN : 0);
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
#define MAX_TRANS_STACK 16

struct double_stack {
public:
	double_stack() : p(&stk[0])  { }
	inline void push(double v)
	{
		//Store THEN increment
		assert(p <= &stk[MAX_TRANS_STACK-1]);
		*p++ = v;
	}
	inline double pop(void)
	{
		//decrement THEN read
		assert(p > &stk[0]);
		p--;
		return *p;
	}
private:
	double stk[MAX_TRANS_STACK];
	double *p;
};

DISCRETE_STEP(dst_transform)
{
	double_stack    stack;
	double  top;

	enum token *fPTR = &precomp[0];

	top = HUGE_VAL;

	while(*fPTR != TOK_END)
	{
		switch (*fPTR++)
		{
			case TOK_MULT:      top = stack.pop() * top;                    break;
			case TOK_DIV:       top = stack.pop() / top;                    break;
			case TOK_ADD:       top = stack.pop() + top;                    break;
			case TOK_MINUS:     top = stack.pop() - top;                    break;
			case TOK_0:         stack.push(top); top = I_IN0();             break;
			case TOK_1:         stack.push(top); top = I_IN1();             break;
			case TOK_2:         stack.push(top); top = I_IN2();             break;
			case TOK_3:         stack.push(top); top = I_IN3();             break;
			case TOK_4:         stack.push(top); top = I_IN4();             break;
			case TOK_DUP:       stack.push(top);                            break;
			case TOK_ABS:       top = fabs(top);                            break;  /* absolute value */
			case TOK_NEG:       top = -top;                                 break;  /* * -1 */
			case TOK_NOT:       top = !top;                                 break;  /* Logical NOT of Last Value */
			case TOK_EQUAL:     top = (int)stack.pop() == (int)top;         break;  /* Logical = */
			case TOK_GREATER:   top = (stack.pop() > top);                  break;  /* Logical > */
			case TOK_LESS:      top = (stack.pop() < top);                  break;  /* Logical < */
			case TOK_AND:       top = (int)stack.pop() & (int)top;          break;  /* Bitwise AND */
			case TOK_OR:        top = (int)stack.pop() | (int)top;          break;  /* Bitwise OR */
			case TOK_XOR:       top = (int)stack.pop() ^ (int)top;          break;  /* Bitwise XOR */
			case TOK_END:       break; /* please compiler */
		}
	}
	set_output(0,  top);
}

DISCRETE_RESET(dst_transform)
{
	const char *fPTR = (const char *)this->custom_data();
	enum token *p = &precomp[0];

	while(*fPTR != 0)
	{
		switch (*fPTR++)
		{
			case '*':   *p = TOK_MULT;      break;
			case '/':   *p = TOK_DIV;       break;
			case '+':   *p = TOK_ADD;       break;
			case '-':   *p = TOK_MINUS;     break;
			case '0':   *p = TOK_0;         break;
			case '1':   *p = TOK_1;         break;
			case '2':   *p = TOK_2;         break;
			case '3':   *p = TOK_3;         break;
			case '4':   *p = TOK_4;         break;
			case 'P':   *p = TOK_DUP;       break;
			case 'a':   *p = TOK_ABS;       break; /* absolute value */
			case 'i':   *p = TOK_NEG;       break; /* * -1 */
			case '!':   *p = TOK_NOT;       break; /* Logical NOT of Last Value */
			case '=':   *p = TOK_EQUAL;     break; /* Logical = */
			case '>':   *p = TOK_GREATER;   break; /* Logical > */
			case '<':   *p = TOK_LESS;      break; /* Logical < */
			case '&':   *p = TOK_AND;       break; /* Bitwise AND */
			case '|':   *p = TOK_OR;        break; /* Bitwise OR */
			case '^':   *p = TOK_XOR;       break; /* Bitwise XOR */
			default:
				m_device->discrete_log("dst_transform_step - Invalid function type/variable passed: %s",(const char *)this->custom_data());
				/* that is enough to fatalerror */
				fatalerror("dst_transform_step - Invalid function type/variable passed: %s\n", (const char *)this->custom_data());
				break;
		}
		p++;
	}
	*p = TOK_END;
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
#define DST_OP_AMP__ENABLE  DISCRETE_INPUT(0)
#define DST_OP_AMP__INP0    DISCRETE_INPUT(1)
#define DST_OP_AMP__INP1    DISCRETE_INPUT(2)

DISCRETE_STEP(dst_op_amp)
{
	DISCRETE_DECLARE_INFO(discrete_op_amp_info)

	double i_pos = 0;
	double i_neg = 0;
	double i    = 0;
	double v_out;

	if (DST_OP_AMP__ENABLE)
	{
		switch (info->type)
		{
			case DISC_OP_AMP_IS_NORTON:
				/* work out neg pin current */
				if  (m_has_r1)
				{
					i_neg = (DST_OP_AMP__INP0 - OP_AMP_NORTON_VBE) / info->r1;
					if (i_neg < 0) i_neg = 0;
				}
				i_neg += m_i_fixed;

				/* work out neg pin current */
				i_pos = (DST_OP_AMP__INP1 - OP_AMP_NORTON_VBE) / info->r2;
				if (i_pos < 0) i_pos = 0;

				/* work out current across r4 */
				i = i_pos - i_neg;

				if (m_has_cap)
				{
					if (m_has_r4)
					{
						/* voltage across r4 charging cap */
						i *= info->r4;
						/* exponential charge */
						m_v_cap += (i - m_v_cap) * m_exponent;
					}
					else
						/* linear charge */
						m_v_cap += i / m_exponent;
					v_out = m_v_cap;
				}
				else
					if (m_has_r4)
						v_out = i * info->r4;
					else
						/* output just swings to rail when there is no r4 */
						if (i > 0)
							v_out = m_v_max;
						else
							v_out = 0;

				/* clamp output */
				if (v_out > m_v_max) v_out = m_v_max;
				else if (v_out < info->vN) v_out = info->vN;
				m_v_cap = v_out;

				set_output(0, v_out);
				break;

			default:
				set_output(0, 0);
		}
	}
	else
		set_output(0, 0);
}

DISCRETE_RESET(dst_op_amp)
{
	DISCRETE_DECLARE_INFO(discrete_op_amp_info)

	m_has_r1 = info->r1 > 0;
	m_has_r4 = info->r4 > 0;

	m_v_max = info->vP - OP_AMP_NORTON_VBE;

	m_v_cap = 0;
	if (info->c > 0)
	{
		m_has_cap = 1;
		/* Setup filter constants */
		if (m_has_r4)
		{
			/* exponential charge */
			m_exponent = RC_CHARGE_EXP(info->r4 * info->c);
		}
		else
			/* linear charge */
			m_exponent = this->sample_rate() * info->c;
	}

	if (info->r3 > 0)
		m_i_fixed = (info->vP - OP_AMP_NORTON_VBE) / info->r3;
	else
		m_i_fixed = 0;
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
#define DST_OP_AMP_1SHT__TRIGGER    DISCRETE_INPUT(0)

DISCRETE_STEP(dst_op_amp_1sht)
{
	DISCRETE_DECLARE_INFO(discrete_op_amp_1sht_info)

	double i_pos;
	double i_neg;
	double v;

	/* update trigger circuit */
	i_pos  = (DST_OP_AMP_1SHT__TRIGGER - m_v_cap2) / info->r2;
	i_pos += m_v_out / info->r5;
	m_v_cap2 += (DST_OP_AMP_1SHT__TRIGGER - m_v_cap2) * m_exponent2;

	/* calculate currents and output */
	i_neg = (m_v_cap1 - OP_AMP_NORTON_VBE) / info->r3;
	if (i_neg < 0) i_neg = 0;
	i_neg += m_i_fixed;

	if (i_pos > i_neg) m_v_out = m_v_max;
	else m_v_out = info->vN;

	/* update c1 */
	/* rough value of voltage at anode of diode if discharging */
	v = m_v_out + 0.6;
	if (m_v_cap1 > m_v_out)
	{
		/* discharge */
		if (m_v_cap1 > v)
			/* immediate discharge through diode */
			m_v_cap1 = v;
		else
			/* discharge through r4 */
			m_v_cap1 += (m_v_out - m_v_cap1) * m_exponent1d;
	}
	else
		/* charge */
		m_v_cap1 += ((m_v_out - OP_AMP_NORTON_VBE) * m_r34ratio + OP_AMP_NORTON_VBE - m_v_cap1) * m_exponent1c;

	set_output(0, m_v_out);
}

DISCRETE_RESET(dst_op_amp_1sht)
{
	DISCRETE_DECLARE_INFO(discrete_op_amp_1sht_info)

	m_exponent1c = RC_CHARGE_EXP(RES_2_PARALLEL(info->r3, info->r4) * info->c1);
	m_exponent1d = RC_CHARGE_EXP(info->r4 * info->c1);
	m_exponent2  = RC_CHARGE_EXP(info->r2 * info->c2);
	m_i_fixed  = (info->vP - OP_AMP_NORTON_VBE) / info->r1;
	m_v_cap1   = m_v_cap2 = 0;
	m_v_max    = info->vP - OP_AMP_NORTON_VBE;
	m_r34ratio = info->r3 / (info->r3 + info->r4);
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
#define DST_TVCA_OP_AMP__TRG0   DISCRETE_INPUT(0)
#define DST_TVCA_OP_AMP__TRG1   DISCRETE_INPUT(1)
#define DST_TVCA_OP_AMP__TRG2   DISCRETE_INPUT(2)
#define DST_TVCA_OP_AMP__INP0   DISCRETE_INPUT(3)
#define DST_TVCA_OP_AMP__INP1   DISCRETE_INPUT(4)

DISCRETE_STEP(dst_tvca_op_amp)
{
	DISCRETE_DECLARE_INFO(discrete_op_amp_tvca_info)

	int     trig0, trig1, trig2, f3;
	double  i2 = 0;     /* current through r2 */
	double  i3 = 0;     /* current through r3 */
	double  i_neg = 0;  /* current into - input */
	double  i_pos = 0;  /* current into + input */
	double  i_out = 0;  /* current at output */

	double  v_out;

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
	i_neg = m_i_fixed + i2 + i3;

	/* Update the c1 cap voltage. */
	if (dst_trigger_function(trig0, trig1, trig2, info->f2))
	{
		/* F2 is not grounding the circuit so we charge the cap. */
		m_v_cap1 += (m_v_trig[f3] - m_v_cap1) * m_exponent_c[f3];
	}
	else
	{
		/* F2 is at ground.  The diode blocks this so F2 and r5 are out of circuit.
		 * So now the discharge rate is dependent upon F3.
		 * If F3 is at ground then we discharge to 0V through r6.
		 * If F3 is out of circuit then we discharge to OP_AMP_NORTON_VBE through r6+r7. */
		m_v_cap1 += ((f3 ? OP_AMP_NORTON_VBE : 0.0) - m_v_cap1) * m_exponent_d[f3];
	}

	/* Calculate c1 current going in to + input. */
	i_pos = (m_v_cap1 - OP_AMP_NORTON_VBE) / m_r67;
	if ((i_pos < 0) || !f3) i_pos = 0;

	/* Update the c2 cap voltage and current. */
	if (info->r9 != 0)
	{
		f3 = dst_trigger_function(trig0, trig1, trig2, info->f4);
		m_v_cap2 += ((f3 ? m_v_trig2 : 0) - m_v_cap2) * m_exponent2[f3];
		i_pos += m_v_cap2 / info->r9;
	}

	/* Update the c3 cap voltage and current. */
	if (info->r11 != 0)
	{
		f3 = dst_trigger_function(trig0, trig1, trig2, info->f5);
		m_v_cap3 += ((f3 ? m_v_trig3 : 0) - m_v_cap3) * m_exponent3[f3];
		i_pos += m_v_cap3 / info->r11;
	}

	/* Calculate output current. */
	i_out = i_pos - i_neg;
	if (i_out < 0) i_out = 0;

	/* Convert to voltage for final output. */
	if (m_has_c4)
	{
		if (m_has_r4)
		{
			/* voltage across r4 charging cap */
			i_out *= info->r4;
			/* exponential charge */
			m_v_cap4 += (i_out - m_v_cap4) * m_exponent4;
		}
		else
		/* linear charge */
			m_v_cap4 += i_out / m_exponent4;
		if (m_v_cap4 < 0)
			m_v_cap4 = 0;
		v_out = m_v_cap4;
	}
	else
		v_out = i_out * info->r4;



	/* Clip the output if needed. */
	if (v_out > m_v_out_max) v_out = m_v_out_max;

	set_output(0, v_out);
}

DISCRETE_RESET(dst_tvca_op_amp)
{
	DISCRETE_DECLARE_INFO(discrete_op_amp_tvca_info)

	m_r67 = info->r6 + info->r7;

	m_v_out_max = info->vP - OP_AMP_NORTON_VBE;
	/* This is probably overkill because R5 is usually much lower then r6 or r7,
	 * but it is better to play it safe. */
	m_v_trig[0] = (info->v1 - 0.6) * RES_VOLTAGE_DIVIDER(info->r5, info->r6);
	m_v_trig[1] = (info->v1 - 0.6 - OP_AMP_NORTON_VBE) * RES_VOLTAGE_DIVIDER(info->r5, m_r67) + OP_AMP_NORTON_VBE;
	m_i_fixed   = m_v_out_max / info->r1;

	m_v_cap1 = 0;
	/* Charge rate through r5 */
	/* There can be a different charge rates depending on function F3. */
	m_exponent_c[0] = RC_CHARGE_EXP(RES_2_PARALLEL(info->r5, info->r6) * info->c1);
	m_exponent_c[1] = RC_CHARGE_EXP(RES_2_PARALLEL(info->r5, m_r67) * info->c1);
	/* Discharge rate through r6 + r7 */
	m_exponent_d[1] = RC_CHARGE_EXP(m_r67 * info->c1);
	/* Discharge rate through r6 */
	if (info->r6 != 0)
	{
		m_exponent_d[0] = RC_CHARGE_EXP(info->r6 * info->c1);
	}
	m_v_cap2       = 0;
	m_v_trig2      = (info->v2 - 0.6 - OP_AMP_NORTON_VBE) * RES_VOLTAGE_DIVIDER(info->r8, info->r9);
	m_exponent2[0] = RC_CHARGE_EXP(info->r9 * info->c2);
	m_exponent2[1] = RC_CHARGE_EXP(RES_2_PARALLEL(info->r8, info->r9) * info->c2);
	m_v_cap3       = 0;
	m_v_trig3      = (info->v3 - 0.6 - OP_AMP_NORTON_VBE) * RES_VOLTAGE_DIVIDER(info->r10, info->r11);
	m_exponent3[0] = RC_CHARGE_EXP(info->r11 * info->c3);
	m_exponent3[1] = RC_CHARGE_EXP(RES_2_PARALLEL(info->r10, info->r11) * info->c3);
	m_v_cap4       = 0;
	if (info->r4 != 0) m_has_r4 = 1;
	if (info->c4 != 0) m_has_c4 = 1;
	if (m_has_r4 && m_has_c4)
		m_exponent4    = RC_CHARGE_EXP(info->r4 * info->c4);

	this->step();
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
#define DST_XTIME_BUFFER__IN            DISCRETE_INPUT(0)
#define DST_XTIME_BUFFER_OUT_LOW        DISCRETE_INPUT(1)
#define DST_XTIME_BUFFER_OUT_HIGH       DISCRETE_INPUT(2)
#define DST_XTIME_BUFFER_INVERT         DISCRETE_INPUT(3)

DISCRETE_STEP(dst_xtime_buffer)
{
	int in0 = (int)DST_XTIME_BUFFER__IN;
	int out = in0;
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
			set_output(0,  out_low + diff);
		}
		else
			set_output(0,  out ? out_high : out_low);
	}
	else
		set_output(0,  out + x_time);
}


/************************************************************************
 *
 * DST_XTIME_AND - AND/NAND gate implementation using X_TIME
 *
 * If OUT_LOW and OUT_HIGH are defined then the output will be energy.
 * If they are both 0, then the output will be X_TIME logic.
 *
 ************************************************************************/
#define DST_XTIME_AND__IN0          DISCRETE_INPUT(0)
#define DST_XTIME_AND__IN1          DISCRETE_INPUT(1)
#define DST_XTIME_AND_OUT_LOW       DISCRETE_INPUT(2)
#define DST_XTIME_AND_OUT_HIGH      DISCRETE_INPUT(3)
#define DST_XTIME_AND_INVERT        DISCRETE_INPUT(4)

DISCRETE_STEP(dst_xtime_and)
{
	int in0 = (int)DST_XTIME_AND__IN0;
	int in1 = (int)DST_XTIME_AND__IN1;
	int out = 0;
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
		//  break;

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
			//  x_time = time1 - x_time0;
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
			//  x_time = x_time0 - x_time1;
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
			set_output(0,  out_low + diff);
		}
		else
			set_output(0,  out ? out_high : out_low);
	}
	else
		set_output(0,  out + x_time);
}


/************************************************************************
 *
 * DST_XTIME_OR - OR/NOR gate implementation using X_TIME
 *
 * If OUT_LOW and OUT_HIGH are defined then the output will be energy.
 * If they are both 0, then the output will be X_TIME logic.
 *
 ************************************************************************/
#define DST_XTIME_OR__IN0           DISCRETE_INPUT(0)
#define DST_XTIME_OR__IN1           DISCRETE_INPUT(1)
#define DST_XTIME_OR_OUT_LOW        DISCRETE_INPUT(2)
#define DST_XTIME_OR_OUT_HIGH       DISCRETE_INPUT(3)
#define DST_XTIME_OR_INVERT         DISCRETE_INPUT(4)

DISCRETE_STEP(dst_xtime_or)
{
	int in0 = (int)DST_XTIME_OR__IN0;
	int in1 = (int)DST_XTIME_OR__IN1;
	int out = 1;
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
		//  break;

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
			set_output(0,  out_low + diff);
		}
		else
			set_output(0,  out ? out_high : out_low);
	}
	else
		set_output(0,  out + x_time);
}


/************************************************************************
 *
 * DST_XTIME_XOR - XOR/XNOR gate implementation using X_TIME
 *
 * If OUT_LOW and OUT_HIGH are defined then the output will be energy.
 * If they are both 0, then the output will be X_TIME logic.
 *
 ************************************************************************/
#define DST_XTIME_XOR__IN0          DISCRETE_INPUT(0)
#define DST_XTIME_XOR__IN1          DISCRETE_INPUT(1)
#define DST_XTIME_XOR_OUT_LOW       DISCRETE_INPUT(2)
#define DST_XTIME_XOR_OUT_HIGH      DISCRETE_INPUT(3)
#define DST_XTIME_XOR_INVERT        DISCRETE_INPUT(4)

DISCRETE_STEP(dst_xtime_xor)
{
	int in0 = (int)DST_XTIME_XOR__IN0;
	int in1 = (int)DST_XTIME_XOR__IN1;
	int out = 1;
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
		//  break;

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
			set_output(0,  out_low + diff);
		}
		else
			set_output(0,  out ? out_high : out_low);
	}
	else
		set_output(0,  out + x_time);
}
