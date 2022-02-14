// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_82S16.cpp
 *
 *
 *  DM82S16: 256 Bit bipolar ram
 *
 *          +--------------+
 *       A1 |1     ++    16| VCC
 *       A0 |2           15| A2
 *     CE1Q |3           14| A3
 *     CE2Q |4   82S16   13| DIN
 *     CE3Q |5           12| WEQ
 *    DOUTQ |6           11| A7
 *       A4 |7           10| A6
 *      GND |8            9| A5
 *          +--------------+
 *
 *
 *  Naming conventions follow Signetics datasheet
 *
 */

#include "nl_base.h"

namespace netlist::devices {

	NETLIB_OBJECT(82S16)
	{
		NETLIB_CONSTRUCTOR(82S16)
		, m_A(*this, {"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"}, NETLIB_DELEGATE(addr))
		, m_CE1Q(*this, "CE1Q", NETLIB_DELEGATE(enq))
		, m_CE2Q(*this, "CE2Q", NETLIB_DELEGATE(enq))
		, m_CE3Q(*this, "CE3Q", NETLIB_DELEGATE(enq))
		, m_WEQ(*this, "WEQ", NETLIB_DELEGATE(inputs))
		, m_DIN(*this, "DIN", NETLIB_DELEGATE(inputs))
		, m_DOUTQ(*this, "DOUTQ")
		, m_ram(*this, "m_ram", 0)
		, m_addr(*this, "m_addr", 0)
		, m_enq(*this, "m_enq", 0)
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			for (std::size_t i=0; i<4; i++)
			{
				m_ram[i] = 0;
			}
			m_addr = 0;
			m_enq = 0;
		}

	private:
		// FIXME: timing!
		// FIXME: optimize device (separate address decoder!)
		NETLIB_HANDLERI(inputs)
		{
			if (!m_enq)
			{
				const decltype(m_addr)::value_type adr(m_addr);
				if (!m_WEQ())
				{
					m_ram[adr >> 6] = (m_ram[adr >> 6]
							& ~(static_cast<uint64_t>(1) << (adr & 0x3f)))
							| (static_cast<uint64_t>(m_DIN()) << (adr & 0x3f));
				}
				m_DOUTQ.push(((m_ram[adr >> 6] >> (adr & 0x3f)) & 1) ^ 1, NLTIME_FROM_NS(20));
			}
		}

		NETLIB_HANDLERI(addr)
		{
			uint8_t adr = 0;
			for (std::size_t i=0; i<8; i++)
			{
				//m_A[i].activate();
				adr |= (m_A[i]() << i);
			}
			m_addr = adr;
			inputs();
		}
		NETLIB_HANDLERI(enq)
		{
			const decltype(m_enq)::value_type last = m_enq;
			m_enq = m_CE1Q() || m_CE2Q() || m_CE3Q();
			if (!last && m_enq)
			{
				// FIXME: Outputs are tristate. This needs to be properly implemented
				m_DOUTQ.push(1, NLTIME_FROM_NS(20));
				for (std::size_t i=0; i<8; i++)
					m_A[i].inactivate();
				m_WEQ.inactivate();
				m_DIN.inactivate();
			}
			else if (last && !m_enq)
			{
				for (std::size_t i=0; i<8; i++)
					m_A[i].activate();
				m_WEQ.activate();
				m_DIN.activate();
				inputs();
			}
		}

		object_array_t<logic_input_t, 8> m_A;
		logic_input_t m_CE1Q;
		logic_input_t m_CE2Q;
		logic_input_t m_CE3Q;
		logic_input_t m_WEQ;
		logic_input_t m_DIN;
		logic_output_t m_DOUTQ;

		state_container<std::array<uint64_t, 4>> m_ram; // 256 bits
		state_var_u8 m_addr; // 256 bits
		state_var_sig m_enq;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(82S16,     "TTL_82S16",     "")

} // namespace netlist::devices
