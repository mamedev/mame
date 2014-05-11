// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7432.h
 *
 *  DM7432: Quad 2-Input OR Gates
 *
 *          +--------------+
 *       Y1 |1     ++    14| VCC
 *       A1 |2           13| Y4
 *       B1 |3           12| B4
 *       Y2 |4    7432   11| A4
 *       A2 |5           10| Y3
 *       B2 |6            9| B3
 *      GND |7            8| A3
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

#define TTL_7432_OR(_name, _I1, _I2)                                               \
		NET_REGISTER_DEV(7432, _name)                                               \
		NET_CONNECT(_name, A, _I1)                                                  \
		NET_CONNECT(_name, B, _I2)

#define TTL_7432_DIP(_name)                                                         \
		NET_REGISTER_DEV(7432_dip, _name)


NETLIB_SIGNAL(7432, 2, 1, 1);

NETLIB_DEVICE(7432_dip,

	NETLIB_NAME(7432) m_1;
	NETLIB_NAME(7432) m_2;
	NETLIB_NAME(7432) m_3;
	NETLIB_NAME(7432) m_4;
);

#endif /* NLD_7432_H_ */
