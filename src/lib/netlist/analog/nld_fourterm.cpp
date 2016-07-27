// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_fourterm.c
 *
 */

#include "solver/nld_solver.h"
#include "nld_fourterm.h"
#include "nl_setup.h"

namespace netlist
{
	namespace devices
	{
// ----------------------------------------------------------------------------------------
// nld_VCCS
// ----------------------------------------------------------------------------------------

NETLIB_RESET(VCCS)
{
	const nl_double m_mult = m_G() * m_gfac; // 1.0 ==> 1V ==> 1A
	const nl_double GI = NL_FCONST(1.0) / m_RI();

	m_IP.set(GI);
	m_IN.set(GI);

	m_OP.set(m_mult, NL_FCONST(0.0));
	m_OP1.set(-m_mult, NL_FCONST(0.0));

	m_ON.set(-m_mult, NL_FCONST(0.0));
	m_ON1.set(m_mult, NL_FCONST(0.0));
}

NETLIB_UPDATE(VCCS)
{
	/* only called if connected to a rail net ==> notify the solver to recalculate */
	if (!m_IP.net().isRailNet())
		m_IP.schedule_solve();
	else if (!m_IN.net().isRailNet())
		m_IN.schedule_solve();
	else if (!m_OP.net().isRailNet())
		m_OP.schedule_solve();
	else if (!m_ON.net().isRailNet())
		m_ON.schedule_solve();
}

// ----------------------------------------------------------------------------------------
// nld_LVCCS
// ----------------------------------------------------------------------------------------

NETLIB_RESET(LVCCS)
{
	NETLIB_NAME(VCCS)::reset();
}

NETLIB_UPDATE_PARAM(LVCCS)
{
	NETLIB_NAME(VCCS)::update_param();
}

NETLIB_UPDATE(LVCCS)
{
	NETLIB_NAME(VCCS)::update();
}

NETLIB_UPDATE_TERMINALS(LVCCS)
{
	const nl_double m_mult = m_G() * m_gfac; // 1.0 ==> 1V ==> 1A
	const nl_double vi = m_IP.net().Q_Analog() - m_IN.net().Q_Analog();

	if (std::abs(m_mult / m_cur_limit() * vi) > 0.5)
		m_vi = m_vi + 0.2*std::tanh((vi - m_vi)/0.2);
	else
		m_vi = vi;

	const nl_double x = m_mult / m_cur_limit() * m_vi;
	const nl_double X = std::tanh(x);

	const nl_double beta = m_mult * (1.0 - X*X);
	const nl_double I = m_cur_limit() * X - beta * m_vi;

	m_OP.set(beta, NL_FCONST(0.0), I);
	m_OP1.set(-beta, NL_FCONST(0.0));

	m_ON.set(-beta, NL_FCONST(0.0), -I);
	m_ON1.set(beta, NL_FCONST(0.0));
}

// ----------------------------------------------------------------------------------------
// nld_CCCS
// ----------------------------------------------------------------------------------------

NETLIB_RESET(CCCS)
{
	NETLIB_NAME(VCCS)::reset();
}

NETLIB_UPDATE_PARAM(CCCS)
{
	NETLIB_NAME(VCCS)::update_param();
}

NETLIB_UPDATE(CCCS)
{
	NETLIB_NAME(VCCS)::update();
}

// ----------------------------------------------------------------------------------------
// nld_VCVS
// ----------------------------------------------------------------------------------------

NETLIB_RESET(VCVS)
{
	m_gfac = NL_FCONST(1.0) / m_RO();
	NETLIB_NAME(VCCS)::reset();

	m_OP2.set(NL_FCONST(1.0) / m_RO());
	m_ON2.set(NL_FCONST(1.0) / m_RO());
}

	} //namespace devices
} // namespace netlist
