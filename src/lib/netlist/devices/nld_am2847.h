// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_am2847.h
 *
 *  Am2847: Quad 80-Bit Static Shift Register
 *
 *          +--------------+
 *     OUTA |1     ++    16| VSS
 *      RCA |2           15| IND
 *      INA |3           14| RCD
 *     OUTB |4   Am2847  13| OUTD
 *      RCB |5           12| VGG
 *      INB |6           11| CP
 *     OUTC |7           10| INC
 *      VDD |8            9| RCC
 *          +--------------+
 *
 */

#ifndef NLD_AM2847_H_
#define NLD_AM2847_H_

#include "netlist/nl_setup.h"

#define TTL_AM2847(name, cCP, cINA, cINB, cINC, cIND, cRCA, cRCB, cRCC, cRCD)  \
		NET_REGISTER_DEV(TTL_AM2847, name)                                     \
		NET_CONNECT(name, VDD, VDD)                                            \
		NET_CONNECT(name, VSS, VSS)                                            \
		NET_CONNECT(name, CP,  cCP)                                            \
		NET_CONNECT(name, INA, cINA)                                           \
		NET_CONNECT(name, INB, cINB)                                           \
		NET_CONNECT(name, INC, cINC)                                           \
		NET_CONNECT(name, IND, cIND)                                           \
		NET_CONNECT(name, RCA, cRCA)                                           \
		NET_CONNECT(name, RCB, cRCB)                                           \
		NET_CONNECT(name, RCC, cRCC)                                           \
		NET_CONNECT(name, RCD, cRCD)

#define TTL_AM2847_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_AM2847_DIP, name)

#endif /* NLD_AM2847_H_ */
