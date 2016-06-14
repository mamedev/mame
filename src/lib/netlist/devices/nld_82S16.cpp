// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_82S16.c
 *
 */

#include "nld_82S16.h"

namespace netlist
{
	namespace devices
	{

	NETLIB_OBJECT(82S16)
	{
		NETLIB_CONSTRUCTOR(82S16)
		, m_A(*this, {{"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7" }})
		, m_CE1Q(*this, "CE1Q")
		, m_CE2Q(*this, "CE2Q")
		, m_CE3Q(*this, "CE3Q")
		, m_WEQ(*this, "WEQ")
		, m_DIN(*this, "DIN")
		, m_DOUTQ(*this, "DOUTQ")
		, m_ram(*this, "m_ram", 0)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		object_array_t<logic_input_t, 8> m_A;
		logic_input_t m_CE1Q;
		logic_input_t m_CE2Q;
		logic_input_t m_CE3Q;
		logic_input_t m_WEQ;
		logic_input_t m_DIN;
		logic_output_t m_DOUTQ;

		state_var<uint_fast64_t[4]> m_ram; // 256 bits
	};

	NETLIB_OBJECT_DERIVED(82S16_dip, 82S16)
	{
		NETLIB_CONSTRUCTOR_DERIVED(82S16_dip, 82S16)
		{
			register_subalias("2",     m_A[0]);
			register_subalias("1",     m_A[1]);
			register_subalias("15",    m_A[2]);
			register_subalias("14",    m_A[3]);
			register_subalias("7",     m_A[4]);
			register_subalias("9",     m_A[5]);
			register_subalias("10",    m_A[6]);
			register_subalias("11",    m_A[7]);

			register_subalias("3",     m_CE1Q);
			register_subalias("4",     m_CE2Q);
			register_subalias("5",     m_CE3Q);

			register_subalias("12",    m_WEQ);
			register_subalias("13",    m_DIN);

			register_subalias("6",    m_DOUTQ);
		}
	};

	// FIXME: timing!
	// FIXME: optimize device (separate address decoder!)
	NETLIB_UPDATE(82S16)
	{
		if (INPLOGIC(m_CE1Q) || INPLOGIC(m_CE2Q) || INPLOGIC(m_CE3Q))
		{
			// FIXME: Outputs are tristate. This needs to be properly implemented
			OUTLOGIC(m_DOUTQ, 1, NLTIME_FROM_NS(20));
			//for (int i=0; i<8; i++)
				//m_A[i].inactivate();
		}
		else
		{
			unsigned int adr = 0;
			for (int i=0; i<8; i++)
			{
				//m_A[i].activate();
				adr |= (INPLOGIC(m_A[i]) << i);
			}

			if (!INPLOGIC(m_WEQ))
			{
				m_ram[adr >> 6] = (m_ram[adr >> 6] & ~((uint_fast64_t) 1 << (adr & 0x3f))) | ((uint_fast64_t) INPLOGIC(m_DIN) << (adr & 0x3f));
			}
			OUTLOGIC(m_DOUTQ, ((m_ram[adr >> 6] >> (adr & 0x3f)) & 1) ^ 1, NLTIME_FROM_NS(20));
		}
	}

	NETLIB_RESET(82S16)
	{
		for (int i=0; i<4; i++)
		{
			m_ram[i] = 0;
		}
	}

	NETLIB_DEVICE_IMPL(82S16)
	NETLIB_DEVICE_IMPL(82S16_dip)

	} //namespace devices
} // namespace netlist
