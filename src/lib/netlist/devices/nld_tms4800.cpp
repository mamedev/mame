// license:BSD-3-Clause
// copyright-holders:Felipe Sanches
/*
 * nld_tms4800.cpp
 *
 *  TMS-4800: 16 Kbit (2Kb x 8) READ ONLY MEMORY
 *
 *          +----------------+
 *      VSS |1      ++     24| OE1
 *       A1 |2             23| O1
 *       A2 |3             22| O2
 *       A3 |4   TMS-4800  21| O3
 *       A4 |5             20| O4
 *       A5 |6             19| O5
 *       A6 |7             18| O6
 *      A10 |8             17| O7
 *      VGG |9             16| O8
 *       A9 |10            15| A11
 *       A8 |11            14| OE2
 *       A7 |12            13| AR
 *          +----------------+
 *
 *
 *  Naming conventions follow Texas Instruments datasheet:
 *  http://bitsavers.trailing-edge.com/components/ti/_dataBooks/1975_TI_The_Semiconductor_Memory_Data_Book.pdf
 */

#include "nl_base.h"

namespace netlist::devices {

	NETLIB_OBJECT(TMS4800)
	{
		NETLIB_CONSTRUCTOR(TMS4800)
		, m_A(*this, { "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10" }, NETLIB_DELEGATE(inputs))
		, m_AR(*this, "AR", NETLIB_DELEGATE(inputs))
		, m_OE1(*this, "OE1", NETLIB_DELEGATE(inputs))
		, m_OE2(*this, "OE2", NETLIB_DELEGATE(inputs))
		, m_D(*this, { "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7" })
		, m_last_data(*this, "m_last_data", 1)
		, m_ROM(*this, "ROM")
		, m_supply(*this)
		{
		}

	private:
		// FIXME: timing!
		// FIXME: CS: The code looks odd, looks like m_last_data should be pushed out.
		NETLIB_HANDLERI(inputs)
		{
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
				unsigned d = 0x00;
				for (std::size_t i=0; i<4; i++)
				{
					if (m_OE1())
						m_D[i].push((d >> i) & 1, delay);

					if (m_OE2())
						m_D[i+4].push((d >> (i+4)) & 1, delay);
				}
			}
		}

		object_array_t<logic_input_t, 11> m_A;
		logic_input_t m_AR;
		logic_input_t m_OE1;
		logic_input_t m_OE2;
		object_array_t<logic_output_t, 8> m_D;

		state_var<unsigned> m_last_data;

		param_rom_t<uint8_t, 11, 8> m_ROM; // 16 Kbits, used as 2 Kbit x 8
		NETLIB_NAME(power_pins) m_supply;
	};


	NETLIB_DEVICE_IMPL(TMS4800,     "ROM_TMS4800",     "+AR,+OE1,+OE2,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+A9,+A10,@VCC,@GND")

} // namespace netlist::devices
