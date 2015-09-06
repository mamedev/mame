// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7425.h
 *
 *  DM7425: Dual 4-Input NOR Gates
 *
 *          +--------------+
 *       A1 |1     ++    14| VCC
 *       B1 |2           13| D2
 *       X1 |3           12| C2
 *       C1 |4    7425   11| X2
 *       D1 |5           10| B2
 *       Y1 |6            9| A2
 *      GND |7            8| Y2
 *          +--------------+
 *                  _______
 *              Y = A+B+C+D
 *          +---+---+---+---+---++---+
 *          | A | B | C | D | X || Y |
 *          +===+===+===+===+===++===+
 *          | X | X | X | X | 0 || Z |
 *          | 0 | 0 | 0 | 0 | 1 || 1 |
 *          | X | X | X | 1 | 1 || 0 |
 *          | X | X | 1 | X | 1 || 0 |
 *          | X | 1 | X | X | 1 || 0 |
 *          | 1 | X | X | X | 1 || 0 |
 *          +---+---+---+---+---++---+
 *
 *  TODO: The "X" input and high impedance output are currently not simulated.
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7425_H_
#define NLD_7425_H_

#include "nld_signal.h"

#define TTL_7425_NOR(_name, _I1, _I2, _I3, _I4)                                     \
		NET_REGISTER_DEV(TTL_7425_NOR, _name)                                               \
		NET_CONNECT(_name, A, _I1)                                                  \
		NET_CONNECT(_name, B, _I2)                                                  \
		NET_CONNECT(_name, C, _I3)                                                  \
		NET_CONNECT(_name, D, _I4)

#define TTL_7425_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7425_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_SIGNAL(7425, 4, 1, 0);

NETLIB_DEVICE(7425_dip,

	NETLIB_NAME(7425) m_1;
	NETLIB_NAME(7425) m_2;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7425_H_ */
