// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7474.h
 *
 *  DM7474: Dual Positive-Edge-Triggered D Flip-Flops
 *          with Preset Clear and Complementary Outputs
 *
 *          +--------------+
 *     CLR1 |1     ++    14| VCC
 *       D1 |2           13| CLR2
 *     CLK1 |3           12| D2
 *      PR1 |4    7474   11| CLK2
 *       Q1 |5           10| Q2
 *      Q1Q |6            9| Q2Q
 *      GND |7            8| Y2
 *          +--------------+
 *
 *          +-----+-----+-----+---++---+-----+
 *          | PR  | CLR | CLK | D || Q | QQ  |
 *          +=====+=====+=====+===++===+=====+
 *          |  0  |  1  |  X  | X || 1 |  0  |
 *          |  1  |  0  |  X  | X || 0 |  1  |
 *          |  0  |  0  |  X  | X || 1 |  1  | (*)
 *          |  1  |  1  |  R  | 1 || 1 |  0  |
 *          |  1  |  1  |  R  | 0 || 0 |  1  |
 *          |  1  |  1  |  L  | X || Q0| Q0Q |
 *          +-----+-----+-----+---++---+-----+
 *
 *  (*) his configuration is nonstable that is it will not persist
 *  when either the preset and or clear inputs return to their inactive (high) level
 *
 *  Q0 The output logic level of Q before the indicated input conditions were established
 *
 *  R:  0 --> 1
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 *  FIXME: Check that (*) is emulated properly
 */

#ifndef NLD_7474_H_
#define NLD_7474_H_

#include "nld_signal.h"

#define TTL_7474(_name, _CLK, _D, _CLRQ, _PREQ)                                     \
		NET_REGISTER_DEV(7474, _name)                                               \
		NET_CONNECT(_name, CLK, _CLK)                                               \
		NET_CONNECT(_name, D,  _D)                                                  \
		NET_CONNECT(_name, CLRQ,  _CLRQ)                                            \
		NET_CONNECT(_name, PREQ,  _PREQ)

NETLIB_SUBDEVICE(7474sub,
	netlist_ttl_input_t m_clk;

	UINT8 m_nextD;
	netlist_ttl_output_t m_Q;
	netlist_ttl_output_t m_QQ;

	ATTR_HOT inline void newstate(const UINT8 state);
);

NETLIB_DEVICE(7474,
	NETLIB_NAME(7474sub) sub;

	netlist_ttl_input_t m_D;
	netlist_ttl_input_t m_clrQ;
	netlist_ttl_input_t m_preQ;
);


#endif /* NLD_7474_H_ */
