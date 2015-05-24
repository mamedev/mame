// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7483.h
 *
 *  DM7483: 4-Bit Binary Adder with Fast Carry
 *
 *          +--------------+
 *       A4 |1     ++    16| B4
 *       S3 |2           15| S4
 *       A3 |3           14| C4
 *       B3 |4    7483   13| C0
 *      VCC |5           12| GND
 *       S2 |6           11| B1
 *       B2 |7           10| A1
 *       A2 |8            9| S1
 *          +--------------+
 *
 *          S = (A + B + C0) & 0x0f
 *
 *          C4 = (A + B + C) > 15 ? 1 : 0
 *
 *  Naming conventions follow Fairchild Semiconductor datasheet
 *
 */

#ifndef NLD_7483_H_
#define NLD_7483_H_

#include "../nl_base.h"

#define TTL_7483(_name, _A1, _A2, _A3, _A4, _B1, _B2, _B3, _B4, _CI)                \
		NET_REGISTER_DEV(7483, _name)                                               \
		NET_CONNECT(_name, A1, _A1)                                                 \
		NET_CONNECT(_name, A2, _A2)                                                 \
		NET_CONNECT(_name, A3, _A3)                                                 \
		NET_CONNECT(_name, A4, _A4)                                                 \
		NET_CONNECT(_name, B1, _B1)                                                 \
		NET_CONNECT(_name, B2, _B2)                                                 \
		NET_CONNECT(_name, B3, _B3)                                                 \
		NET_CONNECT(_name, B4, _B4)                                                 \
		NET_CONNECT(_name, C0, _CI)

#define TTL_7483_DIP(_name)                                                         \
		NET_REGISTER_DEV(7483_dip, _name)

NETLIB_DEVICE(7483,
	netlist_logic_input_t m_C0;
	netlist_logic_input_t m_A1;
	netlist_logic_input_t m_A2;
	netlist_logic_input_t m_A3;
	netlist_logic_input_t m_A4;
	netlist_logic_input_t m_B1;
	netlist_logic_input_t m_B2;
	netlist_logic_input_t m_B3;
	netlist_logic_input_t m_B4;

	UINT8 m_lastr;

	netlist_logic_output_t m_S1;
	netlist_logic_output_t m_S2;
	netlist_logic_output_t m_S3;
	netlist_logic_output_t m_S4;
	netlist_logic_output_t m_C4;

);

NETLIB_DEVICE_DERIVED_PURE(7483_dip, 7483);

#endif /* NLD_7483_H_ */
