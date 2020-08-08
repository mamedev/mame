// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_74165.cpp
 *
 *  74165: Parallel-Load 8-Bit Shift Register
 *
 *          +--------------+
 *   SH/LDQ |1     ++    16| VCC
 *      CLK |2           15| CLKINH
 *        E |3           14| D
 *        F |4    74165  13| C
 *        G |5           12| B
 *        H |6           11| A
 *      QHQ |7           10| SER
 *      GND |8            9| QH
 *          +--------------+
 *
 * SH/LDQ: Shift / !Load
 * CLKINH: Clock Inhibit
 * SER: Serial In
 *
 *  Naming convention attempts to follow NTE Electronics datasheet
 *
 */

#include "nld_74165.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(74165)
	{
		NETLIB_CONSTRUCTOR(74165)
		, m_DATA(*this, { "H", "G", "F", "E", "D", "C", "B", "A" }, NETLIB_DELEGATE(inputs))
		, m_SER(*this, "SER", NETLIB_DELEGATE(inputs))
		, m_SH_LDQ(*this, "SH_LDQ", NETLIB_DELEGATE(inputs))
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(inputs))
		, m_CLKINH(*this, "CLKINH", NETLIB_DELEGATE(inputs))
		, m_QH(*this, "QH")
		, m_QHQ(*this, "QHQ")
		, m_shifter(*this, "m_shifter", 0)
		, m_last_CLK(*this, "m_last_CLK", 0)
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_shifter = 0;
			m_last_CLK = 0;
		}

		NETLIB_HANDLERI(inputs)
		{
			{
				netlist_sig_t old_qh = m_QH.net().Q();
				netlist_sig_t qh = 0;

				if (!m_SH_LDQ())
				{
					m_shifter = 0;
					for (std::size_t i=0; i<8; i++)
						m_shifter |= (m_DATA[i]() << i);
				}
				else if (!m_CLK() || m_CLKINH())
				{
					// FIXME: qh is overwritten below?
					qh = old_qh;
				}
				else if (!m_last_CLK)
				{
					unsigned high_bit = m_SER() ? 0x80 : 0;
					m_shifter = high_bit | (m_shifter >> 1);
				}

				qh = m_shifter & 1;

				m_last_CLK = m_CLK();

				m_QH.push(qh, NLTIME_FROM_NS(20)); // FIXME: Timing
			}

		}

		friend class NETLIB_NAME(74165_dip);
	private:
		object_array_t<logic_input_t, 8> m_DATA;
		logic_input_t m_SER;
		logic_input_t m_SH_LDQ;
		logic_input_t m_CLK;
		logic_input_t m_CLKINH;
		logic_output_t m_QH;
		logic_output_t m_QHQ;

		state_var<unsigned> m_shifter;
		state_var<unsigned> m_last_CLK;
		nld_power_pins m_power_pins;
	};

	// FIXME: Timing
	NETLIB_DEVICE_IMPL(74165, "TTL_74165", "+CLK,+CLKINH,+SH_LDQ,+SER,+A,+B,+C,+D,+E,+F,+G,+H,@VCC,@GND")

	} //namespace devices
} // namespace netlist
