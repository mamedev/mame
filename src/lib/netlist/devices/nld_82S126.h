// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_82S126.h
 *
 *  82S126: 1K-bit TTL bipolar PROM
 *
 *          +--------------+
 *       A6 |1     ++    16| VCC
 *       A5 |2           15| Q7
 *       A4 |3           14| CE2Q
 *       A3 |4   82S126  13| CE1Q
 *       A0 |5           12| O1
 *       A1 |6           11| O2
 *       A2 |7           10| O3
 *      GND |8            9| O4
 *          +--------------+
 *
 *
 *  Naming conventions follow Signetics datasheet
 *
 */

#ifndef NLD_82S126_H_
#define NLD_82S126_H_

#include "netlist/nl_setup.h"

#define PROM_82S126(name, cCE1Q, cCE2Q, cA0, cA1, cA2, cA3, cA4, cA5, cA6, cA7) \
		NET_REGISTER_DEV(PROM_82S126, name)                                    \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, CE1Q, cCE1Q)                                         \
		NET_CONNECT(name, CE2Q, cCE2Q)                                         \
		NET_CONNECT(name, A0,   cA0)                                           \
		NET_CONNECT(name, A1,   cA1)                                           \
		NET_CONNECT(name, A2,   cA2)                                           \
		NET_CONNECT(name, A3,   cA3)                                           \
		NET_CONNECT(name, A4,   cA4)                                           \
		NET_CONNECT(name, A5,   cA5)                                           \
		NET_CONNECT(name, A6,   cA6)                                           \
		NET_CONNECT(name, A7,   cA7)

#define PROM_82S126_DIP(name)                                                  \
		NET_REGISTER_DEV(PROM_82S126_DIP, name)

#endif /* NLD_82S126_H_ */
