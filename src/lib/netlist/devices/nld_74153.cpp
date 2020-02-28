// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74153.c
 *
 */

#include "nld_74153.h"
#include "netlist/nl_base.h"
#include "nlid_system.h"

namespace netlist
{
namespace devices
{

	/* FIXME: timing is not 100% accurate, Strobe and Select inputs have a
	 *        slightly longer timing.
	 *        Convert this to sub-devices at some time.
	 */

	NETLIB_OBJECT(74153)
	{
		NETLIB_CONSTRUCTOR(74153)
		, m_C(*this, {{"C0", "C1", "C2", "C3"}}, NETLIB_DELEGATE(74153, sub))
		, m_G(*this, "G", NETLIB_DELEGATE(74153, sub))
		, m_Y(*this, "AY") //FIXME: Change netlists
		, m_chan(*this, "m_chan", 0)
		, m_A(*this, "A")
		, m_B(*this, "B")
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_chan = 0;
		}

		NETLIB_UPDATEI()
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

	NETLIB_OBJECT(74153_dip)
	{
		NETLIB_CONSTRUCTOR(74153_dip)
		, m_A(*this, "A")
		, m_B(*this, "B")
		{
			register_subalias("1", m_A.m_G);
			register_subalias("2", m_A.m_B);
			register_subalias("3", m_A.m_C[3]);
			register_subalias("4", m_A.m_C[2]);
			register_subalias("5", m_A.m_C[1]);
			register_subalias("6", m_A.m_C[0]);
			register_subalias("7", m_A.m_Y);
			register_subalias("8", "A.GND");

			register_subalias("9", m_B.m_Y);
			register_subalias("10", m_B.m_C[0]);
			register_subalias("11", m_B.m_C[1]);
			register_subalias("12", m_B.m_C[2]);
			register_subalias("13", m_B.m_C[3]);
			register_subalias("14", m_A.m_A);

			register_subalias("15", m_B.m_G);
			register_subalias("16", "A.VCC");

			connect("A.GND", "B.GND");
			connect("A.VCC", "B.VCC");
			connect(m_A.m_A, m_B.m_A);
			connect(m_A.m_B, m_B.m_B);
		}
		//NETLIB_RESETI();
		//NETLIB_UPDATEI();

	protected:
		NETLIB_SUB(74153) m_A;
		NETLIB_SUB(74153) m_B;
	};

	NETLIB_DEVICE_IMPL(74153, "TTL_74153", "+C0,+C1,+C2,+C3,+A,+B,+G,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74153_dip, "TTL_74153_DIP", "")

} //namespace devices
} // namespace netlist
