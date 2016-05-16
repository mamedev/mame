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

NETLIB_NAMESPACE_DEVICES_END()
