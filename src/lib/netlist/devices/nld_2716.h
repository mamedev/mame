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
 *       A0 |8             17| Q7
 *       Q0 |9             16| Q6
 *       Q1 |10            15| Q5
 *       Q2 |11            14| Q4
 *      VSS |12            13| Q3
 *          +----------------+
 *
 *
 *  Naming conventions follow STMicro datasheet
 *
 */

#ifndef NLD_2716_H_
#define NLD_2716_H_

#include "nl_setup.h"

#define TTL_2716(name)                                     \
		NET_REGISTER_DEV(TTL_2716, name)
#define TTL_2716_DIP(name)                                 \
		NET_REGISTER_DEV(TTL_2716_DIP, name)

#endif /* NLD_2716_H_ */
