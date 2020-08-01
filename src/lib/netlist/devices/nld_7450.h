// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7450.h
 *
 *  DM7450: DUAL 2-WIDE 2-INPUT AND-OR-INVERT GATES (ONE GATE EXPANDABLE)
 *
 *          +--------------+
 *       1A |1     ++    14| VCC
 *       2A |2           13| 1B
 *       2B |3           12| 1XQ
 *       2C |4    7450   11| 1X
 *       2D |5           10| 1D
 *       2Y |6            9| 1C
 *      GND |7            8| 1Y
 *          +--------------+
 *                  _________________
 *              Y = (A & B) | (C & D)
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 */

#ifndef NLD_7450_H_
#define NLD_7450_H_

#include "netlist/nl_setup.h"

// usage: TTL_7450_ANDORINVERT(name, cI1, cI2, cI3, cI4)
#define TTL_7450_ANDORINVERT(...)                                              \
		NET_REGISTER_DEVEXT(TTL_7450_ANDORINVERT, __VA_ARGS__)

#endif /* NLD_7450_H_ */
