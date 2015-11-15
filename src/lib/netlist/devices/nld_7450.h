// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7450.h
 *
 *  DM7450: DUAL 2-WIDE 2-INPUT AND-OR-INVERT GATES (ONE GATE EXPANDABLE)
 *
 *          +--------------+
 *       1A |1     ++    14| VCC
 *       2A |2           13| 1B
 *       2B |3           12| 1XQ
 *       2C |4    7450   11| 1X
 *       2D |5           10| 1D
 *       2Y |6            9| 1C
 *      GND |7            8| 1Y
 *          +--------------+
 *                  _________________
 *              Y = (A & B) | (C & D)
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 */

#ifndef NLD_7450_H_
#define NLD_7450_H_

#include "nld_signal.h"

#define TTL_7450_ANDORINVERT(_name, _I1, _I2, _I3, _I4)                             \
		NET_REGISTER_DEV(TTL_7450_ANDORINVERT, _name)                                               \
		NET_CONNECT(_name, A, _I1)                                                  \
		NET_CONNECT(_name, B, _I2)                                                  \
		NET_CONNECT(_name, C, _I3)                                                  \
		NET_CONNECT(_name, D, _I4)

#define TTL_7450_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7450_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_DEVICE(7450,
public:
	logic_input_t m_A;
	logic_input_t m_B;
	logic_input_t m_C;
	logic_input_t m_D;
	logic_output_t m_Q;
);

NETLIB_DEVICE(7450_dip,

	NETLIB_NAME(7450) m_1;
	NETLIB_NAME(7450) m_2;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7450_H_ */
