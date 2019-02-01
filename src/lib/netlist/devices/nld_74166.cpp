// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_74166.cpp
 *
 */

#include "nld_74166.h"
#include "../nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(74166)
	{
		NETLIB_CONSTRUCTOR(74166)
		, m_DATA(*this, {{ "H", "G", "F", "E", "D", "C", "B", "A" }})
		, m_SER(*this, "SER")
		, m_CLRQ(*this, "CLRQ")
		, m_SH_LDQ(*this, "SH_LDQ")
		, m_CLK(*this, "CLK")
		, m_CLKINH(*this, "CLKINH")
		, m_QH(*this, "QH")
		, m_shifter(*this, "m_shifter", 0)
		, m_last_CLRQ(*this, "m_last_CLRQ", 0)
		, m_last_CLK(*this, "m_last_CLK", 0)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		object_array_t<logic_input_t, 8> m_DATA;
		logic_input_t m_SER;
		logic_input_t m_CLRQ;
		logic_input_t m_SH_LDQ;
		logic_input_t m_CLK;
		logic_input_t m_CLKINH;
		logic_output_t m_QH;

		state_var<unsigned> m_shifter;
		state_var<unsigned> m_last_CLRQ;
		state_var<unsigned> m_last_CLK;
	};

	NETLIB_OBJECT_DERIVED(74166_dip, 74166)
	{
		NETLIB_CONSTRUCTOR_DERIVED(74166_dip, 74166)
		{
			register_subalias("1", m_SER);
			register_subalias("2", m_DATA[7]);
			register_subalias("3", m_DATA[6]);
			register_subalias("4", m_DATA[5]);
			register_subalias("5", m_DATA[4]);
			register_subalias("6", m_CLKINH);
			register_subalias("7", m_CLK);

			register_subalias("9", m_CLRQ);
			register_subalias("10", m_DATA[3]);
			register_subalias("11", m_DATA[2]);
			register_subalias("12", m_DATA[1]);
			register_subalias("13", m_QH);
			register_subalias("14", m_DATA[0]);
			register_subalias("15", m_SH_LDQ);

		}
	};

	NETLIB_RESET(74166)
	{
		m_shifter = 0;
		m_last_CLRQ = 0;
		m_last_CLK = 0;
	}

	NETLIB_UPDATE(74166)
	{
		netlist_sig_t old_qh = m_QH.net().Q();
		netlist_sig_t qh = 0;

		netlist_time delay = NLTIME_FROM_NS(26);
		if (m_CLRQ())
		{
			bool clear_unset = !m_last_CLRQ;
			if (clear_unset)
			{
				delay = NLTIME_FROM_NS(35);
			}

			if (!m_CLK() || m_CLKINH())
			{
				qh = old_qh;
			}
			else if (!m_last_CLK)
			{
				if (!m_SH_LDQ())
				{
					m_shifter = 0;
					for (std::size_t i=0; i<8; i++)
						m_shifter |= (m_DATA[i]() << i);
				}
				else
				{
					unsigned high_bit = m_SER() ? 0x80 : 0;
					m_shifter = high_bit | (m_shifter >> 1);
				}

				qh = m_shifter & 1;
				if (!qh && !clear_unset)
				{
					delay = NLTIME_FROM_NS(30);
				}
			}
		}

		m_last_CLRQ = m_CLRQ();
		m_last_CLK = m_CLK();

		m_QH.push(qh, delay); //FIXME
	}

	NETLIB_DEVICE_IMPL(74166,    "TTL_74166", "+CLK,+CLKINH,+SH_LDQ,+SER,+A,+B,+C,+D,+E,+F,+G,+H,+CLRQ")
	NETLIB_DEVICE_IMPL(74166_dip,"TTL_74166_DIP", "")

	} //namespace devices
} // namespace netlist
