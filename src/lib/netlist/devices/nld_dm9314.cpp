// license:BSD-3-Clause
// copyright-holders:Felipe Sanches
/*
 * nld_dm9314.cpp
 *
 *  DM9314: 4-Bit Latches
 *
 *          +--------------+
 *       /E |1     ++    16| VCC
 *      /S0 |2           15| Q0
 *       D0 |3           14| /S1
 *       D1 |4   DM9314  13| Q1
 *      /S2 |5           12| Q2
 *       D2 |6           11| /S3
 *       D3 |7           10| Q3
 *      GND |8            9| /MR
 *          +--------------+
 *
 */

#include "nl_base.h"

namespace netlist::devices {

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

	private:

		NETLIB_RESETI()
		{
			m_last_MRQ = 0;
			m_last_EQ = 0;
			m_last_SQ = 0;
			m_last_D = 0;
			m_last_Q = 0;
		}

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
						// RS mode is just an "extension of regular D mode"
						// The way RS mode works is that D and S bar go high (keeps old value)
						// S bar going low sets output high
						// D going low and S bar high sets output low
						// S bar going low AND D going low sets output low (D takes precedence)
						if (!m_EQ())
						{
							if (!m_D[i]())  // if D low and SQ high we clear the bit
							{
								m_last_Q &= ~(1 << i);
								m_Q[i].push((m_last_Q & (1<<i))>>i, delay);
							}
						}
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

	NETLIB_DEVICE_IMPL(9314,     "TTL_9314",     "+EQ,+MRQ,+S0Q,+S1Q,+S2Q,+S3Q,+D0,+D1,+D2,+D3,@VCC,@GND")

} // namespace netlist::devices
