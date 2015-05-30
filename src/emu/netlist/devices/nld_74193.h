// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74193.h
 *
 *  DM74193: Synchronous 4-Bit Binary Counter with Dual Clock
 *
 *          +--------------+
 *        B |1     ++    16| VCC
 *       QB |2           15| A
 *       QA |3           14| CLEAR
 *       CD |4    74193  13| BORROWQ
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

#ifndef NLD_74193_H_
#define NLD_74193_H_

#include "../nl_base.h"

#define TTL_74193(_name)                                              \
		NET_REGISTER_DEV(74193, _name)

#define TTL_74193_DIP(_name)                                                         \
		NET_REGISTER_DEV(74193_dip, _name)

NETLIB_DEVICE(74193,
	ATTR_HOT void update_outputs();

	netlist_logic_input_t m_A;
	netlist_logic_input_t m_B;
	netlist_logic_input_t m_C;
	netlist_logic_input_t m_D;
	netlist_logic_input_t m_CLEAR;
	netlist_logic_input_t m_LOADQ;
	netlist_logic_input_t m_CU;
	netlist_logic_input_t m_CD;

	INT8 m_cnt;
	UINT8 m_last_CU;
	UINT8 m_last_CD;

	netlist_logic_output_t m_Q[4];
	netlist_logic_output_t m_BORROWQ;
	netlist_logic_output_t m_CARRYQ;
);

NETLIB_DEVICE_DERIVED_PURE(74193_dip, 74193);

#endif /* NLD_74193_H_ */
