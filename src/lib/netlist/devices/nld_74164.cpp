// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*
 * nld_74164.cpp
 *
 * Thanks to the 74161 work of Ryan and the huge Netlist effort by Couriersud
 * implementing this was simple.
 *
 */

#include "nld_74164.h"
#include "netlist/nl_base.h"

// FIXME: clk input to be separated - only falling edge relevant

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(74164)
	{
		NETLIB_CONSTRUCTOR(74164)
		, m_AB(*this, {"A", "B"}, NETLIB_DELEGATE(ab))
		, m_CLRQ(*this, "CLRQ", NETLIB_DELEGATE(clrq))
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_cnt(*this, "m_cnt", 0)
		, m_ab(*this, "m_ab", 0)
		, m_Q(*this, {"QA", "QB", "QC", "QD", "QE", "QF", "QG", "QH"})
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_CLK.set_state(logic_t::STATE_INP_LH);
			m_cnt = 0;
		}

		NETLIB_HANDLERI(clrq)
		{
			if (m_CLRQ())
			{
				m_CLK.activate_lh();
			}
			else
			{
				m_CLK.inactivate();
				if (m_cnt != 0)
				{
					m_cnt = 0;
					m_Q.push(0, NLTIME_FROM_NS(30));
				}
			}
		}

		NETLIB_HANDLERI(clk)
		{
			m_cnt = (m_cnt << 1) | m_ab;
			m_Q.push(m_cnt, NLTIME_FROM_NS(30));
		}

		NETLIB_HANDLERI(ab)
		{
			m_ab = static_cast<unsigned>((m_AB() == 3) ? 1 : 0);
		}

		friend class NETLIB_NAME(74164_dip);
	private:
		object_array_t<logic_input_t, 2> m_AB;
		logic_input_t m_CLRQ;
		logic_input_t m_CLK;

		state_var<unsigned> m_cnt;
		state_var<unsigned> m_ab;

		object_array_t<logic_output_t, 8> m_Q;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(74164, "TTL_74164", "+A,+B,+CLRQ,+CLK,@VCC,@GND")

	} //namespace devices
} // namespace netlist
