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

// usage: TTL_74393(name, cCP, cMR)
#define TTL_74393(...)                                                        \
		NET_REGISTER_DEVEXT(TTL_74393, __VA_ARGS__)

#endif /* NLD_74193_H_ */
