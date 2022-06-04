// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*
 * nld_7497.cpp
 *
 * To do:
 *
 * - STRB and EN
 * - Timing
 *
 *  SN7497: Synchronous 6-Bit Binary Rate Multiplier
 *
 *          +--------------+
 *       B1 |1           16| VCC
 *       B4 |2           15| B3
 *       B5 |3           14| B2
 *       B0 |4    7497   13| CLR
 *        Z |5           12| UNITY/CAS
 *        Y |6           11| ENin (EN)
 *    ENout |7           10| STRB
 *      GND |8            9| CLK
 *          +--------------+
 *
 *  Naming conventions follow TI datasheet
 *
 *  The counter is enabled when the clear, strobe, and enable inputs are low.
 *
 *  When the rate input is binary 0 (all rate inputs low), Z remains high [and Y low].
 *
 *  The unity/cascade input, when connected to the clock input, passes
 *    clock frequency (inverted) to the Y output when the rate input/decoding
 *    gates are inhibited by the strobe.
 *
 *  When CLR is H, states of CLK and STRB can affect Y and Z.  Default are
 *    Y L, Z H, ENout H.
 *
 *  Unity/cascade is used to inhibit output Y (UNITY L -> Y H)
 */

#include "nl_base.h"

namespace netlist::devices {

	static constexpr const std::array<netlist_time, 2> out_delay_CLK_Y = { NLTIME_FROM_NS(20), NLTIME_FROM_NS(26) }; // tPHL, tPLH
	static constexpr const std::array<netlist_time, 2> out_delay_CLK_Z = { NLTIME_FROM_NS(17), NLTIME_FROM_NS(12) };

	// FIXME: room for improvement -> clock handling

	NETLIB_OBJECT(7497)
	{
		NETLIB_CONSTRUCTOR(7497)
		, m_B(*this, {"B5", "B4", "B3", "B2", "B1", "B0"}, NETLIB_DELEGATE(inputs))
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(clk_strb))
		, m_STRBQ(*this, "STRBQ", NETLIB_DELEGATE(clk_strb))
		, m_ENQ(*this, "ENQ", NETLIB_DELEGATE(inputs))
		, m_UNITYQ(*this, "UNITYQ", NETLIB_DELEGATE(unity))
		, m_CLR(*this, "CLR", NETLIB_DELEGATE(clr))
		, m_Y(*this, "Y")
		, m_ZQ(*this, "ZQ")
		, m_ENOUTQ(*this, "ENOUTQ")
		, m_cnt(*this, "_m_cnt", 0)
		, m_rate(*this, "_m_rate", 0)
		, m_state(*this, "_m_state", 0)
		, m_last_clock(*this, "_m_lastclock", 0)
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_cnt = 0;
			m_rate = 0;
			m_last_clock = 0;
		}

		NETLIB_HANDLERI(unity)
		{
			newstate (m_state);
		}

		NETLIB_HANDLERI(clr)
		{
			m_cnt = 0;
			clk_strb();
		}

		NETLIB_HANDLERI(clk_strb)
		{
			netlist_sig_t clk = m_CLK();

			if (!m_last_clock && clk && !m_ENQ() && !m_CLR())
			{
				m_cnt++;
				m_cnt &= 63;
			}
			m_last_clock = clk;

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

		NETLIB_HANDLERI(inputs)
		{
			m_rate = rate();
			clk_strb();
		}

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
		state_var_sig m_last_clock;
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

	NETLIB_DEVICE_IMPL(7497,      "TTL_7497", "+CLK,+STRBQ,+ENQ,+UNITYQ,+CLR,+B0,+B1,+B2,+B3,+B4,+B5,@VCC,@GND")

} // namespace netlist::devices
