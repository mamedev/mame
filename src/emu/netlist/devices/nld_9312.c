// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9312.c
 *
 */

/*
 *          +---+---+---+---++---+---+
 *          | C | B | A | G || Y | YQ|
 *          +===+===+===+===++===+===+
 *          | X | X | X | 1 ||  0| 1 |
 *          | 0 | 0 | 0 | 0 || D0|D0Q|
 *          | 0 | 0 | 1 | 0 || D1|D1Q|
 *          | 0 | 1 | 0 | 0 || D2|D2Q|
 *          | 0 | 1 | 1 | 0 || D3|D3Q|
 *          | 1 | 0 | 0 | 0 || D4|D4Q|
 *          | 1 | 0 | 1 | 0 || D5|D5Q|
 *          | 1 | 1 | 0 | 0 || D6|D6Q|
 *          | 1 | 1 | 1 | 0 || D7|D7Q|
 *          +---+---+---+---++---+---+
*/
#include "nld_9312.h"

NETLIB_NAMESPACE_DEVICES_START()

#if (1 && USE_TRUTHTABLE)
nld_9312::truthtable_t nld_9312::m_ttbl;

/* FIXME: Data changes are propagating faster than changing selects A,B,C
 *        Please refer to data sheet.
 *        This would require a state machine, thus we do not
 *        do this right now.
 */

const char *nld_9312::m_desc[] = {
		" C, B, A, G,D0,D1,D2,D3,D4,D5,D6,D7| Y,YQ",
		" X, X, X, 1, X, X, X, X, X, X, X, X| 0, 1|33,19",
		" 0, 0, 0, 0, 0, X, X, X, X, X, X, X| 0, 1|33,28",
		" 0, 0, 0, 0, 1, X, X, X, X, X, X, X| 1, 0|33,28",
		" 0, 0, 1, 0, X, 0, X, X, X, X, X, X| 0, 1|33,28",
		" 0, 0, 1, 0, X, 1, X, X, X, X, X, X| 1, 0|33,28",
		" 0, 1, 0, 0, X, X, 0, X, X, X, X, X| 0, 1|33,28",
		" 0, 1, 0, 0, X, X, 1, X, X, X, X, X| 1, 0|33,28",
		" 0, 1, 1, 0, X, X, X, 0, X, X, X, X| 0, 1|33,28",
		" 0, 1, 1, 0, X, X, X, 1, X, X, X, X| 1, 0|33,28",
		" 1, 0, 0, 0, X, X, X, X, 0, X, X, X| 0, 1|33,28",
		" 1, 0, 0, 0, X, X, X, X, 1, X, X, X| 1, 0|33,28",
		" 1, 0, 1, 0, X, X, X, X, X, 0, X, X| 0, 1|33,28",
		" 1, 0, 1, 0, X, X, X, X, X, 1, X, X| 1, 0|33,28",
		" 1, 1, 0, 0, X, X, X, X, X, X, 0, X| 0, 1|33,28",
		" 1, 1, 0, 0, X, X, X, X, X, X, 1, X| 1, 0|33,28",
		" 1, 1, 1, 0, X, X, X, X, X, X, X, 0| 0, 1|33,28",
		" 1, 1, 1, 0, X, X, X, X, X, X, X, 1| 1, 0|33,28",
		""
};
#else

NETLIB_UPDATE(9312)
{
	const UINT8 G = INPLOGIC(m_G);
	if (G)
	{
		/* static */ const netlist_time delay[2] = { NLTIME_FROM_NS(33), NLTIME_FROM_NS(19) };
		OUTLOGIC(m_Y, 0, delay[0]);
		OUTLOGIC(m_YQ, 1, delay[1]);

		m_A.inactivate();
		m_B.inactivate();
		m_C.inactivate();
		m_last_G = G;
	}
	else
	{
		if (m_last_G)
		{
			m_last_G = G;
			m_A.activate();
			m_B.activate();
			m_C.activate();
		}
		/* static */ const netlist_time delay[2] = { NLTIME_FROM_NS(33), NLTIME_FROM_NS(28) };
		const UINT8 chan = INPLOGIC(m_A) | (INPLOGIC(m_B)<<1) | (INPLOGIC(m_C)<<2);
		if (m_last_chan != chan)
		{
			m_D[m_last_chan].inactivate();
			m_D[chan].activate();
		}
		const UINT8 val = INPLOGIC(m_D[chan]);
		OUTLOGIC(m_Y, val, delay[val]);
		OUTLOGIC(m_YQ, !val, delay[!val]);
		m_last_chan = chan;
	}
}

NETLIB_START(9312)
{
	register_input("G",         m_G);
	register_input("A",         m_A);
	register_input("B",         m_B);
	register_input("C",         m_C);

	register_input("D0",        m_D[0]);
	register_input("D1",        m_D[1]);
	register_input("D2",        m_D[2]);
	register_input("D3",        m_D[3]);
	register_input("D4",        m_D[4]);
	register_input("D5",        m_D[5]);
	register_input("D6",        m_D[6]);
	register_input("D7",        m_D[7]);

	register_output("Y",        m_Y);
	register_output("YQ",       m_YQ);

	m_last_chan = 0;
	m_last_G = 0;

	save(NLNAME(m_last_chan));
	save(NLNAME(m_last_G));
}

NETLIB_RESET(9312)
{
}
#endif

NETLIB_START(9312_dip)
{
	register_sub("1", m_sub);

#if (1 && USE_TRUTHTABLE)

	register_subalias("13", m_sub.m_I[0]);
	register_subalias("12", m_sub.m_I[1]);
	register_subalias("11", m_sub.m_I[2]);
	register_subalias("10", m_sub.m_I[3]);

	register_subalias("1", m_sub.m_I[4]);
	register_subalias("2", m_sub.m_I[5]);
	register_subalias("3", m_sub.m_I[6]);
	register_subalias("4", m_sub.m_I[7]);
	register_subalias("5", m_sub.m_I[8]);
	register_subalias("6", m_sub.m_I[9]);
	register_subalias("7", m_sub.m_I[10]);
	register_subalias("9", m_sub.m_I[11]);

	register_subalias("15", m_sub.m_Q[0]); // Y
	register_subalias("14", m_sub.m_Q[1]); // YQ

#else

	register_subalias("13", m_sub.m_C);
	register_subalias("12", m_sub.m_B);
	register_subalias("11", m_sub.m_A);
	register_subalias("10", m_sub.m_G);

	register_subalias("1", m_sub.m_D[0]);
	register_subalias("2", m_sub.m_D[1]);
	register_subalias("3", m_sub.m_D[2]);
	register_subalias("4", m_sub.m_D[3]);
	register_subalias("5", m_sub.m_D[4]);
	register_subalias("6", m_sub.m_D[5]);
	register_subalias("7", m_sub.m_D[6]);
	register_subalias("9", m_sub.m_D[7]);

	register_subalias("15", m_sub.m_Y); // Y
	register_subalias("14", m_sub.m_YQ); // YQ

#endif
}

NETLIB_UPDATE(9312_dip)
{
	/* only called during startup */
	m_sub.update_dev();
}

NETLIB_RESET(9312_dip)
{
	m_sub.do_reset();
}
NETLIB_NAMESPACE_DEVICES_END()
