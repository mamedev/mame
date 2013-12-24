// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7404.h
 *
 *  DM7404: Hex Inverting Gates
 *
 *          +--------------+
 *       A1 |1     ++    14| VCC
 *       Y1 |2           13| A6
 *       A2 |3           12| Y6
 *       Y2 |4    7404   11| A5
 *       A3 |5           10| Y5
 *       Y3 |6            9| A4
 *      GND |7            8| Y4
 *          +--------------+
 *
 *             Y = A+B
 *          +---++---+
 *          | A || Y |
 *          +===++===+
 *          | 0 || 1 |
 *          | 1 || 0 |
 *          +---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7404_H_
#define NLD_7404_H_

#include "nld_signal.h"

NETLIB_DEVICE(nic7404,
	netlist_ttl_input_t m_I;
	netlist_ttl_output_t m_Q;
);

#define TTL_7404_INVERT(_name, _A)                                                  \
		NET_REGISTER_DEV(nic7404, _name)                                            \
		NET_CONNECT(_name, A, _A)

#endif /* NLD_7404_H_ */
