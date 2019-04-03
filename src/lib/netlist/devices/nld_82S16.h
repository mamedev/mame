// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_82S16.h
 *
 *  DM82S16: 256 Bit bipolar ram
 *
 *          +--------------+
 *       A1 |1     ++    16| VCC
 *       A0 |2           15| A2
 *     CE1Q |3           14| A3
 *     CE2Q |4   82S16   13| DIN
 *     CE3Q |5           12| WEQ
 *    DOUTQ |6           11| A7
 *       A4 |7           10| A6
 *      GND |8            9| A5
 *          +--------------+
 *
 *
 *  Naming conventions follow Signetics datasheet
 *
 */

#ifndef NLD_82S16_H_
#define NLD_82S16_H_

#include "netlist/nl_setup.h"

#define TTL_82S16(name)                                     \
		NET_REGISTER_DEV(TTL_82S16, name)
#define TTL_82S16_DIP(name)                                 \
		NET_REGISTER_DEV(TTL_82S16_DIP, name)

#endif /* NLD_82S16_H_ */
