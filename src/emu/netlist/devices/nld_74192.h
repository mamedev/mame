// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74192.h
 *
 *  DM74192: Synchronous 4-Bit Binary Counter with Dual Clock
 *  		 Decade counter
 *
 *  FIXME: This should be merged with the 74193 which counts to 16
 *
 *          +--------------+
 *        B |1     ++    16| VCC
 *       QB |2           15| A
 *       QA |3           14| CLEAR
 *       CD |4    74192  13| BORROWQ
 *       CU |5           12| CARRYQ
 *       QC |6           11| LOADQ
 *       QD |7           10| C
 *      GND |8            9| D
 *          +--------------+
 *
 * CD: Count up
 * CU: Count down
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_74192_H_
#define NLD_74192_H_

#include "../nl_base.h"

#define TTL_74192(_name) 				                              \
		NET_REGISTER_DEV(74192, _name)

#define TTL_74192_DIP(_name)                                                         \
		NET_REGISTER_DEV(74192_dip, _name)

NETLIB_DEVICE(74192,
	ATTR_HOT void update_outputs();

	netlist_ttl_input_t m_A;
	netlist_ttl_input_t m_B;
	netlist_ttl_input_t m_C;
	netlist_ttl_input_t m_D;
	netlist_ttl_input_t m_CLEAR;
	netlist_ttl_input_t m_LOADQ;
	netlist_ttl_input_t m_CU;
	netlist_ttl_input_t m_CD;

	netlist_state_t<INT8> m_cnt;
	netlist_state_t<UINT8> m_last_CU;
	netlist_state_t<UINT8> m_last_CD;

	netlist_ttl_output_t m_Q[4];
	netlist_ttl_output_t m_BORROWQ;
	netlist_ttl_output_t m_CARRYQ;
);

NETLIB_DEVICE_DERIVED(74192_dip, 74192,
);

#endif /* NLD_74192_H_ */
