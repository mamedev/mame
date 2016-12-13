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

#include "nl_setup.h"

#define TTL_82S126(name)                                     \
		NET_REGISTER_DEV(TTL_82S126, name)
#define TTL_82S126_DIP(name)                                 \
		NET_REGISTER_DEV(TTL_82S126_DIP, name)

#endif /* NLD_82S126_H_ */
