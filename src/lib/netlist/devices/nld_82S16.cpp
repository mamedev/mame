// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_82S16.c
 *
 */

#include "nld_82S16.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(82S16)
	{
		NETLIB_CONSTRUCTOR(82S16)
		, m_A(*this, {"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"}, NETLIB_DELEGATE(addr))
		, m_CE1Q(*this, "CE1Q", NETLIB_DELEGATE(enq))
		, m_CE2Q(*this, "CE2Q", NETLIB_DELEGATE(enq))
		, m_CE3Q(*this, "CE3Q", NETLIB_DELEGATE(enq))
		, m_WEQ(*this, "WEQ")
		, m_DIN(*this, "DIN")
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
		NETLIB_UPDATEI();
		NETLIB_HANDLERI(addr)
		{
			uint8_t adr = 0;
			for (std::size_t i=0; i<8; i++)
			{
				//m_A[i].activate();
				adr |= (m_A[i]() << i);
			}
			m_addr = adr;
			NETLIB_NAME(82S16)::update();
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
				NETLIB_NAME(82S16)::update();
			}
		}

		friend class NETLIB_NAME(82S16_dip);
	private:
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

	NETLIB_OBJECT(82S16_dip)
	{
		NETLIB_CONSTRUCTOR(82S16_dip)
		, A(*this, "A")
		{
			register_subalias("2",     A.m_A[0]);
			register_subalias("1",     A.m_A[1]);
			register_subalias("15",    A.m_A[2]);
			register_subalias("14",    A.m_A[3]);
			register_subalias("7",     A.m_A[4]);
			register_subalias("9",     A.m_A[5]);
			register_subalias("10",    A.m_A[6]);
			register_subalias("11",    A.m_A[7]);

			register_subalias("3",     A.m_CE1Q);
			register_subalias("4",     A.m_CE2Q);
			register_subalias("5",     A.m_CE3Q);

			register_subalias("12",    A.m_WEQ);
			register_subalias("13",    A.m_DIN);

			register_subalias("6",     A.m_DOUTQ);

			register_subalias("8",     "A.GND");
			register_subalias("16",    "A.VCC");
		}
		NETLIB_RESETI() {}
		NETLIB_UPDATEI() {}
	private:
		NETLIB_SUB(82S16) A;
	};

	// FIXME: timing!
	// FIXME: optimize device (separate address decoder!)
	NETLIB_UPDATE(82S16)
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

	NETLIB_DEVICE_IMPL(82S16,     "TTL_82S16",     "")
	NETLIB_DEVICE_IMPL(82S16_dip, "TTL_82S16_DIP", "")

	} //namespace devices
} // namespace netlist
