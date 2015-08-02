// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74153.h
 *
 *  DM74153: Dual 4-Line to 1-Line Data Selectors Multiplexers
 *
 *          +--------------+
 *       G1 |1     ++    16| VCC
 *        B |2           15| G2
 *      1C3 |3           14| A
 *      1C2 |4   74153   13| 2C3
 *      1C1 |5           12| 2C2
 *      1C0 |6           11| 2C1
 *       Y1 |7           10| 2C0
 *      GND |8            9| Y2
 *          +--------------+
 *
 *
 *          Function table
 *
 *          +-----+-----++----+----+----+----++----+----+
 *          |  B  |  A  || C0 | C1 | C2 | C3 ||  G |  Y |
 *          +=====+=====++====+====+====+====++====+====+
 *          |  X  |  X  ||  X |  X |  X |  X ||  H |  L |
 *          |  L  |  L  ||  L |  X |  X |  X ||  L |  L |
 *          |  L  |  L  ||  H |  X |  X |  X ||  L |  H |
 *          |  L  |  H  ||  X |  L |  X |  X ||  L |  L |
 *          |  L  |  H  ||  X |  H |  X |  X ||  L |  H |
 *          |  H  |  L  ||  X |  X |  L |  X ||  L |  L |
 *          |  H  |  L  ||  X |  X |  H |  X ||  L |  H |
 *          |  H  |  H  ||  X |  X |  X |  L ||  L |  L |
 *          |  H  |  H  ||  X |  X |  X |  H ||  L |  H |
 *          +-----+-----++----+----+----+----++----+----+
 *
 *  A, B : Select Inputs
 *  C*   : Data inputs
 *  G    : Strobe
 *  Y    : Output
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_74153_H_
#define NLD_74153_H_

#include "nl_base.h"

#define TTL_74153(_name, _C0, _C1, _C2, _C3, _A, _B, _G)                            \
		NET_REGISTER_DEV(TTL_74153, _name)                                              \
		NET_CONNECT(_name, C0, _C0)                                                 \
		NET_CONNECT(_name, C1, _C1)                                                 \
		NET_CONNECT(_name, C2, _C2)                                                 \
		NET_CONNECT(_name, C3, _C3)                                                 \
		NET_CONNECT(_name, A, _A)                                                   \
		NET_CONNECT(_name, B, _B)                                                   \
		NET_CONNECT(_name, G, _G)

#define TTL_74153_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_74153_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_SUBDEVICE(74153sub,
	logic_input_t m_C[4];
	logic_input_t m_G;

	logic_output_t m_Y;

	int m_chan;
);

NETLIB_DEVICE(74153,
public:
	NETLIB_NAME(74153sub) m_sub;
	logic_input_t m_A;
	logic_input_t m_B;
);

NETLIB_DEVICE(74153_dip,

	NETLIB_NAME(74153sub) m_1;
	NETLIB_NAME(74153sub) m_2;
	logic_input_t m_A;
	logic_input_t m_B;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_74153_H_ */
