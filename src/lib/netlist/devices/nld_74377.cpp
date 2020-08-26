// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74377.cpp
 *
 *  DM74377: Octal D Flip-Flop With Enable
 *
 *          +--------------+
 *       /E |1     ++    20| VCC
 *       Q0 |2           19| Q7
 *       D0 |3           18| D7
 *       D1 |4   74377   17| D6
 *       Q1 |5           16| Q6
 *       Q2 |6           15| Q5
 *       D2 |7           14| D5
 *       D3 |8           13| D4
 *       Q3 |9           12| Q4
 *      GND |10          11| CP
 *          +--------------+
 *
 *  DM74378: Hex D Flip-Flop With Enable
 *
 *          +--------------+
 *       /E |1     ++    16| VCC
 *       Q0 |2           15| Q5
 *       D0 |3           14| D5
 *       D1 |4   74378   13| D4
 *       Q1 |5           12| Q4
 *       D2 |6           11| D3
 *       Q2 |7           10| Q3
 *      GND |8            9| CP
 *          +--------------+
 *
 *  DM74379: 4-bit D Flip-Flop With Enable
 *
 *          +--------------+
 *       /E |1     ++    16| VCC
 *       Q0 |2           15| Q3
 *      /Q0 |3           14| /Q3
 *       D0 |4   74379   13| D3
 *       D1 |5           12| D2
 *      /Q1 |6           11| /Q2
 *       Q1 |7           10| Q2
 *      GND |8            9| CP
 *          +--------------+
 *
 *  Naming conventions follow Motorola datasheet
 *
 */

#include "nld_74377.h"
#include "nl_base.h"

namespace netlist
{
	namespace devices
	{

	constexpr const std::array<netlist_time, 2> delay = { NLTIME_FROM_NS(25), NLTIME_FROM_NS(25) };

	NETLIB_OBJECT(74377_GATE)
	{
		NETLIB_CONSTRUCTOR(74377_GATE)
		, m_E(*this, "E", NETLIB_DELEGATE(inputs))
		, m_D(*this, "D", NETLIB_DELEGATE(inputs))
		, m_CP(*this, "CP", NETLIB_DELEGATE(inputs))
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_cp(*this, "m_cp", 0)
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
		}

	private:
		NETLIB_HANDLERI(inputs)
		{
			netlist_sig_t last_cp = m_cp;

			m_cp = m_CP();

			if (!m_E() && !last_cp && m_cp)
			{
				netlist_sig_t d = m_D();
				m_Q.push(d, delay[d]);
				m_QQ.push(d ^ 1, delay[d ^ 1]);
			}
		}

		logic_input_t m_E;
		logic_input_t m_D;
		logic_input_t m_CP;
		logic_output_t m_Q;
		logic_output_t m_QQ;

		state_var_sig m_cp;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(74377_GATE, "TTL_74377_GATE", "")

	} //namespace devices
} // namespace netlist
