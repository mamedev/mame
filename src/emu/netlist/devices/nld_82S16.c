// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_82S16.c
 *
 */

#include "nld_82S16.h"

NETLIB_NAMESPACE_DEVICES_START()

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
		int adr = 0;
		for (int i=0; i<8; i++)
		{
			//m_A[i].activate();
			adr |= (INPLOGIC(m_A[i]) << i);
		}

		if (!INPLOGIC(m_WEQ))
		{
			m_ram[adr] = INPLOGIC(m_DIN);
		}
		OUTLOGIC(m_DOUTQ, m_ram[adr] ^ 1, NLTIME_FROM_NS(20));
	}
}

NETLIB_START(82S16)
{
	register_input("A0",    m_A[0]);
	register_input("A1",    m_A[1]);
	register_input("A2",    m_A[2]);
	register_input("A3",    m_A[3]);
	register_input("A4",    m_A[4]);
	register_input("A5",    m_A[5]);
	register_input("A6",    m_A[6]);
	register_input("A7",    m_A[7]);

	register_input("CE1Q",  m_CE1Q);
	register_input("CE2Q",  m_CE2Q);
	register_input("CE3Q",  m_CE3Q);

	register_input("WEQ",   m_WEQ);
	register_input("DIN",   m_DIN);

	register_output("DOUTQ",m_DOUTQ);

	save(NLNAME(m_ram));
}

NETLIB_RESET(82S16)
{
	for (int i=0; i<256; i++)
	{
		m_ram[i] = 0;
	}
}

NETLIB_START(82S16_dip)
{
	register_input("2",     m_A[0]);
	register_input("1",     m_A[1]);
	register_input("15",    m_A[2]);
	register_input("14",    m_A[3]);
	register_input("7",     m_A[4]);
	register_input("9",     m_A[5]);
	register_input("10",    m_A[6]);
	register_input("11",    m_A[7]);

	register_input("3",     m_CE1Q);
	register_input("4",     m_CE2Q);
	register_input("5",     m_CE3Q);

	register_input("12",    m_WEQ);
	register_input("13",    m_DIN);

	register_output("6",    m_DOUTQ);

	save(NLNAME(m_ram));
}

NETLIB_RESET(82S16_dip)
{
	NETLIB_NAME(82S16)::reset();
}

NETLIB_UPDATE(82S16_dip)
{
	NETLIB_NAME(82S16)::update();
}

NETLIB_NAMESPACE_DEVICES_END()
