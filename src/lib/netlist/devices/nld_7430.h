// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7430.h
 *
 *  DM7430: 8-Input NAND Gate
 *
 *          +--------------+
 *       A  |1     ++    14| VCC
 *       B  |2           13| NC
 *       C  |3           12| H
 *       D  |4    7430   11| G
 *       E  |5           10| NC
 *       F  |6            9| NC
 *      GND |7            8| Y
 *          +--------------+
 *                  ________
 *              Y = ABCDEFGH
 *          +---+---+---+---+---+---+---+---++---+
 *          | A | B | C | D | E | F | G | H || Y |
 *          +===+===+===+===+===+===+===+===++===+
 *          | X | X | X | X | X | X | X | 0 || 1 |
 *          | X | X | X | X | X | X | 0 | X || 1 |
 *          | X | X | X | X | X | 0 | X | X || 1 |
 *          | X | X | X | X | 0 | X | X | X || 1 |
 *          | X | X | X | 0 | X | X | X | X || 1 |
 *          | X | X | 0 | X | X | X | X | X || 1 |
 *          | X | 0 | X | X | X | X | X | X || 1 |
 *          | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 || 0 |
 *          +---+---+---+---+---+---+---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */


#ifndef NLD_7430_H_
#define NLD_7430_H_

#include "nld_signal.h"
#include "nld_truthtable.h"

#define TTL_7430_NAND(_name, _I1, _I2, _I3, _I4, _I5, _I6, _I7, _I8)                \
		NET_REGISTER_DEV(TTL_7430_NAND, _name)                                               \
		NET_CONNECT(_name, A, _I1)                                                  \
		NET_CONNECT(_name, B, _I2)                                                  \
		NET_CONNECT(_name, C, _I3)                                                  \
		NET_CONNECT(_name, D, _I4)                                                  \
		NET_CONNECT(_name, E, _I5)                                                  \
		NET_CONNECT(_name, F, _I6)                                                  \
		NET_CONNECT(_name, G, _I7)                                                  \
		NET_CONNECT(_name, H, _I8)


#define TTL_7430_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7430_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

#if (USE_TRUTHTABLE)
NETLIB_TRUTHTABLE(7430, 8, 1, 0);
#else
NETLIB_SIGNAL(7430, 8, 0, 0);
#endif


NETLIB_DEVICE(7430_dip,

	NETLIB_NAME(7430) m_1;
);

NETLIB_NAMESPACE_DEVICES_END()


#endif /* NLD_7430_H_ */
