// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7474.h
 *
 *  DM7474: Dual Positive-Edge-Triggered D Flip-Flops
 *          with Preset, Clear and Complementary Outputs
 *
 *          +--------------+
 *     CLR1 |1     ++    14| VCC
 *       D1 |2           13| CLR2
 *     CLK1 |3           12| D2
 *      PR1 |4    7474   11| CLK2
 *       Q1 |5           10| PR2
 *      Q1Q |6            9| Q2
 *      GND |7            8| Q2Q
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
 *          |  1  |  1  |  0  | X || Q0| Q0Q |
 *          +-----+-----+-----+---++---+-----+
 *
 *  (*) This configuration is not stable, i.e. it will not persist
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
		NET_REGISTER_DEV(TTL_7474, _name)                                               \
		NET_CONNECT(_name, CLK, _CLK)                                               \
		NET_CONNECT(_name, D,  _D)                                                  \
		NET_CONNECT(_name, CLRQ,  _CLRQ)                                            \
		NET_CONNECT(_name, PREQ,  _PREQ)

#define TTL_7474_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7474_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_SUBDEVICE(7474sub,
	logic_input_t m_CLK;

	INT8 m_nextD;
	logic_output_t m_Q;
	logic_output_t m_QQ;

	ATTR_HOT inline void newstate(const UINT8 stateQ, const UINT8 stateQQ);
);

NETLIB_DEVICE(7474,
public:
	NETLIB_NAME(7474sub) sub;

	logic_input_t m_D;
	logic_input_t m_CLRQ;
	logic_input_t m_PREQ;
);

NETLIB_DEVICE(7474_dip,

	NETLIB_NAME(7474) m_1;
	NETLIB_NAME(7474) m_2;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7474_H_ */
