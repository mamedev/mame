// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74192.c
 *
 */

#include "nld_74192.h"
#include "netlist/nl_base.h"
#include "nlid_system.h"

namespace netlist
{
	namespace devices
	{

	static constexpr const unsigned MAXCNT = 9;

	NETLIB_OBJECT(74192)
	{
		NETLIB_CONSTRUCTOR(74192)
		, m_CLEAR(*this, "CLEAR")
		, m_LOADQ(*this, "LOADQ")
		, m_CU(*this, "CU")
		, m_CD(*this, "CD")
		, m_cnt(*this, "m_cnt", 0)
		, m_last_CU(*this, "m_last_CU", 0)
		, m_last_CD(*this, "m_last_CD", 0)
		, m_Q(*this, {{"QA", "QB", "QC", "QD"}})
		, m_BORROWQ(*this, "BORROWQ")
		, m_CARRYQ(*this, "CARRYQ")
		, m_A(*this, "A", NETLIB_DELEGATE(74192, sub))
		, m_B(*this, "B", NETLIB_DELEGATE(74192, sub))
		, m_C(*this, "C", NETLIB_DELEGATE(74192, sub))
		, m_D(*this, "D", NETLIB_DELEGATE(74192, sub))
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		logic_input_t m_CLEAR;
		logic_input_t m_LOADQ;
		logic_input_t m_CU;
		logic_input_t m_CD;

		state_var<unsigned> m_cnt;
		state_var<unsigned> m_last_CU;
		state_var<unsigned> m_last_CD;

		object_array_t<logic_output_t, 4> m_Q;
		logic_output_t m_BORROWQ;
		logic_output_t m_CARRYQ;

		logic_input_t m_A;
		logic_input_t m_B;
		logic_input_t m_C;
		logic_input_t m_D;
		nld_power_pins m_power_pins;

		NETLIB_HANDLERI(sub)
		{
			// FIXME: can use more optimization
		}

		unsigned read_ABCD() const
		{
			return (m_D() << 3) | (m_C() << 2) | (m_B() << 1) | (m_A() << 0);
		}

	};

	NETLIB_OBJECT_DERIVED(74192_dip, 74192)
	{
		NETLIB_CONSTRUCTOR_DERIVED(74192_dip, 74192)
		{
			register_subalias("1", m_B);
			register_subalias("2", m_Q[1]);
			register_subalias("3", m_Q[0]);
			register_subalias("4", m_CD);
			register_subalias("5", m_CU);
			register_subalias("6", m_Q[2]);
			register_subalias("7", m_Q[3]);
			register_subalias("8", "GND");

			register_subalias("9", m_D);
			register_subalias("10", m_C);
			register_subalias("11", m_LOADQ);
			register_subalias("12", m_CARRYQ);
			register_subalias("13", m_BORROWQ);
			register_subalias("14", m_CLEAR);
			register_subalias("15", m_A);
			register_subalias("16", "VCC");
		}
	};

	NETLIB_RESET(74192)
	{
		m_cnt = 0;
		m_last_CU = 0;
		m_last_CD = 0;
	}

	// FIXME: Timing
	static constexpr const std::array<netlist_time, 4> delay =
	{
			NLTIME_FROM_NS(40),
			NLTIME_FROM_NS(40),
			NLTIME_FROM_NS(40),
			NLTIME_FROM_NS(40)
	};

	NETLIB_UPDATE(74192)
	{
		netlist_sig_t tCarry = 1;
		netlist_sig_t tBorrow = 1;
		if (m_CLEAR())
		{
			m_cnt = 0;
		}
		else if (!m_LOADQ())
		{
			m_cnt = read_ABCD();
		}
		else
		{
			if (m_CD() && !m_last_CU && m_CU())
			{
				if (++m_cnt > MAXCNT)
					m_cnt = 0;
			}
			if (m_CU() && !m_last_CD && m_CD())
			{
				if (m_cnt > 0)
					--m_cnt;
				else
					m_cnt = MAXCNT;
			}
		}

		if (!m_CU() && (m_cnt == MAXCNT))
			tCarry = 0;

		if (!m_CD() && (m_cnt == 0))
			tBorrow = 0;

		m_last_CD = m_CD();
		m_last_CU = m_CU();

		for (std::size_t i=0; i<4; i++)
			m_Q[i].push((m_cnt >> i) & 1, delay[i]);

		m_BORROWQ.push(tBorrow, NLTIME_FROM_NS(20)); //FIXME
		m_CARRYQ.push(tCarry, NLTIME_FROM_NS(20)); //FIXME
	}

	NETLIB_DEVICE_IMPL(74192,    "TTL_74192", "+A,+B,+C,+D,+CLEAR,+LOADQ,+CU,+CD,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74192_dip,"TTL_74192_DIP", "")

	} //namespace devices
} // namespace netlist
