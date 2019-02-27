// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_74365.h
 *
 *  SN74365: Hex Bus Driver with 3-State Outputs
 *
 *          +--------------+
 *      G1Q |1     ++    16| VCC
 *       A1 |2           15| G2Q
 *       Y1 |3           14| A6
 *       A2 |4    74365  13| Y6
 *       Y2 |5           12| A5
 *       A3 |6           11| Y5
 *       Y3 |7           10| A4
 *      GND |8            9| Y4
 *          +--------------+
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 *  Note: Currently the netlist system does not support proper tristate output, so this
 *        is not a "real" bus driver, it simply outputs 0 if the chip is not enabled.
 */

#ifndef NLD_74365_H_
#define NLD_74365_H_

#include "netlist/nl_setup.h"

#define TTL_74365(name, cG1Q, cG2Q, cA1, cA2, cA3, cA4, cA5, cA6)   \
		NET_REGISTER_DEV(TTL_74365, name)   \
		NET_CONNECT(name, G1Q,   cG1Q)  \
		NET_CONNECT(name, G2Q,   cG2Q)  \
		NET_CONNECT(name, A1,    cA1)   \
		NET_CONNECT(name, A2,    cA2)   \
		NET_CONNECT(name, A3,    cA3)   \
		NET_CONNECT(name, A4,    cA4)   \
		NET_CONNECT(name, A5,    cA5)   \
		NET_CONNECT(name, A6,    cA6)

#define TTL_74365_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74365_DIP, name)

#endif /* NLD_74365_H_ */
