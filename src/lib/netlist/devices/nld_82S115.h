// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_82S115.h
 *
 *  82S115: 4K-bit TTL bipolar PROM (512 x 8)
 *
 *          +--------------+
 *       A3 |1     ++    24| VCC
 *       A4 |2           23| A2
 *       A5 |3           22| A1
 *       A6 |4   82S115  21| A0
 *       A7 |5           20| CE1Q
 *       A8 |6           19| CE2
 *       O1 |7           18| STROBE
 *       O2 |8           17| O8
 *       O3 |9           16| O7
 *       O4 |10          15| O6
 *      FE2 |11          14| O5
 *      GND |12          13| FE1
 *          +--------------+
 *
 *
 *  Naming conventions follow Signetics datasheet
 *
 */

#ifndef NLD_82S115_H_
#define NLD_82S115_H_

#include "netlist/nl_setup.h"

#define PROM_82S115(name, cCE1Q, cCE2, cA0, cA1, cA2, cA3, cA4, cA5, cA6, cA7, cA8, cSTROBE)    \
		NET_REGISTER_DEV(PROM_82S115, name)                                    \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, CE1Q,   cCE1Q)                                       \
		NET_CONNECT(name, CE2,    cCE2)                                        \
		NET_CONNECT(name, A0,     cA0)                                         \
		NET_CONNECT(name, A1,     cA1)                                         \
		NET_CONNECT(name, A2,     cA2)                                         \
		NET_CONNECT(name, A3,     cA3)                                         \
		NET_CONNECT(name, A4,     cA4)                                         \
		NET_CONNECT(name, A5,     cA5)                                         \
		NET_CONNECT(name, A6,     cA6)                                         \
		NET_CONNECT(name, A7,     cA7)                                         \
		NET_CONNECT(name, A8,     cA8)                                         \
		NET_CONNECT(name, STROBE, cSTROBE)

#define PROM_82S115_DIP(name)                                                  \
		NET_REGISTER_DEV(PROM_82S115_DIP, name)

#endif /* NLD_82S115_H_ */
