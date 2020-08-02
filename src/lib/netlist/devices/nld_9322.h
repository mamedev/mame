// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_9322.h
 *
 *  9322: Quad 2-Line to 1-Line Data Selectors/Multiplexers
 *
 *          +------------+
 *   SELECT |1    ++   16| VCC
 *       A1 |2         15| STROBE
 *       B1 |3         14| A4
 *       Y1 |4   9322  13| B4
 *       A2 |5         12| Y4
 *       B2 |6         11| A3
 *       Y2 |7         10| B3
 *      GND |8          9| Y3
 *          +------------+
 *
 */

#ifndef NLD_9322_H_
#define NLD_9322_H_

#include "netlist/nl_setup.h"

#define TTL_9322_GATE(...)                                                \
		NET_REGISTER_DEVEXT(TTL_9322_GATE, __VA_ARGS__)

// usage: TTL_9322(name, cSELECT, cA1, cB1, cA2, cB2, cA3, cB3, cA4, cB4, cSTROBE)
#define TTL_9322(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_9322, __VA_ARGS__)

#endif /* NLD_9322_H_ */
