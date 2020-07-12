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
		, m_CP(*this, "CP", NETLIB_DELEGATE(inputs))
		, m_MR(*this, "MR", NETLIB_DELEGATE(inputs))
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
			m_cnt = 0;
		}

		NETLIB_UPDATEI()
		{
			inputs();
		}

		NETLIB_HANDLERI(inputs)
		{
			netlist_sig_t last_cp = m_cp;
			netlist_sig_t last_mr = m_mr;

			m_cp = m_CP();
			m_mr = m_MR();

			if (!last_mr && m_mr)
			{
				m_cnt = 0;
				update_outputs_all(0, NLTIME_FROM_NS(24), NLTIME_FROM_NS(24), NLTIME_FROM_NS(24), NLTIME_FROM_NS(24));
			}
			else if (!m_mr && last_cp && !m_cp)
			{
				auto cnt = (m_cnt + 1) & MAXCNT;
				update_outputs_all(cnt, NLTIME_FROM_NS(13), NLTIME_FROM_NS(22), NLTIME_FROM_NS(31), NLTIME_FROM_NS(40));
				m_cnt = cnt;
			}
		}

		logic_input_t m_CP;
		logic_input_t m_MR;
		object_array_t<logic_output_t, 4> m_Q;

		state_var<unsigned> m_cnt;
		state_var_sig m_cp;
		state_var_sig m_mr;

		nld_power_pins m_power_pins;

		void update_outputs_all(unsigned cnt, netlist_time q0_delay, netlist_time q1_delay, netlist_time q2_delay, netlist_time q3_delay) noexcept
		{
			m_Q[0].push((cnt >> 0) & 1, q0_delay);
			m_Q[1].push((cnt >> 1) & 1, q1_delay);
			m_Q[2].push((cnt >> 2) & 1, q2_delay);
			m_Q[3].push((cnt >> 3) & 1, q3_delay);
		}
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
		NETLIB_UPDATEI() { }
		NETLIB_RESETI() { }

	private:
		NETLIB_SUB(74393) m_A;
		NETLIB_SUB(74393) m_B;
	};

	NETLIB_DEVICE_IMPL(74393,     "TTL_74393", "+CP,+MR,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74393_dip, "TTL_74393_DIP", "")

	} //namespace devices
} // namespace netlist
