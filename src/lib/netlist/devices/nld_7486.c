// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7486.c
 *
 */

#include "nld_7486.h"

NETLIB_NAMESPACE_DEVICES_START()

#if (USE_TRUTHTABLE)
nld_7486::truthtable_t nld_7486::m_ttbl;
const char *nld_7486::m_desc[] = {
		"A , B | Q ",
		"0,0|0|15",
		"0,1|1|22",
		"1,0|1|22",
		"1,1|0|15",
		""
};
#else
NETLIB_START(7486)
{
	register_input("A", m_I[0]);
	register_input("B", m_I[1]);
	register_output("Q", m_Q[0]);

	save(NLNAME(m_active));
}

NETLIB_RESET(7486)
{
	m_active = 1;
}

NETLIB_UPDATE(7486)
{
	const netlist_time delay[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22) };

	UINT8 t = INPLOGIC(m_I[0]) ^ INPLOGIC(m_I[1]);
	OUTLOGIC(m_Q[0], t, delay[t]);
}

ATTR_HOT void NETLIB_NAME(7486)::inc_active()
{
	const netlist_time delay[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22) };
	nl_assert(netlist().use_deactivate());
	if (++m_active == 1)
	{
		m_I[0].activate();
		m_I[1].activate();

		netlist_time mt = this->m_I[0].net().time();
		if (this->m_I[1].net().time() > mt)
			mt = this->m_I[1].net().time();

		UINT8 t = INPLOGIC(m_I[0]) ^ INPLOGIC(m_I[1]);
		m_Q[0].net().set_Q_time(t, mt + delay[t]);
	}
}

ATTR_HOT void NETLIB_NAME(7486)::dec_active()
{
#if 1
	nl_assert(netlist().use_deactivate());
	if (--m_active == 0)
	{
		m_I[0].inactivate();
		m_I[1].inactivate();
	}
#endif
}
#endif

NETLIB_START(7486_dip)
{
	register_sub("1", m_1);
	register_sub("2", m_2);
	register_sub("3", m_3);
	register_sub("4", m_4);

	register_subalias("1", m_1.m_I[0]);
	register_subalias("2", m_1.m_I[1]);
	register_subalias("3", m_1.m_Q[0]);

	register_subalias("4", m_2.m_I[0]);
	register_subalias("5", m_2.m_I[1]);
	register_subalias("6", m_2.m_Q[0]);

	register_subalias("9", m_3.m_I[0]);
	register_subalias("10", m_3.m_I[1]);
	register_subalias("8", m_3.m_Q[0]);

	register_subalias("12", m_4.m_I[0]);
	register_subalias("13", m_4.m_I[1]);
	register_subalias("11", m_4.m_Q[0]);
}

NETLIB_UPDATE(7486_dip)
{
	/* only called during startup */
	m_1.update_dev();
	m_2.update_dev();
	m_3.update_dev();
	m_4.update_dev();
}

NETLIB_RESET(7486_dip)
{
	m_1.do_reset();
	m_2.do_reset();
	m_3.do_reset();
	m_4.do_reset();
}

NETLIB_NAMESPACE_DEVICES_END()
