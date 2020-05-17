// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4013.h
 *
 *  CD4013: Dual Positive-Edge-Triggered D Flip-Flops
 *          with Set, Reset and Complementary Outputs
 *
 *          +--------------+
 *       Q1 |1     ++    14| VDD
 *      Q1Q |2           13| Q2
 *   CLOCK1 |3           12| Q2Q
 *   RESET1 |4    4013   11| CLOCK2
 *    DATA1 |5           10| RESET2
 *     SET1 |6            9| DATA2
 *      VSS |7            8| SET2
 *          +--------------+
 *
 *          +-----+-----+-----+---++---+-----+
 *          | SET | RES | CLK | D || Q | QQ  |
 *          +=====+=====+=====+===++===+=====+
 *          |  1  |  0  |  X  | X || 1 |  0  |
 *          |  0  |  1  |  X  | X || 0 |  1  |
 *          |  1  |  1  |  X  | X || 1 |  1  | (*)
 *          |  0  |  0  |  R  | 1 || 1 |  0  |
 *          |  0  |  0  |  R  | 0 || 0 |  1  |
 *          |  0  |  0  |  0  | X || Q0| Q0Q |
 *          +-----+-----+-----+---++---+-----+
 *
 *  (*) This configuration is not stable, i.e. it will not persist
 *  when either the preset and or clear inputs return to their inactive (high) level
 *
 *  Q0 The output logic level of Q before the indicated input conditions were established
 *
 *  R:  0 -. 1
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 *  FIXME: Check that (*) is emulated properly
 */

#ifndef NLD_4013_H_
#define NLD_4013_H_

#include "netlist/nl_setup.h"

// usage: CD4013(name, cCLOCK, cDATA, cRESET, cSET)
#define CD4013(...)                                                            \
		NET_REGISTER_DEVEXT(CD4013, __VA_ARGS__)

#define CD4013_DIP(name)                                                       \
		NET_REGISTER_DEV(CD4013_DIP, name)

#endif /* NLD_4013_H_ */
