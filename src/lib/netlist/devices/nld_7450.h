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

#include "nl_setup.h"

#define TTL_7450_ANDORINVERT(name, cI1, cI2, cI3, cI4)                          \
		NET_REGISTER_DEV(TTL_7450_ANDORINVERT, name)                            \
		NET_CONNECT(name, A, cI1)                                               \
		NET_CONNECT(name, B, cI2)                                               \
		NET_CONNECT(name, C, cI3)                                               \
		NET_CONNECT(name, D, cI4)

#define TTL_7450_DIP(name)                                                      \
		NET_REGISTER_DEV(TTL_7450_DIP, name)

#endif /* NLD_7450_H_ */
