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
#include "netlist/nl_base.h"
#include "nlid_system.h"

namespace netlist
{
	namespace devices
	{

	static constexpr const std::array<netlist_time, 2> out_delay_CLK_Y = { NLTIME_FROM_NS(20), NLTIME_FROM_NS(26) }; // tPHL, tPLH
	static constexpr const std::array<netlist_time, 2> out_delay_CLK_Z = { NLTIME_FROM_NS(17), NLTIME_FROM_NS(12) };

	NETLIB_OBJECT(7497)
	{
		NETLIB_CONSTRUCTOR(7497)
		, m_B(*this, {{"B5", "B4", "B3", "B2", "B1", "B0"}})
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(7497, clk_strb))
		, m_STRBQ(*this, "STRBQ", NETLIB_DELEGATE(7497, clk_strb))
		, m_ENQ(*this, "ENQ")
		, m_UNITYQ(*this, "UNITYQ", NETLIB_DELEGATE(7497, unity))
		, m_CLR(*this, "CLR", NETLIB_DELEGATE(7497, clr))
		, m_Y(*this, "Y")
		, m_ZQ(*this, "ZQ")
		, m_ENOUTQ(*this, "ENOUTQ")
		, m_cnt(*this, "_m_cnt", 0)
		, m_rate(*this, "_m_rate", 0)
		, m_state(*this, "_m_state", 0)
		, m_lastclock(*this, "_m_lastclock", 0)
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI();
		NETLIB_UPDATEI();

		NETLIB_HANDLERI(noop) { }
		NETLIB_HANDLERI(unity);
		NETLIB_HANDLERI(clr);
		NETLIB_HANDLERI(clk_strb);

	protected:
		object_array_t<logic_input_t, 6> m_B;
		logic_input_t m_CLK;
		logic_input_t m_STRBQ;
		logic_input_t m_ENQ;
		logic_input_t m_UNITYQ;
		logic_input_t m_CLR;

		logic_output_t m_Y;
		logic_output_t m_ZQ;
		logic_output_t m_ENOUTQ;

		state_var_u8 m_cnt;
		state_var_u8 m_rate;
		state_var_sig m_state;
		state_var_sig m_lastclock;
		nld_power_pins m_power_pins;

		void newstate(const netlist_sig_t state)
		{
			m_state = state;
			m_ZQ.push(state, out_delay_CLK_Z[state]);
			//netlist_sig_t y = (state ^ 1) | (m_UNITY() ^ 1); // OR with negated inputs == NAND
			netlist_sig_t y = (state & m_UNITYQ()) ^ 1; // OR with negated inputs == NAND
			m_Y.push(y, out_delay_CLK_Y[y]);
		}

		uint8_t rate()
		{
			uint8_t a = 0;

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
		clk_strb();
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

		if (!m_lastclock && clk && !m_ENQ() && !m_CLR())
		{
			m_cnt++;
			m_cnt &= 63;
		}
		m_lastclock = clk;

		const netlist_sig_t clk_strb = (clk ^ 1) & (m_STRBQ() ^ 1);

		const netlist_sig_t cntQ = m_cnt;

		// NOR GATE
		netlist_sig_t p1 = ((cntQ & 63)  == 31  && (m_rate & 32)) ||
			((cntQ & 31)  == 15  && (m_rate & 16)) ||
			((cntQ & 15)  == 7  && (m_rate & 8))  ||
			((cntQ & 7) == 3  && (m_rate & 4))  ||
			((cntQ & 3) == 1 && (m_rate & 2))  ||
			((cntQ & 1) == 0 && (m_rate & 1));

		p1 = (p1 & clk_strb) ^ 1;

		newstate(p1);

		// NAND gate
		if ((m_cnt == 63) && !m_ENQ())
			m_ENOUTQ.push(0, out_delay_CLK_Y[0]); // XXX timing
		else
			m_ENOUTQ.push(1, out_delay_CLK_Y[1]);

	}

	NETLIB_OBJECT_DERIVED(7497_dip, 7497)
	{
		NETLIB_CONSTRUCTOR_DERIVED(7497_dip, 7497)
		{
			register_subalias("1", m_B[4]);  // B0
			register_subalias("2", m_B[1]);  // B4
			register_subalias("3", m_B[0]);  // B5
			register_subalias("4", m_B[5]);  // B0
			register_subalias("5", m_ZQ);
			register_subalias("6", m_Y);
			register_subalias("7", m_ENOUTQ);
			register_subalias("8", "GND");

			register_subalias("9", m_CLK);
			register_subalias("10", m_STRBQ);
			register_subalias("11", m_UNITYQ);
			register_subalias("12", m_ENQ);
			register_subalias("13", m_CLR);
			register_subalias("14", m_B[3]); // B2
			register_subalias("15", m_B[2]); // B3
			register_subalias("16", "VCC");
		}
	};


	NETLIB_DEVICE_IMPL(7497,      "TTL_7497", "+CLK,+STRBQ,+ENQ,+UNITYQ,+CLR,+B0,+B1,+B2,+B3,+B4,+B5,@VCC,@GND")
	NETLIB_DEVICE_IMPL(7497_dip,  "TTL_7497_DIP", "")

	} //namespace devices
} // namespace netlist
