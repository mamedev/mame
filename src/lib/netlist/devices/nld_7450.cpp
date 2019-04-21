	// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7450.c
 *
 */

#include "nld_7450.h"
#include "netlist/nl_base.h"
#include "nlid_system.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(7450)
	{
		NETLIB_CONSTRUCTOR(7450)
		, m_A(*this, "A")
		, m_B(*this, "B")
		, m_C(*this, "C")
		, m_D(*this, "D")
		, m_Q(*this, "Q")
		, m_power_pins(*this)
		{
		}
		//NETLIB_RESETI();
		NETLIB_UPDATEI();

	public:
		logic_input_t m_A;
		logic_input_t m_B;
		logic_input_t m_C;
		logic_input_t m_D;
		logic_output_t m_Q;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT(7450_dip)
	{
		NETLIB_CONSTRUCTOR(7450_dip)
		, m_A(*this, "A")
		, m_B(*this, "B")
		{
			register_subalias("1", m_A.m_A);
			register_subalias("2", m_B.m_A);
			register_subalias("3", m_B.m_B);
			register_subalias("4", m_B.m_C);
			register_subalias("5", m_B.m_D);
			register_subalias("6", m_B.m_Q);
			register_subalias("7", "A.GND");

			register_subalias("8", m_A.m_Q);
			register_subalias("9", m_A.m_C);
			register_subalias("10", m_A.m_D);
			//register_subalias("11", m_1.m_X1);
			//register_subalias("12", m_1.m_X1Q);
			register_subalias("13", m_A.m_B);
			register_subalias("14", "A.VCC");

			connect("A.GND", "B.GND");
			connect("A.VCC", "B.VCC");

		}
		//NETLIB_RESETI();
		//NETLIB_UPDATEI();

		NETLIB_SUB(7450) m_A;
		NETLIB_SUB(7450) m_B;
	};

	NETLIB_UPDATE(7450)
	{
		m_A.activate();
		m_B.activate();
		m_C.activate();
		m_D.activate();
		auto t1 = m_A() & m_B();
		auto t2 = m_C() & m_D();

		const netlist_time times[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22) };

		uint_fast8_t res = 0;
		if (t1 ^ 1)
		{
			if (t2 ^ 1)
			{
				res = 1;
			}
			else
			{
				m_A.inactivate();
				m_B.inactivate();
			}
		}
		else
		{
			if (t2 ^ 1)
			{
				m_C.inactivate();
				m_D.inactivate();
			}
		}
		m_Q.push(res, times[res]);// ? 22000 : 15000);
	}

	NETLIB_DEVICE_IMPL(7450,        "TTL_7450_ANDORINVERT", "+A,+B,+C,+D,@VCC,@GND")
	NETLIB_DEVICE_IMPL(7450_dip,    "TTL_7450_DIP", "")

	} //namespace devices
} // namespace netlist
