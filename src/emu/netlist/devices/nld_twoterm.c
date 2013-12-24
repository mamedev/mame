/*
 * nld_twoterm.c
 *
 */

#include "nld_twoterm.h"
#include "nld_system.h"
#include "nld_solver.h"

// ----------------------------------------------------------------------------------------
// nld_twoterm
// ----------------------------------------------------------------------------------------

ATTR_COLD NETLIB_NAME(twoterm)::NETLIB_NAME(twoterm)(const family_t afamily) :
		netlist_device_t(afamily)
{
	m_P.m_otherterm = &m_N;
	m_N.m_otherterm = &m_P;
}

NETLIB_START(twoterm)
{
}

NETLIB_UPDATE(twoterm)
{
	/* only called if connected to a rail net ==> notify the solver to recalculate */
	netlist().solver()->schedule();
}

// ----------------------------------------------------------------------------------------
// nld_R
// ----------------------------------------------------------------------------------------

NETLIB_START(R_base)
{
	register_terminal("1", m_P);
	register_terminal("2", m_N);
}

NETLIB_UPDATE(R_base)
{
	NETLIB_NAME(twoterm)::update();
}

NETLIB_START(R)
{
	NETLIB_NAME(R_base)::start();
	register_param("R", m_R, 1.0 / NETLIST_GMIN);
}

NETLIB_UPDATE(R)
{
	NETLIB_NAME(twoterm)::update();
}

NETLIB_UPDATE_PARAM(R)
{
	set_R(m_R.Value());
}

// ----------------------------------------------------------------------------------------
// nld_POT
// ----------------------------------------------------------------------------------------

NETLIB_START(POT)
{
	register_sub(m_R1, "R1");
	register_sub(m_R2, "R2");

	register_subalias("1", m_R1.m_P);
	register_subalias("2", m_R1.m_N);
	register_subalias("3", m_R2.m_N);

	setup().connect(m_R2.m_P, m_R1.m_N);

	register_param("R", m_R, 1.0 / NETLIST_GMIN);
	register_param("DIAL", m_Dial, 0.5);

}

NETLIB_UPDATE(POT)
{
	m_R1.update_dev();
	m_R2.update_dev();
}

NETLIB_UPDATE_PARAM(POT)
{
	m_R1.set_R(MAX(m_R.Value() * m_Dial.Value(), NETLIST_GMIN));
	m_R2.set_R(MAX(m_R.Value() * (1.0 - m_Dial.Value()), NETLIST_GMIN));
}
// ----------------------------------------------------------------------------------------
// nld_C
// ----------------------------------------------------------------------------------------

NETLIB_START(C)
{
	register_terminal("1", m_P);
	register_terminal("2", m_N);

	register_param("C", m_C, 1e-6);
}

NETLIB_UPDATE_PARAM(C)
{
	// set to some very small step time for now
	step_time(1e-9);
}

NETLIB_UPDATE(C)
{
	NETLIB_NAME(twoterm)::update();
}

// ----------------------------------------------------------------------------------------
// nld_D
// ----------------------------------------------------------------------------------------

NETLIB_START(D)
{
	register_terminal("A", m_P);
	register_terminal("K", m_N);
	register_param("model", m_model, "");

	m_Vd = 0.7;

	save(NAME(m_Vd));

}


NETLIB_UPDATE_PARAM(D)
{
	m_Is = m_model.dValue("Is", 1e-15);
	m_n = m_model.dValue("N", 1);

	m_Vt = 0.0258 * m_n;

	m_Vcrit = m_Vt * log(m_Vt / m_Is / sqrt(2.0));
	m_VtInv = 1.0 / m_Vt;
	NL_VERBOSE_OUT(("VCutoff: %f\n", m_Vcrit));
}

NETLIB_UPDATE(D)
{
	NETLIB_NAME(twoterm)::update();
}

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

template <NETLIB_NAME(Q)::q_type _type>
NETLIB_START(QBJT_switch<_type>)
{
	NETLIB_NAME(Q)::start();

	register_sub(m_RB, "RB");
	register_sub(m_RC, "RC");
	register_input("BV", m_BV);
	register_input("EV", m_EV);

	register_subalias("B", m_RB.m_P);
	register_subalias("E", m_RB.m_N);
	register_subalias("C", m_RC.m_P);

	setup().connect(m_RB.m_N, m_RC.m_N);
	setup().connect(m_RB.m_P, m_BV);
	setup().connect(m_RB.m_N, m_EV);

	save(NAME(m_state_on));
}

