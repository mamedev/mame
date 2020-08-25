// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_dm9334.cpp
 *
 *  DM9334: 8-Bit Addressable Latch
 *
 *          +--------------+
 *       A0 |1     ++    16| VCC
 *       A1 |2           15| /C
 *       A2 |3           14| /E
 *       Q0 |4   DM9334  13| D
 *       Q1 |5           12| Q7
 *       Q2 |6           11| Q6
 *       Q3 |7           10| Q5
 *      GND |8            9| Q4
 *          +--------------+
 *
 *          +---+---++---++---+---+---++---+---+---+---+---+---+---+---+
 *          | C | E || D || A0| A1| A2|| Q0| Q1| Q2| Q3| Q4| Q5| Q6| Q7|
 *          +===+===++===++===+===+===++===+===+===+===+===+===+===+===+
 *          | 1 | 0 || X || X | X | X || 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
 *          +---+---++---++---+---+---++---+---+---+---+---+---+---+---+
 *          | 1 | 1 || 0 || 0 | 0 | 0 || 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
 *          | 1 | 1 || 1 || 0 | 0 | 0 || 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
 *          | 1 | 1 || 0 || 0 | 0 | 1 || 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
 *          | 1 | 1 || 1 || 0 | 0 | 1 || 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 |
 *          | 1 | 1 || 0 || 0 | 1 | 0 || 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
 *          | 1 | 1 || 1 || 0 | 1 | 0 || 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 |
 *          | 1 | 1 || 0 || 0 | 1 | 1 || 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
 *          | 1 | 1 || 1 || 0 | 1 | 1 || 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 |
 *          | 1 | 1 || 0 || 1 | 0 | 0 || 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
 *          | 1 | 1 || 1 || 1 | 0 | 0 || 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |
 *          | 1 | 1 || 0 || 1 | 0 | 1 || 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
 *          | 1 | 1 || 1 || 1 | 0 | 1 || 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 |
 *          | 1 | 1 || 0 || 1 | 1 | 0 || 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
 *          | 1 | 1 || 1 || 1 | 1 | 0 || 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
 *          | 1 | 1 || 0 || 1 | 1 | 1 || 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
 *          | 1 | 1 || 1 || 1 | 1 | 1 || 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 |
 *          +---+---++---++---+---+---++---+---+---+---+---+---+---+---+
 *          | 0 | 0 || X || X | X | X || P | P | P | P | P | P | P | P |
 *          +---+---++---++---+---+---++---+---+---+---+---+---+---+---+
 *          | 0 | 1 || 0 || 0 | 0 | 0 || 0 | P | P | P | P | P | P | P |
 *          | 0 | 1 || 1 || 0 | 0 | 0 || 1 | P | P | P | P | P | P | P |
 *          | 0 | 1 || 0 || 0 | 0 | 1 || P | 0 | P | P | P | P | P | P |
 *          | 0 | 1 || 1 || 0 | 0 | 1 || P | 1 | P | P | P | P | P | P |
 *          | 0 | 1 || 0 || 0 | 1 | 0 || P | P | 0 | P | P | P | P | P |
 *          | 0 | 1 || 1 || 0 | 1 | 0 || P | P | 1 | P | P | P | P | P |
 *          | 0 | 1 || 0 || 0 | 1 | 1 || P | P | P | 0 | P | P | P | P |
 *          | 0 | 1 || 1 || 0 | 1 | 1 || P | P | P | 1 | P | P | P | P |
 *          | 0 | 1 || 0 || 1 | 0 | 0 || P | P | P | P | 0 | P | P | P |
 *          | 0 | 1 || 1 || 1 | 0 | 0 || P | P | P | P | 1 | P | P | P |
 *          | 0 | 1 || 0 || 1 | 0 | 1 || P | P | P | P | P | 0 | P | P |
 *          | 0 | 1 || 1 || 1 | 0 | 1 || P | P | P | P | P | 1 | P | P |
 *          | 0 | 1 || 0 || 1 | 1 | 0 || P | P | P | P | P | P | 0 | P |
 *          | 0 | 1 || 1 || 1 | 1 | 0 || P | P | P | P | P | P | 1 | P |
 *          | 0 | 1 || 0 || 1 | 1 | 1 || P | P | P | P | P | P | P | 0 |
 *          | 0 | 1 || 1 || 1 | 1 | 1 || P | P | P | P | P | P | P | 1 |
 *          +---+---++---++---+---+---++---+---+---+---+---+---+---+---+
 *
 *  Naming convention attempts to follow Texas Instruments / National Semiconductor datasheet Literature Number SNOS382A
 *
 */

