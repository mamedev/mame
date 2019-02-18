// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74107.c
 *
 */

#include "nld_74107.h"
#include "../nl_base.h"

namespace netlist
{
	namespace devices
	{

	static constexpr netlist_time delay_107[2] = { NLTIME_FROM_NS(16), NLTIME_FROM_NS(25) };
	static constexpr netlist_time delay_107A[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(15) };

	NETLIB_OBJECT(74107A)
	{
		NETLIB_CONSTRUCTOR(74107A)
		, m_clk(*this, "CLK", NETLIB_DELEGATE(74107A, clk))
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_J(*this, "J")
		, m_K(*this, "K")
		, m_clrQ(*this, "CLRQ")
		{
			m_delay[0] = delay_107A[0];
			m_delay[1] = delay_107A[1];
		}

		friend class NETLIB_NAME(74107_dip);
		friend class NETLIB_NAME(74107);
		//friend class NETLIB_NAME(74107A_dip);

		NETLIB_RESETI();
		NETLIB_UPDATEI();
		NETLIB_HANDLERI(clk);

	private:
		logic_input_t m_clk;

		logic_output_t m_Q;
		logic_output_t m_QQ;

		netlist_time m_delay[2];

		logic_input_t m_J;
		logic_input_t m_K;
		logic_input_t m_clrQ;

		void newstate(const netlist_sig_t state)
		{
			m_Q.push(state, m_delay[state]);
			m_QQ.push(state ^ 1, m_delay[state ^ 1]);
		}
	};

	NETLIB_OBJECT_DERIVED(74107, 74107A)
	{
	public:
		NETLIB_CONSTRUCTOR_DERIVED(74107, 74107A)
		{
			m_delay[0] = delay_107[0];
			m_delay[1] = delay_107[1];
		}
	};

	NETLIB_OBJECT(74107_dip)
	{
		NETLIB_CONSTRUCTOR(74107_dip)
		, m_1(*this, "1")
		, m_2(*this, "2")
		{
			register_subalias("1", m_1.m_J);
			register_subalias("2", m_1.m_QQ);
			register_subalias("3", m_1.m_Q);

			register_subalias("4", m_1.m_K);
			register_subalias("5", m_2.m_Q);
			register_subalias("6", m_2.m_QQ);

			// register_subalias("7", ); ==> GND

			register_subalias("8", m_2.m_J);
			register_subalias("9", m_2.m_clk);
			register_subalias("10", m_2.m_clrQ);

			register_subalias("11", m_2.m_K);
			register_subalias("12", m_1.m_clk);
			register_subalias("13", m_1.m_clrQ);

			// register_subalias("14", ); ==> VCC

		}
		//NETLIB_RESETI();
		//NETLIB_UPDATEI();

	private:
		NETLIB_SUB(74107) m_1;
		NETLIB_SUB(74107) m_2;
	};

	NETLIB_RESET(74107A)
	{
		m_clk.set_state(logic_t::STATE_INP_HL);
		//m_Q.initial(0);
		//m_QQ.initial(1);
	}

	NETLIB_HANDLER(74107A, clk)
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

	NETLIB_UPDATE(74107A)
	{
		if (!m_clrQ())
		{
			m_clk.inactivate();
			newstate(0);
		}
		else if (m_J() | m_K())
			m_clk.activate_hl();
	}

	NETLIB_DEVICE_IMPL(74107,       "TTL_74107",    "+CLK,+J,+K,+CLRQ")
	NETLIB_DEVICE_IMPL(74107A,      "TTL_74107A",   "+CLK,+J,+K,+CLRQ")
	NETLIB_DEVICE_IMPL(74107_dip,   "TTL_74107_DIP", "")

	} //namespace devices
} // namespace netlist
