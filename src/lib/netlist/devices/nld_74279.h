// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74279.h
 *
 *  DM74279: Quad S-R Latch
 *
 *          +--------------+
 *       1R |1     ++    16| VCC
 *      1S1 |2           15| 4S
 *      1S2 |3           14| 4R
 *       1Q |4    74279  13| 4Q
 *       2R |5           12| 3S2
 *       2S |6           11| 3S1
 *       2Q |7           10| 3R
 *      GND |8            9| 3Q
 *          +--------------+
 *                  ___
 *
 *          +---+---+---++---+
 *          |S1 |S2 | R || Q |
 *          +===+===+===++===+
 *          | 0 | 0 | 0 || 1 |
 *          | 0 | 1 | 1 || 1 |
 *          | 1 | 0 | 1 || 1 |
 *          | 1 | 1 | 0 || 0 |
 *          | 1 | 1 | 1 ||QP |
 *          +---+---+---++---+
 *
 *  QP: Previous Q
 *
 *  Naming conventions follow Fairchild Semiconductor datasheet
 *
 */

#ifndef NLD_74279_H_
#define NLD_74279_H_

#include "nld_truthtable.h"

#define TTL_74279_DIP(name)                                                         \
		NET_REGISTER_DEV(TTL_74279_DIP, name)

namespace netlist
{
	namespace devices
	{

#if 0
NETLIB_TRUTHTABLE(74279A, 2, 1, 1);
NETLIB_TRUTHTABLE(74279B, 3, 1, 1);
#else
NETLIB_TRUTHTABLE(74279A, 3, 1, 0);
NETLIB_TRUTHTABLE(74279B, 4, 1, 0);
//NETLIB_TRUTHTABLE(74279A, 4, 2, 0);
//NETLIB_TRUTHTABLE(74279B, 5, 2, 0);
#endif

NETLIB_OBJECT(74279_dip)
{
	NETLIB_CONSTRUCTOR(74279_dip)
	, m_1(*this, "1")
	, m_2(*this, "2")
	, m_3(*this, "3")
	, m_4(*this, "4")
	{
		register_subalias("1", m_1.m_I[2]);  //R
		register_subalias("2", m_1.m_I[0]);
		register_subalias("3", m_1.m_I[1]);
		register_subalias("4", m_1.m_Q[0]);

		register_subalias("5", m_2.m_I[1]);  //R
		register_subalias("6", m_2.m_I[0]);
		register_subalias("7", m_2.m_Q[0]);

		register_subalias("9", m_3.m_Q[0]);
		register_subalias("10", m_3.m_I[2]); //R
		register_subalias("11", m_3.m_I[0]);
		register_subalias("12", m_3.m_I[1]);

		register_subalias("13", m_4.m_Q[0]);
		register_subalias("14", m_4.m_I[1]); //R
		register_subalias("15", m_4.m_I[0]);

	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();

protected:
	NETLIB_SUB(74279B) m_1;
	NETLIB_SUB(74279A) m_2;
	NETLIB_SUB(74279B) m_3;
	NETLIB_SUB(74279A) m_4;
};

	} //namespace devices
} // namespace netlist

#endif /* NLD_74279_H_ */
