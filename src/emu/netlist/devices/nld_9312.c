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

//#if (USE_TRUTHTABLE)
nld_9312::truthtable_t nld_9312::m_ttbl;

/* FIXME: Data changes are propagating faster than changing selects A,B,C
 * 		  Please refer to data sheet.
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
//#endif


NETLIB_START(9312_dip)
{
	register_sub(m_sub, "1");

	register_subalias("13", m_sub.m_i[0]);
	register_subalias("12", m_sub.m_i[1]);
	register_subalias("11", m_sub.m_i[2]);
	register_subalias("10", m_sub.m_i[3]);

	register_subalias("1", m_sub.m_i[4]);
	register_subalias("2", m_sub.m_i[5]);
	register_subalias("3", m_sub.m_i[6]);
	register_subalias("4", m_sub.m_i[7]);
	register_subalias("5", m_sub.m_i[8]);
	register_subalias("6", m_sub.m_i[9]);
	register_subalias("7", m_sub.m_i[10]);
	register_subalias("9", m_sub.m_i[11]);

	register_subalias("15", m_sub.m_Q[0]); // Y
	register_subalias("14", m_sub.m_Q[1]); // YQ
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
