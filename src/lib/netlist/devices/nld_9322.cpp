// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_9322.cpp
 *
 *  9322: Quad 2-Line to 1-Line Data Selectors/Multiplexers
 *
 *          +------------+
 *   SELECT |1    ++   16| VCC
 *       A1 |2         15| STROBE
 *       B1 |3         14| A4
 *       Y1 |4   9322  13| B4
 *       A2 |5         12| Y4
 *       B2 |6         11| A3
 *       Y2 |7         10| B3
 *      GND |8          9| Y3
 *          +------------+
 *
 */

#include "nl_base.h"

namespace netlist::devices {

	class NETLIB_NAME(9322);

	NETLIB_OBJECT(9322_GATE)
	{
		NETLIB_CONSTRUCTOR(9322_GATE)
		, m_A(*this, "A", NETLIB_DELEGATE(inputs))
		, m_B(*this, "B", NETLIB_DELEGATE(inputs))
		, m_SELECT(*this, "SELECT", NETLIB_DELEGATE(inputs))
		, m_STROBE(*this, "STROBE", NETLIB_DELEGATE(inputs))
		, m_Y(*this, "Y")
		, m_power_pins(*this)
		{
		}

		// FIXME: Timing
		NETLIB_HANDLERI(inputs)
		{
			if (m_STROBE())
				m_Y.push(0, NLTIME_FROM_NS(21));
			else if (m_SELECT())
				m_Y.push(m_B(), NLTIME_FROM_NS(14));
			else
				m_Y.push(m_A(), NLTIME_FROM_NS(14));
		}

	public:
		logic_input_t m_A;
		logic_input_t m_B;
		logic_input_t m_SELECT;
		logic_input_t m_STROBE;
		logic_output_t m_Y;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT(9322)
	{
		NETLIB_CONSTRUCTOR(9322)
		, m_1(*this, "A")
		, m_2(*this, "B")
		, m_3(*this, "C")
		, m_4(*this, "D")
		{
			register_sub_alias("A1", "A.A");
			register_sub_alias("B1", "A.B");
			register_sub_alias("Y1", "A.Y");
			register_sub_alias("A2", "B.A");
			register_sub_alias("B2", "B.B");
			register_sub_alias("Y2", "B.Y");
			register_sub_alias("A3", "C.A");
			register_sub_alias("B3", "C.B");
			register_sub_alias("Y3", "C.Y");
			register_sub_alias("A4", "D.A");
			register_sub_alias("B4", "D.B");
			register_sub_alias("Y4", "D.Y");

			connect("A.VCC", "B.VCC");
			connect("A.VCC", "C.VCC");
			connect("A.VCC", "D.VCC");
			connect("A.GND", "B.GND");
			connect("A.GND", "C.GND");
			connect("A.GND", "D.GND");
			connect("A.SELECT", "B.SELECT");
			connect("A.SELECT", "C.SELECT");
			connect("A.SELECT", "D.SELECT");
			connect("A.STROBE", "B.STROBE");
			connect("A.STROBE", "C.STROBE");
			connect("A.STROBE", "D.STROBE");

			register_sub_alias("SELECT", "A.SELECT");
			register_sub_alias("STROBE", "A.STROBE");
			register_sub_alias("GND", "A.GND");
			register_sub_alias("VCC", "B.VCC");
		}

	private:
		NETLIB_SUB(9322_GATE) m_1;
		NETLIB_SUB(9322_GATE) m_2;
		NETLIB_SUB(9322_GATE) m_3;
		NETLIB_SUB(9322_GATE) m_4;

	};

	NETLIB_DEVICE_IMPL(9322,      "TTL_9322",      "+SELECT,+A1,+B1,+A2,+B2,+A3,+B3,+A4,+B4,+STROBE,@VCC,@GND")

} // namespace netlist::devices
