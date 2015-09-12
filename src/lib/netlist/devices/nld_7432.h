// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7432.h
 *
 *  DM7432: Quad 2-Input OR Gates
 *
 *          +--------------+
 *       A1 |1     ++    14| VCC
 *       B1 |2           13| B4
 *       Y1 |3           12| A4
 *       A2 |4    7432   11| Y4
 *       B2 |5           10| B3
 *       Y2 |6            9| A3
 *      GND |7            8| Y3
 *          +--------------+
 *                  ___
 *              Y = A+B
 *          +---+---++---+
 *          | A | B || Y |
 *          +===+===++===+
 *          | 0 | 0 || 0 |
 *          | 0 | 1 || 1 |
 *          | 1 | 0 || 1 |
 *          | 1 | 1 || 1 |
 *          +---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7432_H_
#define NLD_7432_H_

#include "nld_signal.h"
#include "nld_truthtable.h"

#define TTL_7432_OR(_name, _I1, _I2)                                           \
		NET_REGISTER_DEV(TTL_7432_OR, _name)                                   \
		NET_CONNECT(_name, A, _I1)                                             \
		NET_CONNECT(_name, B, _I2)

#define TTL_7432_DIP(_name)                                                    \
		NET_REGISTER_DEV(TTL_7432_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

#if (USE_TRUTHTABLE)
NETLIB_TRUTHTABLE(7432, 2, 1, 0);
#else
NETLIB_SIGNAL(7432, 2, 1, 1);
#endif

NETLIB_DEVICE(7432_dip,

	NETLIB_NAME(7432) m_1;
	NETLIB_NAME(7432) m_2;
	NETLIB_NAME(7432) m_3;
	NETLIB_NAME(7432) m_4;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7432_H_ */
