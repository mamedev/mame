// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_bjt.c
 *
 */

#include "solver/nld_solver.h"
#include "analog/nld_bjt.h"
#include "nl_setup.h"

NETLIB_NAMESPACE_DEVICES_START()

class diode
{
public:
	diode() : m_Is(1e-15), m_VT(0.0258), m_VT_inv(1.0 / m_VT) {}
	diode(const nl_double Is, const nl_double n)
	{
		m_Is = Is;
		m_VT = 0.0258 * n;
		m_VT_inv = 1.0 / m_VT;
	}
	void set(const nl_double Is, const nl_double n)
	{
		m_Is = Is;
		m_VT = 0.0258 * n;
		m_VT_inv = 1.0 / m_VT;
	}
	nl_double I(const nl_double V) const { return m_Is * nl_math::exp(V * m_VT_inv) - m_Is; }
	nl_double g(const nl_double V) const { return m_Is * m_VT_inv * nl_math::exp(V * m_VT_inv); }
	nl_double V(const nl_double I) const { return nl_math::e_log1p(I / m_Is) * m_VT; } // log1p(x)=log(1.0 + x)
	nl_double gI(const nl_double I) const { return m_VT_inv * (I + m_Is); }

private:
	nl_double m_Is;
	nl_double m_VT;
	nl_double m_VT_inv;
};



// ----------------------------------------------------------------------------------------
// nld_Q
// ----------------------------------------------------------------------------------------

NETLIB_NAME(Q)::NETLIB_NAME(Q)(const family_t afamily)
: device_t(afamily)
, m_qtype(BJT_NPN) { }

NETLIB_NAME(Q)::~NETLIB_NAME(Q)()
{
}


NETLIB_START(Q)
{
	register_param("MODEL", m_model, "");
}

NETLIB_RESET(Q)
{
}

NETLIB_UPDATE(Q)
{
//    netlist().solver()->schedule1();
}

// ----------------------------------------------------------------------------------------
// nld_QBJT_switch
// ----------------------------------------------------------------------------------------

NETLIB_START(QBJT_switch)
{
	NETLIB_NAME(Q)::start();

	register_terminal("B", m_RB.m_P);
	register_terminal("E", m_RB.m_N);
	register_terminal("C", m_RC.m_P);
	register_terminal("_E1", m_RC.m_N);

	register_terminal("_B1", m_BC_dummy.m_P);
	register_terminal("_C1", m_BC_dummy.m_N);

	connect_late(m_RB.m_N, m_RC.m_N);

	connect_late(m_RB.m_P, m_BC_dummy.m_P);
	connect_late(m_RC.m_P, m_BC_dummy.m_N);

	save(NLNAME(m_state_on));

}

NETLIB_RESET(QBJT_switch)
{
	NETLIB_NAME(Q)::reset();

	m_state_on = 0;

	m_RB.set(netlist().gmin(), 0.0, 0.0);
	m_RC.set(netlist().gmin(), 0.0, 0.0);

	m_BC_dummy.set(netlist().gmin() / 10.0, 0.0, 0.0);

}

NETLIB_UPDATE(QBJT_switch)
{
	if (!m_RB.m_P.net().isRailNet())
		m_RB.m_P.schedule_solve();   // Basis
	else if (!m_RB.m_N.net().isRailNet())
		m_RB.m_N.schedule_solve();   // Emitter
	else if (!m_RC.m_P.net().isRailNet())
		m_RC.m_P.schedule_solve();   // Collector
}


NETLIB_UPDATE_PARAM(QBJT_switch)
{
	nl_double IS = m_model.model_value("IS");
	nl_double BF = m_model.model_value("BF");
	nl_double NF = m_model.model_value("NF");
	//nl_double VJE = m_model.dValue("VJE", 0.75);

	set_qtype((m_model.model_type() == "NPN") ? BJT_NPN : BJT_PNP);

	nl_double alpha = BF / (1.0 + BF);

	diode d(IS, NF);

	// Assume 5mA Collector current for switch operation

	m_V = d.V(0.005 / alpha);

	/* Base current is 0.005 / beta
	 * as a rough estimate, we just scale the conductance down */

	m_gB = 1.0 / (m_V/(0.005 / BF));

	//m_gB = d.gI(0.005 / alpha);

	if (m_gB < netlist().gmin())
		m_gB = netlist().gmin();
	m_gC =  d.gI(0.005); // very rough estimate
}

