// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7490.h
 *
 *  DM7490: Decade Counters
 *
 *          +--------------+
 *        B |1     ++    14| A
 *      R01 |2           13| NC
 *      R02 |3           12| QA
 *       NC |4    7490   11| QD
 *      VCC |5           10| GND
 *      R91 |6            9| QB
 *      R92 |7            8| QC
 *          +--------------+
 *
 *          Counter Sequence
 *
 *          +-------++----+----+----+----+
 *          | COUNT || QD | QC | QB | QA |
 *          +=======++====+====+====+====+
 *          |    0  ||  0 |  0 |  0 |  0 |
 *          |    1  ||  0 |  0 |  0 |  1 |
 *          |    2  ||  0 |  0 |  1 |  0 |
 *          |    3  ||  0 |  0 |  1 |  1 |
 *          |    4  ||  0 |  1 |  0 |  0 |
 *          |    5  ||  0 |  1 |  0 |  1 |
 *          |    6  ||  0 |  1 |  1 |  0 |
 *          |    7  ||  0 |  1 |  1 |  1 |
 *          |    8  ||  1 |  0 |  0 |  0 |
 *          |    9  ||  1 |  0 |  0 |  1 |
 *          +-------++----+----+----+----+
 *
 *          Note A Output QA is connected to input B for BCD count
 *
 *          Reset Count Function table
 *
 *          +-----+-----+-----+-----++----+----+----+----+
 *          | R01 | R02 | R91 | R92 || QD | QC | QB | QA |
 *          +=====+=====+=====+=====++====+====+====+====+
 *          |  1  |  1  |  0  |  X  ||  0 |  0 |  0 |  0 |
 *          |  1  |  1  |  X  |  0  ||  0 |  0 |  0 |  0 |
 *          |  X  |  X  |  1  |  1  ||  1 |  0 |  0 |  1 |
 *          |  X  |  0  |  X  |  0  ||       COUNT       |
 *          |  0  |  X  |  0  |  X  ||       COUNT       |
 *          |  0  |  X  |  X  |  0  ||       COUNT       |
 *          |  X  |  0  |  0  |  X  ||       COUNT       |
 *          +-----+-----+-----+-----++----+----+----+----+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7490_H_
#define NLD_7490_H_

#include "netlist/nl_setup.h"

#define TTL_7490(name, cA, cB, cR1, cR2, cR91, cR92)                           \
		NET_REGISTER_DEV(TTL_7490, name)                                       \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, A, cA)                                               \
		NET_CONNECT(name, B, cB)                                               \
		NET_CONNECT(name, R1,  cR1)                                            \
		NET_CONNECT(name, R2,  cR2)                                            \
		NET_CONNECT(name, R91, cR91)                                           \
		NET_CONNECT(name, R92, cR92)

#define TTL_7490_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_7490_DIP, name)

#endif /* NLD_7490_H_ */
