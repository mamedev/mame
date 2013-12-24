/*
 * nld_7493.c
 *
 */

#include "nld_7493.h"

NETLIB_START(7493)
{
	register_sub(A, "A");
	register_sub(B, "B");
	register_sub(C, "C");
	register_sub(D, "D");

	register_subalias("CLKA", A.m_I);
	register_subalias("CLKB", B.m_I);
	register_input("R1",  m_R1);
	register_input("R2",  m_R2);

	register_subalias("QA", A.m_Q);
	register_subalias("QB", B.m_Q);
	register_subalias("QC", C.m_Q);
	register_subalias("QD", D.m_Q);

	register_link_internal(C, C.m_I, B.m_Q, netlist_input_t::STATE_INP_HL);
	register_link_internal(D, D.m_I, C.m_Q, netlist_input_t::STATE_INP_HL);
}

NETLIB_START(7493ff)
{
	m_reset = 0;

	register_input("CLK", m_I, netlist_input_t::STATE_INP_HL);
	register_output("Q", m_Q);

	save(NAME(m_reset));
}

NETLIB_UPDATE(7493ff)
{
	if (m_reset == 0)
		OUTLOGIC(m_Q, !m_Q.net().new_Q(), NLTIME_FROM_NS(18));
}

NETLIB_UPDATE(7493)
{
	netlist_sig_t r = INPLOGIC(m_R1) & INPLOGIC(m_R2);

	if (r)
	{
		A.m_reset = B.m_reset = C.m_reset = D.m_reset = 1;
		A.m_I.inactivate();
		B.m_I.inactivate();
		OUTLOGIC(A.m_Q, 0, NLTIME_FROM_NS(40));
		OUTLOGIC(B.m_Q, 0, NLTIME_FROM_NS(40));
		OUTLOGIC(C.m_Q, 0, NLTIME_FROM_NS(40));
		OUTLOGIC(D.m_Q, 0, NLTIME_FROM_NS(40));
	}
	else
	{
		A.m_reset = B.m_reset = C.m_reset = D.m_reset = 0;
		A.m_I.activate_hl();
		B.m_I.activate_hl();
	}
}
