// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_2716.cpp
 *
 */

#include "nld_2716.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(2716)
	{
		NETLIB_CONSTRUCTOR(2716)
		, m_A(*this, { "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10" })
		, m_GQ(*this, "GQ")
		, m_EPQ(*this, "EPQ")
		, m_D(*this, { "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7" })
		, m_last_EPQ(*this, "m_last_EPQ", 1)
		, m_ROM(*this, "ROM")
		, m_power_pins(*this)
		{
		}

		NETLIB_UPDATEI();

		friend class NETLIB_NAME(2716_dip);
	private:
		object_array_t<logic_input_t, 11> m_A;
		logic_input_t m_GQ;
		logic_input_t m_EPQ;
		object_array_t<logic_output_t, 8> m_D;

		state_var<unsigned> m_last_EPQ;

		param_rom_t<uint8_t, 11, 8> m_ROM; // 16 Kbits, used as 2 Kbit x 8
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT(2716_dip)
	{
		NETLIB_CONSTRUCTOR(2716_dip)
		, A(*this, "A")
		{
			register_subalias("8",     A.m_A[0]);
			register_subalias("7",     A.m_A[1]);
			register_subalias("6",     A.m_A[2]);
			register_subalias("5",     A.m_A[3]);
			register_subalias("4",     A.m_A[4]);
			register_subalias("3",     A.m_A[5]);
			register_subalias("2",     A.m_A[6]);
			register_subalias("1",     A.m_A[7]);
			register_subalias("23",    A.m_A[8]);
			register_subalias("22",    A.m_A[9]);
			register_subalias("19",    A.m_A[10]);

			register_subalias("20",    A.m_GQ);
			register_subalias("18",    A.m_EPQ);

			register_subalias("9",     A.m_D[0]);
			register_subalias("10",    A.m_D[1]);
			register_subalias("11",    A.m_D[2]);
			register_subalias("13",    A.m_D[3]);
			register_subalias("14",    A.m_D[4]);
			register_subalias("15",    A.m_D[5]);
			register_subalias("16",    A.m_D[6]);
			register_subalias("17",    A.m_D[7]);

			register_subalias("12",    "A.GND");
			register_subalias("24",    "A.VCC");
		}
		NETLIB_RESETI() {}
		NETLIB_UPDATEI() {}
	private:
		NETLIB_SUB(2716) A;
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

	NETLIB_DEVICE_IMPL(2716, "EPROM_2716", "+GQ,+EPQ,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+A9,+A10,@VCC,@GND")
	NETLIB_DEVICE_IMPL(2716_dip, "EPROM_2716_DIP",         "")

	} //namespace devices
} // namespace netlist
