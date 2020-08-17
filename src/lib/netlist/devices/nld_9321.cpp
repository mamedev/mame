// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_9321.cpp
 *
 *  9321: Dual One-of-Four Decoder
 *
 *          +------------+
 *       /E |1    ++   16| VCC
 *       A0 |2         15| /E
 *       A1 |3         14| A0
 *      /D0 |4   9321  13| A1
 *      /D1 |5         12| /D0
 *      /D2 |6         11| /D1
 *      /D3 |7         10| /D2
 *      GND |8          9| /D3
 *          +------------+
 *
 */

#include "nld_9321.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{
	class NETLIB_NAME(9321);

	NETLIB_OBJECT(9321_GATE)
	{
		NETLIB_CONSTRUCTOR(9321_GATE)
		, m_enable(*this, "m_enable", true)
		, m_o(*this, "m_o", 0)
		, m_A(*this, 0, "A{}", NETLIB_DELEGATE(in))
		, m_D(*this, 0, "D{}")
		, m_E(*this, "E", NETLIB_DELEGATE(e))
		, m_power_pins(*this)
		{
		}

		NETLIB_HANDLERI(in)
		{
			m_enable = m_E() ? 0 : 1;
			m_o = (m_A[1]() << 1) | m_A[0]();
			for (std::size_t i=0; i<4; i++)
				m_D[i].push((i == m_o && m_enable) ? 0 : 1, NLTIME_FROM_NS(21));
		}

		NETLIB_HANDLERI(e)
		{
			m_enable = m_E() ? 0 : 1;
			m_o = (m_A[1]() << 1) | m_A[0]();
			for (std::size_t i=0; i<4; i++)
				m_D[i].push((i == m_o && m_enable) ? 0 : 1, NLTIME_FROM_NS(18));
		}

	public:
		state_var<bool> m_enable;
		state_var<uint32_t> m_o;
		object_array_t<logic_input_t, 2> m_A;
		object_array_t<logic_output_t, 4> m_D;
		logic_input_t m_E;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT(9321)
	{
		NETLIB_CONSTRUCTOR(9321)
		, m_A(*this, "A")
		, m_B(*this, "B")
		{
			register_subalias("AE1", m_A.m_E);
			register_subalias("BE1", m_B.m_E);
			register_subalias("AA0", m_A.m_A[0]);
			register_subalias("BA0", m_B.m_A[0]);
			register_subalias("AA1", m_A.m_A[1]);
			register_subalias("BA1", m_B.m_A[1]);
			register_subalias("AD0", m_A.m_D[0]);
			register_subalias("BD0", m_B.m_D[0]);
			register_subalias("AD1", m_A.m_D[1]);
			register_subalias("BD1", m_B.m_D[1]);
			register_subalias("AD2", m_A.m_D[2]);
			register_subalias("BD2", m_B.m_D[2]);
			register_subalias("AD3", m_A.m_D[3]);
			register_subalias("BD3", m_B.m_D[3]);

			connect("A.VCC", "B.VCC");
			connect("A.GND", "B.GND");

			register_subalias("GND", "A.GND");
			register_subalias("VCC", "B.VCC");
		}

		friend class NETLIB_NAME(9321_dip);
	private:
		NETLIB_SUB(9321_GATE) m_A;
		NETLIB_SUB(9321_GATE) m_B;
	};

	NETLIB_DEVICE_IMPL(9321_GATE, "TTL_9321_GATE", "")
	NETLIB_DEVICE_IMPL(9321,      "TTL_9321",      "+SELECT,+A1,+B1,+A2,+B2,+A3,+B3,+A4,+B4,+STROBE,@VCC,@GND")

	} //namespace devices
} // namespace netlist
