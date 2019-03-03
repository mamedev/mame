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

#include "netlist/nl_setup.h"

#define TTL_74161(name, cA, cB, cC, cD, cCLRQ, cLOADQ, cCLK, cENABLEP, cENABLET)    \
		NET_REGISTER_DEV(TTL_74161, name)   \
		NET_CONNECT(name, A,       cA)      \
		NET_CONNECT(name, B,       cB)      \
		NET_CONNECT(name, C,       cC)      \
		NET_CONNECT(name, D,       cD)      \
		NET_CONNECT(name, CLRQ,   cCLRQ)    \
		NET_CONNECT(name, LOADQ,   cLOADQ)  \
		NET_CONNECT(name, CLK,     cCLK)    \
		NET_CONNECT(name, ENABLEP, cENABLEP) \
		NET_CONNECT(name, ENABLET, cENABLET)

#define TTL_74161_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74161_DIP, name)

#endif /* NLD_74161_H_ */
