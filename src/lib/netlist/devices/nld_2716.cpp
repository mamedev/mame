// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_2716.cpp
 *
 */

#include "nld_2716.h"
#include "../nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(2716)
	{
		NETLIB_CONSTRUCTOR(2716)
		, m_A(*this, {{ "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10" }})
		, m_GQ(*this, "GQ")
		, m_EPQ(*this, "EPQ")
		, m_D(*this, {{ "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7" }})
		, m_last_EPQ(*this, "m_last_EPQ", 1)
		, m_ROM(*this, "ROM")
		{
		}

		NETLIB_UPDATEI();

	protected:
		object_array_t<logic_input_t, 11> m_A;
		logic_input_t m_GQ;
		logic_input_t m_EPQ;
		object_array_t<logic_output_t, 8> m_D;

		state_var<unsigned> m_last_EPQ;

		param_rom_t<uint8_t, 11, 8> m_ROM; // 16 Kbits, used as 2 Kbit x 8
	};

	NETLIB_OBJECT_DERIVED(2716_dip, 2716)
	{
		NETLIB_CONSTRUCTOR_DERIVED(2716_dip, 2716)
		{
			register_subalias("8",     m_A[0]);
			register_subalias("7",     m_A[1]);
			register_subalias("6",     m_A[2]);
			register_subalias("5",     m_A[3]);
			register_subalias("4",     m_A[4]);
			register_subalias("3",     m_A[5]);
			register_subalias("2",     m_A[6]);
			register_subalias("1",     m_A[7]);
			register_subalias("23",    m_A[8]);
			register_subalias("22",    m_A[9]);
			register_subalias("19",    m_A[10]);

			register_subalias("20",    m_GQ);
			register_subalias("18",    m_EPQ);

			register_subalias("9",     m_D[0]);
			register_subalias("10",    m_D[1]);
			register_subalias("11",    m_D[2]);
			register_subalias("13",    m_D[3]);
			register_subalias("14",    m_D[4]);
			register_subalias("15",    m_D[5]);
			register_subalias("16",    m_D[6]);
			register_subalias("17",    m_D[7]);
		}
	};

	// FIXME: timing!
	NETLIB_UPDATE(2716)
	{
		unsigned d = 0xff;

		netlist_time delay = NLTIME_FROM_NS(450);
		if (!m_GQ() && !m_EPQ())
		{
			unsigned a = 0;
			for (std::size_t i=0; i<11; i++)
			a |= (m_A[i]() << i);

			d = m_ROM[a];

			if (m_last_EPQ)
				delay = NLTIME_FROM_NS(120);
		}

		m_last_EPQ = m_EPQ();

		// FIXME: Outputs are tristate. This needs to be properly implemented
		for (std::size_t i=0; i<8; i++)
			m_D[i].push((d >> i) & 1, delay);
	}

	NETLIB_DEVICE_IMPL(2716, "EPROM_2716", "+GQ,+EPQ,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+A9,+A10")
	NETLIB_DEVICE_IMPL(2716_dip, "EPROM_2716_DIP",         "")

	} //namespace devices
} // namespace netlist
