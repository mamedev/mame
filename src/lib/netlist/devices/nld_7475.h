// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_7475.h
 *
 *  7475: 4-Bit Bistable Latches with Complementary Outputs
 *  7477: 4-Bit Bistable Latches
 *
 *          +----------+               +----------+
 *      1QQ |1   ++  16| 1Q         1D |1   ++  14| 1Q
 *       1D |2       15| 2Q         2D |2       13| 2Q
 *       2D |3       14| 2QQ      3C4C |3       12| 1C2C
 *     3C4C |4  7475 13| 1C2C      VCC |4  7477 11| GND
 *      VCC |5       12| GND        3D |5       10| NC
 *       3D |6       11| 3QQ        4D |6        9| 3Q
 *       4D |7       10| 3Q         NC |7        8| 4Q
 *      4QQ |8        9| 4Q            +----------+
 *          +----------+
 *
 *
 *          Function table
 *
 *          +---+---++---+-----+
 *          | D | C || Q | QQ  |
 *          +===+===++===+=====+
 *          | 0 | 1 || 0 |  1  |
 *          | 1 | 1 || 1 |  0  |
 *          | X | 0 || Q0| Q0Q |
 *          +---+---++---+-----+
 *
 *  Naming conventions follow Texas instruments datasheet
 *
 */

#ifndef NLD_7475_H_
#define NLD_7475_H_

#include "netlist/nl_setup.h"

#define TTL_7475_GATE(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_7475_GATE, __VA_ARGS__)

#define TTL_7477_GATE(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_7477_GATE, __VA_ARGS__)

#endif /* NLD_7475_H_ */
