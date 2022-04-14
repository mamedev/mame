// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_2102A.cpp
 *
 *  2102: 1024 x 1-bit Static RAM
 *
 *          +--------------+
 *       A6 |1     ++    16| A7
 *       A5 |2           15| A8
 *      RWQ |3           14| A9
 *       A1 |4   82S16   13| CEQ
 *       A2 |5           12| DO
 *       A3 |6           11| DI
 *       A4 |7           10| VCC
 *       A0 |8            9| GND
 *          +--------------+
 *
 *
 *  Naming conventions follow Intel datasheet
 *
 */

#include "nl_base.h"

#define ADDR2BYTE(a)    ((a) >> 3)
#define ADDR2BIT(a)     ((a) & 0x7)

namespace netlist::devices {

	NETLIB_OBJECT(2102A)
	{
		NETLIB_CONSTRUCTOR(2102A)
		, m_A(*this, {"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9" }, NETLIB_DELEGATE(inputs))
		, m_CEQ(*this, "CEQ", NETLIB_DELEGATE(inputs))
		, m_RWQ(*this, "RWQ", NETLIB_DELEGATE(inputs))
		, m_DI(*this, "DI", NETLIB_DELEGATE(inputs))
		, m_DO(*this, "DO")
		, m_ram(*this, "m_ram", 0)
		, m_RAM(*this, "m_RAM", &m_ram[0])
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_RAM.set(&m_ram[0]);
			for (std::size_t i=0; i<128; i++)
				m_ram[i] = 0;
		}

		NETLIB_HANDLERI(inputs)
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

	private:
		object_array_t<logic_input_t, 10> m_A;
		logic_input_t m_CEQ;
		logic_input_t m_RWQ;
		logic_input_t m_DI;

		logic_output_t m_DO;

		state_container<std::array<uint8_t, 128>> m_ram; // 1024x1 bits
		param_ptr_t m_RAM;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(2102A,    "RAM_2102A",   "+CEQ,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+A9,+RWQ,+DI,@VCC,@GND")

} // namespace netlist::devices
