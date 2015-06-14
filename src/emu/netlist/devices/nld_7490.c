// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7490.c
 *
 */

#include "nld_7490.h"

NETLIB_NAMESPACE_DEVICES_START()

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

	save(NLNAME(m_cnt));
	save(NLNAME(m_last_A));
	save(NLNAME(m_last_B));

}

NETLIB_RESET(7490)
{
	m_cnt = 0;
	m_last_A = 0;
	m_last_B = 0;
}

static const netlist_time delay[4] =
{
		NLTIME_FROM_NS(18),
		NLTIME_FROM_NS(36) - NLTIME_FROM_NS(18),
		NLTIME_FROM_NS(54) - NLTIME_FROM_NS(18),
		NLTIME_FROM_NS(72) - NLTIME_FROM_NS(18)};

NETLIB_UPDATE(7490)
{
	const netlist_sig_t new_A = INPLOGIC(m_A);
	const netlist_sig_t new_B = INPLOGIC(m_B);

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
	else
	{
		if (m_last_A && !new_A)  // High - Low
		{
			m_cnt ^= 1;
			OUTLOGIC(m_Q[0], m_cnt & 1, delay[0]);
		}
		if (m_last_B && !new_B)  // High - Low
		{
			m_cnt += 2;
			if (m_cnt >= 10)
				m_cnt &= 1; /* Output A is not reset! */
			update_outputs();
		}
	}
	m_last_A = new_A;
	m_last_B = new_B;
}

NETLIB_FUNC_VOID(7490, update_outputs, (void))
{
	for (int i=0; i<4; i++)
		OUTLOGIC(m_Q[i], (m_cnt >> i) & 1, delay[i]);
}

NETLIB_START(7490_dip)
{
	NETLIB_NAME(7490)::start();
	register_subalias("1", m_B);
	register_subalias("2", m_R1);
	register_subalias("3", m_R2);

	// register_subalias("4", ); --> NC
	// register_subalias("5", ); --> VCC
	register_subalias("6", m_R91);
	register_subalias("7", m_R92);

	register_subalias("8", m_Q[2]);
	register_subalias("9", m_Q[1]);
	// register_subalias("10", ); --> GND
	register_subalias("11", m_Q[3]);
	register_subalias("12", m_Q[0]);
	// register_subalias("13", ); --> NC
	register_subalias("14", m_A);

}

NETLIB_UPDATE(7490_dip)
{
	NETLIB_NAME(7490)::update();
}

NETLIB_RESET(7490_dip)
{
	NETLIB_NAME(7490)::reset();
}

NETLIB_NAMESPACE_DEVICES_END()
