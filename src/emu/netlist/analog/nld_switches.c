// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_legacy.c
 *
 */

#include "nld_switches.h"
#include "../nl_setup.h"

#define R_OFF   (1.0 / netlist().gmin())
#define R_ON    0.01

NETLIB_NAMESPACE_DEVICES_START()

// ----------------------------------------------------------------------------------------
// SWITCH
// ----------------------------------------------------------------------------------------

NETLIB_START(switch1)
{
	register_sub("R", m_R);

	register_param("POS", m_POS, 0);

	register_subalias("1", m_R.m_P);
	register_subalias("2", m_R.m_N);
}

NETLIB_RESET(switch1)
{
	m_R.do_reset();

	m_R.set_R(R_OFF);
}

NETLIB_UPDATE(switch1)
{
	if (m_POS.Value() == 0)
	{
		m_R.set_R(R_OFF);
	}
	else
	{
		m_R.set_R(R_ON);
	}

	m_R.update_dev();
}

NETLIB_UPDATE_PARAM(switch1)
{
	update();
}

// ----------------------------------------------------------------------------------------
// SWITCH2
// ----------------------------------------------------------------------------------------


NETLIB_START(switch2)
{
	register_sub("R1", m_R[0]);
	register_sub("R2", m_R[1]);

	register_param("POS", m_POS, 0);

	connect_late(m_R[0].m_N, m_R[1].m_N);

	register_subalias("1", m_R[0].m_P);
	register_subalias("2", m_R[1].m_P);

	register_subalias("Q", m_R[0].m_N);
}

NETLIB_RESET(switch2)
{
	m_R[0].do_reset();
	m_R[1].do_reset();

	m_R[0].set_R(R_ON);
	m_R[1].set_R(R_OFF);
}

NETLIB_UPDATE(switch2)
{
	if (m_POS.Value() == 0)
	{
		m_R[0].set_R(R_ON);
		m_R[1].set_R(R_OFF);
	}
	else
	{
		m_R[0].set_R(R_OFF);
		m_R[1].set_R(R_ON);
	}

	m_R[0].update_dev();
	m_R[1].update_dev();
}

NETLIB_UPDATE_PARAM(switch2)
{
	update();
}

NETLIB_NAMESPACE_DEVICES_END()
