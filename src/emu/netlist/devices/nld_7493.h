// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7493.h
 *
 *  DM7493: Binary Counters
 *
 *          +--------------+
 *        B |1     ++    14| A
 *      R01 |2           13| NC
 *      R02 |3           12| QA
 *       NC |4    7493   11| QD
 *      VCC |5           10| GND
 *       NC |6            9| QB
 *       NC |7            8| QC
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
 *          |   10  ||  1 |  0 |  1 |  0 |
 *          |   11  ||  1 |  0 |  1 |  1 |
 *          |   12  ||  1 |  1 |  0 |  0 |
 *          |   13  ||  1 |  1 |  0 |  1 |
 *          |   14  ||  1 |  1 |  1 |  0 |
 *          |   15  ||  1 |  1 |  1 |  1 |
 *          +-------++----+----+----+----+
 *
 *          Note C Output QA is connected to input B
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
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7493_H_
#define NLD_7493_H_

#include "nl_base.h"

#define TTL_7493(_name, _CLKA, _CLKB, _R1, _R2)                                     \
		NET_REGISTER_DEV(TTL_7493, _name)                                               \
		NET_CONNECT(_name, CLKA, _CLKA)                                             \
		NET_CONNECT(_name, CLKB, _CLKB)                                             \
		NET_CONNECT(_name, R1,  _R1)                                                \
		NET_CONNECT(_name, R2,  _R2)

#define TTL_7493_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7493_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_SUBDEVICE(7493ff,
	logic_input_t m_I;
	logic_output_t m_Q;

	UINT8 m_reset;
	UINT8 m_state;
);

NETLIB_DEVICE(7493,
	logic_input_t m_R1;
	logic_input_t m_R2;

	NETLIB_NAME(7493ff) A;
	NETLIB_NAME(7493ff) B;
	NETLIB_NAME(7493ff) C;
	NETLIB_NAME(7493ff) D;
);

NETLIB_DEVICE_DERIVED_PURE(7493_dip, 7493);

NETLIB_NAMESPACE_DEVICES_END()


#endif /* NLD_7493_H_ */
