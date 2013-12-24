// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9316.h
 *
 *  DM9316: Synchronous 4-Bit Counters
 *
 *          +--------------+
 *    CLEAR |1     ++    16| VCC
 *    CLOCK |2           15| RC (Ripple Carry)
 *        A |3           14| QA
 *        B |4    9316   13| QB
 *        C |5           12| QC
 *        D |6           11| QD
 * Enable P |7           10| Enable T
 *      GND |8            9| LOAD
 *          +--------------+
 *
 *          Counter Sequence
 *
 *          +-------++----+----+----+----+----+
 *          | COUNT || QD | QC | QB | QA | RC |
 *          +=======++====+====+====+====+====+
 *          |    0  ||  0 |  0 |  0 |  0 |  0 |
 *          |    1  ||  0 |  0 |  0 |  1 |  0 |
 *          |    2  ||  0 |  0 |  1 |  0 |  0 |
 *          |    3  ||  0 |  0 |  1 |  1 |  0 |
 *          |    4  ||  0 |  1 |  0 |  0 |  0 |
 *          |    5  ||  0 |  1 |  0 |  1 |  0 |
 *          |    6  ||  0 |  1 |  1 |  0 |  0 |
 *          |    7  ||  0 |  1 |  1 |  1 |  0 |
 *          |    8  ||  1 |  0 |  0 |  0 |  0 |
 *          |    9  ||  1 |  0 |  0 |  1 |  0 |
 *          |   10  ||  1 |  0 |  1 |  0 |  0 |
 *          |   11  ||  1 |  0 |  1 |  1 |  0 |
 *          |   12  ||  1 |  1 |  0 |  0 |  0 |
 *          |   13  ||  1 |  1 |  0 |  1 |  0 |
 *          |   14  ||  1 |  1 |  1 |  0 |  0 |
 *          |   15  ||  1 |  1 |  1 |  1 |  1 |
 *          +-------++----+----+----+----+----+
 *
 *          Reset count function: Please refer to
 *          National Semiconductor datasheet (timing diagramm)
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_9316_H_
#define NLD_9316_H_

#include "../nl_base.h"

#define TTL_9316(_name, _CLK, _ENP, _ENT, _CLRQ, _LOADQ, _A, _B, _C, _D)            \
		NET_REGISTER_DEV(9316, _name)                                               \
		NET_CONNECT(_name, CLK, _CLK)                                               \
		NET_CONNECT(_name, ENP,  _ENP)                                              \
		NET_CONNECT(_name, ENT,  _ENT)                                              \
		NET_CONNECT(_name, CLRQ, _CLRQ)                                             \
		NET_CONNECT(_name, LOADQ,_LOADQ)                                            \
		NET_CONNECT(_name, A,    _A)                                                \
		NET_CONNECT(_name, B,    _B)                                                \
		NET_CONNECT(_name, C,    _C)                                                \
		NET_CONNECT(_name, D,    _D)

NETLIB_SUBDEVICE(9316_sub,
	ATTR_HOT void update_outputs_all(const UINT8 cnt);
	ATTR_HOT void update_outputs(const UINT8 cnt);

	netlist_ttl_input_t m_clk;

	netlist_ttl_input_t m_A;
	netlist_ttl_input_t m_B;
	netlist_ttl_input_t m_C;
	netlist_ttl_input_t m_D;

	UINT8 m_cnt;
	netlist_sig_t m_loadq;
	netlist_sig_t m_ent;

	netlist_ttl_output_t m_QA;
	netlist_ttl_output_t m_QB;
	netlist_ttl_output_t m_QC;
	netlist_ttl_output_t m_QD;
	netlist_ttl_output_t m_RC;
);

NETLIB_DEVICE(9316,
	NETLIB_NAME(9316_sub) sub;
	netlist_ttl_input_t m_ENP;
	netlist_ttl_input_t m_ENT;
	netlist_ttl_input_t m_CLRQ;
	netlist_ttl_input_t m_LOADQ;
);

#endif /* NLD_9316_H_ */
