// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7410.h
 *
 *  DM7410: Triple 3-Input NAND Gates
 *
 *          +--------------+
 *       A1 |1     ++    14| VCC
 *       B1 |2           13| C1
 *       A2 |3           12| Y1
 *       B2 |4    7410   11| C3
 *       C2 |5           10| B3
 *       Y2 |6            9| A3
 *      GND |7            8| Y3
 *          +--------------+
 *                  ___
 *              Y = ABC
 *          +---+---+---++---+
 *          | A | B | C || Y |
 *          +===+===+===++===+
 *          | X | X | 0 || 1 |
 *          | X | 0 | X || 1 |
 *          | 0 | X | X || 1 |
 *          | 1 | 1 | 1 || 0 |
 *          +---+---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7410_H_
#define NLD_7410_H_

#include "nld_signal.h"
#include "nld_truthtable.h"

#define TTL_7410_NAND(_name, _I1, _I2, _I3)                                         \
		NET_REGISTER_DEV(TTL_7410_NAND, _name)                                               \
		NET_CONNECT(_name, A, _I1)                                                  \
		NET_CONNECT(_name, B, _I2)                                                  \
		NET_CONNECT(_name, C, _I3)

#define TTL_7410_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7410_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

#if (USE_TRUTHTABLE)
NETLIB_TRUTHTABLE(7410, 3, 1, 0);
#else
NETLIB_SIGNAL(7410, 3, 0, 0);
#endif

NETLIB_DEVICE(7410_dip,

	NETLIB_NAME(7410) m_1;
	NETLIB_NAME(7410) m_2;
	NETLIB_NAME(7410) m_3;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7410_H_ */
