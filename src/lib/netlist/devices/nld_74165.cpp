// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_74165.cpp
 *
 */

#include "nld_74165.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(74165)
	{
		NETLIB_CONSTRUCTOR(74165)
		, m_DATA(*this, { "H", "G", "F", "E", "D", "C", "B", "A" }, NETLIB_DELEGATE(inputs))
		, m_SER(*this, "SER", NETLIB_DELEGATE(inputs))
		, m_SH_LDQ(*this, "SH_LDQ", NETLIB_DELEGATE(inputs))
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(inputs))
		, m_CLKINH(*this, "CLKINH", NETLIB_DELEGATE(inputs))
		, m_QH(*this, "QH")
		, m_QHQ(*this, "QHQ")
		, m_shifter(*this, "m_shifter", 0)
		, m_last_CLK(*this, "m_last_CLK", 0)
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_shifter = 0;
			m_last_CLK = 0;
		}

		NETLIB_HANDLERI(inputs)
		{
			{
				netlist_sig_t old_qh = m_QH.net().Q();
				netlist_sig_t qh = 0;

				if (!m_SH_LDQ())
				{
					m_shifter = 0;
					for (std::size_t i=0; i<8; i++)
						m_shifter |= (m_DATA[i]() << i);
				}
				else if (!m_CLK() || m_CLKINH())
				{
					// FIXME: qh is overwritten below?
					qh = old_qh;
				}
				else if (!m_last_CLK)
				{
					unsigned high_bit = m_SER() ? 0x80 : 0;
					m_shifter = high_bit | (m_shifter >> 1);
				}

				qh = m_shifter & 1;

				m_last_CLK = m_CLK();

				m_QH.push(qh, NLTIME_FROM_NS(20)); // FIXME: Timing
			}

		}

		NETLIB_UPDATEI()
		{
			inputs();
		}

		friend class NETLIB_NAME(74165_dip);
	private:
		object_array_t<logic_input_t, 8> m_DATA;
		logic_input_t m_SER;
		logic_input_t m_SH_LDQ;
		logic_input_t m_CLK;
		logic_input_t m_CLKINH;
		logic_output_t m_QH;
		logic_output_t m_QHQ;

		state_var<unsigned> m_shifter;
		state_var<unsigned> m_last_CLK;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT(74165_dip)
	{
		NETLIB_CONSTRUCTOR(74165_dip)
		, A(*this, "A")
		{
			register_subalias("1", A.m_SH_LDQ);
			register_subalias("2", A.m_CLK);
			register_subalias("3", A.m_DATA[4]);
			register_subalias("4", A.m_DATA[5]);
			register_subalias("5", A.m_DATA[6]);
			register_subalias("6", A.m_DATA[7]);
			register_subalias("7", A.m_QHQ);
			register_subalias("8", "A.GND");

			register_subalias("9",  A.m_QH);
			register_subalias("10", A.m_SER);
			register_subalias("11", A.m_DATA[0]);
			register_subalias("12", A.m_DATA[1]);
			register_subalias("13", A.m_DATA[2]);
			register_subalias("14", A.m_DATA[3]);
			register_subalias("15", A.m_CLKINH);
			register_subalias("16", "A.VCC");
		}
		//NETLIB_RESETI() {}
		//NETLIB_UPDATEI() {}
	private:
		NETLIB_SUB(74165) A;
	};

	// FIXME: Timing
	NETLIB_DEVICE_IMPL(74165, "TTL_74165", "+CLK,+CLKINH,+SH_LDQ,+SER,+A,+B,+C,+D,+E,+F,+G,+H,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74165_dip, "TTL_74165_DIP", "")

	} //namespace devices
} // namespace netlist
