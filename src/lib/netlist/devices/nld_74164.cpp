// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*
 * nld_74164.cpp
 *
 * Thanks to the 74161 work of Ryan and the huge Netlist effort by Couriersud
 * implementing this was simple.
 *
 */
/*****************************************************************************

    5/74164 8-bit parallel-out serial shift registers

***********************************************************************

    Connection Diagram:
              ___ ___
        A  1 |*  u   | 14  Vcc
        B  2 |       | 13  QH
       QA  3 |       | 12  QG
       QB  4 |       | 11  QF
       QC  5 |       | 10  QE
       QD  6 |       |  9  *Clear
      GND  7 |_______|  8  Clock

***********************************************************************
    Function Table:
    +-------------------------+----------------+
    |       Inputs            |  Outputs*      |
    +-------+-------+---------+----------------+
    | Clear | Clock |  A   B  | QA  QB ... QH  |
    +-------+-------+---------+----------------+
    |   L   |   X   |  X   X  |  L   L      L  |
    |   H   |   L   |  X   X  | QA0 QB0    QH0 |
    |   H   |   ^   |  H   H  |  H  QAn    QGn |
    |   H   |   ^   |  L   X  |  L  QAn    QGn |
    |   H   |   ^   |  X   L  |  L  QAn    QGn |
    +-------+-------+---------+----------------+

    H = High Level (steady state)
    L = Low Level (steady state)
    X = Don't Care
    ^ = Transition from low to high level
    QA0, QB0 ... QH0 = The level of QA, QB ... QH before the indicated steady-state input conditions were established.
    QAn, QGn = The level of QA or QG before the most recent ^ transition of the clock; indicates a 1 bit shift.

**********************************************************************/

#include "nl_base.h"

// FIXME: clk input to be separated - only falling edge relevant

namespace netlist::devices {

	NETLIB_OBJECT(74164)
	{
		NETLIB_CONSTRUCTOR(74164)
		, m_AB(*this, {"A", "B"}, NETLIB_DELEGATE(ab))
		, m_CLRQ(*this, "CLRQ", NETLIB_DELEGATE(clrq))
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_cnt(*this, "m_cnt", 0)
		, m_ab(*this, "m_ab", 0)
		, m_Q(*this, {"QA", "QB", "QC", "QD", "QE", "QF", "QG", "QH"})
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_CLK.set_state(logic_t::STATE_INP_LH);
			m_cnt = 0;
		}

		NETLIB_HANDLERI(clrq)
		{
			if (m_CLRQ())
			{
				m_CLK.activate_lh();
			}
			else
			{
				m_CLK.inactivate();
				if (m_cnt != 0)
				{
					m_cnt = 0;
					m_Q.push(0, NLTIME_FROM_NS(30));
				}
			}
		}

		NETLIB_HANDLERI(clk)
		{
			m_cnt = (m_cnt << 1) | m_ab;
			m_Q.push(m_cnt, NLTIME_FROM_NS(30));
		}

		NETLIB_HANDLERI(ab)
		{
			m_ab = static_cast<unsigned>((m_AB() == 3) ? 1 : 0);
		}

		object_array_t<logic_input_t, 2> m_AB;
		logic_input_t m_CLRQ;
		logic_input_t m_CLK;

		state_var<unsigned> m_cnt;
		state_var<unsigned> m_ab;

		object_array_t<logic_output_t, 8> m_Q;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(74164, "TTL_74164", "+A,+B,+CLRQ,+CLK,@VCC,@GND")

} // namespace netlist::devices
