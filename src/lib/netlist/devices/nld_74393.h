// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74393.h
 *
 *  DM74393: Dual 4-Stage Binary Counter
 *
 *          +--------------+
 *      /CP |1     ++    14| VCC
 *       MR |2           13| /CP
 *       Q0 |3           12| MR
 *       Q1 |4    74393  11| Q0
 *       Q2 |5           10| Q1
 *       Q3 |6            9| Q2
 *      GND |7            8| Q3
 *          +--------------+
 *
 *  Naming conventions follow Motorola datasheet
 *
 */

#ifndef NLD_74393_H_
#define NLD_74393_H_

#include "netlist/nl_setup.h"

#define TTL_74393(name, cCP, cMR)                                \
		NET_REGISTER_DEV(TTL_74393, name)                        \
		NET_CONNECT(name, GND, GND)                              \
		NET_CONNECT(name, VCC, VCC)                              \
		NET_CONNECT(name, CP, cCP)                               \
		NET_CONNECT(name, MR, cMR)

#define TTL_74393_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74393_DIP, name)

#endif /* NLD_74193_H_ */
