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
 *                 _
 *             Y = A
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
#include "nld_truthtable.h"

#define TTL_7404_INVERT(_name, _A)                                                  \
		NET_REGISTER_DEV(TTL_7404_INVERT, _name)                                               \
		NET_CONNECT(_name, A, _A)

#define TTL_7404_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7404_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

#if 1 && (USE_TRUTHTABLE)
NETLIB_TRUTHTABLE(7404, 1, 1, 0);
#else
NETLIB_DEVICE(7404,
public:
	logic_input_t m_I[1];
	logic_output_t m_Q[1];
);
#endif

NETLIB_DEVICE(7404_dip,

	NETLIB_NAME(7404) m_1;
	NETLIB_NAME(7404) m_2;
	NETLIB_NAME(7404) m_3;
	NETLIB_NAME(7404) m_4;
	NETLIB_NAME(7404) m_5;
	NETLIB_NAME(7404) m_6;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7404_H_ */
