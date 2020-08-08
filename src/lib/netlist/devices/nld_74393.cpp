// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74393.cpp
 *
 *  DM74393: Dual 4-Stage Binary Counter
 *
 *          +--------------+
 *      /CP |1     ++    14| VCC
 *       MR |2           13| /CP
 *       Q0 |3           12| MR
 *       Q1 |4    74393  11| Q0
 *       Q2 |5           10| Q1
 *       Q3 |6            9| Q2
 *      GND |7            8| Q3
 *          +--------------+
 *
 *  Naming conventions follow Motorola datasheet
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

	NETLIB_DEVICE_IMPL(74393,     "TTL_74393", "+CP,+MR,@VCC,@GND")

	} //namespace devices
} // namespace netlist
