// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74175.c
 *
 */

#include "nld_74175.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{

	NETLIB_OBJECT(74175)
	{
		NETLIB_CONSTRUCTOR(74175)
		, m_D(*this, {{"D1", "D2", "D3", "D4"}})
		, m_CLRQ(*this, "CLRQ")
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(74175, clk))
		, m_Q(*this, {{"Q1", "Q2", "Q3", "Q4"}})
		, m_QQ(*this, {{"Q1Q", "Q2Q", "Q3Q", "Q4Q"}})
		, m_data(*this, "m_data", 0)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();
		NETLIB_HANDLERI(clk);

	protected:
		object_array_t<logic_input_t, 4> m_D;
		logic_input_t m_CLRQ;

		logic_input_t m_CLK;
		object_array_t<logic_output_t, 4> m_Q;
		object_array_t<logic_output_t, 4> m_QQ;

		state_var<unsigned>      m_data;
	};

	NETLIB_OBJECT_DERIVED(74175_dip, 74175)
	{
		NETLIB_CONSTRUCTOR_DERIVED(74175_dip, 74175)
		{
			register_subalias("9", m_CLK);
			register_subalias("1",  m_CLRQ);

			register_subalias("4",    m_D[0]);
			register_subalias("2",   m_Q[0]);
			register_subalias("3",  m_QQ[0]);

			register_subalias("5",    m_D[1]);
			register_subalias("7",   m_Q[1]);
			register_subalias("6",  m_QQ[1]);

			register_subalias("12",    m_D[2]);
			register_subalias("10",   m_Q[2]);
			register_subalias("11",  m_QQ[2]);

			register_subalias("13",    m_D[3]);
			register_subalias("15",   m_Q[3]);
			register_subalias("14",  m_QQ[3]);
		}
	};

	constexpr const netlist_time delay[2] = { NLTIME_FROM_NS(25), NLTIME_FROM_NS(25) };
	constexpr const netlist_time delay_clear[2] = { NLTIME_FROM_NS(40), NLTIME_FROM_NS(25) };

	NETLIB_HANDLER(74175, clk)
	{
		if (m_CLRQ())
		{
			for (std::size_t i=0; i<4; i++)
			{
				netlist_sig_t d = (m_data >> i) & 1;
				m_Q[i].push(d, delay[d]);
				m_QQ[i].push(d ^ 1, delay[d ^ 1]);
			}
			m_CLK.inactivate();
		}
	}

	NETLIB_UPDATE(74175)
	{
		uint_fast8_t d = 0;
		for (std::size_t i=0; i<4; i++)
		{
			d |= (m_D[i]() << i);
		}
		if (!m_CLRQ())
		{
			for (std::size_t i=0; i<4; i++)
			{
				m_Q[i].push(0, delay_clear[0]);
				m_QQ[i].push(1, delay_clear[1]);
			}
			m_data = 0;
		} else if (d != m_data)
		{
			m_data = d;
			m_CLK.activate_lh();
		}
	}


	NETLIB_RESET(74175)
	{
		m_CLK.set_state(logic_t::STATE_INP_LH);
		m_data = 0xFF;
	}

	NETLIB_DEVICE_IMPL(74175,   "TTL_74175", "+CLK,+D1,+D2,+D3,+D4,+CLRQ")
	NETLIB_DEVICE_IMPL(74175_dip,"TTL_74175_DIP", "")

	} //namespace devices
} // namespace netlist
