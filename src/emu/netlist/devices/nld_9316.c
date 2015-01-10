/*
 * nld_9316.c
 *
 */

#include "nld_9316.h"

NETLIB_START(9316)
{
	register_sub(subABCD, "subABCD");
	sub.m_ABCD = &subABCD;
	register_sub(sub, "sub");

	register_subalias("CLK", sub.m_CLK);

	register_input("ENP", m_ENP);
	register_input("ENT", m_ENT);
	register_input("CLRQ", m_CLRQ);
	register_input("LOADQ", m_LOADQ);

	register_subalias("A", subABCD.m_A);
	register_subalias("B", subABCD.m_B);
	register_subalias("C", subABCD.m_C);
	register_subalias("D", subABCD.m_D);

	register_subalias("QA", sub.m_QA);
	register_subalias("QB", sub.m_QB);
	register_subalias("QC", sub.m_QC);
	register_subalias("QD", sub.m_QD);
	register_subalias("RC", sub.m_RC);

}

NETLIB_RESET(9316)
{
	sub.do_reset();
	subABCD.do_reset();
}

NETLIB_START(9316_subABCD)
{
	register_input("A", m_A);
	register_input("B", m_B);
	register_input("C", m_C);
	register_input("D", m_D);

}

NETLIB_RESET(9316_subABCD)
{
	m_A.inactivate();
	m_B.inactivate();
	m_C.inactivate();
	m_D.inactivate();
}

ATTR_HOT inline UINT8 NETLIB_NAME(9316_subABCD::read_ABCD)()
{
	return (INPLOGIC_PASSIVE(m_D) << 3) | (INPLOGIC_PASSIVE(m_C) << 2) | (INPLOGIC_PASSIVE(m_B) << 1) | (INPLOGIC_PASSIVE(m_A) << 0);
}

NETLIB_UPDATE(9316_subABCD)
{
}

NETLIB_START(9316_sub)
{
	register_input("CLK", m_CLK);

	register_output("QA", m_QA);
	register_output("QB", m_QB);
	register_output("QC", m_QC);
	register_output("QD", m_QD);
	register_output("RC", m_RC);

	save(NAME(m_cnt.ref()));
	save(NAME(m_loadq.ref()));
	save(NAME(m_ent.ref()));
}

NETLIB_RESET(9316_sub)
{
	m_CLK.set_state(netlist_input_t::STATE_INP_LH);
	m_cnt = 0;
	m_loadq = 1;
	m_ent = 1;
}

NETLIB_UPDATE(9316_sub)
{
	UINT8 cnt = m_cnt;
	if (m_loadq)
	{
		cnt = ( cnt + 1) & 0x0f;
		update_outputs(cnt);
		OUTLOGIC(m_RC, m_ent & (cnt == 0x0f), NLTIME_FROM_NS(20));
#if 0
		if (cnt == 0x0f)
			OUTLOGIC(m_RC, m_ent, NLTIME_FROM_NS(20));
		else if (cnt == 0)
			OUTLOGIC(m_RC, 0, NLTIME_FROM_NS(20));
#endif
	}
	else
	{
		cnt = m_ABCD.get()->read_ABCD();
		update_outputs_all(cnt);
		OUTLOGIC(m_RC, m_ent & (cnt == 0x0f), NLTIME_FROM_NS(20));
	}
	m_cnt = cnt;
}

NETLIB_UPDATE(9316)
{
	sub.m_loadq = INPLOGIC(m_LOADQ);
	sub.m_ent = INPLOGIC(m_ENT);
	const netlist_sig_t clrq = INPLOGIC(m_CLRQ);

	if ((!sub.m_loadq || (sub.m_ent & INPLOGIC(m_ENP))) && clrq)
	{
		sub.m_CLK.activate_lh();
	}
	else
	{
		UINT8 cnt = sub.m_cnt;
		sub.m_CLK.inactivate();
		if (!clrq && (cnt>0))
		{
			cnt = 0;
			sub.update_outputs(cnt);
			//OUTLOGIC(sub.m_RC, 0, NLTIME_FROM_NS(20));
			sub.m_cnt = cnt;
			//return;
		}
	}
	OUTLOGIC(sub.m_RC, sub.m_ent & (sub.m_cnt == 0x0f), NLTIME_FROM_NS(20));
}

inline NETLIB_FUNC_VOID(9316_sub, update_outputs_all, (const UINT8 cnt))
{
	const netlist_time out_delay = NLTIME_FROM_NS(20);
	OUTLOGIC(m_QA, (cnt >> 0) & 1, out_delay);
	OUTLOGIC(m_QB, (cnt >> 1) & 1, out_delay);
	OUTLOGIC(m_QC, (cnt >> 2) & 1, out_delay);
	OUTLOGIC(m_QD, (cnt >> 3) & 1, out_delay);
}

inline NETLIB_FUNC_VOID(9316_sub, update_outputs, (const UINT8 cnt))
{
	const netlist_time out_delay = NLTIME_FROM_NS(20);
#if 1
//    for (int i=0; i<4; i++)
//        OUTLOGIC(m_Q[i], (cnt >> i) & 1, delay[i]);
	OUTLOGIC(m_QA, (cnt >> 0) & 1, out_delay);
	OUTLOGIC(m_QB, (cnt >> 1) & 1, out_delay);
	OUTLOGIC(m_QC, (cnt >> 2) & 1, out_delay);
	OUTLOGIC(m_QD, (cnt >> 3) & 1, out_delay);
#else
	if ((cnt & 1) == 1)
		OUTLOGIC(m_QA, 1, out_delay);
	else
	{
		OUTLOGIC(m_QA, 0, out_delay);
		switch (cnt)
		{
		case 0x00:
			OUTLOGIC(m_QB, 0, out_delay);
			OUTLOGIC(m_QC, 0, out_delay);
			OUTLOGIC(m_QD, 0, out_delay);
			break;
		case 0x02:
		case 0x06:
		case 0x0A:
		case 0x0E:
			OUTLOGIC(m_QB, 1, out_delay);
			break;
		case 0x04:
		case 0x0C:
			OUTLOGIC(m_QB, 0, out_delay);
			OUTLOGIC(m_QC, 1, out_delay);
			break;
		case 0x08:
			OUTLOGIC(m_QB, 0, out_delay);
			OUTLOGIC(m_QC, 0, out_delay);
			OUTLOGIC(m_QD, 1, out_delay);
			break;
		}

	}
#endif
}

NETLIB_START(9316_dip)
{
	NETLIB_NAME(9316)::start();

	register_subalias("1", m_CLRQ);
	register_subalias("2", sub.m_CLK);
	register_subalias("3", subABCD.m_A);
	register_subalias("4", subABCD.m_B);
	register_subalias("5", subABCD.m_C);
	register_subalias("6", subABCD.m_D);
	register_subalias("7", m_ENP);
	// register_subalias("8", ); --> GND

	register_subalias("9", m_LOADQ);
	register_subalias("10", m_ENT);
	register_subalias("11", sub.m_QD);
	register_subalias("12", sub.m_QC);
	register_subalias("13", sub.m_QB);
	register_subalias("14", sub.m_QA);
	register_subalias("15", sub.m_RC);
	// register_subalias("16", ); --> VCC
}

NETLIB_UPDATE(9316_dip)
{
	NETLIB_NAME(9316)::update();
}

NETLIB_RESET(9316_dip)
{
	NETLIB_NAME(9316)::reset();
}
