// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_74174.cpp
 *
 *  DM74174: Hex D Flip-Flops with Clear
 *
 *          +--------------+
 *      CLR |1     ++    16| VCC
 *       Q1 |2           15| Q6
 *       D1 |3           14| D6
 *       D2 |4   74174   13| D5
 *       Q2 |5           12| Q5
 *       D3 |6           11| D4
 *       Q3 |7           10| Q4
 *      GND |8            9| CLK
 *          +--------------+
 *
 *          +-----+-----+---++---+-----+
 *          | CLR | CLK | D || Q | QQ  |
 *          +=====+=====+===++===+=====+
 *          |  0  |  X  | X || 0 |  1  |
 *          |  1  |  R  | 1 || 1 |  0  |
 *          |  1  |  R  | 0 || 0 |  1  |
 *          |  1  |  0  | X || Q0| Q0Q |
 *          +-----+-----+---++---+-----+
 *
 *   Q0 The output logic level of Q before the indicated input conditions were established
 *
 *  R:  0 -> 1
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#include "nl_base.h"

namespace netlist::devices {

	NETLIB_OBJECT(74174)
	{
		NETLIB_CONSTRUCTOR(74174)
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_Q(*this, 1, "Q{}")
		, m_clrq(*this, "m_clr", 0)
		, m_data(*this, "m_data", 0)
		, m_D(*this, 1, "D{}", NETLIB_DELEGATE(other))
		, m_CLRQ(*this, "CLRQ", NETLIB_DELEGATE(clrq))
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_CLK.set_state(logic_t::STATE_INP_LH);
			m_clrq = 0;
			m_data = 0xFF;
		}

	private:
		NETLIB_HANDLERI(clrq)
		{
			m_clrq = m_CLRQ();
			if (!m_clrq)
			{
				m_data = 0;
				m_Q.push(m_data, NLTIME_FROM_NS(40));
				m_CLK.inactivate();
			}
			else
				m_CLK.activate_lh();
		}

		NETLIB_HANDLERI(other)
		{
			m_data = m_D();
			if (m_clrq)
				m_CLK.activate_lh();
		}

		NETLIB_HANDLERI(clk)
		{
			if (m_clrq)
			{
				m_Q.push(m_data, NLTIME_FROM_NS(25));
				m_CLK.inactivate();
			}
		}

		logic_input_t m_CLK;
		object_array_t<logic_output_t, 6> m_Q;

		state_var<netlist_sig_t> m_clrq;
		state_var<netlist_sig_t> m_data;

		object_array_t<logic_input_t, 6> m_D;
		logic_input_t m_CLRQ;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(74174,      "TTL_74174", "+CLK,+D1,+D2,+D3,+D4,+D5,+D6,+CLRQ,@VCC,@GND")

} // namespace netlist::devices