NETLIB_UPDATE(Q)
{
	netlist().solver()->schedule();
}

template <NETLIB_NAME(Q)::q_type _type>
NETLIB_UPDATE_PARAM(QBJT_switch<_type>)
{
	double IS = m_model.dValue("IS", 1e-15);
	double BF = m_model.dValue("BF", 100);
	double NF = m_model.dValue("NF", 1);
	//double VJE = m_model.dValue("VJE", 0.75);

	double alpha = BF / (1.0 + BF);

	diode d(IS, NF);

	// Assume 5mA Collector current for switch operation

	if (_type == BJT_NPN)
		m_V = d.V(0.005 / alpha);
	else
		m_V = - d.V(0.005 / alpha);

	m_gB = d.gI(0.005 / alpha);
	if (m_gB < NETLIST_GMIN)
		m_gB = NETLIST_GMIN;
	m_gC = BF * m_gB; // very rough estimate
	//printf("%f %f \n", m_V, m_gB);
	m_RB.set(NETLIST_GMIN, 0.0, 0.0);
	m_RC.set(NETLIST_GMIN, 0.0, 0.0);
}

template NETLIB_START(QBJT_switch<NETLIB_NAME(Q)::BJT_NPN>);
template NETLIB_START(QBJT_switch<NETLIB_NAME(Q)::BJT_PNP>);
template NETLIB_UPDATE_PARAM(QBJT_switch<NETLIB_NAME(Q)::BJT_NPN>);
template NETLIB_UPDATE_PARAM(QBJT_switch<NETLIB_NAME(Q)::BJT_PNP>);

// ----------------------------------------------------------------------------------------
// nld_VCCS
// ----------------------------------------------------------------------------------------

NETLIB_START(VCCS)
{
	configure(1.0, NETLIST_GMIN);
}

ATTR_COLD void NETLIB_NAME(VCCS)::configure(const double Gfac, const double GI)
{
	register_param("G", m_G, 1.0);

	register_terminal("IP", m_IP);
	register_terminal("IN", m_IN);
	register_terminal("OP", m_OP);
	register_terminal("ON", m_ON);

	m_OP1.init_object(*this, name() + ".OP1", netlist_core_terminal_t::STATE_INP_ACTIVE);
	m_ON1.init_object(*this, name() + ".ON1", netlist_core_terminal_t::STATE_INP_ACTIVE);

	const double m_mult = m_G.Value() * Gfac; // 1.0 ==> 1V ==> 1A
	m_IP.set(GI);
	m_IP.m_otherterm = &m_IN; // <= this should be NULL and terminal be filtered out prior to solving...
	m_IN.set(GI);
	m_IN.m_otherterm = &m_IP; // <= this should be NULL and terminal be filtered out prior to solving...

	m_OP.set(m_mult, 0.0);
	m_OP.m_otherterm = &m_IP;
	m_OP1.set(-m_mult, 0.0);
	m_OP1.m_otherterm = &m_IN;

	m_ON.set(-m_mult, 0.0);
	m_ON.m_otherterm = &m_IP;
	m_ON1.set(m_mult, 0.0);
	m_ON1.m_otherterm = &m_IN;

	setup().connect(m_OP, m_OP1);
	setup().connect(m_ON, m_ON1);
}

NETLIB_UPDATE_PARAM(VCCS)
{
}

NETLIB_UPDATE(VCCS)
{
	/* only called if connected to a rail net ==> notify the solver to recalculate */
	netlist().solver()->schedule();
}

// ----------------------------------------------------------------------------------------
// nld_VCVS
// ----------------------------------------------------------------------------------------

NETLIB_START(VCVS)
{
	register_param("RO", m_RO, 1.0);

	const double gRO = 1.0 / m_RO.Value();

	configure(gRO, NETLIST_GMIN);

	m_OP2.init_object(*this, "OP2", netlist_core_terminal_t::STATE_INP_ACTIVE);
	m_ON2.init_object(*this, "ON2", netlist_core_terminal_t::STATE_INP_ACTIVE);

	m_OP2.set(gRO);
	m_ON2.set(gRO);
	m_OP2.m_otherterm = &m_ON2;
	m_ON2.m_otherterm = &m_OP2;

	setup().connect(m_OP2, m_OP1);
	setup().connect(m_ON2, m_ON1);
}

NETLIB_UPDATE_PARAM(VCVS)
{
}
