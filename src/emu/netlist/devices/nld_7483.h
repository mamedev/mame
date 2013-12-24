// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7483.h
 *
 *  DM7483: 4-Bit Binary Adder with Fast Carry
 *
 *          +--------------+
 *       A4 |1     ++    16| B4
 *       ??3 |2           15| ??3
 *       A3 |3           14| C4
 *       B3 |4    7483   13| C0
 *      VCC |5           12| GND
 *       ??3 |6           11| B1
 *       B2 |7           10| A1
 *       A2 |8            9| ??3
 *          +--------------+
 *
 *          ?? = (A + B + C) & 0x0f
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

NETLIB_DEVICE(7483,
	netlist_ttl_input_t m_C0;
	netlist_ttl_input_t m_A1;
	netlist_ttl_input_t m_A2;
	netlist_ttl_input_t m_A3;
	netlist_ttl_input_t m_A4;
	netlist_ttl_input_t m_B1;
	netlist_ttl_input_t m_B2;
	netlist_ttl_input_t m_B3;
	netlist_ttl_input_t m_B4;

	UINT8 m_lastr;

	netlist_ttl_output_t m_SA;
	netlist_ttl_output_t m_SB;
	netlist_ttl_output_t m_SC;
	netlist_ttl_output_t m_SD;
	netlist_ttl_output_t m_C4;

);

#endif /* NLD_7483_H_ */
