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

#define TTL_74166(name, cCLK, cCLKINH, cSH_LDQ, cSER, cA, cB, cC, cD, cE, cF, cG, cH, cCLRQ)    \
		NET_REGISTER_DEV(TTL_74166, name)   \
		NET_CONNECT(name, CLK,    cCLK)     \
		NET_CONNECT(name, CLKINH, cCLKINH)  \
		NET_CONNECT(name, SH_LDQ, cSH_LDQ)  \
		NET_CONNECT(name, SER,    cSER)     \
		NET_CONNECT(name, A,      cA)       \
		NET_CONNECT(name, B,      cB)       \
		NET_CONNECT(name, C,      cC)       \
		NET_CONNECT(name, D,      cD)       \
		NET_CONNECT(name, E,      cE)       \
		NET_CONNECT(name, F,      cF)       \
		NET_CONNECT(name, G,      cG)       \
		NET_CONNECT(name, H,      cH)       \
		NET_CONNECT(name, CLRQ,   cCLRQ)

#define TTL_74166_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74166_DIP, name)

#endif /* NLD_74166_H_ */
