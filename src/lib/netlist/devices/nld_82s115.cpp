// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_82S115.cpp
 *
 *  82S115: 4K-bit TTL bipolar PROM (512 x 8)
 *
 *          +--------------+
 *       A3 |1     ++    24| VCC
 *       A4 |2           23| A2
 *       A5 |3           22| A1
 *       A6 |4   82S115  21| A0
 *       A7 |5           20| CE1Q
 *       A8 |6           19| CE2
 *       O1 |7           18| STROBE
 *       O2 |8           17| O8
 *       O3 |9           16| O7
 *       O4 |10          15| O6
 *      FE2 |11          14| O5
 *      GND |12          13| FE1
 *          +--------------+
 *
 *
 *  Naming conventions follow Signetics datasheet
 *
 */

#include "nl_base.h"

namespace netlist::devices {

	NETLIB_OBJECT(82S115)
	{
		NETLIB_CONSTRUCTOR(82S115)
		, m_A(*this, {"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"}, NETLIB_DELEGATE(inputs))
		, m_CE1Q(*this, "CE1Q", NETLIB_DELEGATE(inputs))
		, m_CE2(*this, "CE2", NETLIB_DELEGATE(inputs))
		, m_STROBE(*this, "STROBE", NETLIB_DELEGATE(inputs))
		, m_O(*this, {"O1", "O2", "O3", "O4", "O5", "O6", "O7", "O8"})
		, m_last_O(*this, "m_last_O", 0)
		, m_last_enabled(*this, "m_last_enabled", 0)
		, m_last_strobe(*this, "m_last_strobe", 0)
		, m_ROM(*this, "ROM")
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_last_O = 0;
			m_last_strobe = 0;
			m_last_enabled = 0;
		}

	private:
		NETLIB_HANDLERI(inputs)
		{
			unsigned o = 0;
			bool enabled = !m_CE1Q() && m_CE2();

			if (enabled)
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

			// The Signetics datasheet says STROBE HIGH = hold, LOW = transparent.
			// Indy 4 (the only known netlist driving STROBE dynamically) wires it
			// so the intended behaviour is a latch resampling at the strobe fall;
			// the datasheet polarity would sample the unsettled address. Keep the
			// functional polarity (HIGH = transparent), and push latch-edge output
			// changes fast: real tS is 15-25 ns, but the settle chain is hidden by
			// CRT bloom on real hardware which a crisp render does not reproduce.
			const bool strobe = m_STROBE();
			const bool control_edge = (strobe != (m_last_strobe != 0)) || (enabled != (m_last_enabled != 0));
			const netlist_time delay = control_edge ? NLTIME_FROM_NS(2) : NLTIME_FROM_NS(40);
			m_last_strobe = strobe ? 1 : 0;
			m_last_enabled = enabled;
			m_last_O = o;

			// FIXME: Outputs are tristate. This needs to be properly implemented
			for (std::size_t i=0; i<8; i++)
				m_O[i].push((o >> i) & 1, delay);
		}

		object_array_t<logic_input_t, 9> m_A;
		logic_input_t m_CE1Q;
		logic_input_t m_CE2;
		logic_input_t m_STROBE;
		object_array_t<logic_output_t, 8> m_O;

		state_var<unsigned> m_last_O;
		state_var<unsigned> m_last_enabled;
		state_var<unsigned> m_last_strobe;

		param_rom_t<uint8_t, 9, 8> m_ROM; // 4096 bits, 512x8
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(82S115,     "PROM_82S115",     "+CE1Q,+CE2,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+STROBE,@VCC,@GND")

} // namespace netlist::devices