#include "nld_dm9334.h"
#include "nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(9334)
	{
		NETLIB_CONSTRUCTOR(9334)
		, m_CQ(*this, "CQ", NETLIB_DELEGATE(inputs))
		, m_EQ(*this, "EQ", NETLIB_DELEGATE(inputs))
		, m_D(*this, "D", NETLIB_DELEGATE(inputs))
		, m_A(*this, {"A0", "A1", "A2"}, NETLIB_DELEGATE(inputs))
		, m_Q(*this, {"Q0", "Q1", "Q2", "Q3", "Q4", "Q5", "Q6", "Q7"})
		, m_last_CQ(*this, "m_last_CQ", 0)
		, m_last_EQ(*this, "m_last_EQ", 0)
		, m_last_D(*this, "m_last_D", 0)
		, m_last_A(*this, "m_last_A", 0)
		, m_last_Q(*this, "m_last_Q", 0)
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_last_CQ = 0;
			m_last_EQ = 0;
			m_last_D = 0;
			m_last_A = 0;
			m_last_Q = 0;
		}

		NETLIB_HANDLERI(inputs)
		{
			uint_fast8_t a = 0;
			for (std::size_t i=0; i<3; i++)
			{
				a |= (m_A[i]() << i);
			}

			netlist_time delay = NLTIME_FROM_NS(27); // Clear Low to High Level Output (not documented, making reasonable guess)

			if (a != m_last_A)
			{
				delay = NLTIME_FROM_NS(35);
			}
			else if (m_D() != m_last_D)
			{
				if (m_last_D)
				{
					delay = NLTIME_FROM_NS(28);
				}
				else
				{
					delay = NLTIME_FROM_NS(35);
				}
			}
			else if (m_EQ() != m_last_EQ)
			{
				if (m_last_EQ)
				{
					delay = NLTIME_FROM_NS(27);
				}
				else
				{
					delay = NLTIME_FROM_NS(28);
				}
			}

			unsigned q = m_last_Q;

			if (!m_CQ())
			{
				if (m_EQ())
				{
					q = 0;
				}
				else
				{
					q = m_D() << a;
				}
			}
			else if(!m_EQ())
			{
				q &= ~(1 << a);
				q |= (m_D() << a);
			}

			m_last_CQ = m_CQ();
			m_last_EQ = m_EQ();
			m_last_D = m_D();
			m_last_A = a;
			m_last_Q = q;

			for (std::size_t i=0; i<8; i++)
				m_Q[i].push((q >> i) & 1, delay);
		}

		logic_input_t m_CQ;
		logic_input_t m_EQ;
		logic_input_t m_D;
		object_array_t<logic_input_t, 3> m_A;
		object_array_t<logic_output_t, 8> m_Q;

		state_var<unsigned> m_last_CQ;
		state_var<unsigned> m_last_EQ;
		state_var<unsigned> m_last_D;
		state_var<unsigned> m_last_A;
		state_var<unsigned> m_last_Q;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(9334,     "TTL_9334",     "+CQ,+EQ,+D,+A0,+A1,+A2,@VCC,@GND")

	} //namespace devices
} // namespace netlist
