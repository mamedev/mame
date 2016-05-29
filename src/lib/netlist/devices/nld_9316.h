// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9316.h
 *
 *  DM9316: Synchronous 4-Bit Counters
 *
 *          +--------------+
 *    CLEAR |1     ++    16| VCC
 *    CLOCK |2           15| RC (Ripple Carry)
 *        A |3           14| QA
 *        B |4    9316   13| QB
 *        C |5           12| QC
 *        D |6           11| QD
 * Enable P |7           10| Enable T
 *      GND |8            9| LOAD
 *          +--------------+
 *
 *          Counter Sequence
 *
 *          +-------++----+----+----+----+----+
 *          | COUNT || QD | QC | QB | QA | RC |
 *          +=======++====+====+====+====+====+
 *          |    0  ||  0 |  0 |  0 |  0 |  0 |
 *          |    1  ||  0 |  0 |  0 |  1 |  0 |
 *          |    2  ||  0 |  0 |  1 |  0 |  0 |
 *          |    3  ||  0 |  0 |  1 |  1 |  0 |
 *          |    4  ||  0 |  1 |  0 |  0 |  0 |
 *          |    5  ||  0 |  1 |  0 |  1 |  0 |
 *          |    6  ||  0 |  1 |  1 |  0 |  0 |
 *          |    7  ||  0 |  1 |  1 |  1 |  0 |
 *          |    8  ||  1 |  0 |  0 |  0 |  0 |
 *          |    9  ||  1 |  0 |  0 |  1 |  0 |
 *          |   10  ||  1 |  0 |  1 |  0 |  0 |
 *          |   11  ||  1 |  0 |  1 |  1 |  0 |
 *          |   12  ||  1 |  1 |  0 |  0 |  0 |
 *          |   13  ||  1 |  1 |  0 |  1 |  0 |
 *          |   14  ||  1 |  1 |  1 |  0 |  0 |
 *          |   15  ||  1 |  1 |  1 |  1 |  1 |
 *          +-------++----+----+----+----+----+
 *
 *          Reset count function: Please refer to
 *          National Semiconductor datasheet (timing diagramm)
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_9316_H_
#define NLD_9316_H_

#include "nl_setup.h"

#define TTL_9316(name, cCLK, cENP, cENT, cCLRQ, cLOADQ, cA, cB, cC, cD)         \
		NET_REGISTER_DEV(TTL_9316, name)                                        \
		NET_CONNECT(name, CLK, cCLK)                                            \
		NET_CONNECT(name, ENP,  cENP)                                           \
		NET_CONNECT(name, ENT,  cENT)                                           \
		NET_CONNECT(name, CLRQ, cCLRQ)                                          \
		NET_CONNECT(name, LOADQ, cLOADQ)                                        \
		NET_CONNECT(name, A,    cA)                                             \
		NET_CONNECT(name, B,    cB)                                             \
		NET_CONNECT(name, C,    cC)                                             \
		NET_CONNECT(name, D,    cD)

#define TTL_9316_DIP(name)                                                      \
		NET_REGISTER_DEV(TTL_9316_DIP, name)

#endif /* NLD_9316_H_ */
