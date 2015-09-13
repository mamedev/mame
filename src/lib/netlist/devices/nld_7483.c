// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7483.c
 *
 */

#include "nld_7483.h"

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_START(7483)
{
	register_input("A1", m_A1);
	register_input("A2", m_A2);
	register_input("A3", m_A3);
	register_input("A4", m_A4);
	register_input("B1", m_B1);
	register_input("B2", m_B2);
	register_input("B3", m_B3);
	register_input("B4", m_B4);
	register_input("C0", m_C0);

	register_output("S1", m_S1);
	register_output("S2", m_S2);
	register_output("S3", m_S3);
	register_output("S4", m_S4);
	register_output("C4", m_C4);

	save(NLNAME(m_lastr));
}

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

NETLIB_START(7483_dip)
{
	NETLIB_NAME(7483)::start();
	register_subalias("1", m_A4);
	register_subalias("2", m_S3);
	register_subalias("3", m_A3);
	register_subalias("4", m_B3);
	// register_subalias("5", ); --> VCC
	register_subalias("6", m_S2);
	register_subalias("7", m_B2);
	register_subalias("8", m_A2);

	register_subalias("9", m_S1);
	register_subalias("10", m_A1);
	register_subalias("11", m_B1);
	// register_subalias("12", ); --> GND
	register_subalias("13", m_C0);
	register_subalias("14", m_C4);
	register_subalias("15", m_S4);
	register_subalias("16", m_B4);

}

NETLIB_UPDATE(7483_dip)
{
	NETLIB_NAME(7483)::update();
}

NETLIB_RESET(7483_dip)
{
	NETLIB_NAME(7483)::reset();
}

NETLIB_NAMESPACE_DEVICES_END()
