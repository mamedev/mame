// license:BSD-3-Clause
// copyright-holders:Felipe Sanches
/*
 * nld_dm9314.cpp
 *
 */

#include "nld_dm9314.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(9314)
	{
		NETLIB_CONSTRUCTOR(9314)
		, m_EQ(*this, "EQ")
		, m_MRQ(*this, "MRQ")
		, m_SQ(*this, {{"S0Q", "S1Q", "S2Q", "S3Q"}})
		, m_D(*this, {{"D0", "D1", "D2", "D3"}})
		, m_Q(*this, {{"Q0", "Q1", "Q2", "Q3"}})
		, m_last_EQ(*this, "m_last_EQ", 0)
		, m_last_MRQ(*this, "m_last_MRQ", 0)
		, m_last_SQ(*this, "m_last_SQ", 0)
		, m_last_D(*this, "m_last_D", 0)
		, m_last_Q(*this, "m_last_Q", 0)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		logic_input_t m_EQ;
		logic_input_t m_MRQ;
		object_array_t<logic_input_t, 4> m_SQ;
		object_array_t<logic_input_t, 4> m_D;
		object_array_t<logic_output_t, 4> m_Q;

		state_var<unsigned> m_last_EQ;
		state_var<unsigned> m_last_MRQ;
		state_var<unsigned> m_last_SQ;
		state_var<unsigned> m_last_D;
		state_var<unsigned> m_last_Q;
	};

	NETLIB_OBJECT_DERIVED(9314_dip, 9314)
	{
		NETLIB_CONSTRUCTOR_DERIVED(9314_dip, 9314)
		{
			register_subalias("1", m_EQ);
			register_subalias("2", m_SQ[0]);
			register_subalias("3", m_D[0]);
			register_subalias("4", m_D[1]);
			register_subalias("5", m_SQ[2]);
			register_subalias("6", m_D[2]);
			register_subalias("7", m_D[3]);

			register_subalias("9",  m_MRQ);
			register_subalias("10", m_Q[3]);
			register_subalias("11", m_SQ[3]);
			register_subalias("12", m_Q[2]);
			register_subalias("13", m_Q[1]);
			register_subalias("14", m_SQ[1]);
			register_subalias("15", m_Q[0]);

		}
	};

	NETLIB_RESET(9314)
	{
		m_last_MRQ = 0;
		m_last_EQ = 0;
		m_last_SQ = 0;
		m_last_D = 0;
		m_last_Q = 0;
	}

	NETLIB_UPDATE(9314)
	{
		netlist_time delay = NLTIME_FROM_NS(24); //FIXME!
		if (!m_MRQ())
		{
			/* Reset! */
			for (std::size_t i=0; i<4; i++)
				m_Q[i].push(0, delay);
		} else {
			for (std::size_t i=0; i<4; i++)
			{
				if (m_SQ[i]())
				{
					/* R-S Mode */
					// FIXME: R-S mode is not yet implemented!
				}
				else
				{
					/* D Mode */
					if (!m_EQ())
					{
						m_Q[i].push(m_D[i](), delay);
						m_last_Q &= ~(1 << i);
						m_last_Q |= (m_D[i]() << i);
					}
				}
			}
		}
	}

	NETLIB_DEVICE_IMPL(9314,     "TTL_9314",     "+EQ,+MRQ,+S0Q,+S1Q,+S2Q,+S3Q,+D0,+D1,+D2,+D3")
	NETLIB_DEVICE_IMPL(9314_dip, "TTL_9314_DIP", "")

	} //namespace devices
} // namespace netlist
