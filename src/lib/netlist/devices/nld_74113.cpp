// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_74113.cpp
 *
 *  74113: Dual Master-Slave J-K Flip-Flops with Set and Complementary Outputs
 *  74113A: Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops with Set and Complementary Outputs
 *
 *          +----------+
 *     1CLK |1   ++  14| VCC
 *       1K |2       13| 2CLK
 *       1J |3       12| 2K
 *    1SETQ |4 74113 11| 2J
 *       1Q |5       10| 2SETQ
 *      1QQ |6        9| 2Q
 *      GND |7        8| 2QQ
 *          +----------+
 *
 *
 *          Function table 113
 *
 *          +-----+-----+-----+---++---+-----+
 *          | SETQ| CLK |  J  | K || Q | QQ  |
 *          +=====+=====+=====+===++===+=====+
 *          |  0  |  X  |  X  | X || 1 |  0  |
 *          |  1  |  *  |  0  | 0 || Q0| Q0Q |
 *          |  1  |  *  |  1  | 0 || 1 |  0  |
 *          |  1  |  *  |  0  | 1 || 0 |  1  |
 *          |  1  |  *  |  1  | 1 || TOGGLE  |
 *          +-----+-----+-----+---++---+-----+
 *                _
 *          * = _| |_
 *
 *          This is positive triggered, J and K
 *          are latched during clock high and
 *          transferred when CLK falls.
 *
 *          Function table 113A
 *
 *          +-----+-----+-----+---++---+-----+
 *          | CLRQ| CLK |  J  | K || Q | QQ  |
 *          +=====+=====+=====+===++===+=====+
 *          |  0  |  X  |  X  | X || 0 |  1  |
 *          |  1  |  F  |  0  | 0 || Q0| Q0Q |
 *          |  1  |  F  |  1  | 0 || 1 |  0  |
 *          |  1  |  F  |  0  | 1 || 0 |  1  |
 *          |  1  |  F  |  1  | 1 || TOGGLE  |
 *          |  1  |  1  |  X  | X || Q0| Q0Q |
 *          +-----+-----+-----+---++---+-----+
 *
 *          THe 113A is negative triggered.
 *
 *  Naming conventions follow Texas instruments datasheet
 *
 *  FIXME: Currently, only the 113 is implemented.
 *         The 113A uses the same model.
 *
 */

#include "nl_base.h"

// FIXME: this can probably be merged with nld_7473.cpp
// FIXME: timing, see 74107 for example, use template

namespace netlist::devices {

	NETLIB_OBJECT(74113)
	{
		NETLIB_CONSTRUCTOR(74113)
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(inputs))
		, m_J(*this, "J", NETLIB_DELEGATE(inputs))
		, m_K(*this, "K", NETLIB_DELEGATE(inputs))
		, m_SETQ(*this, "SETQ", NETLIB_DELEGATE(inputs))
		, m_last_CLK(*this, "m_last_CLK", 0)
		, m_q(*this, "m_q", 0)
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_last_CLK = 0;
		}

		NETLIB_HANDLERI(inputs)
		{
			const auto JK = (m_J() << 1) | m_K();

			if (m_SETQ())
			{
				if (!m_CLK() && m_last_CLK)
				{
					switch (JK)
					{
						case 1:             // (!m_J) & m_K))
							m_q = 0;
							break;
						case 2:             // (m_J) & !m_K))
							m_q = 1;
							break;
						case 3:             // (m_J) & m_K))
							m_q ^= 1;
							break;
						default:
						case 0:
							break;
					}
				}
			}
			else
			{
				m_q = 1;
			}

			m_last_CLK = m_CLK();

			m_Q.push(m_q, NLTIME_FROM_NS(20)); // FIXME: timing
			m_QQ.push(m_q ^ 1, NLTIME_FROM_NS(20)); // FIXME: timing
		}

		logic_input_t m_CLK;
		logic_input_t m_J;
		logic_input_t m_K;
		logic_input_t m_SETQ;

		state_var<unsigned> m_last_CLK;
		state_var<unsigned> m_q;

		logic_output_t m_Q;
		logic_output_t m_QQ;
		nld_power_pins m_power_pins;
	};

	using NETLIB_NAME(74113A) = NETLIB_NAME(74113);

	NETLIB_DEVICE_IMPL(74113, "TTL_74113", "+CLK,+J,+K,+CLRQ,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74113A, "TTL_74113A", "+CLK,+J,+K,+CLRQ,@VCC,@GND")

} // namespace netlist::devices
