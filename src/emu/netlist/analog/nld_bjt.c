/*
 * nld_bjt.c
 *
 */

#include "nld_bjt.h"
#include "../nl_setup.h"
#include "nld_solver.h"

class diode
{
public:
	diode() : m_Is(1e-15), m_VT(0.0258), m_VT_inv(1.0 / m_VT) {}
	diode(const double Is, const double n)
	{
		m_Is = Is;
		m_VT = 0.0258 * n;
		m_VT_inv = 1.0 / m_VT;
	}
	void set(const double Is, const double n)
	{
		m_Is = Is;
		m_VT = 0.0258 * n;
		m_VT_inv = 1.0 / m_VT;
	}
	double I(const double V) const { return m_Is * exp(V * m_VT_inv) - m_Is; }
	double g(const double V) const { return m_Is * m_VT_inv * exp(V * m_VT_inv); }
	double V(const double I) const { return log(1.0 + I / m_Is) * m_VT; }
	double gI(const double I) const { return m_VT_inv * (I + m_Is); }

private:
	double m_Is;
	double m_VT;
	double m_VT_inv;
};



// ----------------------------------------------------------------------------------------
// nld_Q
// ----------------------------------------------------------------------------------------

NETLIB_START(Q)
{
	register_param("model", m_model, "");
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

	connect(m_RB.m_N, m_RC.m_N);

	connect(m_RB.m_P, m_BC_dummy.m_P);
	connect(m_RC.m_P, m_BC_dummy.m_N);

	save(NAME(m_state_on));

	m_RB.set(netlist().gmin(), 0.0, 0.0);
	m_RC.set(netlist().gmin(), 0.0, 0.0);

	m_BC_dummy.set(netlist().gmin(), 0.0, 0.0);

	m_state_on = 0;

	{
		double IS = m_model.model_value("IS", 1e-15);
		double BF = m_model.model_value("BF", 100);
		double NF = m_model.model_value("NF", 1);
		//double VJE = m_model.dValue("VJE", 0.75);

		set_qtype((m_model.model_type() == "NPN") ? BJT_NPN : BJT_PNP);

		double alpha = BF / (1.0 + BF);

		diode d(IS, NF);

		// Assume 5mA Collector current for switch operation

		m_V = d.V(0.005 / alpha);

		/* Base current is 0.005 / beta
		 * as a rough estimate, we just scale the conductance down */

		m_gB = d.gI(0.005 / alpha);

		if (m_gB < netlist().gmin())
			m_gB = netlist().gmin();
		m_gC =  d.gI(0.005); // very rough estimate
		//printf("%f %f \n", m_V, m_gB);
	}

}

NETLIB_UPDATE_PARAM(QBJT_switch)
{
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

	connect(m_D_EB.m_P, m_D_EC.m_P);
	connect(m_D_EB.m_N, m_D_CB.m_N);
	connect(m_D_CB.m_P, m_D_EC.m_N);

	m_gD_BE.save("m_D_BE", *this);
	m_gD_BC.save("m_D_BC", *this);

	{
		double IS = m_model.model_value("IS", 1e-15);
		double BF = m_model.model_value("BF", 100);
		double NF = m_model.model_value("NF", 1);
		double BR = m_model.model_value("BR", 1);
		double NR = m_model.model_value("NR", 1);
		//double VJE = m_model.dValue("VJE", 0.75);

		set_qtype((m_model.model_type() == "NPN") ? BJT_NPN : BJT_PNP);
		//printf("type %s\n", m_model.model_type().cstr());

		m_alpha_f = BF / (1.0 + BF);
		m_alpha_r = BR / (1.0 + BR);

		m_gD_BE.set_param(IS / m_alpha_f, NF, netlist().gmin());
		m_gD_BC.set_param(IS / m_alpha_r, NR, netlist().gmin());
	}
}

NETLIB_UPDATE(QBJT_EB)
{
	if (!m_D_EB.m_P.net().isRailNet())
		m_D_EB.m_P.net().solve();   // Basis
	else if (!m_D_EB.m_N.net().isRailNet())
		m_D_EB.m_N.net().solve();   // Emitter
	else
		m_D_CB.m_N.net().solve();   // Collector
}

NETLIB_RESET(QBJT_EB)
{
	NETLIB_NAME(Q)::reset();
}



NETLIB_UPDATE_PARAM(QBJT_EB)
{
}
