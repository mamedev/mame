// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_7475.cpp
 *
 * TODO: Correct timing for clock-induced state changes, rather than assuming timing is always due to data-induced state changes
 */

#include "nld_7475.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{
	template<bool _HasQQ>
	NETLIB_OBJECT(7475_GATE_BASE)
	{
		NETLIB_CONSTRUCTOR(7475_GATE_BASE)
		, m_D(*this, "D", NETLIB_DELEGATE(inputs))
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_nextD(*this, "m_nextD", 0)
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_CLK.set_state(logic_t::STATE_INP_LH);
			m_nextD = 0;
		}

		NETLIB_HANDLERI(clk)
		{
			newstate(m_nextD, !m_nextD);
			m_CLK.inactivate();
		}

		NETLIB_HANDLERI(inputs)
		{
			m_nextD = m_D();
			m_CLK.activate_lh();
		}

	private:
		logic_input_t m_D;
		logic_input_t m_CLK;
		logic_output_t m_Q;
		logic_output_t m_QQ;

		state_var<netlist_sig_t> m_nextD;

		nld_power_pins m_power_pins;

		void newstate(const netlist_sig_t stateQ, const netlist_sig_t stateQQ)
		{
			// 0: High-to-low 40 ns, 1: Low-to-high 25 ns
			static constexpr const std::array<netlist_time, 2> delay = { NLTIME_FROM_NS(40), NLTIME_FROM_NS(25) };
			m_Q.push(stateQ, delay[stateQ]);
			if (_HasQQ)
				m_QQ.push(stateQQ, delay[stateQQ]);
		}
	};

	using NETLIB_NAME(7475_GATE) = NETLIB_NAME(7475_GATE_BASE)<true>;
	using NETLIB_NAME(7477_GATE) = NETLIB_NAME(7475_GATE_BASE)<false>;

	NETLIB_DEVICE_IMPL(7475_GATE, "TTL_7475_GATE", "")
	NETLIB_DEVICE_IMPL(7477_GATE, "TTL_7477_GATE", "")

	} //namespace devices
} // namespace netlist
