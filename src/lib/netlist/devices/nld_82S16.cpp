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
		, m_DOUTQ(*this, "DOUTQ")
		{
			enregister("A0",    m_A[0]);
			enregister("A1",    m_A[1]);
			enregister("A2",    m_A[2]);
			enregister("A3",    m_A[3]);
			enregister("A4",    m_A[4]);
			enregister("A5",    m_A[5]);
			enregister("A6",    m_A[6]);
			enregister("A7",    m_A[7]);

			enregister("CE1Q",  m_CE1Q);
			enregister("CE2Q",  m_CE2Q);
			enregister("CE3Q",  m_CE3Q);

			enregister("WEQ",   m_WEQ);
			enregister("DIN",   m_DIN);

			save(NLNAME(m_ram));

		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		logic_input_t m_A[8];
		logic_input_t m_CE1Q;
		logic_input_t m_CE2Q;
		logic_input_t m_CE3Q;
		logic_input_t m_WEQ;
		logic_input_t m_DIN;
		logic_output_t m_DOUTQ;

		//netlist_state_t<UINT8[256]> m_ram;
		UINT64 m_ram[4]; // 256 bits
	};

	NETLIB_OBJECT_DERIVED(82S16_dip, 82S16)
	{
		NETLIB_CONSTRUCTOR_DERIVED(82S16_dip, 82S16)
		{
			enregister("2",     m_A[0]);
			enregister("1",     m_A[1]);
			enregister("15",    m_A[2]);
			enregister("14",    m_A[3]);
			enregister("7",     m_A[4]);
			enregister("9",     m_A[5]);
			enregister("10",    m_A[6]);
			enregister("11",    m_A[7]);

			enregister("3",     m_CE1Q);
			enregister("4",     m_CE2Q);
			enregister("5",     m_CE3Q);

			enregister("12",    m_WEQ);
			enregister("13",    m_DIN);

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
				m_ram[adr >> 6] = (m_ram[adr >> 6] & ~((UINT64) 1 << (adr & 0x3f))) | ((UINT64) INPLOGIC(m_DIN) << (adr & 0x3f));
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
