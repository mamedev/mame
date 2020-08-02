// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_74166.h
 *
 *  74166: Parallel-Load 8-Bit Shift Register
 *
 *          +--------------+
 *      SER |1     ++    16| VCC
 *        A |2           15| SH/LDQ
 *        B |3           14| H
 *        C |4    74166  13| QH
 *        D |5           12| G
 *   CLKINH |6           11| F
 *      CLK |7           10| E
 *      GND |8            9| CLRQ
 *          +--------------+
 *
 * SH/LDQ: Shift / !Load
 * CLKINH: Clock Inhibit
 * SER: Serial In
 *
 *  Naming convention attempts to follow Texas Instruments datasheet
 *
 */

#ifndef NLD_74166_H_
#define NLD_74166_H_

#include "netlist/nl_setup.h"

// usage: TTL_74166(name, cCLK, cCLKINH, cSH_LDQ, cSER, cA, cB, cC, cD, cE, cF, cG, cH, cCLRQ)
#define TTL_74166(...)                                                        \
		NET_REGISTER_DEVEXT(TTL_74166, __VA_ARGS__)

#endif /* NLD_74166_H_ */
