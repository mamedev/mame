// license:BSD-3-Clause
// copyright-holders:Felipe Sanches
/*
 * nld_DM9314.h
 *
 *  DM9314: 4-Bit Latches
 *
 *          +--------------+
 *       /E |1     ++    16| VCC
 *      /S0 |2           15| Q0
 *       D0 |3           14| /S1
 *       D1 |4   DM9314  13| Q1
 *      /S2 |5           12| Q2
 *       D2 |6           11| /S3
 *       D3 |7           10| Q3
 *      GND |8            9| /MR
 *          +--------------+
 *
 */

#ifndef NLD_DM9314_H_
#define NLD_DM9314_H_

#include "netlist/nl_setup.h"

#define TTL_9314(name, cEQ, cMRQ, cS0Q, cS1Q, cS2Q, cS3Q, cD0, cD1, cD2, cD3) \
		NET_REGISTER_DEV(TTL_9314, name)    \
		NET_CONNECT(name,  EQ,  cEQ)  \
		NET_CONNECT(name, MRQ, cMRQ)  \
		NET_CONNECT(name, S0Q, cS0Q)  \
		NET_CONNECT(name, S1Q, cS1Q)  \
		NET_CONNECT(name, S2Q, cS2Q)  \
		NET_CONNECT(name, S3Q, cS3Q)  \
		NET_CONNECT(name, D0, cD0)  \
		NET_CONNECT(name, D1, cD1)  \
		NET_CONNECT(name, D2, cD2)  \
		NET_CONNECT(name, D3, cD3)

#define TTL_9314_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_9314_DIP, name)

#endif /* NLD_DM9314_H_ */
