// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74153.c
 *
 */

#include "nld_74153.h"

NETLIB_NAMESPACE_DEVICES_START()

/* FIXME: timing is not 100% accurate, Strobe and Select inputs have a
 *        slightly longer timing.
 *        Convert this to sub-devices at some time.
 */


NETLIB_RESET(74153sub)
{
	m_chan = 0;
}

NETLIB_UPDATE(74153sub)
{
	const netlist_time delay[2] = { NLTIME_FROM_NS(23), NLTIME_FROM_NS(18) };
	if (!INPLOGIC(m_G))
	{
		UINT8 t = INPLOGIC(m_C[m_chan]);
		OUTLOGIC(m_Y, t, delay[t] );
	}
	else
	{
		OUTLOGIC(m_Y, 0, delay[0]);
	}
}


NETLIB_UPDATE(74153)
{
	m_sub.m_chan = (INPLOGIC(m_A) | (INPLOGIC(m_B)<<1));
	m_sub.do_update();
}


NETLIB_UPDATE(74153_dip)
{
	m_2.m_chan = m_1.m_chan = (INPLOGIC(m_A) | (INPLOGIC(m_B)<<1));
	m_1.do_update();
	m_2.do_update();
}

NETLIB_NAMESPACE_DEVICES_END()
