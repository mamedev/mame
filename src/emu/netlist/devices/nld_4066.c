// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4066.c
 *
 */

#include "nld_4066.h"

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_START(4066)
{
	register_input("CTL", m_control);
	register_sub("R", m_R);
	m_base_r = 270.0;
}

NETLIB_RESET(4066)
{
	m_R.do_reset();
}

NETLIB_UPDATE(4066)
{
	nl_double sup = (m_supply->vdd() - m_supply->vss());
	nl_double low = NL_FCONST(0.45) * sup;
	nl_double high = NL_FCONST(0.55) * sup;
	nl_double in = INPANALOG(m_control) - m_supply->vss();
	nl_double rON = m_base_r * NL_FCONST(5.0) / sup;
	nl_double R = -1.0;

	if (in < low)
	{
		R = NL_FCONST(1.0) / netlist().gmin();
	}
	else if (in > high)
	{
		R = rON;
	}
	if (R > NL_FCONST(0.0))
	{
		// We only need to update the net first if this is a time stepping net
		if (1) // m_R.m_P.net().as_analog().solver()->is_timestep())
		{
			m_R.update_dev();
			m_R.set_R(R);
			m_R.m_P.schedule_after(NLTIME_FROM_NS(1));
		}
		else
		{
			m_R.set_R(R);
			m_R.update_dev();
		}
	}
}


NETLIB_START(4066_dip)
{
	register_sub("supply", m_supply);
	m_A.m_supply = m_B.m_supply = m_C.m_supply = m_D.m_supply = &m_supply;
	register_sub("A", m_A);
	register_sub("B", m_B);
	register_sub("C", m_C);
	register_sub("D", m_D);

	m_A.m_base_r = m_B.m_base_r = m_C.m_base_r = m_D.m_base_r = 270;

	register_subalias("1", m_A.m_R.m_P);
	register_subalias("2", m_A.m_R.m_N);
	register_subalias("3", m_B.m_R.m_P);
	register_subalias("4", m_B.m_R.m_N);
	register_subalias("5", m_B.m_control);
	register_subalias("6", m_C.m_control);
	register_subalias("7", m_supply.m_vss);

	register_subalias("8", m_C.m_R.m_P);
	register_subalias("9", m_C.m_R.m_N);
	register_subalias("10", m_D.m_R.m_P);
	register_subalias("11", m_D.m_R.m_N);
	register_subalias("12", m_D.m_control);
	register_subalias("13", m_A.m_control);
	register_subalias("14", m_supply.m_vdd);

}

NETLIB_RESET(4066_dip)
{
	m_A.do_reset();
	m_B.do_reset();
	m_C.do_reset();
	m_D.do_reset();
}

NETLIB_UPDATE(4066_dip)
{
	/* only called during startup */
	m_A.update_dev();
	m_B.update_dev();
	m_C.update_dev();
	m_D.update_dev();
}

NETLIB_START(4016_dip)
{
	NETLIB_NAME(4066_dip)::start();

	m_A.m_base_r = m_B.m_base_r = m_C.m_base_r = m_D.m_base_r = 1000.0;
}

NETLIB_RESET(4016_dip)
{
	NETLIB_NAME(4066_dip)::reset();
}

NETLIB_UPDATE(4016_dip)
{
	/* only called during startup */
	NETLIB_NAME(4066_dip)::update();
}

NETLIB_NAMESPACE_DEVICES_END()
