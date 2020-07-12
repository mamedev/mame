// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4013.cpp
 *
 */

#include "netlist/nl_base.h"
#include "netlist/nl_factory.h"

namespace netlist
{
	namespace devices
	{

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

		NETLIB_UPDATEI()
		{
			inputs();
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
				newstate_setreset(set, reset);
				m_CLK.inactivate();
				m_D.inactivate();
			}
		}

		NETLIB_HANDLERI(clk)
		{
			newstate_clk(m_nextD);
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

		void newstate_clk(const netlist_sig_t stateQ)
		{
			static constexpr auto delay = NLTIME_FROM_NS(150);
			m_Q.push(stateQ, delay);
			m_QQ.push(!stateQ, delay);
		}

		void newstate_setreset(const netlist_sig_t stateQ, const netlist_sig_t stateQQ)
		{
			// Q: 150 ns, QQ: 200 ns
			static constexpr const std::array<netlist_time, 2> delay = { NLTIME_FROM_NS(150), NLTIME_FROM_NS(200) };
			m_Q.push(stateQ, delay[0]);
			m_QQ.push(stateQQ, delay[1]);
		}
	};

	NETLIB_OBJECT(CD4013_dip)
	{
		NETLIB_CONSTRUCTOR(CD4013_dip)
		, m_A(*this, "A")
		, m_B(*this, "B")
		{
			register_subalias("1", "A.Q");
			register_subalias("2", "A.QQ");
			register_subalias("3", "A.CLOCK");
			register_subalias("4", "A.RESET");
			register_subalias("5", "A.DATA");
			register_subalias("6", "A.SET");
			register_subalias("7", "A.VSS");

			register_subalias("8", "B.SET");
			register_subalias("9", "B.DATA");
			register_subalias("10", "B.RESET");
			register_subalias("11", "B.CLOCK");
			register_subalias("12", "B.QQ");
			register_subalias("13", "B.Q");
			register_subalias("14", "A.VDD");

			connect("A.VSS", "B.VSS");
			connect("A.VDD", "B.VDD");
		}
		NETLIB_UPDATEI();
		NETLIB_RESETI();

	private:
		NETLIB_SUB(CD4013) m_A;
		NETLIB_SUB(CD4013) m_B;
	};

	NETLIB_RESET(CD4013_dip)
	{
	}

	NETLIB_UPDATE(CD4013_dip)
	{
	}

	NETLIB_DEVICE_IMPL(CD4013, "CD4013", "+CLOCK,+DATA,+RESET,+SET,@VDD,@VSS")
	NETLIB_DEVICE_IMPL(CD4013_dip, "CD4013_DIP", "")

	} //namespace devices
} // namespace netlist
