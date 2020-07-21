// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_9322.cpp
 *
 */

#include "nld_9322.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{
	class NETLIB_NAME(9322);

	NETLIB_OBJECT(9322_selector)
	{
		NETLIB_CONSTRUCTOR(9322_selector)
		, m_parent(owner)
		, m_A(*this, "A", NETLIB_DELEGATE(inputs))
		, m_B(*this, "B", NETLIB_DELEGATE(inputs))
		, m_Y(*this, "Y")
		, m_power_pins(*this)
		{
		}

		// FIXME: Timing
		NETLIB_HANDLERI(inputs);

	public:
		NETLIB_NAME(9322) &m_parent;
		logic_input_t m_A;
		logic_input_t m_B;
		logic_output_t m_Y;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT(9322)
	{
		NETLIB_CONSTRUCTOR(9322)
		, m_SELECT(*this, "SELECT", NETLIB_DELEGATE(inputs))
		, m_STROBE(*this, "STROBE", NETLIB_DELEGATE(inputs))
		, m_1(*this, "A")
		, m_2(*this, "B")
		, m_3(*this, "C")
		, m_4(*this, "D")
		{
			register_subalias("A1", m_1.m_A);
			register_subalias("B1", m_1.m_B);
			register_subalias("Y1", m_1.m_Y);
			register_subalias("A2", m_2.m_A);
			register_subalias("B2", m_2.m_B);
			register_subalias("Y2", m_2.m_Y);
			register_subalias("A3", m_3.m_A);
			register_subalias("B3", m_3.m_B);
			register_subalias("Y3", m_3.m_Y);
			register_subalias("A4", m_4.m_A);
			register_subalias("B4", m_4.m_B);
			register_subalias("Y4", m_4.m_Y);

			connect("A.VCC", "B.VCC");
			connect("A.VCC", "C.VCC");
			connect("A.VCC", "D.VCC");
			connect("A.GND", "B.GND");
			connect("A.GND", "C.GND");
			connect("A.GND", "D.GND");

			register_subalias("GND", "A.GND");
			register_subalias("VCC", "B.VCC");

		}

		friend class NETLIB_NAME(9322_dip);
	public:
		logic_input_t m_SELECT;
		logic_input_t m_STROBE;
	private:
		NETLIB_HANDLERI(inputs)
		{
			m_1.inputs();
			m_2.inputs();
			m_3.inputs();
			m_4.inputs();
		}

		NETLIB_SUB(9322_selector) m_1;
		NETLIB_SUB(9322_selector) m_2;
		NETLIB_SUB(9322_selector) m_3;
		NETLIB_SUB(9322_selector) m_4;

	};

	NETLIB_HANDLER(9322_selector, inputs)
	{
		if (m_parent.m_STROBE())
			m_Y.push(0, NLTIME_FROM_NS(21));
		else if (m_parent.m_SELECT())
			m_Y.push(m_B(), NLTIME_FROM_NS(14));
		else
			m_Y.push(m_A(), NLTIME_FROM_NS(14));
	}

	NETLIB_OBJECT(9322_dip)
	{
		NETLIB_CONSTRUCTOR(9322_dip)
		, A(*this, "A")
		{
			register_subalias("1", A.m_SELECT);
			register_subalias("2", A.m_1.m_A);
			register_subalias("3", A.m_1.m_B);
			register_subalias("4", A.m_1.m_Y);
			register_subalias("5", A.m_2.m_A);
			register_subalias("6", A.m_2.m_B);
			register_subalias("7", A.m_2.m_Y);
			register_subalias("8", "A.GND");

			register_subalias("9",  A.m_3.m_Y);
			register_subalias("10", A.m_3.m_B);
			register_subalias("11", A.m_3.m_A);
			register_subalias("12", A.m_4.m_Y);
			register_subalias("13", A.m_4.m_B);
			register_subalias("14", A.m_4.m_A);
			register_subalias("15", A.m_STROBE);
			register_subalias("16", "A.VCC");
		}

		//NETLIB_RESETI() {}
	private:
		NETLIB_SUB(9322) A;
	};

	NETLIB_DEVICE_IMPL(9322,     "TTL_9322",     "+SELECT,+A1,+B1,+A2,+B2,+A3,+B3,+A4,+B4,+STROBE,@VCC,@GND")
	NETLIB_DEVICE_IMPL(9322_dip, "TTL_9322_DIP", "")

	} //namespace devices
} // namespace netlist
