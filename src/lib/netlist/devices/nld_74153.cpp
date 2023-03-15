// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_74153.cpp
 *
 *  DM74153: Dual 4-Line to 1-Line Data Selectors Multiplexers
 *
 *          +--------------+
 *       G1 |1     ++    16| VCC
 *        B |2           15| G2
 *      1C3 |3           14| A
 *      1C2 |4   74153   13| 2C3
 *      1C1 |5           12| 2C2
 *      1C0 |6           11| 2C1
 *       Y1 |7           10| 2C0
 *      GND |8            9| Y2
 *          +--------------+
 *
 *
 *          Function table
 *
 *          +-----+-----++----+----+----+----++----+----+
 *          |  B  |  A  || C0 | C1 | C2 | C3 ||  G |  Y |
 *          +=====+=====++====+====+====+====++====+====+
 *          |  X  |  X  ||  X |  X |  X |  X ||  H |  L |
 *          |  L  |  L  ||  L |  X |  X |  X ||  L |  L |
 *          |  L  |  L  ||  H |  X |  X |  X ||  L |  H |
 *          |  L  |  H  ||  X |  L |  X |  X ||  L |  L |
 *          |  L  |  H  ||  X |  H |  X |  X ||  L |  H |
 *          |  H  |  L  ||  X |  X |  L |  X ||  L |  L |
 *          |  H  |  L  ||  X |  X |  H |  X ||  L |  H |
 *          |  H  |  H  ||  X |  X |  X |  L ||  L |  L |
 *          |  H  |  H  ||  X |  X |  X |  H ||  L |  H |
 *          +-----+-----++----+----+----+----++----+----+
 *
 *  A, B : Select Inputs
 *  C*   : Data inputs
 *  G    : Strobe
 *  Y    : Output
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#include "nl_base.h"

namespace netlist::devices {

	// FIXME: timing is not 100% accurate, Strobe and Select inputs have a
	//        slightly longer timing .
	// FIXME: Truth table candidate

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

	private:
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

		object_array_t<logic_input_t, 4> m_C;
		logic_input_t m_G;

		logic_output_t m_Y;

		state_var<unsigned> m_chan;

		logic_input_t m_A;
		logic_input_t m_B;

		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(74153, "TTL_74153", "+C0,+C1,+C2,+C3,+A,+B,+G,@VCC,@GND")

} // namespace netlist::devices
