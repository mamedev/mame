// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7411.h
 *
 *  DM7411: Triple 3-Input AND Gates
 *
 *          +--------------+
 *       A1 |1     ++    14| VCC
 *       B1 |2           13| C1
 *       A2 |3           12| Y1
 *       B2 |4    7411   11| C3
 *       C2 |5           10| B3
 *       Y2 |6            9| A3
 *      GND |7            8| Y3
 *          +--------------+
 *
 *              Y = ABC
 *          +---+---+---++---+
 *          | A | B | C || Y |
 *          +===+===+===++===+
 *          | X | X | 0 || 0 |
 *          | X | 0 | X || 0 |
 *          | 0 | X | X || 0 |
 *          | 1 | 1 | 1 || 1 |
 *          +---+---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7411_H_
#define NLD_7411_H_

#include "nld_signal.h"
#include "nld_truthtable.h"

#define TTL_7411_AND(_name, _I1, _I2, _I3)                                         \
		NET_REGISTER_DEV(TTL_7411_AND, _name)                                               \
		NET_CONNECT(_name, A, _I1)                                                  \
		NET_CONNECT(_name, B, _I2)                                                  \
		NET_CONNECT(_name, C, _I3)

#define TTL_7411_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7411_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

#if (USE_TRUTHTABLE)
NETLIB_TRUTHTABLE(7411, 3, 1, 0);
#else
NETLIB_SIGNAL(7411, 3, 0, 1);
#endif


NETLIB_DEVICE(7411_dip,

	NETLIB_NAME(7411) m_1;
	NETLIB_NAME(7411) m_2;
	NETLIB_NAME(7411) m_3;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7411_H_ */
