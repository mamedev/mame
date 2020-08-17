// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74393.c
 *
 */

#include "nld_74393.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{
	static constexpr const unsigned MAXCNT = 15;

	NETLIB_OBJECT(74393)
	{
		NETLIB_CONSTRUCTOR(74393)
		, m_CP(*this, "CP", NETLIB_DELEGATE(cp))
		, m_MR(*this, "MR", NETLIB_DELEGATE(mr))
		, m_Q(*this, {"Q0", "Q1", "Q2", "Q3"})
		, m_cnt(*this, "m_cnt", 0)
		, m_cp(*this, "m_cp", 0)
		, m_mr(*this, "m_mr", 0)
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_CP.set_state(logic_t::STATE_INP_HL);
			m_cnt = 0;
		}

		NETLIB_HANDLERI(mr)
		{
			if (!m_MR())
			{
				m_CP.activate_hl();
			}
			else
			{
				m_CP.inactivate();
				if (m_cnt != 0)
				{
					m_cnt = 0;
					m_Q.push(0, NLTIME_FROM_NS(24));
				}
			}
		}

		NETLIB_HANDLERI(cp)
		{
			auto cnt = (m_cnt + 1) & MAXCNT;
			m_cnt = cnt;
			m_Q[0].push((cnt >> 0) & 1, NLTIME_FROM_NS(13));
			m_Q[1].push((cnt >> 1) & 1, NLTIME_FROM_NS(22));
			m_Q[2].push((cnt >> 2) & 1, NLTIME_FROM_NS(31));
			m_Q[3].push((cnt >> 3) & 1, NLTIME_FROM_NS(40));
		}

		logic_input_t m_CP;
		logic_input_t m_MR;
		object_array_t<logic_output_t, 4> m_Q;

		state_var<unsigned> m_cnt;
		state_var_sig m_cp;
		state_var_sig m_mr;

		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT(74393_dip)
	{
		NETLIB_CONSTRUCTOR(74393_dip)
		, m_A(*this, "A")
		, m_B(*this, "B")
		{
			register_subalias("1", "A.CP");
			register_subalias("2", "A.MR");
			register_subalias("3", "A.Q0");
			register_subalias("4", "A.Q1");
			register_subalias("5", "A.Q2");
			register_subalias("6", "A.Q3");
			register_subalias("7", "A.GND");

			register_subalias("8", "B.Q3");
			register_subalias("9", "B.Q2");
			register_subalias("10", "B.Q1");
			register_subalias("11", "B.Q0");
			register_subalias("12", "B.MR");
			register_subalias("13", "B.CP");
			register_subalias("14", "A.VCC");

			connect("A.GND", "B.GND");
			connect("A.VCC", "B.VCC");
		}
		//NETLIB_RESETI() {}

	private:
		NETLIB_SUB(74393) m_A;
		NETLIB_SUB(74393) m_B;
	};

	NETLIB_DEVICE_IMPL(74393,     "TTL_74393", "+CP,+MR,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74393_dip, "TTL_74393_DIP", "")

	} //namespace devices
} // namespace netlist
