// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_twoterm.c
 *
 */

#include <solver/nld_solver.h>
#include <algorithm>

#include "nld_twoterm.h"

NETLIB_NAMESPACE_DEVICES_START()

// ----------------------------------------------------------------------------------------
// generic_diode
// ----------------------------------------------------------------------------------------

ATTR_COLD generic_diode::generic_diode()
{
	m_Vd = 0.7;
	set_param(1e-15, 1, 1e-15);
	m_G = m_gmin;
	m_Id = 0.0;
}

ATTR_COLD void generic_diode::set_param(const nl_double Is, const nl_double n, nl_double gmin)
{
	static const double csqrt2 = nl_math::sqrt(2.0);
	m_Is = Is;
	m_n = n;
	m_gmin = gmin;

	m_Vt = 0.0258 * m_n;

	m_Vcrit = m_Vt * nl_math::log(m_Vt / m_Is / csqrt2);
	m_VtInv = 1.0 / m_Vt;
}

ATTR_COLD void generic_diode::save(pstring name, object_t &parent)
{
	parent.save(m_Vd, name + ".m_Vd");
	parent.save(m_Id, name + ".m_Id");
	parent.save(m_G, name + ".m_G");
}

// ----------------------------------------------------------------------------------------
// nld_twoterm
// ----------------------------------------------------------------------------------------

ATTR_COLD NETLIB_NAME(twoterm)::NETLIB_NAME(twoterm)(const family_t afamily)
		: device_t(afamily)
{
	m_P.m_otherterm = &m_N;
	m_N.m_otherterm = &m_P;
}

ATTR_COLD NETLIB_NAME(twoterm)::NETLIB_NAME(twoterm)()
		: device_t(TWOTERM)
{
	m_P.m_otherterm = &m_N;
	m_N.m_otherterm = &m_P;
}

NETLIB_START(twoterm)
{
}

NETLIB_RESET(twoterm)
{
}


NETLIB_UPDATE(twoterm)
{
	/* only called if connected to a rail net ==> notify the solver to recalculate */
	/* we only need to call the non-rail terminal */
	if (!m_P.net().isRailNet())
		m_P.schedule_solve();
	else
		m_N.schedule_solve();
}

// ----------------------------------------------------------------------------------------
// nld_R
// ----------------------------------------------------------------------------------------

NETLIB_START(R_base)
{
	NETLIB_NAME(twoterm)::start();
	register_terminal("1", m_P);
	register_terminal("2", m_N);
}

NETLIB_RESET(R_base)
{
	NETLIB_NAME(twoterm)::reset();
	set_R(1.0 / netlist().gmin());
}

NETLIB_UPDATE(R_base)
{
	NETLIB_NAME(twoterm)::update();
}

NETLIB_START(R)
{
	NETLIB_NAME(R_base)::start();
	register_param("R", m_R, 1.0 / netlist().gmin());
}

NETLIB_RESET(R)
{
	NETLIB_NAME(R_base)::reset();
}

NETLIB_UPDATE(R)
{
	NETLIB_NAME(twoterm)::update();
}

NETLIB_UPDATE_PARAM(R)
{
	update_dev();
	if (m_R.Value() > 1e-9)
		set_R(m_R.Value());
	else
		set_R(1e-9);
}

// ----------------------------------------------------------------------------------------
// nld_POT
// ----------------------------------------------------------------------------------------

NETLIB_START(POT)
{
	register_sub("R1", m_R1);
	register_sub("R2", m_R2);

	register_subalias("1", m_R1.m_P);
	register_subalias("2", m_R1.m_N);
	register_subalias("3", m_R2.m_N);

	connect_late(m_R2.m_P, m_R1.m_N);

	register_param("R", m_R, 1.0 / netlist().gmin());
	register_param("DIAL", m_Dial, 0.5);
	register_param("DIALLOG", m_DialIsLog, 0);

}

NETLIB_RESET(POT)
{
	m_R1.do_reset();
	m_R2.do_reset();
}

NETLIB_UPDATE(POT)
{
	m_R1.update_dev();
	m_R2.update_dev();
}

NETLIB_UPDATE_PARAM(POT)
{
	nl_double v = m_Dial.Value();
	if (m_DialIsLog.Value())
		v = (nl_math::exp(v) - 1.0) / (nl_math::exp(1.0) - 1.0);

	m_R1.update_dev();
	m_R2.update_dev();

	m_R1.set_R(std::max(m_R.Value() * v, netlist().gmin()));
	m_R2.set_R(std::max(m_R.Value() * (NL_FCONST(1.0) - v), netlist().gmin()));

}

