/*
 * nld_fourterm.c
 *
 */

#include "nld_fourterm.h"
#include "../nl_setup.h"
#include "nld_solver.h"

// ----------------------------------------------------------------------------------------
// nld_VCCS
// ----------------------------------------------------------------------------------------

NETLIB_START(VCCS)
{
	register_param("G", m_G, 1.0);
	register_param("RI", m_RI, 1.0 / netlist().gmin());

	register_terminal("IP", m_IP);
	register_terminal("IN", m_IN);
	register_terminal("OP", m_OP);
	register_terminal("ON", m_ON);

	register_terminal("_OP1", m_OP1);
	register_terminal("_ON1", m_ON1);

	m_IP.m_otherterm = &m_IN; // <= this should be NULL and terminal be filtered out prior to solving...
	m_IN.m_otherterm = &m_IP; // <= this should be NULL and terminal be filtered out prior to solving...

	m_OP.m_otherterm = &m_IP;
	m_OP1.m_otherterm = &m_IN;

	m_ON.m_otherterm = &m_IP;
	m_ON1.m_otherterm = &m_IN;

	connect(m_OP, m_OP1);
	connect(m_ON, m_ON1);

	m_gfac = 1.0;
}

NETLIB_RESET(VCCS)
{
	const double m_mult = m_G.Value() * m_gfac; // 1.0 ==> 1V ==> 1A
	const double GI = 1.0 / m_RI.Value();

	m_IP.set(GI);
	m_IN.set(GI);

	m_OP.set(m_mult, 0.0);
	m_OP1.set(-m_mult, 0.0);

	m_ON.set(-m_mult, 0.0);
	m_ON1.set(m_mult, 0.0);
}

NETLIB_UPDATE_PARAM(VCCS)
{
}

NETLIB_UPDATE(VCCS)
{
	/* only called if connected to a rail net ==> notify the solver to recalculate */
	/* Big FIXME ... */
	if (!m_IP.net().isRailNet())
		m_IP.net().solve();
	else if (!m_IN.net().isRailNet())
		m_IN.net().solve();
	else if (!m_OP.net().isRailNet())
		m_OP.net().solve();
	else if (!m_ON.net().isRailNet())
		m_ON.net().solve();
}

// ----------------------------------------------------------------------------------------
// nld_VCVS
// ----------------------------------------------------------------------------------------

NETLIB_START(VCVS)
{
	NETLIB_NAME(VCCS)::start();

	register_param("RO", m_RO, 1.0);

	register_terminal("_OP2", m_OP2);
	register_terminal("_ON2", m_ON2);

	m_OP2.m_otherterm = &m_ON2;
	m_ON2.m_otherterm = &m_OP2;

	connect(m_OP2, m_OP1);
	connect(m_ON2, m_ON1);
}

NETLIB_RESET(VCVS)
{
	m_gfac = 1.0 / m_RO.Value();
	NETLIB_NAME(VCCS)::reset();

	m_OP2.set(1.0 / m_RO.Value());
	m_ON2.set(1.0 / m_RO.Value());
}

NETLIB_UPDATE_PARAM(VCVS)
{
}
