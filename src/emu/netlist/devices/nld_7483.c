/*
 * nld_7483.c
 *
 */

#include "nld_7483.h"

NETLIB_START(7483)
{
	m_lastr = 0;

	register_input("A1", m_A1);
	register_input("A2", m_A2);
	register_input("A3", m_A3);
	register_input("A4", m_A4);
	register_input("B1", m_B1);
	register_input("B2", m_B2);
	register_input("B3", m_B3);
	register_input("B4", m_B4);
	register_input("C0", m_C0);

	register_output("SA", m_SA);
	register_output("SB", m_SB);
	register_output("SC", m_SC);
	register_output("SD", m_SD);
	register_output("C4", m_C4);

	save(NAME(m_lastr));
}

NETLIB_UPDATE(7483)
{
	UINT8 a = (INPLOGIC(m_A1) << 0) | (INPLOGIC(m_A2) << 1) | (INPLOGIC(m_A3) << 2) | (INPLOGIC(m_A4) << 3);
	UINT8 b = (INPLOGIC(m_B1) << 0) | (INPLOGIC(m_B2) << 1) | (INPLOGIC(m_B3) << 2) | (INPLOGIC(m_B4) << 3);

	UINT8 r = a + b + INPLOGIC(m_C0);

	if (r != m_lastr)
	{
		m_lastr = r;
		OUTLOGIC(m_SA, (r >> 0) & 1, NLTIME_FROM_NS(23));
		OUTLOGIC(m_SB, (r >> 1) & 1, NLTIME_FROM_NS(23));
		OUTLOGIC(m_SC, (r >> 2) & 1, NLTIME_FROM_NS(23));
		OUTLOGIC(m_SD, (r >> 3) & 1, NLTIME_FROM_NS(23));
		OUTLOGIC(m_C4, (r >> 4) & 1, NLTIME_FROM_NS(23));
	}
}
