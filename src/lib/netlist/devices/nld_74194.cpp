// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_74194.cpp
 *
 */

#include "nld_74194.h"
#include "netlist/nl_base.h"
#include "nlid_system.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(74194)
	{
		NETLIB_CONSTRUCTOR(74194)
		, m_DATA(*this, {{"D", "C", "B", "A"}})
		, m_SLIN(*this, "SLIN")
		, m_SRIN(*this, "SRIN")
		, m_CLK(*this, "CLK")
		, m_S0(*this, "S0")
		, m_S1(*this, "S1")
		, m_CLRQ(*this, "CLRQ")
		, m_Q(*this, {{"QD", "QC", "QB", "QA"}})
		, m_last_CLK(*this, "m_last_CLK", 0)
		, m_last_Q(*this, "m_last_Q", 0)
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		object_array_t<logic_input_t, 4> m_DATA;
		logic_input_t m_SLIN;
		logic_input_t m_SRIN;
		logic_input_t m_CLK;
		logic_input_t m_S0;
		logic_input_t m_S1;
		logic_input_t m_CLRQ;
		object_array_t<logic_output_t, 4> m_Q;

		state_var<unsigned> m_last_CLK;
		state_var<unsigned> m_last_Q;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT_DERIVED(74194_dip, 74194)
	{
		NETLIB_CONSTRUCTOR_DERIVED(74194_dip, 74194)
		{
			register_subalias("1", m_CLRQ);
			register_subalias("2", m_SRIN);
			register_subalias("3", m_DATA[3]);
			register_subalias("4", m_DATA[2]);
			register_subalias("5", m_DATA[1]);
			register_subalias("6", m_DATA[0]);
			register_subalias("7", m_SLIN);
			register_subalias("8", "GND");

			register_subalias("9",  m_S0);
			register_subalias("10", m_S1);
			register_subalias("11", m_CLK);
			register_subalias("12", m_Q[0]);
			register_subalias("13", m_Q[1]);
			register_subalias("14", m_Q[2]);
			register_subalias("15", m_Q[3]);
			register_subalias("16", "VCC");

		}
	};

	NETLIB_RESET(74194)
	{
		m_last_CLK = 0;
		m_last_Q = 0;
	}

	// FIXME: Timing
	NETLIB_UPDATE(74194)
	{
		unsigned q = m_last_Q;

		if (!m_CLRQ())
		{
			q = 0;
		}
		else
		{
			if (!m_last_CLK && m_CLK())
			{
				unsigned s = (m_S1() << 1) | m_S0();
				switch (s)
				{
					case 0: // LL: Keep old value
						break;
					case 1: // LH: Shift right
						q >>= 1;
						q |= m_SRIN() ? 8 : 0;
						break;
					case 2:
						q <<= 1;
						q |= m_SLIN() ? 1 : 0;
						break;
					case 3:
						q = 0;
						for (std::size_t i=0; i<4; i++)
							q |= m_DATA[i]() << i;
						break;
				}
			}
		}

		m_last_Q = q;
		m_last_CLK = m_CLK();

		for (std::size_t i=0; i<4; i++)
			m_Q[i].push((q >> i) & 1, NLTIME_FROM_NS(26)); // FIXME: Timing
	}

	NETLIB_DEVICE_IMPL(74194,    "TTL_74194",     "+CLK,+S0,+S1,+SRIN,+A,+B,+C,+D,+SLIN,+CLRQ,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74194_dip, "TTL_74194_DIP", "")

	} //namespace devices
} // namespace netlist
