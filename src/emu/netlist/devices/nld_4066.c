/*
 * nld_4066.c
 *
 */

#include "nld_4066.h"


NETLIB_START(4066)
{
	register_input("CTL", m_control);
	register_sub(m_R, "R");
}

NETLIB_RESET(4066)
{
	m_R.do_reset();
}

NETLIB_UPDATE(4066)
{
	double sup = (m_supply.get()->vdd() - m_supply.get()->vss());
	double low = 0.45 * sup;
	double high = 0.55 * sup;
	double in = INPANALOG(m_control) - m_supply.get()->vss();
	double rON = 270.0 * 5.0 / sup;

	if (in < low)
	{
		m_R.set_R(1.0 / netlist().gmin());
		m_R.update_dev();
	}
	else if (in > high)
	{
		m_R.set_R(rON);
		m_R.update_dev();
	}
}


NETLIB_START(4066_dip)
{
	register_sub(m_supply, "supply");
	m_A.m_supply = m_B.m_supply = m_C.m_supply = m_D.m_supply = &m_supply;
	register_sub(m_A, "A");
	register_sub(m_B, "B");
	register_sub(m_C, "C");
	register_sub(m_D, "D");

	register_subalias("1", m_A.m_R.m_P);
	register_subalias("2", m_A.m_R.m_N);
	register_subalias("3", m_B.m_R.m_P);
	register_subalias("4", m_B.m_R.m_N);
	register_subalias("5", m_B.m_control);
	register_subalias("6", m_C.m_control);
	register_input("7", m_supply.m_vss);

	register_subalias("8", m_C.m_R.m_P);
	register_subalias("9", m_C.m_R.m_N);
	register_subalias("10", m_D.m_R.m_P);
	register_subalias("11", m_D.m_R.m_N);
	register_subalias("12", m_D.m_control);
	register_subalias("13", m_A.m_control);
	register_input("14", m_supply.m_vdd);

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
