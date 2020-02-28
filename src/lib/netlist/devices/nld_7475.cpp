// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_7475.cpp
 *
 * TODO: Correct timing for clock-induced state changes, rather than assuming timing is always due to data-induced state changes
 */

#include "nld_7475.h"
#include "netlist/nl_base.h"
#include "nlid_system.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(7477)
	{
		NETLIB_CONSTRUCTOR(7477)
		, m_C1C2(*this, "C1C2")
		, m_C3C4(*this, "C3C4")
		, m_last_Q(*this, "m_last_Q", 0)
		, m_D(*this, {{"D1", "D2", "D3", "D4"}})
		, m_Q(*this, {{"Q1", "Q2", "Q3", "Q4"}})
		, m_power_pins(*this)
		{
			register_subalias("Q1", m_Q[0]);
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

		void update_outputs(std::size_t start, std::size_t end);

	public:
		logic_input_t m_C1C2;
		logic_input_t m_C3C4;
		state_var<unsigned> m_last_Q;
		object_array_t<logic_input_t, 4> m_D;
		object_array_t<logic_output_t, 4> m_Q;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT_DERIVED(7475, 7477)
	{
		NETLIB_CONSTRUCTOR_DERIVED(7475, 7477)
		, m_QQ(*this, {{"QQ1", "QQ2", "QQ3", "QQ4"}})
		{
		}

		NETLIB_UPDATEI();

	public:
		object_array_t<logic_output_t, 4> m_QQ;
	};

	NETLIB_OBJECT_DERIVED(7475_dip, 7475)
	{
		NETLIB_CONSTRUCTOR_DERIVED(7475_dip, 7475)
		{
			register_subalias("1", m_QQ[0]);
			register_subalias("2", m_D[0]);
			register_subalias("3", m_D[1]);
			register_subalias("4", m_C3C4);
			register_subalias("5", "VCC");
			register_subalias("6", m_D[2]);
			register_subalias("7", m_D[3]);
			register_subalias("8", m_QQ[3]);

			register_subalias("9",  m_Q[3]);
			register_subalias("10", m_Q[2]);
			register_subalias("11", m_QQ[2]);
			register_subalias("12", "GND");
			register_subalias("13", m_C1C2);
			register_subalias("14", m_QQ[1]);
			register_subalias("15", m_Q[1]);
			register_subalias("16", m_Q[0]);
		}
	};

	NETLIB_OBJECT_DERIVED(7477_dip, 7477)
	{
		NETLIB_CONSTRUCTOR_DERIVED(7477_dip, 7477)
		{
			register_subalias("1", m_D[0]);
			register_subalias("2", m_D[1]);
			register_subalias("3", m_C3C4);
			//register_subalias("4", ); ==> VCC
			register_subalias("5", m_D[2]);
			register_subalias("6", m_D[3]);
			//register_subalias("7", ); ==> NC

			register_subalias("8",  m_Q[3]);
			register_subalias("9",  m_Q[2]);
			//register_subalias("10", ); ==> NC
			//register_subalias("11", ); ==> GND
			register_subalias("12", m_C1C2);
			register_subalias("13", m_Q[1]);
			register_subalias("14", m_Q[0]);
		}
	};

	NETLIB_UPDATE(7475)
	{
		unsigned start_q = m_last_Q;

		NETLIB_NAME(7477)::update();

		for (std::size_t i=0; i<4; i++)
		{
			unsigned last_bit = (m_last_Q >> i) & 1;
			unsigned start_bit = (start_q >> i) & 1;
			if (last_bit != start_bit)
				m_QQ[i].push(last_bit ^ 1, last_bit != 0 ? NLTIME_FROM_NS(15) : NLTIME_FROM_NS(40));
		}
	}

	void NETLIB_NAME(7477)::update_outputs(std::size_t start, std::size_t end)
	{
		for (std::size_t i=start; i<end; i++)
		{
			netlist_sig_t d = m_D[i]();
			if (d != ((m_last_Q >> i) & 1))
				m_Q[i].push(d, d != 0 ? NLTIME_FROM_NS(30) : NLTIME_FROM_NS(25));
			m_last_Q &= ~(1 << i);
			m_last_Q |= d << i;
		}
	}

	NETLIB_RESET(7477)
	{
		m_last_Q = 0;
	}

	NETLIB_UPDATE(7477)
	{
		netlist_sig_t c1c2 = m_C1C2();
		netlist_sig_t c3c4 = m_C3C4();
		if (c1c2 && c3c4)
		{
			update_outputs(0, 4);
		}
		else if (c1c2)
		{
			update_outputs(0, 2);
		}
		else if (c3c4)
		{
			update_outputs(2, 4);
		}

	}

	NETLIB_DEVICE_IMPL(7475,     "TTL_7475",     "")
	NETLIB_DEVICE_IMPL(7475_dip, "TTL_7475_DIP", "")
	NETLIB_DEVICE_IMPL(7477,     "TTL_7477",     "")
	NETLIB_DEVICE_IMPL(7477_dip, "TTL_7477_DIP", "")

	} //namespace devices
} // namespace netlist
