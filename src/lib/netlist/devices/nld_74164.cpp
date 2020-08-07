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

	NETLIB_OBJECT(74164_dip)
	{
		NETLIB_CONSTRUCTOR(74164_dip)
		, A(*this, "A")
		{
			register_subalias("1", A.m_AB[0]);
			register_subalias("2", A.m_AB[1]);
			register_subalias("3", A.m_Q[0]);
			register_subalias("4", A.m_Q[1]);
			register_subalias("5", A.m_Q[2]);
			register_subalias("6", A.m_Q[3]);
			register_subalias("7", "A.GND");

			register_subalias("8", A.m_CLK);
			register_subalias("9", A.m_CLRQ);
			register_subalias("10", A.m_Q[4]);
			register_subalias("11", A.m_Q[5]);
			register_subalias("12", A.m_Q[6]);
			register_subalias("13", A.m_Q[7]);
			register_subalias("14", "A.VCC");
		}
	private:
		NETLIB_SUB(74164) A;
	};


	NETLIB_DEVICE_IMPL(74164, "TTL_74164", "+A,+B,+CLRQ,+CLK,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74164_dip, "TTL_74164_DIP", "")

	} //namespace devices
} // namespace netlist
