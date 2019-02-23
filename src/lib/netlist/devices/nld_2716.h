// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_82S126.h
 *
 *  2716: 16 Kbit (2Kb x 8) UV EPROM
 *
 *          +----------------+
 *       A7 |1      ++     24| VCC
 *       A6 |2             23| A8
 *       A5 |3             22| A9
 *       A4 |4     2716    21| VPP
 *       A3 |5             20| GQ
 *       A2 |6             19| A10
 *       A1 |7             18| EPQ
 *       A0 |8             17| D7
 *       D0 |9             16| D6
 *       D1 |10            15| D5
 *       D2 |11            14| D4
 *      VSS |12            13| D3
 *          +----------------+
 *
 *
 *  Naming conventions follow STMicro datasheet
 *
 */

#ifndef NLD_2716_H_
#define NLD_2716_H_

#include "netlist/nl_setup.h"

#define EPROM_2716(name, cGQ, cEPQ, cA0, cA1, cA2, cA3, cA4, cA5, cA6, cA7, cA8, cA9, cA10) \
		NET_REGISTER_DEV(EPROM_2716, name) \
		NET_CONNECT(name, GQ,  cGQ)     \
		NET_CONNECT(name, EPQ, cEPQ)    \
		NET_CONNECT(name, A0,  cA0)     \
		NET_CONNECT(name, A1,  cA1)     \
		NET_CONNECT(name, A2,  cA2)     \
		NET_CONNECT(name, A3,  cA3)     \
		NET_CONNECT(name, A4,  cA4)     \
		NET_CONNECT(name, A5,  cA5)     \
		NET_CONNECT(name, A6,  cA6)     \
		NET_CONNECT(name, A7,  cA7)     \
		NET_CONNECT(name, A8,  cA8)     \
		NET_CONNECT(name, A9,  cA9)     \
		NET_CONNECT(name, A10, cA10)

#define EPROM_2716_DIP(name)                                 \
		NET_REGISTER_DEV(EPROM_2716_DIP, name)

#endif /* NLD_2716_H_ */
