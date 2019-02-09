// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_fourterm.c
 *
 */

#include "../solver/nld_solver.h"
#include "../nl_factory.h"
#include "nlid_fourterm.h"

#include <cmath>

namespace netlist
{
	namespace analog
	{


// ----------------------------------------------------------------------------------------
// nld_VCCS
// ----------------------------------------------------------------------------------------

NETLIB_RESET(VCCS)
{
	const nl_double m_mult = m_G() * m_gfac; // 1.0 ==> 1V ==> 1A
	const nl_double GI = plib::constants<nl_double>::one() / m_RI();

	m_IP.set(GI);
	m_IN.set(GI);

	m_OP.set(m_mult, plib::constants<nl_double>::zero());
	m_OP1.set(-m_mult, plib::constants<nl_double>::zero());

	m_ON.set(-m_mult, plib::constants<nl_double>::zero());
	m_ON1.set(m_mult, plib::constants<nl_double>::zero());
}

NETLIB_UPDATE(VCCS)
{
	/* only called if connected to a rail net ==> notify the solver to recalculate */
	if (!m_IP.net().isRailNet())
		m_IP.solve_now();
	else if (!m_IN.net().isRailNet())
		m_IN.solve_now();
	else if (!m_OP.net().isRailNet())
		m_OP.solve_now();
	else if (!m_ON.net().isRailNet())
		m_ON.solve_now();
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

	m_OP.set(beta, plib::constants<nl_double>::zero(), I);
	m_OP1.set(-beta, plib::constants<nl_double>::zero());

	m_ON.set(-beta, plib::constants<nl_double>::zero(), -I);
	m_ON1.set(beta, plib::constants<nl_double>::zero());
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

// ----------------------------------------------------------------------------------------
// nld_VCVS
// ----------------------------------------------------------------------------------------

NETLIB_RESET(VCVS)
{
	m_gfac = plib::constants<nl_double>::one() / m_RO();
	NETLIB_NAME(VCCS)::reset();

	m_OP2.set(plib::constants<nl_double>::one() / m_RO());
	m_ON2.set(plib::constants<nl_double>::one() / m_RO());
}

	} //namespace analog

	namespace devices {
		NETLIB_DEVICE_IMPL_NS(analog, VCVS,  "VCVS",  "")
		NETLIB_DEVICE_IMPL_NS(analog, VCCS,  "VCCS",  "")
		NETLIB_DEVICE_IMPL_NS(analog, CCCS,  "CCCS",  "")
		NETLIB_DEVICE_IMPL_NS(analog, LVCCS, "LVCCS", "")
	} // namespace devices
} // namespace netlist
