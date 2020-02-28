// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_74161.cpp
 *
 */

#include "nld_74161.h"
#include "netlist/nl_base.h"
#include "nlid_system.h"

#include <array>

namespace netlist
{
	namespace devices
	{

	static constexpr const unsigned MAXCNT = 15;

	NETLIB_OBJECT(74161)
	{
		NETLIB_CONSTRUCTOR(74161)
		, m_A(*this, "A")
		, m_B(*this, "B")
		, m_C(*this, "C")
		, m_D(*this, "D")
		, m_CLRQ(*this, "CLRQ")
		, m_LOADQ(*this, "LOADQ")
		, m_CLK(*this, "CLK")
		, m_ENABLEP(*this, "ENABLEP")
		, m_ENABLET(*this, "ENABLET")
		, m_cnt(*this, "m_cnt", 0)
		, m_last_CLK(*this, "m_last_CLK", 0)
		, m_Q(*this, {{"QA", "QB", "QC", "QD"}})
		, m_RCO(*this, "RCO")
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		logic_input_t m_A;
		logic_input_t m_B;
		logic_input_t m_C;
		logic_input_t m_D;
		logic_input_t m_CLRQ;
		logic_input_t m_LOADQ;
		logic_input_t m_CLK;
		logic_input_t m_ENABLEP;
		logic_input_t m_ENABLET;

		state_var<unsigned> m_cnt;
		state_var<unsigned> m_last_CLK;

		object_array_t<logic_output_t, 4> m_Q;
		logic_output_t m_RCO;

		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT_DERIVED(74161_dip, 74161)
	{
		NETLIB_CONSTRUCTOR_DERIVED(74161_dip, 74161)
		{
			register_subalias("1", m_CLRQ);
			register_subalias("2", m_CLK);
			register_subalias("3", m_A);
			register_subalias("4", m_B);
			register_subalias("5", m_C);
			register_subalias("6", m_D);
			register_subalias("7", m_ENABLEP);
			register_subalias("8", "GND");

			register_subalias("9", m_LOADQ);
			register_subalias("10", m_ENABLET);
			register_subalias("11", m_Q[3]);
			register_subalias("12", m_Q[2]);
			register_subalias("13", m_Q[1]);
			register_subalias("14", m_Q[0]);
			register_subalias("15", m_RCO);
			register_subalias("16", "VCC");

		}
	};

	NETLIB_RESET(74161)
	{
		m_cnt = 0;
		m_last_CLK = 0;
	}

	// FIXME: Timing
	static constexpr const std::array<netlist_time, 4> delay =
	{
			NLTIME_FROM_NS(40),
			NLTIME_FROM_NS(40),
			NLTIME_FROM_NS(40),
			NLTIME_FROM_NS(40)
	};

	NETLIB_UPDATE(74161)
	{
		netlist_sig_t tRippleCarryOut = 0;
		if (!m_CLRQ())
		{
			m_cnt = 0;
		}
		else if (m_CLK() && !m_last_CLK)
		{
			if (!m_LOADQ())
			{
				m_cnt = (m_D() << 3) | (m_C() << 2)
						| (m_B() << 1) | (m_A() << 0);
			}
			else if (m_ENABLET() && m_ENABLEP())
			{
				++m_cnt &= MAXCNT;
			}
		}

		if (m_ENABLET() && (m_cnt == MAXCNT))
		{
			tRippleCarryOut = 1;
		}

		m_last_CLK = m_CLK();

		for (std::size_t i=0; i<4; i++)
			m_Q[i].push((m_cnt >> i) & 1, delay[i]);

		m_RCO.push(tRippleCarryOut, NLTIME_FROM_NS(20)); //FIXME
	}

	NETLIB_DEVICE_IMPL(74161, "TTL_74161", "+A,+B,+C,+D,+CLRQ,+LOADQ,+CLK,+ENABLEP,+ENABLET,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74161_dip, "TTL_74161_DIP", "")

	} //namespace devices
} // namespace netlist
