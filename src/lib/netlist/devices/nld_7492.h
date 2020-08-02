// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7492.h
 *
 *  SN7492: Divide-by-12 Counter
 *
 *          +--------------+
 *        B |1     ++    14| A
 *       NC |2           13| NC
 *       NC |3           12| QA
 *       NC |4    7492   11| QD
 *      VCC |5           10| GND
 *      R01 |6            9| QB
 *      R02 |7            8| QC
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
 *          |    6  ||  1 |  0 |  0 |  0 |
 *          |    7  ||  1 |  0 |  0 |  1 |
 *          |    8  ||  1 |  0 |  1 |  0 |
 *          |    9  ||  1 |  0 |  1 |  1 |
 *          |   10  ||  1 |  1 |  0 |  0 |
 *          |   11  ||  1 |  1 |  0 |  1 |
 *          +-------++----+----+----+----+
 *
 *          Note A Output QA is connected to input B
 *
 *          Reset Count Function table
 *
 *          +-----+-----++----+----+----+----+
 *          | R01 | R02 || QD | QC | QB | QA |
 *          +=====+=====++====+====+====+====+
 *          |  1  |  1  ||  0 |  0 |  0 |  0 |
 *          |  0  |  X  ||       COUNT       |
 *          |  X  |  0  ||       COUNT       |
 *          +-----+-----++----+----+----+----+
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 */

#ifndef NLD_7492_H_
#define NLD_7492_H_

#include "netlist/nl_setup.h"

// usage: TTL_7492(name, cA, cB, cR1, cR2)
#define TTL_7492(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_7492, __VA_ARGS__)

#endif /* NLD_7492_H_ */
