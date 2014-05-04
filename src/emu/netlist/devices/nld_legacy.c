/*
 * nld_legacy.c
 *
 */

#include "nld_legacy.h"
#include "netlist/nl_setup.h"

NETLIB_START(nicRSFF)
{
	register_input("S", m_S);
	register_input("R", m_R);
	register_output("Q", m_Q);
	register_output("QQ", m_QQ);
}

NETLIB_RESET(nicRSFF)
{
	m_Q.initial(0);
	m_QQ.initial(1);
}

NETLIB_UPDATE(nicRSFF)
{
    if (!INPLOGIC(m_S))
    {
        OUTLOGIC(m_Q,  1, NLTIME_FROM_NS(20));
        OUTLOGIC(m_QQ, 0, NLTIME_FROM_NS(20));
    }
    else if (!INPLOGIC(m_R))
	{
		OUTLOGIC(m_Q,  0, NLTIME_FROM_NS(20));
		OUTLOGIC(m_QQ, 1, NLTIME_FROM_NS(20));
	}
}
