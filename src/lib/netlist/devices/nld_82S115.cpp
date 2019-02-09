// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_82S115.cpp
 *
 */

#include "nld_82S115.h"
#include "../nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(82S115)
	{
		NETLIB_CONSTRUCTOR(82S115)
		, m_A(*this, {{"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"}})
		, m_CE1Q(*this, "CE1Q")
		, m_CE2(*this, "CE2")
		, m_STROBE(*this, "STROBE")
		, m_O(*this, {{"O1", "O2", "O3", "O4", "O5", "O6", "O7", "O8"}})
		, m_last_O(*this, "m_last_O", 0)
		, m_ROM(*this, "ROM")
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		object_array_t<logic_input_t, 9> m_A;
		logic_input_t m_CE1Q;
		logic_input_t m_CE2;
		logic_input_t m_STROBE;
		object_array_t<logic_output_t, 8> m_O;

		state_var<unsigned> m_last_O;

		param_rom_t<uint8_t, 9, 8> m_ROM; // 4096 bits, 512x8
	};

	NETLIB_OBJECT_DERIVED(82S115_dip, 82S115)
	{
		NETLIB_CONSTRUCTOR_DERIVED(82S115_dip, 82S115)
		{
			register_subalias("21",    m_A[0]);
			register_subalias("22",    m_A[1]);
			register_subalias("23",    m_A[2]);
			register_subalias("1",     m_A[3]);
			register_subalias("2",     m_A[4]);
			register_subalias("3",     m_A[5]);
			register_subalias("4",     m_A[6]);
			register_subalias("5",     m_A[7]);
			register_subalias("6",     m_A[8]);

			register_subalias("20",    m_CE1Q);
			register_subalias("19",    m_CE2);

			// register_subalias("13",    m_FE1);
			// register_subalias("11",    m_FE2);

			register_subalias("18",    m_STROBE);

			register_subalias("7",     m_O[0]);
			register_subalias("8",     m_O[1]);
			register_subalias("9",     m_O[2]);
			register_subalias("10",    m_O[3]);
			register_subalias("14",    m_O[4]);
			register_subalias("15",    m_O[5]);
			register_subalias("16",    m_O[6]);
			register_subalias("17",    m_O[7]);
		}
	};

	NETLIB_RESET(82S115)
	{
		m_last_O = 0;
	}

	// FIXME: timing!
	NETLIB_UPDATE(82S115)
	{
		unsigned o = 0;

		if (!m_CE1Q() && m_CE2())
		{
			if (m_STROBE())
			{
				unsigned a = 0;
				for (std::size_t i=0; i<9; i++)
					a |= (m_A[i]() << i);

				o = m_ROM[a];
			}
			else
			{
				o = m_last_O;
			}
		}

		m_last_O = o;

		// FIXME: Outputs are tristate. This needs to be properly implemented
		for (std::size_t i=0; i<8; i++)
			m_O[i].push((o >> i) & 1, NLTIME_FROM_NS(40)); // FIXME: Timing
	}

	NETLIB_DEVICE_IMPL(82S115,     "PROM_82S115",     "+CE1Q,+CE2,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+STROBE")
	NETLIB_DEVICE_IMPL(82S115_dip, "PROM_82S115_DIP", "")

	} //namespace devices
} // namespace netlist
