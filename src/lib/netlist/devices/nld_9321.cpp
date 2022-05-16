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

#include "nl_base.h"

namespace netlist::devices {

	// FIXME: m_E should activate deactivate m_A

	NETLIB_OBJECT(9321)
	{
		NETLIB_CONSTRUCTOR(9321)
		, m_enable(*this, "m_enable", true)
		, m_o(*this, "m_o", 0)
		, m_A(*this, 0, "A{}", NETLIB_DELEGATE(in))
		, m_D(*this, 0, "D{}")
		, m_E(*this, "E", NETLIB_DELEGATE(e))
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_HANDLERI(in)
		{
			m_enable = m_E() ? false : true; // NOLINT
			m_o = (m_A[1]() << 1) | m_A[0]();
			for (std::size_t i=0; i<4; i++)
				m_D[i].push((i == m_o && m_enable) ? 0 : 1, NLTIME_FROM_NS(21));
		}

		NETLIB_HANDLERI(e)
		{
			m_enable = m_E() ? false : true; // NOLINT
			m_o = (m_A[1]() << 1) | m_A[0]();
			for (std::size_t i=0; i<4; i++)
				m_D[i].push((i == m_o && m_enable) ? 0 : 1, NLTIME_FROM_NS(18));
		}

		state_var<bool> m_enable;
		state_var<uint32_t> m_o;
		object_array_t<logic_input_t, 2> m_A;
		object_array_t<logic_output_t, 4> m_D;
		logic_input_t m_E;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(9321, "TTL_9321", "+E,+A0,+A1")

} // namespace netlist::devices
