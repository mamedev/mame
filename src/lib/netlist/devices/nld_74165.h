// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_74165.h
 *
 *  74165: Parallel-Load 8-Bit Shift Register
 *
 *          +--------------+
 *   SH/LDQ |1     ++    16| VCC
 *      CLK |2           15| CLKINH
 *        E |3           14| D
 *        F |4    74165  13| C
 *        G |5           12| B
 *        H |6           11| A
 *      QHQ |7           10| SER
 *      GND |8            9| QH
 *          +--------------+
 *
 * SH/LDQ: Shift / !Load
 * CLKINH: Clock Inhibit
 * SER: Serial In
 *
 *  Naming convention attempts to follow NTE Electronics datasheet
 *
 */

#ifndef NLD_74165_H_
#define NLD_74165_H_

#include "netlist/nl_setup.h"

#define TTL_74165(name, cCLK, cCLKINH, cSH_LDQ, cSER, cA, cB, cC, cD, cE, cF, cG, cH)   \
		NET_REGISTER_DEV(TTL_74165, name)   \
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
		NET_CONNECT(name, H,      cH)

#define TTL_74165_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74165_DIP, name)

#endif /* NLD_74165_H_ */
