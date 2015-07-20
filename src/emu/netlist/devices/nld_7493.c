// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7493.c
 *
 */

#include "nld_7493.h"
#include "../nl_setup.h"

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_START(7493)
{
	register_sub("A", A);
	register_sub("B", B);
	register_sub("C", C);
	register_sub("D", D);

	register_subalias("CLKA", A.m_I);
	register_subalias("CLKB", B.m_I);
	register_input("R1",  m_R1);
	register_input("R2",  m_R2);

	register_subalias("QA", A.m_Q);
	register_subalias("QB", B.m_Q);
	register_subalias("QC", C.m_Q);
	register_subalias("QD", D.m_Q);

	connect_late(C.m_I, B.m_Q);
	connect_late(D.m_I, C.m_Q);
}

NETLIB_RESET(7493)
{
	A.do_reset();
	B.do_reset();
	C.do_reset();
	D.do_reset();
}

NETLIB_START(7493ff)
{
	register_input("CLK", m_I);
	register_output("Q", m_Q);

	save(NLNAME(m_reset));
	save(NLNAME(m_state));
}

NETLIB_RESET(7493ff)
{
	m_reset = 1;
	m_state = 0;
	m_I.set_state(logic_t::STATE_INP_HL);
}

NETLIB_UPDATE(7493ff)
{
	const netlist_time out_delay = NLTIME_FROM_NS(18);
	if (m_reset != 0)
	{
		m_state ^= 1;
		OUTLOGIC(m_Q, m_state, out_delay);
	}
}

NETLIB_UPDATE(7493)
{
	const netlist_sig_t r = INPLOGIC(m_R1) & INPLOGIC(m_R2);

	if (r)
	{
		A.m_I.inactivate();
		B.m_I.inactivate();
		OUTLOGIC(A.m_Q, 0, NLTIME_FROM_NS(40));
		OUTLOGIC(B.m_Q, 0, NLTIME_FROM_NS(40));
		OUTLOGIC(C.m_Q, 0, NLTIME_FROM_NS(40));
		OUTLOGIC(D.m_Q, 0, NLTIME_FROM_NS(40));
		A.m_reset = B.m_reset = C.m_reset = D.m_reset = 0;
		A.m_state = B.m_state = C.m_state = D.m_state = 0;
	}
	else
	{
		A.m_I.activate_hl();
		B.m_I.activate_hl();
		A.m_reset = B.m_reset = C.m_reset = D.m_reset = 1;
	}
}

NETLIB_START(7493_dip)
{
	NETLIB_NAME(7493)::start();

	register_subalias("1", B.m_I);
	register_subalias("2", m_R1);
	register_subalias("3", m_R2);

	// register_subalias("4", ); --> NC
	// register_subalias("5", ); --> VCC
	// register_subalias("6", ); --> NC
	// register_subalias("7", ); --> NC

	register_subalias("8", C.m_Q);
	register_subalias("9", B.m_Q);
	// register_subalias("10", ); --> GND
	register_subalias("11", D.m_Q);
	register_subalias("12", A.m_Q);
	// register_subalias("13", ); --> NC
	register_subalias("14", A.m_I);
}


NETLIB_UPDATE(7493_dip)
{
	NETLIB_NAME(7493)::update();
}

NETLIB_RESET(7493_dip)
{
	NETLIB_NAME(7493)::reset();
}

NETLIB_NAMESPACE_DEVICES_END()
