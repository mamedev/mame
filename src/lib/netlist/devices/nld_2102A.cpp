// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_2102A.cpp
 *
 */

#include "nld_2102A.h"
#include "../nl_base.h"

#define ADDR2BYTE(a)    ((a) >> 3)
#define ADDR2BIT(a)     ((a) & 0x7)

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(2102A)
	{
		NETLIB_CONSTRUCTOR(2102A)
		, m_A(*this, {{"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9" }})
		, m_CEQ(*this, "CEQ")
		, m_RWQ(*this, "RWQ")
		, m_DI(*this, "DI")
		, m_DO(*this, "DO")
		, m_ram(*this, "m_ram", 0)
		, m_RAM(*this, "m_RAM", &m_ram[0])
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		object_array_t<logic_input_t, 10> m_A;
		logic_input_t m_CEQ;
		logic_input_t m_RWQ;
		logic_input_t m_DI;

		logic_output_t m_DO;

		state_array<uint8_t, 128> m_ram; // 1024x1 bits
		param_ptr_t m_RAM;
	};

	NETLIB_OBJECT_DERIVED(2102A_dip, 2102A)
	{
		NETLIB_CONSTRUCTOR_DERIVED(2102A_dip, 2102A)
		{
			register_subalias("8",     m_A[0]);
			register_subalias("4",     m_A[1]);
			register_subalias("5",     m_A[2]);
			register_subalias("6",     m_A[3]);
			register_subalias("7",     m_A[4]);
			register_subalias("2",     m_A[5]);
			register_subalias("1",     m_A[6]);
			register_subalias("16",    m_A[7]);
			register_subalias("15",    m_A[8]);
			register_subalias("14",    m_A[9]);

			register_subalias("13",    m_CEQ);
			register_subalias("3",     m_RWQ);

			register_subalias("11",    m_DI);
			register_subalias("12",    m_DO);
		}
	};

	NETLIB_UPDATE(2102A)
	{
		netlist_time max_delay = NLTIME_FROM_NS(350);

		if (!m_CEQ())
		{
			unsigned a = 0;
			for (std::size_t i=0; i<10; i++)
			{
				a |= (m_A[i]() << i);
			}
			const unsigned byte = ADDR2BYTE(a);
			const unsigned bit = ADDR2BIT(a);

			if (!m_RWQ())
			{
				m_ram[byte] &= ~(static_cast<uint8_t>(1)      << bit);
				m_ram[byte] |=  (static_cast<uint8_t>(m_DI()) << bit);
			}

			m_DO.push((m_ram[byte] >> bit) & 1, max_delay);
		}
	}

	NETLIB_RESET(2102A)
	{
		m_RAM.setTo(&m_ram[0]);
		for (std::size_t i=0; i<128; i++)
			m_ram[i] = 0;
	}

	NETLIB_DEVICE_IMPL(2102A,	 "RAM_2102A", 	"+CEQ,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+A9,+RWQ,+DI")
	NETLIB_DEVICE_IMPL(2102A_dip,"RAM_2102A_DIP","")

	} //namespace devices
} // namespace netlist
