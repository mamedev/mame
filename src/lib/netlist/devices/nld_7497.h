// license:GPL-2.0+
// copyright-holders:Sergey Svishchev
/*
 * nld_7497.h
 *
 *  SN7497: Synchronous 6-Bit Binary Rate Multiplier
 *
 *          +--------------+
 *       B1 |1           16| VCC
 *       B4 |2           15| B3
 *       B5 |3           14| B2
 *       B0 |4    7497   13| CLR
 *        Z |5           12| UNITY/CAS
 *        Y |6           11| ENin (EN)
 *    ENout |7           10| STRB
 *      GND |8            9| CLK
 *          +--------------+
 *
 *  Naming conventions follow TI datasheet
 *
 *  The counter is enabled when the clear, strobe, and enable inputs are low.
 *
 *  When the rate input is binary 0 (all rate inputs low), Z remains high [and Y low].
 *
 *  The unity/cascade input, when connected to the clock input, passes
 *    clock frequency (inverted) to the Y output when the rate input/decoding
 *    gates are inhibited by the strobe.
 *
 *  When CLR is H, states of CLK and STRB can affect Y and Z.  Default are
 *    Y L, Z H, ENout H.
 *
 *  Unity/cascade is used to inhibit output Y (UNITY L -> Y H)
 */

#ifndef NLD_7497_H_
#define NLD_7497_H_

#include "netlist/nl_setup.h"

// usage: TTL_7497(name, cCLK, cSTRB, cEN, cUNITY, cCLR, cB0, cB1, cB2, cB3, cB4, cB5)
#define TTL_7497(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_7497, __VA_ARGS__)

#endif /* NLD_7497_H_ */
