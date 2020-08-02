// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74107.c
 *
 */

#include "nld_74107.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{

	static constexpr const std::array<netlist_time, 2> delay_107  = { NLTIME_FROM_NS(16), NLTIME_FROM_NS(25) };
	static constexpr const std::array<netlist_time, 2> delay_107A = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(15) };

	NETLIB_OBJECT(74107A)
	{
		NETLIB_CONSTRUCTOR(74107A)
		, m_clk(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_J(*this, "J", NETLIB_DELEGATE(other))
		, m_K(*this, "K", NETLIB_DELEGATE(other))
		, m_clrQ(*this, "CLRQ", NETLIB_DELEGATE(other))
		, m_power_pins(*this)
		{
			m_delay[0] = delay_107A[0];
			m_delay[1] = delay_107A[1];
		}

		friend class NETLIB_NAME(74107_dip);
		friend class NETLIB_NAME(74107);

		NETLIB_RESETI()
		{
			m_clk.set_state(logic_t::STATE_INP_HL);
			//m_Q.initial(0);
			//m_QQ.initial(1);
		}

		NETLIB_HANDLERI(other)
		{
			if (!m_clrQ())
			{
				m_clk.inactivate();
				newstate(0);
			}
			else if (m_J() | m_K())
				m_clk.activate_hl();
		}

		NETLIB_HANDLERI(clk)
		{
			const netlist_sig_t t(m_Q.net().Q());
			/*
			 *  J K  Q1 Q2 F t   Q
			 *  0 0   0  1 0 0   0
			 *  0 1   0  0 0 0   0
			 *  1 0   0  0 1 0   1
			 *  1 1   1  0 0 0   1
			 *  0 0   0  1 0 1   1
			 *  0 1   0  0 0 1   0
			 *  1 0   0  0 1 1   1
			 *  1 1   1  0 0 1   0
			 */
			if ((m_J() & m_K()) ^ 1)
				m_clk.inactivate();
			newstate(((t ^ 1) & m_J()) | (t & (m_K() ^ 1)));
		}

	private:
		logic_input_t m_clk;

		logic_output_t m_Q;
		logic_output_t m_QQ;

		std::array<netlist_time, 2> m_delay;

		logic_input_t m_J;
		logic_input_t m_K;
		logic_input_t m_clrQ;

		nld_power_pins m_power_pins;
		void newstate(const netlist_sig_t state)
		{
			m_Q.push(state, m_delay[state]);
			m_QQ.push(state ^ 1, m_delay[state ^ 1]);
		}
	};

	NETLIB_OBJECT_DERIVED(74107, 74107A)
	{
	public:
		NETLIB_CONSTRUCTOR(74107)
		{
			m_delay[0] = delay_107[0];
			m_delay[1] = delay_107[1];
		}
	};

#if (!NL_USE_TRUTHTABLE_74107)
	NETLIB_DEVICE_IMPL(74107,       "TTL_74107",    "+CLK,+J,+K,+CLRQ,@VCC,@GND")
#endif
	NETLIB_DEVICE_IMPL(74107A,      "TTL_74107A",   "+CLK,+J,+K,+CLRQ,@VCC,@GND")

	} //namespace devices
} // namespace netlist
