// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_82S123.h
 *
 *  82S23: 256-bit (32x8) TTL bipolar PROM with open-collector outputs
 *  82S123: 256-bit (32x8) TTL bipolar PROM with tri-state outputs
 *
 *          +--------------+
 *       O1 |1     ++    16| VCC
 *       O2 |2           15| CEQ
 *       O3 |3           14| A4
 *       O4 |4   82S123  13| A3
 *       O5 |5           12| A2
 *       O6 |6           11| A1
 *       O7 |7           10| A0
 *      GND |8            9| O8
 *          +--------------+
 *
 *
 *  Naming conventions follow Philips datasheet
 *
 */

#ifndef NLD_82S123_H_
#define NLD_82S123_H_

#include "netlist/nl_setup.h"

#define PROM_82S123(name, cCEQ, cA0, cA1, cA2, cA3, cA4)                       \
		NET_REGISTER_DEV(PROM_82S123, name)                                    \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, CEQ, cCEQ)                                           \
		NET_CONNECT(name, A0,  cA0)                                            \
		NET_CONNECT(name, A1,  cA1)                                            \
		NET_CONNECT(name, A2,  cA2)                                            \
		NET_CONNECT(name, A3,  cA3)                                            \
		NET_CONNECT(name, A4,  cA4)

#define PROM_82S123_DIP(name)                                                  \
		NET_REGISTER_DEV(PROM_82S123_DIP, name)

#endif /* NLD_82S123_H_ */
