/*
 * nld_legacy.c
 *
 */

#include "nld_switches.h"
#include "netlist/nl_setup.h"

#define R_OFF   (1.0 / netlist().gmin())
#define R_ON    0.01

NETLIB_START(switch2)
{
	register_sub(m_R[0], "R1");
	register_sub(m_R[1], "R2");

	register_param("POS", m_POS, 0);

	connect(m_R[0].m_N, m_R[1].m_N);

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
