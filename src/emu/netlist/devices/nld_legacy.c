/*
 * nld_legacy.c
 *
 */

#include "nld_legacy.h"

NETLIB_START(nicMultiSwitch)
{
	static const char *sIN[8] = { "i1", "i2", "i3", "i4", "i5", "i6", "i7", "i8" };
	int i;

	m_position = 0;
	m_low.initial(0);

	for (i=0; i<8; i++)
	{
		register_input(sIN[i], m_I[i]);
		m_low.net().register_con(m_I[i]);
		//m_I[i].set_net(m_low.m_net);
	}
	register_param("POS", m_POS, 0);
	register_output("Q", m_Q);

	save(NAME(m_position));

}

NETLIB_UPDATE(nicMultiSwitch)
{
	assert(m_position<8);
	OUTANALOG(m_Q, INPANALOG(m_I[m_position]), NLTIME_FROM_NS(1));
}

NETLIB_UPDATE_PARAM(nicMultiSwitch)
{
	m_position = m_POS.Value();
	//update();
}

NETLIB_START(nicRSFF)
{
	register_input("S", m_S);
	register_input("R", m_R);
	register_output("Q", m_Q);
	register_output("QQ", m_QQ);
	m_Q.initial(0);
	m_QQ.initial(1);
}

NETLIB_UPDATE(nicRSFF)
{
	if (INPLOGIC(m_S))
	{
		OUTLOGIC(m_Q,  1, NLTIME_FROM_NS(10));
		OUTLOGIC(m_QQ, 0, NLTIME_FROM_NS(10));
	}
	else if (INPLOGIC(m_R))
	{
		OUTLOGIC(m_Q,  0, NLTIME_FROM_NS(10));
		OUTLOGIC(m_QQ, 1, NLTIME_FROM_NS(10));
	}
}
