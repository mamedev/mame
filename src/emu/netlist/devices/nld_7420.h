// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7420.h
 *
 *  DM7420: Dual 4-Input NAND Gates
 *
 *          +--------------+
 *       A1 |1     ++    14| VCC
 *       B1 |2           13| D2
 *       NC |3           12| C2
 *       C1 |4    7420   11| NC
 *       D1 |5           10| B2
 *       Y1 |6            9| A2
 *      GND |7            8| Y2
 *          +--------------+
 *                  ____
 *              Y = ABCD
 *          +---+---+---+---++---+
 *          | A | B | C | D || Y |
 *          +===+===+===+===++===+
 *          | X | X | X | 0 || 1 |
 *          | X | X | 0 | X || 1 |
 *          | X | 0 | X | X || 1 |
 *          | 0 | X | X | X || 1 |
 *          | 1 | 1 | 1 | 1 || 0 |
 *          +---+---+---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7420_H_
#define NLD_7420_H_

#include "nld_signal.h"
#include "nld_truthtable.h"

#define TTL_7420_NAND(_name, _I1, _I2, _I3, _I4)                                    \
		NET_REGISTER_DEV(TTL_7420_NAND, _name)                                               \
		NET_CONNECT(_name, A, _I1)                                                  \
		NET_CONNECT(_name, B, _I2)                                                  \
		NET_CONNECT(_name, C, _I3)                                                  \
		NET_CONNECT(_name, D, _I4)


#define TTL_7420_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7420_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

#if (USE_TRUTHTABLE)
NETLIB_TRUTHTABLE(7420, 4, 1, 0);
#else
NETLIB_SIGNAL(7420, 4, 0, 0);
#endif

NETLIB_DEVICE(7420_dip,

	NETLIB_NAME(7420) m_1;
	NETLIB_NAME(7420) m_2;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7420_H_ */