// ----------------------------------------------------------------------------------------
// nld_POT2
// ----------------------------------------------------------------------------------------

NETLIB_START(POT2)
{
	register_sub("R1", m_R1);

	register_subalias("1", m_R1.m_P);
	register_subalias("2", m_R1.m_N);

	register_param("R", m_R, 1.0 / netlist().gmin());
	register_param("DIAL", m_Dial, 0.5);
	register_param("REVERSE", m_Reverse, 0);
	register_param("DIALLOG", m_DialIsLog, 0);

}

NETLIB_RESET(POT2)
{
	m_R1.do_reset();
}

NETLIB_UPDATE(POT2)
{
	m_R1.update_dev();
}

NETLIB_UPDATE_PARAM(POT2)
{
	nl_double v = m_Dial.Value();

	if (m_DialIsLog.Value())
		v = (nl_math::exp(v) - 1.0) / (nl_math::exp(1.0) - 1.0);
	if (m_Reverse.Value())
		v = 1.0 - v;

	m_R1.update_dev();

	m_R1.set_R(std::max(m_R.Value() * v, netlist().gmin()));
}

// ----------------------------------------------------------------------------------------
// nld_C
// ----------------------------------------------------------------------------------------

NETLIB_START(C)
{
	register_terminal("1", m_P);
	register_terminal("2", m_N);

	register_param("C", m_C, 1e-6);

	// set up the element
	//set(netlist().gmin(), 0.0, -5.0 / netlist().gmin());
	//set(1.0/NETLIST_GMIN, 0.0, -5.0 * NETLIST_GMIN);
}

NETLIB_RESET(C)
{
	set(netlist().gmin(), 0.0, -5.0 / netlist().gmin());
	//set(1.0/NETLIST_GMIN, 0.0, -5.0 * NETLIST_GMIN);
}

NETLIB_UPDATE_PARAM(C)
{
	//step_time(1.0/48000.0);
	m_GParallel = netlist().gmin() * m_C.Value();
}

NETLIB_UPDATE(C)
{
	NETLIB_NAME(twoterm)::update();
}

ATTR_HOT void NETLIB_NAME(C)::step_time(const nl_double st)
{
	/* Gpar should support convergence */
	const nl_double G = m_C.Value() / st +  m_GParallel;
	const nl_double I = -G * deltaV();
	set(G, 0.0, I);
}

// ----------------------------------------------------------------------------------------
// nld_D
// ----------------------------------------------------------------------------------------

NETLIB_START(D)
{
	register_terminal("A", m_P);
	register_terminal("K", m_N);
	register_param("MODEL", m_model, "");

	m_D.save("m_D", *this);

}


NETLIB_UPDATE_PARAM(D)
{
	nl_double Is = m_model.model_value("IS");
	nl_double n = m_model.model_value("N");

	m_D.set_param(Is, n, netlist().gmin());
}

NETLIB_UPDATE(D)
{
	NETLIB_NAME(twoterm)::update();
}

NETLIB_UPDATE_TERMINALS(D)
{
	m_D.update_diode(deltaV());
	set(m_D.G(), 0.0, m_D.Ieq());
}

// ----------------------------------------------------------------------------------------
// nld_VS
// ----------------------------------------------------------------------------------------

NETLIB_START(VS)
{
	NETLIB_NAME(twoterm)::start();

	register_param("R", m_R, 0.1);
	register_param("V", m_V, 0.0);

	register_terminal("P", m_P);
	register_terminal("N", m_N);
}

NETLIB_RESET(VS)
{
	NETLIB_NAME(twoterm)::reset();
	this->set(1.0 / m_R, m_V, 0.0);
}

NETLIB_UPDATE(VS)
{
	NETLIB_NAME(twoterm)::update();
}

// ----------------------------------------------------------------------------------------
// nld_CS
// ----------------------------------------------------------------------------------------

NETLIB_START(CS)
{
	NETLIB_NAME(twoterm)::start();

	register_param("I", m_I, 1.0);

	register_terminal("P", m_P);
	register_terminal("N", m_N);
}

NETLIB_RESET(CS)
{
	NETLIB_NAME(twoterm)::reset();
	this->set(0.0, 0.0, m_I);
}

NETLIB_UPDATE(CS)
{
	NETLIB_NAME(twoterm)::update();
}

NETLIB_NAMESPACE_DEVICES_END()
