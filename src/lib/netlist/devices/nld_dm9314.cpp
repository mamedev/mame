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
		, m_EQ(*this, "EQ", NETLIB_DELEGATE(inputs))
		, m_MRQ(*this, "MRQ", NETLIB_DELEGATE(inputs))
		, m_SQ(*this, {"S0Q", "S1Q", "S2Q", "S3Q"}, NETLIB_DELEGATE(inputs))
		, m_D(*this, {"D0", "D1", "D2", "D3"}, NETLIB_DELEGATE(inputs))
		, m_Q(*this, {"Q0", "Q1", "Q2", "Q3"})
		, m_last_EQ(*this, "m_last_EQ", 0)
		, m_last_MRQ(*this, "m_last_MRQ", 0)
		, m_last_SQ(*this, "m_last_SQ", 0)
		, m_last_D(*this, "m_last_D", 0)
		, m_last_Q(*this, "m_last_Q", 0)
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_last_MRQ = 0;
			m_last_EQ = 0;
			m_last_SQ = 0;
			m_last_D = 0;
			m_last_Q = 0;
		}

		friend class NETLIB_NAME(9314_dip);
	private:
		NETLIB_HANDLERI(inputs)
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
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT(9314_dip)
	{
		NETLIB_CONSTRUCTOR(9314_dip)
		, A(*this, "A")
		{
			register_subalias("1", A.m_EQ);
			register_subalias("2", A.m_SQ[0]);
			register_subalias("3", A.m_D[0]);
			register_subalias("4", A.m_D[1]);
			register_subalias("5", A.m_SQ[2]);
			register_subalias("6", A.m_D[2]);
			register_subalias("7", A.m_D[3]);
			register_subalias("8", "A.GND");

			register_subalias("9",  A.m_MRQ);
			register_subalias("10", A.m_Q[3]);
			register_subalias("11", A.m_SQ[3]);
			register_subalias("12", A.m_Q[2]);
			register_subalias("13", A.m_Q[1]);
			register_subalias("14", A.m_SQ[1]);
			register_subalias("15", A.m_Q[0]);
			register_subalias("16", "A.VCC");
		}
		private:
			NETLIB_SUB(9314) A;
	};

	NETLIB_DEVICE_IMPL(9314,     "TTL_9314",     "+EQ,+MRQ,+S0Q,+S1Q,+S2Q,+S3Q,+D0,+D1,+D2,+D3,@VCC,@GND")
	NETLIB_DEVICE_IMPL(9314_dip, "TTL_9314_DIP", "")

	} //namespace devices
} // namespace netlist
