// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_7485.h
 *
 *  DM7485: 4-bit Magnitude Comparators
 *
 *          +------------+
 *       B3 |1    ++   16| VCC
 *     LTIN |2         15| A3
 *     EQIN |3         14| B2
 *     GTIN |4   7485  13| A2
 *    GTOUT |5         12| A1
 *    EQOUT |6         11| B1
 *    LTOUT |7         10| A0
 *      GND |8          9| B0
 *          +------------+
 *
 *  Naming convention attempts to follow Texas Instruments datasheet
 *
 */

#ifndef NLD_7485_H_
#define NLD_7485_H_

#include "netlist/nl_setup.h"

#define TTL_7485(name, cA0, cA1, cA2, cA3, cB0, cB1, cB2, cB3, cLTIN, cEQIN, cGTIN)    \
		NET_REGISTER_DEV(TTL_7485, name)    \
		NET_CONNECT(name, A0,   cA0)    \
		NET_CONNECT(name, A1,   cA1)    \
		NET_CONNECT(name, A2,   cA2)    \
		NET_CONNECT(name, A3,   cA3)    \
		NET_CONNECT(name, B0,   cB0)    \
		NET_CONNECT(name, B1,   cB1)    \
		NET_CONNECT(name, B2,   cB2)    \
		NET_CONNECT(name, B3,   cB3)    \
		NET_CONNECT(name, LTIN, cLTIN)  \
		NET_CONNECT(name, EQIN, cEQIN)  \
		NET_CONNECT(name, GTIN, cGTIN)

#define TTL_7485_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7485_DIP, name)

#endif /* NLD_7485_H_ */
