// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74174.cpp
 *
 */

#include "nld_74174.h"
#include "../nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(74174_sub)
	{
		NETLIB_CONSTRUCTOR(74174_sub)
		, m_CLK(*this, "CLK")
		, m_Q(*this, {{"Q1", "Q2", "Q3", "Q4", "Q5", "Q6"}})
		, m_clrq(*this, "m_clr", 0)
		, m_data(*this, "m_data", 0)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	public:
		logic_input_t m_CLK;
		object_array_t<logic_output_t, 6> m_Q;

		state_var<netlist_sig_t> m_clrq;
		state_var<unsigned>      m_data;
	};

	NETLIB_OBJECT(74174)
	{
		NETLIB_CONSTRUCTOR(74174)
		, m_sub(*this, "sub")
		, m_D(*this, {{"D1", "D2", "D3", "D4", "D5", "D6"}})
		, m_CLRQ(*this, "CLRQ")
		{
			register_subalias("CLK",   m_sub.m_CLK);

			register_subalias("Q1",   m_sub.m_Q[0]);
			register_subalias("Q2",   m_sub.m_Q[1]);
			register_subalias("Q3",   m_sub.m_Q[2]);
			register_subalias("Q4",   m_sub.m_Q[3]);
			register_subalias("Q5",   m_sub.m_Q[4]);
			register_subalias("Q6",   m_sub.m_Q[5]);
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		NETLIB_SUB(74174_sub) m_sub;
		object_array_t<logic_input_t, 6> m_D;
		logic_input_t m_CLRQ;
	};

	NETLIB_OBJECT_DERIVED(74174_dip, 74174)
	{
		NETLIB_CONSTRUCTOR_DERIVED(74174_dip, 74174)
		{
			register_subalias("1",  m_CLRQ);
			register_subalias("9",  m_sub.m_CLK);

			register_subalias("3",  m_D[0]);
			register_subalias("2",  m_sub.m_Q[0]);

			register_subalias("4",  m_D[1]);
			register_subalias("5",  m_sub.m_Q[1]);

			register_subalias("6",  m_D[2]);
			register_subalias("7",  m_sub.m_Q[2]);

			register_subalias("11", m_D[3]);
			register_subalias("10", m_sub.m_Q[3]);

			register_subalias("13", m_D[4]);
			register_subalias("12", m_sub.m_Q[4]);

			register_subalias("14", m_D[5]);
			register_subalias("15", m_sub.m_Q[5]);
		}
	};

	NETLIB_RESET(74174_sub)
	{
		m_CLK.set_state(logic_t::STATE_INP_LH);
		m_clrq = 0;
		m_data = 0xFF;
	}

	NETLIB_UPDATE(74174_sub)
	{
		if (m_clrq)
		{
			for (std::size_t i=0; i<6; i++)
			{
				netlist_sig_t d = (m_data >> i) & 1;
				m_Q[i].push(d, NLTIME_FROM_NS(25));
			}
			m_CLK.inactivate();
		}
	}

	NETLIB_UPDATE(74174)
	{
		uint_fast8_t d = 0;
		for (std::size_t i=0; i<6; i++)
		{
			d |= (m_D[i]() << i);
		}
		m_sub.m_clrq = m_CLRQ();
		if (!m_sub.m_clrq)
		{
			for (std::size_t i=0; i<6; i++)
			{
				m_sub.m_Q[i].push(0, NLTIME_FROM_NS(40));
			}
			m_sub.m_data = 0;
		} else if (d != m_sub.m_data)
		{
			m_sub.m_data = d;
			m_sub.m_CLK.activate_lh();
		}
	}


	NETLIB_RESET(74174)
	{
		//m_sub.do_reset();
	}

	NETLIB_DEVICE_IMPL(74174,	"TTL_74174", "+CLK,+D1,+D2,+D3,+D4,+D5,+D6,+CLRQ")
	NETLIB_DEVICE_IMPL(74174_dip,"TTL_74174_DIP",          "")

	} //namespace devices
} // namespace netlist
