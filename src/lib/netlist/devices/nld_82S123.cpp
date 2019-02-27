// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_82S123.cpp
 *
 */

#include "nld_82S123.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(82S123)
	{
		NETLIB_CONSTRUCTOR(82S123)
		, m_A(*this, {{"A0", "A1", "A2", "A3", "A4"}})
		, m_CEQ(*this, "CEQ")
		, m_O(*this, {{"O1", "O2", "O3", "O4", "O5", "O6", "O7", "O8"}})
		, m_ROM(*this, "ROM")
		{
		}

		NETLIB_UPDATEI();

	protected:
		object_array_t<logic_input_t, 5> m_A;
		logic_input_t m_CEQ;
		object_array_t<logic_output_t, 8> m_O;

		param_rom_t<uint8_t, 5, 8> m_ROM; // 256 bits, 32x8
	};

	NETLIB_OBJECT_DERIVED(82S123_dip, 82S123)
	{
		NETLIB_CONSTRUCTOR_DERIVED(82S123_dip, 82S123)
		{
			register_subalias("1",     m_O[0]);
			register_subalias("2",     m_O[1]);
			register_subalias("3",     m_O[2]);
			register_subalias("4",     m_O[3]);
			register_subalias("5",     m_O[4]);
			register_subalias("6",     m_O[5]);
			register_subalias("7",     m_O[6]);
			register_subalias("8",     m_O[7]);

			register_subalias("15",    m_CEQ);

			register_subalias("10",    m_A[0]);
			register_subalias("11",    m_A[1]);
			register_subalias("12",    m_A[2]);
			register_subalias("13",    m_A[3]);
			register_subalias("14",    m_A[4]);
		}
	};

	// FIXME: timing!
	NETLIB_UPDATE(82S123)
	{
		unsigned o = 0xff;

		netlist_time delay = NLTIME_FROM_NS(35);
		if (!m_CEQ())
		{
			unsigned a = 0;
			for (std::size_t i=0; i<5; i++)
				a |= (m_A[i]() << i);

			o = m_ROM[a];

			delay = NLTIME_FROM_NS(50);
		}

		// FIXME: Outputs are tristate. This needs to be properly implemented
		for (std::size_t i=0; i<8; i++)
			m_O[i].push((o >> i) & 1, delay);
	}

	NETLIB_DEVICE_IMPL(82S123,     "PROM_82S123",     "+CEQ,+A0,+A1,+A2,+A3,+A4")
	NETLIB_DEVICE_IMPL(82S123_dip, "PROM_82S123_DIP", "")

	} //namespace devices
} // namespace netlist
