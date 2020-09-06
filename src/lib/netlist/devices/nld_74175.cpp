// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74175.cpp
 *
 *  DM74175: Quad D Flip-Flops with Clear
 *
 *          +--------------+
 *      CLR |1     ++    16| VCC
 *       Q1 |2           15| Q4
 *      Q1Q |3           14| Q4Q
 *       D1 |4   74175   13| D4
 *       D2 |5           12| D3
 *      Q2Q |6           11| Q3Q
 *       Q2 |7           10| Q3
 *      GND |8            9| CLK
 *          +--------------+
 *
 *          +-----+-----+---++---+-----+
 *          | CLR | CLK | D || Q | QQ  |
 *          +=====+=====+===++===+=====+
 *          |  0  |  X  | X || 0 |  1  |
 *          |  1  |  R  | 1 || 1 |  0  |
 *          |  1  |  R  | 0 || 0 |  1  |
 *          |  1  |  0  | X || Q0| Q0Q |
 *          +-----+-----+---++---+-----+
 *
 *   Q0 The output logic level of Q before the indicated input conditions were established
 *
 *  R:  0 -> 1
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#include "nl_base.h"

// FIXME: optimize

namespace netlist
{
	namespace devices
	{

	static constexpr const std::array<netlist_time, 2> delay = { NLTIME_FROM_NS(25), NLTIME_FROM_NS(25) };
	static constexpr const std::array<netlist_time, 2> delay_clear = { NLTIME_FROM_NS(40), NLTIME_FROM_NS(25) };


	NETLIB_OBJECT(74175)
	{
		NETLIB_CONSTRUCTOR(74175)
		, m_D(*this, {"D1", "D2", "D3", "D4"}, NETLIB_DELEGATE(other))
		, m_CLRQ(*this, "CLRQ", NETLIB_DELEGATE(other))
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_Q(*this, {"Q1", "Q2", "Q3", "Q4"})
		, m_QQ(*this, {"Q1Q", "Q2Q", "Q3Q", "Q4Q"})
		, m_data(*this, "m_data", 0)
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_CLK.set_state(logic_t::STATE_INP_LH);
			m_data = 0xFF;
		}

		NETLIB_HANDLERI(other)
		{
			uint_fast8_t d = 0;
			for (std::size_t i=0; i<4; i++)
			{
				d |= (m_D[i]() << i);
			}
			if (!m_CLRQ())
			{
				for (std::size_t i=0; i<4; i++)
				{
					m_Q[i].push(0, delay_clear[0]);
					m_QQ[i].push(1, delay_clear[1]);
				}
				m_data = 0;
			} else if (d != m_data)
			{
				m_data = d;
				m_CLK.activate_lh();
			}
		}

		NETLIB_HANDLERI(clk)
		{
			if (m_CLRQ())
			{
				for (std::size_t i=0; i<4; i++)
				{
					netlist_sig_t d = (m_data >> i) & 1;
					m_Q[i].push(d, delay[d]);
					m_QQ[i].push(d ^ 1, delay[d ^ 1]);
				}
				m_CLK.inactivate();
			}
		}

		object_array_t<logic_input_t, 4> m_D;
		logic_input_t m_CLRQ;

		logic_input_t m_CLK;
		object_array_t<logic_output_t, 4> m_Q;
		object_array_t<logic_output_t, 4> m_QQ;

		state_var<unsigned>      m_data;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(74175,   "TTL_74175", "+CLK,+D1,+D2,+D3,+D4,+CLRQ,@VCC,@GND")

	} //namespace devices
} // namespace netlist
