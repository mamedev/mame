// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_74365.cpp
 *
 *  SN74365: Hex Bus Driver with 3-State Outputs
 *
 *          +--------------+
 *      G1Q |1     ++    16| VCC
 *       A1 |2           15| G2Q
 *       Y1 |3           14| A6
 *       A2 |4    74365  13| Y6
 *       Y2 |5           12| A5
 *       A3 |6           11| Y5
 *       Y3 |7           10| A4
 *      GND |8            9| Y4
 *          +--------------+
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 *  Note: Currently the netlist system does not support proper tristate output, so this
 *        is not a "real" bus driver, it simply outputs 0 if the chip is not enabled.
 */

#include "nld_74365.h"
#include "nl_base.h"

namespace netlist
{
	namespace devices
	{

	/* FIXME: This should be a single device, i.e. one tristate buffer only.
	 *
	 * FIXME: Implement tristate output.
	 *
	 */


	NETLIB_OBJECT(74365)
	{
		NETLIB_CONSTRUCTOR(74365)
		, m_G1Q(*this, "G1Q", NETLIB_DELEGATE(inputs))
		, m_G2Q(*this, "G2Q", NETLIB_DELEGATE(inputs))
		, m_A(*this, { "A1", "A2", "A3", "A4", "A5", "A6" }, NETLIB_DELEGATE(inputs))
		, m_Y(*this, { "Y1", "Y2", "Y3", "Y4", "Y5", "Y6" })
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_HANDLERI(inputs)
		{
			if (!m_G1Q() && !m_G2Q())
			{
				for (std::size_t i=0; i<6; i++)
					m_Y[i].push(m_A[i](), NLTIME_FROM_NS(20)); // FIXME: Correct timing
			}
			else
			{
				for (std::size_t i=0; i<6; i++)
					m_Y[i].push(0, NLTIME_FROM_NS(20)); // FIXME: Correct timing
			}
		}

		logic_input_t m_G1Q;
		logic_input_t m_G2Q;
		object_array_t<logic_input_t, 6> m_A;
		object_array_t<logic_output_t, 6> m_Y;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(74365, "TTL_74365", "+G1Q,+G2Q,+A1,+A2,+A3,+A4,+A5,+A6,@VCC,@GND")

	} //namespace devices
} // namespace netlist
