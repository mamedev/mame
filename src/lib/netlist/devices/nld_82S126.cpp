// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_82S126.cpp
 *
 */

#include "nld_82S126.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(82S126)
	{
		NETLIB_CONSTRUCTOR(82S126)
		, m_A(*this, {{"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7" }})
		, m_CE1Q(*this, "CE1Q")
		, m_CE2Q(*this, "CE2Q")
		, m_O(*this, {{"O1", "O2", "O3", "O4" }})
		, m_ROM(*this, "ROM")
		{
		}

		NETLIB_UPDATEI();

	protected:
		object_array_t<logic_input_t, 8> m_A;
		logic_input_t m_CE1Q;
		logic_input_t m_CE2Q;
		object_array_t<logic_output_t, 4> m_O;

		param_rom_t<uint8_t, 8, 4> m_ROM; // 1024 bits, 32x32, used as 256x4
	};

	NETLIB_OBJECT_DERIVED(82S126_dip, 82S126)
	{
		NETLIB_CONSTRUCTOR_DERIVED(82S126_dip, 82S126)
		{
			register_subalias("5",     m_A[0]);
			register_subalias("6",     m_A[1]);
			register_subalias("7",     m_A[2]);
			register_subalias("4",     m_A[3]);
			register_subalias("3",     m_A[4]);
			register_subalias("2",     m_A[5]);
			register_subalias("1",     m_A[6]);
			register_subalias("15",    m_A[7]);

			register_subalias("13",    m_CE1Q);
			register_subalias("14",    m_CE2Q);

			register_subalias("12",    m_O[0]);
			register_subalias("11",    m_O[1]);
			register_subalias("10",    m_O[2]);
			register_subalias("9",     m_O[3]);
		}
	};

	// FIXME: timing!
	NETLIB_UPDATE(82S126)
	{
		unsigned o = 0xf;

		netlist_time delay = NLTIME_FROM_NS(25);
		if (!m_CE1Q() && !m_CE2Q())
		{
			unsigned a = 0;
			for (std::size_t i=0; i<8; i++)
			a |= (m_A[i]() << i);

			o = m_ROM[a];

			delay = NLTIME_FROM_NS(50);
		}

		// FIXME: Outputs are tristate. This needs to be properly implemented
		for (std::size_t i=0; i<4; i++)
			m_O[i].push((o >> i) & 1, delay);
	}

	NETLIB_DEVICE_IMPL(82S126,     "PROM_82S126",     "+CE1Q,+CE2Q,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7")
	NETLIB_DEVICE_IMPL(82S126_dip, "PROM_82S126_DIP", "")

	} //namespace devices
} // namespace netlist
