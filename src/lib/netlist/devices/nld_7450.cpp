// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7450.c
 *
 */

#include "nld_7450.h"

namespace netlist
{
	namespace devices
	{

NETLIB_UPDATE(7450)
{
	m_A.activate();
	m_B.activate();
	m_C.activate();
	m_D.activate();
	UINT8 t1 = INPLOGIC(m_A) & INPLOGIC(m_B);
	UINT8 t2 = INPLOGIC(m_C) & INPLOGIC(m_D);

	const netlist_time times[2] = { NLTIME_FROM_NS(22), NLTIME_FROM_NS(15) };

	UINT8 res = 0;
	if (t1 ^ 1)
	{
		if (t2 ^ 1)
		{
			res = 1;
		}
		else
		{
			m_A.inactivate();
			m_B.inactivate();
		}
	} else {
		if (t2 ^ 1)
		{
			m_C.inactivate();
			m_D.inactivate();
		}
	}
	OUTLOGIC(m_Q, res, times[1 - res]);// ? 22000 : 15000);
}


	} //namespace devices
} // namespace netlist
