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

#define TTL_74279_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_74279_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

#if 0
NETLIB_TRUTHTABLE(74279A, 2, 1, 1);
NETLIB_TRUTHTABLE(74279B, 3, 1, 1);
#else
NETLIB_TRUTHTABLE(74279A, 3, 1, 0);
NETLIB_TRUTHTABLE(74279B, 4, 1, 0);
//NETLIB_TRUTHTABLE(74279A, 4, 2, 0);
//NETLIB_TRUTHTABLE(74279B, 5, 2, 0);
#endif

NETLIB_DEVICE(74279_dip,

	NETLIB_NAME(74279B) m_1;
	NETLIB_NAME(74279A) m_2;
	NETLIB_NAME(74279B) m_3;
	NETLIB_NAME(74279A) m_4;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_74279_H_ */
