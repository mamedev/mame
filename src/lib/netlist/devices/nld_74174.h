// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74174.h
 *
 *  DM74174: Hex D Flip-Flops with Clear
 *
 *          +--------------+
 *      CLR |1     ++    16| VCC
 *       Q1 |2           15| Q6
 *       D1 |3           14| D6
 *       D2 |4   74174   13| D5
 *       Q2 |5           12| Q5
 *       D3 |6           11| D4
 *       Q3 |7           10| Q4
 *      GND |8            9| CLK
 *          +--------------+
 *
 *          +-----+-----+---++---+-----+
 *          | CLR | CLK | D || Q | QQ  |
 *          +=====+=====+===++===+=====+
 *          |  0  |  X  | X || 0 |  1  |
 *          |  1  |  R  | 1 || 1 |  0  |
 *          |  1  |  R  | 0 || 0 |  1  |
 *          |  1  |  0  | X || Q0| Q0Q |
 *          +-----+-----+---++---+-----+
 *
 *   Q0 The output logic level of Q before the indicated input conditions were established
 *
 *  R:  0 -> 1
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_74174_H_
#define NLD_74174_H_

#include "netlist/nl_setup.h"

// usage: TTL_74174_GATE(name)
#define TTL_74174_GATE(...)                                                   \
		NET_REGISTER_DEVEXT(TTL_74174_GATE, __VA_ARGS__)

// usage: TTL_74174(name, cCLK, cD1, cD2, cD3, cD4, cD5, cD6, cCLRQ)
#define TTL_74174(...)                                                        \
		NET_REGISTER_DEVEXT(TTL_74174, __VA_ARGS__)

#endif /* NLD_74174_H_ */
