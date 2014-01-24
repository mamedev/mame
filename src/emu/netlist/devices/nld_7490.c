/*
 * nld_7490.c
 *
 */

#include "nld_7490.h"

NETLIB_START(7490)
{
    register_input("A", m_A);
    register_input("B", m_B);
	register_input("R1",  m_R1);
	register_input("R2",  m_R2);
	register_input("R91", m_R91);
	register_input("R92", m_R92);

	register_output("QA", m_Q[0]);
	register_output("QB", m_Q[1]);
	register_output("QC", m_Q[2]);
	register_output("QD", m_Q[3]);

	save(NAME(m_cnt));

}

NETLIB_RESET(7490)
{
    m_cnt = 0;
}

static const netlist_time delay[4] =
{
        NLTIME_FROM_NS(18),
        NLTIME_FROM_NS(36) - NLTIME_FROM_NS(18),
        NLTIME_FROM_NS(54) - NLTIME_FROM_NS(18),
        NLTIME_FROM_NS(72) - NLTIME_FROM_NS(18)};

NETLIB_UPDATE(7490)
{
	if (INPLOGIC(m_R91) & INPLOGIC(m_R92))
	{
		m_cnt = 9;
		update_outputs();
	}
	else if (INPLOGIC(m_R1) & INPLOGIC(m_R2))
	{
		m_cnt = 0;
		update_outputs();
	}
	else if (INP_HL(m_A))
	{
		m_cnt ^= 1;
        OUTLOGIC(m_Q[0], m_cnt & 1, delay[0]);
	}
    else if (INP_HL(m_B))
    {
        m_cnt += 2;
        if (m_cnt >= 10)
            m_cnt = 0;
        update_outputs();
    }
}

NETLIB_FUNC_VOID(7490, update_outputs, (void))
{

	for (int i=0; i<4; i++)
		OUTLOGIC(m_Q[i], (m_cnt >> i) & 1, delay[i]);
}

NETLIB_START(7490_dip)
{
    NETLIB_NAME(7490)::start();
#if 0
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
#endif
}

/*          +--------------+
 *        B |1     ++    14| A
 *      R01 |2           13| NC
 *      R02 |3           12| QA
 *       NC |4    7490   11| QD
 *      VCC |5           10| GND
 *      R91 |6            9| QB
 *      R92 |7            8| QC
 *          +--------------+
*/
NETLIB_UPDATE(7490_dip)
{
    NETLIB_NAME(7490)::update();
}

NETLIB_RESET(7490_dip)
{
    NETLIB_NAME(7490)::reset();
}
