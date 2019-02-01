// license:BSD-3-Clause
// copyright-holders:Felipe Sanches
/*
 * nld_tms4800.cpp
 *
 */

#include "nld_tms4800.h"
#include "../nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(TMS4800)
	{
		NETLIB_CONSTRUCTOR(TMS4800)
		, m_A(*this, {{ "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10" }})
		, m_AR(*this, "AR")
		, m_OE1(*this, "OE1")
		, m_OE2(*this, "OE2")
		, m_D(*this, {{ "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7" }})
		, m_last_data(*this, "m_last_data", 1)
		, m_ROM(*this, "ROM")
		{
		}

		NETLIB_UPDATEI();

	protected:
		object_array_t<logic_input_t, 11> m_A;
		logic_input_t m_AR;
		logic_input_t m_OE1;
		logic_input_t m_OE2;
		object_array_t<logic_output_t, 8> m_D;

		state_var<unsigned> m_last_data;

		param_rom_t<uint8_t, 11, 8> m_ROM; // 16 Kbits, used as 2 Kbit x 8
	};

	NETLIB_OBJECT_DERIVED(TMS4800_dip, TMS4800)
	{
		NETLIB_CONSTRUCTOR_DERIVED(TMS4800_dip, TMS4800)
		{
			register_subalias("2",     m_A[0]);
			register_subalias("3",     m_A[1]);
			register_subalias("4",     m_A[2]);
			register_subalias("5",     m_A[3]);
			register_subalias("6",     m_A[4]);
			register_subalias("7",     m_A[5]);
			register_subalias("12",    m_A[6]);
			register_subalias("11",    m_A[7]);
			register_subalias("10",    m_A[8]);
			register_subalias("8",     m_A[9]);
			register_subalias("15",    m_A[10]);

			register_subalias("13",    m_AR);
			register_subalias("24",    m_OE1);
			register_subalias("14",    m_OE2);

			register_subalias("23",     m_D[0]);
			register_subalias("22",     m_D[1]);
			register_subalias("21",     m_D[2]);
			register_subalias("20",     m_D[3]);
			register_subalias("19",     m_D[4]);
			register_subalias("18",     m_D[5]);
			register_subalias("17",     m_D[6]);
			register_subalias("16",     m_D[7]);
		}
	};

	// FIXME: timing!
	NETLIB_UPDATE(TMS4800)
	{
		unsigned d = 0x00;

		netlist_time delay = NLTIME_FROM_NS(450);
		if (m_AR())
		{
			unsigned a = 0;
			for (std::size_t i=0; i<11; i++)
				a |= (m_A[i]() << i);

			m_last_data = m_ROM[a];
		}
		else
		{
			for (std::size_t i=0; i<4; i++)
			{
				if (m_OE1())
					m_D[i].push((d >> i) & 1, delay);

				if (m_OE2())
					m_D[i+4].push((d >> (i+4)) & 1, delay);
			}
		}
	}

	NETLIB_DEVICE_IMPL(TMS4800, 	"ROM_TMS4800",     "+AR,+OE1,+OE2,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+A9,+A10")
	NETLIB_DEVICE_IMPL(TMS4800_dip, "ROM_TMS4800_DIP", "")

	} //namespace devices
} // namespace netlist
