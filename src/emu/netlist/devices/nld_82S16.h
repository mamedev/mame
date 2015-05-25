// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_82S16.h
 *
 *  DM82S16: 256 Bit bipolar ram
 *
 *          +--------------+
 *       A1 |1     ++    16| VCC
 *       A0 |2           15| A2
 *     CE1Q |3           14| A3
 *     CE2Q |4   82S16   13| DIN
 *     CE3Q |5           12| WEQ
 *    DOUTQ |6           11| A7
 *       A4 |7           10| A6
 *      GND |8            9| A5
 *          +--------------+
 *
 *
 *  Naming conventions follow Signetics datasheet
 *
 */

#ifndef NLD_82S16_H_
#define NLD_82S16_H_

#include "../nl_base.h"

#define TTL_82S16(_name)                                     \
		NET_REGISTER_DEV(82S16, _name)
#define TTL_82S16_DIP(_name)                                 \
		NET_REGISTER_DEV(82S16_dip, _name)

NETLIB_DEVICE(82S16,

	netlist_logic_input_t m_A[8];
	netlist_logic_input_t m_CE1Q;
	netlist_logic_input_t m_CE2Q;
	netlist_logic_input_t m_CE3Q;
	netlist_logic_input_t m_WEQ;
	netlist_logic_input_t m_DIN;
	netlist_logic_output_t m_DOUTQ;

	//netlist_state_t<UINT8[256]> m_ram;
	UINT8 m_ram[256];
);

NETLIB_DEVICE_DERIVED_PURE(82S16_dip, 82S16);

#endif /* NLD_82S16_H_ */
