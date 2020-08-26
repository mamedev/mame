// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_74194.cpp
 *
 *  74194: Parallel-Load 8-Bit Shift Register
 *
 *          +--------------+
 *     CLRQ |1     ++    16| VCC
 *     SRIN |2           15| QA
 *        A |3           14| QB
 *        B |4    74194  13| QC
 *        C |5           12| QD
 *        D |6           11| CLK
 *     SLIN |7           10| S1
 *      GND |8            9| S0
 *          +--------------+
 *
 * CLR: Clear
 * SRIN: Shift Right Serial Input
 * SLIN: Shift Left Serial Input
 * CLK: Clock
 *
 */


#include "nld_74194.h"
#include "nl_base.h"

namespace netlist
{
	namespace devices
	{

	// FIXME: Optimize
	NETLIB_OBJECT(74194)
	{
		NETLIB_CONSTRUCTOR(74194)
		, m_DATA(*this, {"D", "C", "B", "A"}, NETLIB_DELEGATE(inputs))
		, m_SLIN(*this, "SLIN", NETLIB_DELEGATE(inputs))
		, m_SRIN(*this, "SRIN", NETLIB_DELEGATE(inputs))
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(inputs))
		, m_S0(*this, "S0", NETLIB_DELEGATE(inputs))
		, m_S1(*this, "S1", NETLIB_DELEGATE(inputs))
		, m_CLRQ(*this, "CLRQ", NETLIB_DELEGATE(inputs))
		, m_Q(*this, {"QD", "QC", "QB", "QA"})
		, m_last_CLK(*this, "m_last_CLK", 0)
		, m_last_Q(*this, "m_last_Q", 0)
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_last_CLK = 0;
			m_last_Q = 0;
		}

		// FIXME: Timing
		NETLIB_HANDLERI(inputs)
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

	NETLIB_DEVICE_IMPL(74194,    "TTL_74194",     "+CLK,+S0,+S1,+SRIN,+A,+B,+C,+D,+SLIN,+CLRQ,@VCC,@GND")

	} //namespace devices
} // namespace netlist
