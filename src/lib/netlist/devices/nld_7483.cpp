// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7483.c
 *
 */

#include "nld_7483.h"

namespace netlist
{
	namespace devices
	{

NETLIB_RESET(7483)
{
	m_lastr = 0;
}

NETLIB_UPDATE(7483)
{
	UINT8 a = (INPLOGIC(m_A1) << 0) | (INPLOGIC(m_A2) << 1) | (INPLOGIC(m_A3) << 2) | (INPLOGIC(m_A4) << 3);
	UINT8 b = (INPLOGIC(m_B1) << 0) | (INPLOGIC(m_B2) << 1) | (INPLOGIC(m_B3) << 2) | (INPLOGIC(m_B4) << 3);

	UINT8 r = a + b + INPLOGIC(m_C0);

	if (r != m_lastr)
	{
		m_lastr = r;
		OUTLOGIC(m_S1, (r >> 0) & 1, NLTIME_FROM_NS(23));
		OUTLOGIC(m_S2, (r >> 1) & 1, NLTIME_FROM_NS(23));
		OUTLOGIC(m_S3, (r >> 2) & 1, NLTIME_FROM_NS(23));
		OUTLOGIC(m_S4, (r >> 3) & 1, NLTIME_FROM_NS(23));
		OUTLOGIC(m_C4, (r >> 4) & 1, NLTIME_FROM_NS(23));
	}
}

	} //namespace devices
} // namespace netlist
