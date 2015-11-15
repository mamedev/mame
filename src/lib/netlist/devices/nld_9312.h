// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9312.h
 *
 *  DM9312: One of Eight Line Data Selectors/Multiplexers
 *
 *          +--------------+
 *       D0 |1     ++    16| VCC
 *       D1 |2           15| Y
 *       D2 |3           14| YQ
 *       D3 |4    9312   13| C
 *       D4 |5           12| B
 *       D5 |6           11| A
 *       D6 |7           10| G   Strobe
 *      GND |8            9| D7
 *          +--------------+
 *                  __
 *          +---+---+---+---++---+---+
 *          | C | B | A | G || Y | YQ|
 *          +===+===+===+===++===+===+
 *          | X | X | X | 1 ||  0| 1 |
 *          | 0 | 0 | 0 | 0 || D0|D0Q|
 *          | 0 | 0 | 1 | 0 || D1|D1Q|
 *          | 0 | 1 | 0 | 0 || D2|D2Q|
 *          | 0 | 1 | 1 | 0 || D3|D3Q|
 *          | 1 | 0 | 0 | 0 || D4|D4Q|
 *          | 1 | 0 | 1 | 0 || D5|D5Q|
 *          | 1 | 1 | 0 | 0 || D6|D6Q|
 *          | 1 | 1 | 1 | 0 || D7|D7Q|
 *          +---+---+---+---++---+---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_9312_H_
#define NLD_9312_H_

#include "nld_signal.h"
#include "nld_truthtable.h"

#define TTL_9312(_name)                                                \
		NET_REGISTER_DEV(TTL_9312, _name)

#define TTL_9312_DIP(_name)                                            \
		NET_REGISTER_DEV(TTL_9312_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

#if (USE_TRUTHTABLE)
/* The truthtable implementation is a lot faster than
 * the carefully crafted code :-(
 */
NETLIB_TRUTHTABLE(9312, 12, 2, 0);
#else

NETLIB_DEVICE(9312,
public:
//      C, B, A, G,D0,D1,D2,D3,D4,D5,D6,D7| Y,YQ
	netlist_logic_input_t m_A;
	netlist_logic_input_t m_B;
	netlist_logic_input_t m_C;
	netlist_logic_input_t m_G;
	netlist_logic_input_t m_D[8];
	netlist_logic_output_t m_Y;
	netlist_logic_output_t m_YQ;

	UINT8 m_last_chan;
	UINT8 m_last_G;
);

#endif

NETLIB_DEVICE(9312_dip,
	NETLIB_NAME(9312) m_sub;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_9312_H_ */
