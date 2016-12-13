// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_74161.h
 *
 *  DM74161: Synchronous 4-Bit Binary Counter with Clock
 *
 *          +--------------+
 *    CLEAR |1     ++    16| VCC
 *      CLK |2           15| RCO
 *        A |3           14| QA
 *        B |4    74161  13| QB
 *        C |5           12| QC
 *        D |6           11| QD
 *  ENABLEP |7           10| ENABLET
 *      GND |8            9| LOAD
 *          +--------------+
 *
 * RCO: Ripple carry output
 *
 *  Naming convention attempts to follow National Semiconductor datasheet
 *
 */

#ifndef NLD_74161_H_
#define NLD_74161_H_

#include "nl_setup.h"

#define TTL_74161(name)                                                         \
		NET_REGISTER_DEV(TTL_74161, name)

#define TTL_74161_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74161_DIP, name)

#endif /* NLD_74161_H_ */
