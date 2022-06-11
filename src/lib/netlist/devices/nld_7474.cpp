
// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_7474.cpp
 *
 *  DM7474: Dual Positive-Edge-Triggered D Flip-Flops
 *          with Preset, Clear and Complementary Outputs
 *
 *          +--------------+
 *     CLR1 |1     ++    14| VCC
 *       D1 |2           13| CLR2
 *     CLK1 |3           12| D2
 *      PR1 |4    7474   11| CLK2
 *       Q1 |5           10| PR2
 *      Q1Q |6            9| Q2
 *      GND |7            8| Q2Q
 *          +--------------+
 *
 *          +-----+-----+-----+---++---+-----+
 *          | PR  | CLR | CLK | D || Q | QQ  |
 *          +=====+=====+=====+===++===+=====+
 *          |  0  |  1  |  X  | X || 1 |  0  |
 *          |  1  |  0  |  X  | X || 0 |  1  |
 *          |  0  |  0  |  X  | X || 1 |  1  | (*)
 *          |  1  |  1  |  R  | 1 || 1 |  0  |
 *          |  1  |  1  |  R  | 0 || 0 |  1  |
 *          |  1  |  1  |  0  | X || Q0| Q0Q |
 *          +-----+-----+-----+---++---+-----+
 *
 *  (*) This configuration is not stable, i.e. it will not persist
 *  when either the preset and or clear inputs return to their inactive (high) level
 *
 *  Q0 The output logic level of Q before the indicated input conditions were established
 *
 *  R:  0 -. 1
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 *  FIXME: Check that (*) is emulated properly
 */

#include "nl_base.h"

#include <array>

namespace netlist::devices {

	NETLIB_OBJECT(7474)
	{
		NETLIB_CONSTRUCTOR(7474)
		, m_D(*this, "D", NETLIB_DELEGATE(inputs))
		, m_CLRQ(*this, "CLRQ", NETLIB_DELEGATE(inputs))
		, m_PREQ(*this, "PREQ", NETLIB_DELEGATE(inputs))
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_nextD(*this, "m_nextD", 0)
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_CLK.set_state(logic_t::STATE_INP_LH);
			m_D.set_state(logic_t::STATE_INP_ACTIVE);
			m_nextD = 0;
		}

		NETLIB_HANDLERI(clk)
		{
			newstate(m_nextD, !m_nextD);
			m_CLK.inactivate();
		}

		NETLIB_HANDLERI(inputs)
		{
			const auto preq(m_PREQ());
			const auto clrq(m_CLRQ());
			if (preq & clrq)
			{
				m_D.activate();
				m_nextD = m_D();
				m_CLK.activate_lh();
			}
			else
			{
				newstate(preq ^ 1, clrq ^ 1);
				m_CLK.inactivate();
				m_D.inactivate();
			}
		}

		logic_input_t m_D;
		logic_input_t m_CLRQ;
		logic_input_t m_PREQ;
		logic_input_t m_CLK;
		logic_output_t m_Q;
		logic_output_t m_QQ;

		state_var<netlist_sig_t> m_nextD;

		nld_power_pins m_power_pins;

		void newstate(const netlist_sig_t stateQ, const netlist_sig_t stateQQ) noexcept
		{
			// 0: High-to-low 40 ns, 1: Low-to-high 25 ns
			static constexpr const std::array<netlist_time, 2> delay = { NLTIME_FROM_NS(40), NLTIME_FROM_NS(25) };
			m_Q.push(stateQ, delay[stateQ]);
			m_QQ.push(stateQQ, delay[stateQQ]);
		}
	};

	NETLIB_DEVICE_IMPL(7474, "TTL_7474", "+CLK,+D,+CLRQ,+PREQ,@VCC,@GND")

} // namespace netlist::devices
