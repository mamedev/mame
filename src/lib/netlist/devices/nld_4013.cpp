// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_4013.cpp
 *
 *  CD4013: Dual Positive-Edge-Triggered D Flip-Flops
 *          with Set, Reset and Complementary Outputs
 *
 *          +--------------+
 *       Q1 |1     ++    14| VDD
 *      Q1Q |2           13| Q2
 *   CLOCK1 |3           12| Q2Q
 *   RESET1 |4    4013   11| CLOCK2
 *    DATA1 |5           10| RESET2
 *     SET1 |6            9| DATA2
 *      VSS |7            8| SET2
 *          +--------------+
 *
 *          +-----+-----+-----+---++---+-----+
 *          | SET | RES | CLK | D || Q | QQ  |
 *          +=====+=====+=====+===++===+=====+
 *          |  1  |  0  |  X  | X || 1 |  0  |
 *          |  0  |  1  |  X  | X || 0 |  1  |
 *          |  1  |  1  |  X  | X || 1 |  1  | (*)
 *          |  0  |  0  |  R  | 1 || 1 |  0  |
 *          |  0  |  0  |  R  | 0 || 0 |  1  |
 *          |  0  |  0  |  0  | X || Q0| Q0Q |
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
#include "nl_factory.h"

namespace netlist::devices {

	NETLIB_OBJECT(CD4013)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4013, "CD4XXX")
		, m_D(*this, "DATA", NETLIB_DELEGATE(inputs))
		, m_RESET(*this, "RESET", NETLIB_DELEGATE(inputs))
		, m_SET(*this, "SET", NETLIB_DELEGATE(inputs))
		, m_CLK(*this, "CLOCK", NETLIB_DELEGATE(clk))
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

		NETLIB_HANDLERI(inputs)
		{
			const auto set(m_SET());
			const auto reset(m_RESET());
			if ((set ^ 1) & (reset ^ 1))
			{
				m_D.activate();
				m_nextD = m_D();
				m_CLK.activate_lh();
			}
			else
			{
				set_reset(set, reset);
				m_CLK.inactivate();
				m_D.inactivate();
			}
		}

		NETLIB_HANDLERI(clk)
		{
			set_output(m_nextD);
			m_CLK.inactivate();
		}

		logic_input_t m_D;
		logic_input_t m_RESET;
		logic_input_t m_SET;
		logic_input_t m_CLK;
		logic_output_t m_Q;
		logic_output_t m_QQ;

		state_var<netlist_sig_t> m_nextD;

		nld_power_pins m_power_pins;

		void set_output(const netlist_sig_t stateQ)
		{
			static constexpr const auto delay = NLTIME_FROM_NS(150);
			m_Q.push(stateQ, delay);
			m_QQ.push(!stateQ, delay);
		}

		void set_reset(const netlist_sig_t stateQ, const netlist_sig_t stateQQ)
		{
			// Q: 150 ns, QQ: 200 ns
			static constexpr const std::array<netlist_time, 2> delay = { NLTIME_FROM_NS(150), NLTIME_FROM_NS(200) };
			m_Q.push(stateQ, delay[0]);
			m_QQ.push(stateQQ, delay[1]);
		}
	};

	NETLIB_DEVICE_IMPL(CD4013, "CD4013", "+CLOCK,+DATA,+RESET,+SET,@VDD,@VSS")

} // namespace netlist::devices
