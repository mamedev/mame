// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7402.h
 *
 *  DM7402: Quad 2-Input NOR Gates
 *
 *          +--------------+
 *       Y1 |1     ++    14| VCC
 *       A1 |2           13| Y4
 *       B1 |3           12| B4
 *       Y2 |4    7402   11| A4
 *       A2 |5           10| Y3
 *       B2 |6            9| B3
 *      GND |7            8| A3
 *          +--------------+
 *                  ___
 *              Y = A+B
 *          +---+---++---+
 *          | A | B || Y |
 *          +===+===++===+
 *          | 0 | 0 || 1 |
 *          | 0 | 1 || 0 |
 *          | 1 | 0 || 0 |
 *          | 1 | 1 || 0 |
 *          +---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7402_H_
#define NLD_7402_H_

#include "nld_signal.h"
#include "nld_truthtable.h"

#define TTL_7402_NOR(_name, _I1, _I2)                                               \
		NET_REGISTER_DEV(TTL_7402_NOR, _name)                                               \
		NET_CONNECT(_name, A, _I1)                                                  \
		NET_CONNECT(_name, B, _I2)

#define TTL_7402_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7402_DIP, _name)


NETLIB_NAMESPACE_DEVICES_START()

#if (USE_TRUTHTABLE)
NETLIB_TRUTHTABLE(7402, 2, 1, 0);
#else
NETLIB_SIGNAL(7402, 2, 1, 0);
#endif

NETLIB_DEVICE(7402_dip,

	NETLIB_NAME(7402) m_1;
	NETLIB_NAME(7402) m_2;
	NETLIB_NAME(7402) m_3;
	NETLIB_NAME(7402) m_4;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7402_H_ */
