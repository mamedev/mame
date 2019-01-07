// license:GPL-2.0+
// copyright-holders:Sergey Svishchev
/*
 * nld_7497.cpp
 *
 * To do:
 *
 * - STRB and EN
 * - Timing
 */

#include "nld_7497.h"
#include "../nl_base.h"

namespace netlist
{
	namespace devices
	{

	static constexpr netlist_time out_delay_CLK_Y[2] = { NLTIME_FROM_NS(20), NLTIME_FROM_NS(26) }; // tPHL, tPLH
	static constexpr netlist_time out_delay_CLK_Z[2] = { NLTIME_FROM_NS(17), NLTIME_FROM_NS(12) };

	NETLIB_OBJECT(7497)
	{
		NETLIB_CONSTRUCTOR(7497)
		, m_B(*this, {{"B0", "B1", "B2", "B3", "B4", "B5"}})
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(7497, clk))
		, m_STRB(*this, "STRB")
		, m_EN(*this, "EN")
		, m_UNITY(*this, "UNITY", NETLIB_DELEGATE(7497, unity))
		, m_CLR(*this, "CLR", NETLIB_DELEGATE(7497, clr))
		, m_Y(*this, "Y")
		, m_Z(*this, "Z")
		, m_ENOUT(*this, "ENOUT")
		, m_reset(*this, "_m_reset", 0)
		, m_a(*this, "_m_a", 0)
		, m_rate(*this, "_m_rate", 0)
		, m_state(*this, "_m_state", 0)
		{
		}

	private:
		NETLIB_RESETI();
		NETLIB_UPDATEI();

		NETLIB_HANDLERI(noop) { }
		NETLIB_HANDLERI(unity);
		NETLIB_HANDLERI(clr);
		NETLIB_HANDLERI(clk);

		object_array_t<logic_input_t, 6> m_B;
		logic_input_t m_CLK;
		logic_input_t m_STRB;
		logic_input_t m_EN;
		logic_input_t m_UNITY;
		logic_input_t m_CLR;

		logic_output_t m_Y;
		logic_output_t m_Z;
		logic_output_t m_ENOUT;

		state_var_sig m_reset;
		state_var_sig m_a;
		state_var_sig m_rate;
		state_var_sig m_state;

		void newstate(const netlist_sig_t state)
		{
			m_state = state;
			m_Z.push(state ^ 1, out_delay_CLK_Z[state ^ 1]);
			if (m_UNITY())
				m_Y.push(state, out_delay_CLK_Y[state]);
			else
				m_Y.push(1, out_delay_CLK_Y[1]);

			if (m_CLK())
				m_CLK.set_state(logic_t::STATE_INP_HL);
			else
				m_CLK.set_state(logic_t::STATE_INP_LH);
		}

		int rate()
		{
			int a = 0;

			for (std::size_t i = 0; i < 6; i++)
				a |= (m_B[i]() << i);

			return a;
		}
	};

	NETLIB_RESET(7497)
	{
		m_reset = 1;
		m_a = 0;
		m_rate = 0;
		m_CLK.set_state(logic_t::STATE_INP_HL);
		m_B[0].set_state(logic_t::STATE_INP_LH);
		m_B[1].set_state(logic_t::STATE_INP_LH);
		m_B[2].set_state(logic_t::STATE_INP_LH);
		m_B[3].set_state(logic_t::STATE_INP_LH);
		m_B[4].set_state(logic_t::STATE_INP_LH);
		m_B[5].set_state(logic_t::STATE_INP_LH);
		m_STRB.set_state(logic_t::STATE_INP_HL);
#if 0
		m_EN.set_state(logic_t::STATE_INP_HL);
#endif
		m_UNITY.set_state(logic_t::STATE_INP_LH);
		m_CLR.set_state(logic_t::STATE_INP_LH);
		newstate(0);
		m_ENOUT.push(1, out_delay_CLK_Y[1]);
	}

	NETLIB_UPDATE(7497)
	{
//		m_reset = m_CLR() ^ 1;

		// m_reset = 1 -- normal operation
		if (!m_reset)
		{
			m_CLK.inactivate();
			m_Y.push_force(0, NLTIME_FROM_NS(24));
			m_Z.push_force(1, NLTIME_FROM_NS(15));
			m_ENOUT.push_force(1, NLTIME_FROM_NS(15)); // XXX
		}
	}

	NETLIB_HANDLER(7497, unity)
	{
		newstate (m_state);
	}

	NETLIB_HANDLER(7497, clr)
	{
		m_a = 0;
		newstate (0);
	}

	NETLIB_HANDLER(7497, clk)
	{
		netlist_sig_t clk = m_CLK();

		if (m_reset)
		{
			// lock rate on falling edge of CLK
			if (!clk) m_rate = rate();

			if (m_rate && !m_STRB())
			{
				if (
					((m_a & 1)  == 0  && (m_rate & 32)) ||
					((m_a & 3)  == 1  && (m_rate & 16)) ||
					((m_a & 7)  == 3  && (m_rate & 8))  ||
					((m_a & 15) == 7  && (m_rate & 4))  ||
					((m_a & 31) == 15 && (m_rate & 2))  ||
					((m_a & 63) == 31 && (m_rate & 1)))
					newstate(clk);
			}
			else
			{
				newstate(0);
			}

			if (m_a == 62)
				m_ENOUT.push(0, out_delay_CLK_Y[0]); // XXX timing
			else
				m_ENOUT.push(1, out_delay_CLK_Y[1]);

			if (clk)
			{
				m_CLK.set_state(logic_t::STATE_INP_HL);
				m_a++;
				m_a &= 63;
			}
			else
			{
				m_CLK.set_state(logic_t::STATE_INP_LH);
			}
		}
	}

	NETLIB_DEVICE_IMPL(7497)

	} //namespace devices
} // namespace netlist
