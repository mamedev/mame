// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_74166.cpp
 *
 *  74166: Parallel-Load 8-Bit Shift Register
 *
 *          +--------------+
 *      SER |1     ++    16| VCC
 *        A |2           15| SH/LDQ
 *        B |3           14| H
 *        C |4    74166  13| QH
 *        D |5           12| G
 *   CLKINH |6           11| F
 *      CLK |7           10| E
 *      GND |8            9| CLRQ
 *          +--------------+
 *
 * SH/LDQ: Shift / !Load
 * CLKINH: Clock Inhibit
 * SER: Serial In
 *
 *  Naming convention attempts to follow Texas Instruments datasheet
 *
 */

#include "nld_74166.h"
#include "netlist/nl_base.h"

// FIXME: separate handlers for inputs

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(74166)
	{
		NETLIB_CONSTRUCTOR(74166)
		, m_DATA(*this, { "H", "G", "F", "E", "D", "C", "B", "A" }, NETLIB_DELEGATE(inputs))
		, m_SER(*this, "SER", NETLIB_DELEGATE(inputs))
		, m_CLRQ(*this, "CLRQ", NETLIB_DELEGATE(inputs))
		, m_SH_LDQ(*this, "SH_LDQ", NETLIB_DELEGATE(inputs))
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(inputs))
		, m_CLKINH(*this, "CLKINH", NETLIB_DELEGATE(inputs))
		, m_QH(*this, "QH")
		, m_shifter(*this, "m_shifter", 0)
		, m_last_CLRQ(*this, "m_last_CLRQ", 0)
		, m_last_CLK(*this, "m_last_CLK", 0)
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_shifter = 0;
			m_last_CLRQ = 0;
			m_last_CLK = 0;
		}

		NETLIB_HANDLERI(inputs)
		{
			netlist_sig_t old_qh = m_QH.net().Q();
			netlist_sig_t qh = 0;

			netlist_time delay = NLTIME_FROM_NS(26);
			if (m_CLRQ())
			{
				bool clear_unset = !m_last_CLRQ;
				if (clear_unset)
				{
					delay = NLTIME_FROM_NS(35);
				}

				if (!m_CLK() || m_CLKINH())
				{
					qh = old_qh;
				}
				else if (!m_last_CLK)
				{
					if (!m_SH_LDQ())
					{
						m_shifter = 0;
						for (std::size_t i=0; i<8; i++)
							m_shifter |= (m_DATA[i]() << i);
					}
					else
					{
						unsigned high_bit = m_SER() ? 0x80 : 0;
						m_shifter = high_bit | (m_shifter >> 1);
					}

					qh = m_shifter & 1;
					if (!qh && !clear_unset)
					{
						delay = NLTIME_FROM_NS(30);
					}
				}
			}

			m_last_CLRQ = m_CLRQ();
			m_last_CLK = m_CLK();

			m_QH.push(qh, delay); //FIXME
		}

		friend class NETLIB_NAME(74166_dip);
	private:
		object_array_t<logic_input_t, 8> m_DATA;
		logic_input_t m_SER;
		logic_input_t m_CLRQ;
		logic_input_t m_SH_LDQ;
		logic_input_t m_CLK;
		logic_input_t m_CLKINH;
		logic_output_t m_QH;

		state_var<unsigned> m_shifter;
		state_var<unsigned> m_last_CLRQ;
		state_var<unsigned> m_last_CLK;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(74166,    "TTL_74166", "+CLK,+CLKINH,+SH_LDQ,+SER,+A,+B,+C,+D,+E,+F,+G,+H,+CLRQ,@VCC,@GND")

	} //namespace devices
} // namespace netlist
