// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7437.h
 *
 *  DM7437: Quad 2-Input NAND Gates
 *
 *          +--------------+
 *       A1 |1     ++    14| VCC
 *       B1 |2           13| B4
 *       Y1 |3           12| A4
 *       A2 |4    7437   11| Y4
 *       B2 |5           10| B3
 *       Y2 |6            9| A3
 *      GND |7            8| Y3
 *          +--------------+
 *                  __
 *              Y = AB
 *          +---+---++---+
 *          | A | B || Y |
 *          +===+===++===+
 *          | 0 | 0 || 1 |
 *          | 0 | 1 || 1 |
 *          | 1 | 0 || 1 |
 *          | 1 | 1 || 0 |
 *          +---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 *  FIXME: Same as 7400, but drains higher output currents.
 *         Netlist currently does not model over currents (should it ever?)
 *
 */

#ifndef NLD_7437_H_
#define NLD_7437_H_

#include "nld_signal.h"
#include "nld_truthtable.h"

#define TTL_7437_NAND(_name, _A, _B)                                                \
		NET_REGISTER_DEV(TTL_7437_NAND, _name)                                               \
		NET_CONNECT(_name, A, _A)                                                   \
		NET_CONNECT(_name, B, _B)

#define TTL_7437_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7437_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

#if (USE_TRUTHTABLE)
NETLIB_TRUTHTABLE(7437, 2, 1, 0);
#else
NETLIB_SIGNAL(7437, 2, 0, 0);
#endif

NETLIB_DEVICE(7437_dip,

	NETLIB_NAME(7437) m_1;
	NETLIB_NAME(7437) m_2;
	NETLIB_NAME(7437) m_3;
	NETLIB_NAME(7437) m_4;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7437_H_ */