NETLIB_UPDATE_TERMINALS(QBJT_switch)
{
	const nl_double m = (is_qtype( BJT_NPN) ? 1 : -1);

	const int new_state = (m_RB.deltaV() * m > m_V ) ? 1 : 0;
	if (m_state_on ^ new_state)
	{
		const nl_double gb = new_state ? m_gB : netlist().gmin();
		const nl_double gc = new_state ? m_gC : netlist().gmin();
		const nl_double v  = new_state ? m_V * m : 0;

		m_RB.set(gb, v,   0.0);
		m_RC.set(gc, 0.0, 0.0);
		//m_RB.update_dev();
		//m_RC.update_dev();
		m_state_on = new_state;
	}
}


// ----------------------------------------------------------------------------------------
// nld_Q - Ebers Moll
// ----------------------------------------------------------------------------------------

NETLIB_START(QBJT_EB)
{
	NETLIB_NAME(Q)::start();

	register_terminal("E", m_D_EB.m_P);   // Cathode
	register_terminal("B", m_D_EB.m_N);   // Anode

	register_terminal("C", m_D_CB.m_P);   // Cathode
	register_terminal("_B1", m_D_CB.m_N); // Anode

	register_terminal("_E1", m_D_EC.m_P);
	register_terminal("_C1", m_D_EC.m_N);

	connect_late(m_D_EB.m_P, m_D_EC.m_P);
	connect_late(m_D_EB.m_N, m_D_CB.m_N);
	connect_late(m_D_CB.m_P, m_D_EC.m_N);

	m_gD_BE.save("m_D_BE", *this);
	m_gD_BC.save("m_D_BC", *this);

}

NETLIB_UPDATE(QBJT_EB)
{
	if (!m_D_EB.m_P.net().isRailNet())
		m_D_EB.m_P.schedule_solve();   // Basis
	else if (!m_D_EB.m_N.net().isRailNet())
		m_D_EB.m_N.schedule_solve();   // Emitter
	else
		m_D_CB.m_N.schedule_solve();   // Collector
}

NETLIB_RESET(QBJT_EB)
{
	NETLIB_NAME(Q)::reset();
}

NETLIB_UPDATE_TERMINALS(QBJT_EB)
{
	const nl_double polarity = (qtype() == BJT_NPN ? 1.0 : -1.0);

	m_gD_BE.update_diode(-m_D_EB.deltaV() * polarity);
	m_gD_BC.update_diode(-m_D_CB.deltaV() * polarity);

	const nl_double gee = m_gD_BE.G();
	const nl_double gcc = m_gD_BC.G();
	const nl_double gec =  m_alpha_r * gcc;
	const nl_double gce =  m_alpha_f * gee;
	const nl_double sIe = -m_gD_BE.I() + m_alpha_r * m_gD_BC.I();
	const nl_double sIc = m_alpha_f * m_gD_BE.I() - m_gD_BC.I();
	const nl_double Ie = (sIe + gee * m_gD_BE.Vd() - gec * m_gD_BC.Vd()) * polarity;
	const nl_double Ic = (sIc - gce * m_gD_BE.Vd() + gcc * m_gD_BC.Vd()) * polarity;

	m_D_EB.set_mat(gee, gec - gee, gce - gee, gee - gec, Ie, -Ie);
	m_D_CB.set_mat(gcc, gce - gcc, gec - gcc, gcc - gce, Ic, -Ic);
	m_D_EC.set_mat( 0,    -gec,      -gce,        0,       0,   0);
}


NETLIB_UPDATE_PARAM(QBJT_EB)
{
	nl_double IS = m_model.model_value("IS");
	nl_double BF = m_model.model_value("BF");
	nl_double NF = m_model.model_value("NF");
	nl_double BR = m_model.model_value("BR");
	nl_double NR = m_model.model_value("NR");
	//nl_double VJE = m_model.dValue("VJE", 0.75);

	set_qtype((m_model.model_type() == "NPN") ? BJT_NPN : BJT_PNP);

	m_alpha_f = BF / (1.0 + BF);
	m_alpha_r = BR / (1.0 + BR);

	m_gD_BE.set_param(IS / m_alpha_f, NF, netlist().gmin());
	m_gD_BC.set_param(IS / m_alpha_r, NR, netlist().gmin());
}

NETLIB_NAMESPACE_DEVICES_END()
