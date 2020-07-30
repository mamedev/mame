	// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7450.c
 *
 */

#include "nld_7450.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{

	static constexpr const std::array<netlist_time, 2> times = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22) };

	NETLIB_OBJECT(7450)
	{
		NETLIB_CONSTRUCTOR(7450)
		, m_A(*this, "A", NETLIB_DELEGATE(inputs))
		, m_B(*this, "B", NETLIB_DELEGATE(inputs))
		, m_C(*this, "C", NETLIB_DELEGATE(inputs))
		, m_D(*this, "D", NETLIB_DELEGATE(inputs))
		, m_Q(*this, "Q")
		, m_power_pins(*this)
		{
		}

		//NETLIB_RESETI();

	public:
		NETLIB_HANDLERI(inputs)
		{
			m_A.activate();
			m_B.activate();
			m_C.activate();
			m_D.activate();
			auto t1 = m_A() & m_B();
			auto t2 = m_C() & m_D();

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

		logic_input_t m_A;
		logic_input_t m_B;
		logic_input_t m_C;
		logic_input_t m_D;
		logic_output_t m_Q;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(7450,        "TTL_7450_ANDORINVERT", "+A,+B,+C,+D,@VCC,@GND")

	} //namespace devices
} // namespace netlist
