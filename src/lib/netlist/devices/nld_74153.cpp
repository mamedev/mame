// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74153.c
 *
 */

#include "nld_74153.h"
#include "netlist/nl_base.h"

namespace netlist
{
namespace devices
{

	// FIXME: timing is not 100% accurate, Strobe and Select inputs have a
	//        slightly longer timing .
	// FIXME: Truthtable candidate

	NETLIB_OBJECT(74153)
	{
		NETLIB_CONSTRUCTOR(74153)
		, m_C(*this, {"C0", "C1", "C2", "C3"}, NETLIB_DELEGATE(sub))
		, m_G(*this, "G", NETLIB_DELEGATE(sub))
		, m_Y(*this, "AY") //FIXME: Change netlists
		, m_chan(*this, "m_chan", 0)
		, m_A(*this, "A", NETLIB_DELEGATE(other))
		, m_B(*this, "B", NETLIB_DELEGATE(other))
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_chan = 0;
		}

		NETLIB_HANDLERI(other)
		{
			m_chan = (m_A() | (m_B()<<1));
			sub();
		}

		NETLIB_HANDLERI(sub)
		{
			constexpr const std::array<netlist_time, 2> delay = { NLTIME_FROM_NS(23), NLTIME_FROM_NS(18) };
			if (!m_G())
			{
				auto t = m_C[m_chan]();
				m_Y.push(t, delay[t]);
			}
			else
			{
				m_Y.push(0, delay[0]);
			}
		}

	public:
		object_array_t<logic_input_t, 4> m_C;
		logic_input_t m_G;

		logic_output_t m_Y;

		state_var<unsigned> m_chan;

		logic_input_t m_A;
		logic_input_t m_B;

		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(74153, "TTL_74153", "+C0,+C1,+C2,+C3,+A,+B,+G,@VCC,@GND")

} //namespace devices
} // namespace netlist
