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

// expects: PROM_82S115(name, cCE1Q, cCE2, cA0, cA1, cA2, cA3, cA4, cA5, cA6, cA7, cA8, cSTROBE)
#define PROM_82S115(...)                                           \
	NET_REGISTER_DEVEXT(PROM_82S115, __VA_ARGS__)

#endif /* NLD_82S115_H_ */
