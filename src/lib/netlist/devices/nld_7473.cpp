// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_7473.cpp
 *
 *  7473: Dual Master-Slave J-K Flip-Flops with Clear and Complementary Outputs
 *  7473A: Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops with Clear and Complementary Outputs
 *
 *          +----------+
 *     1CLK |1   ++  14| 1J
 *    1CLRQ |2       13| 1QQ
 *       1K |3       12| 1Q
 *      VCC |4  7473 11| GND
 *     2CLK |5       10| 2K
 *    2CLRQ |6        9| 2Q
 *       2J |7        8| 2QQ
 *          +----------+
 *
 *
 *          Function table 73
 *
 *          +-----+-----+-----+---++---+-----+
 *          | CLRQ| CLK |  J  | K || Q | QQ  |
 *          +=====+=====+=====+===++===+=====+
 *          |  0  |  X  |  X  | X || 0 |  1  |
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
 *          Function table 73A
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
 *          THe 73A is negative triggered.
 *
 *  Naming conventions follow Texas instruments datasheet
 *
 *  FIXME: Currently, only the 73 is implemented.
 *         The 73A uses the same model.
 *
 */

#include "nl_base.h"

namespace netlist::devices {

	NETLIB_OBJECT(7473)
	{
		NETLIB_CONSTRUCTOR(7473)
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(inputs))
		, m_J(*this, "J", NETLIB_DELEGATE(inputs))
		, m_K(*this, "K", NETLIB_DELEGATE(inputs))
		, m_CLRQ(*this, "CLRQ", NETLIB_DELEGATE(inputs))
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

			if (m_CLRQ())
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

			m_last_CLK = m_CLK();

			m_Q.push(m_q, NLTIME_FROM_NS(20)); // FIXME: timing
			m_QQ.push(m_q ^ 1, NLTIME_FROM_NS(20)); // FIXME: timing
		}

		logic_input_t m_CLK;
		logic_input_t m_J;
		logic_input_t m_K;
		logic_input_t m_CLRQ;

		state_var<unsigned> m_last_CLK;
		state_var<unsigned> m_q;

		logic_output_t m_Q;
		logic_output_t m_QQ;
		nld_power_pins m_power_pins;
	};

	using NETLIB_NAME(7473A) = NETLIB_NAME(7473);

	NETLIB_DEVICE_IMPL(7473, "TTL_7473", "+CLK,+J,+K,+CLRQ,@VCC,@GND")
	NETLIB_DEVICE_IMPL(7473A, "TTL_7473A", "+CLK,+J,+K,+CLRQ,@VCC,@GND")

} // namespace netlist::devices
