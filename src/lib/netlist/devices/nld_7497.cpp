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
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(7497, clk_strb))
		, m_STRB(*this, "STRB", NETLIB_DELEGATE(7497, clk_strb))
		, m_EN(*this, "EN")
		, m_UNITY(*this, "UNITY", NETLIB_DELEGATE(7497, unity))
		, m_CLR(*this, "CLR", NETLIB_DELEGATE(7497, clr))
		, m_Y(*this, "Y")
		, m_Z(*this, "Z")
		, m_ENOUT(*this, "ENOUT")
		, m_cnt(*this, "_m_cnt", 0)
		, m_rate(*this, "_m_rate", 0)
		, m_state(*this, "_m_state", 0)
		, m_lastclock(*this, "_m_lastclock", 0)
		{
		}

	private:
		NETLIB_RESETI();
		NETLIB_UPDATEI();

		NETLIB_HANDLERI(noop) { }
		NETLIB_HANDLERI(unity);
		NETLIB_HANDLERI(clr);
		NETLIB_HANDLERI(clk_strb);

		object_array_t<logic_input_t, 6> m_B;
		logic_input_t m_CLK;
		logic_input_t m_STRB;
		logic_input_t m_EN;
		logic_input_t m_UNITY;
		logic_input_t m_CLR;

		logic_output_t m_Y;
		logic_output_t m_Z;
		logic_output_t m_ENOUT;

		state_var_u8 m_cnt;
		state_var_u8 m_rate;
		state_var_sig m_state;
		state_var_sig m_lastclock;

		void newstate(const netlist_sig_t state)
		{
			m_state = state;
			m_Z.push(state, out_delay_CLK_Z[state]);
			netlist_sig_t y = (state ^ 1) | (m_UNITY() ^ 1); // OR with negated inputs == NAND
			m_Y.push(y, out_delay_CLK_Y[y]);
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
		m_cnt = 0;
		m_rate = 0;
		m_lastclock = 0;
	}

	NETLIB_UPDATE(7497)
	{
		m_rate = rate();
	}

	NETLIB_HANDLER(7497, unity)
	{
		newstate (m_state);
	}

	NETLIB_HANDLER(7497, clr)
	{
		m_cnt = 0;
		clk_strb();
	}

	NETLIB_HANDLER(7497, clk_strb)
	{
		netlist_sig_t clk = m_CLK();

		if (!m_lastclock && clk && !m_EN() && !m_CLR())
		{
			m_cnt++;
			m_cnt &= 63;
		}

		const netlist_sig_t clk_strb = (clk ^ 1) & (m_STRB() ^ 1);

		const netlist_sig_t cntQ = m_cnt;
		netlist_sig_t p1 = ((cntQ & 63)  == 31  && (m_rate & 32)) ||
			((cntQ & 31)  == 15  && (m_rate & 16)) ||
			((cntQ & 15)  == 7  && (m_rate & 8))  ||
			((cntQ & 7) == 3  && (m_rate & 4))  ||
			((cntQ & 3) == 1 && (m_rate & 2))  ||
			((cntQ & 1) == 0 && (m_rate & 1));

		p1 = (p1 & clk_strb) ^ 1;

		newstate(p1);

		if (m_cnt == 63 && !m_EN())
			m_ENOUT.push(0, out_delay_CLK_Y[0]); // XXX timing
		else
			m_ENOUT.push(1, out_delay_CLK_Y[1]);

	}

	NETLIB_DEVICE_IMPL(7497)

	} //namespace devices
} // namespace netlist
