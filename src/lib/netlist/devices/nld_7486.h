// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7486.h
 *
 *  DM7486: Quad 2-Input Quad 2-Input Exclusive-OR Gates
 *
 *          +--------------+
 *       A1 |1     ++    14| VCC
 *       B1 |2           13| B4
 *       Y1 |3           12| A4
 *       A2 |4    7486   11| Y4
 *       B2 |5           10| B3
 *       Y2 |6            9| A3
 *      GND |7            8| Y3
 *          +--------------+
 *
 *             Y = A+B
 *          +---+---++---+
 *          | A | B || Y |
 *          +===+===++===+
 *          | 0 | 0 || 0 |
 *          | 0 | 1 || 1 |
 *          | 1 | 0 || 1 |
 *          | 1 | 1 || 0 |
 *          +---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7486_H_
#define NLD_7486_H_

#include "nld_signal.h"
#include "nld_truthtable.h"

#define TTL_7486_XOR(_name, _A, _B)                                                 \
		NET_REGISTER_DEV(TTL_7486_XOR, _name)                                               \
		NET_CONNECT(_name, A, _A)                                                   \
		NET_CONNECT(_name, B, _B)


#define TTL_7486_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7486_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

#if (USE_TRUTHTABLE)
NETLIB_TRUTHTABLE(7486, 2, 1, 0);
#else
NETLIB_DEVICE(7486,
public:
		logic_input_t m_I[2];
		logic_output_t m_Q[1];

		ATTR_HOT void inc_active() override;
		ATTR_HOT void dec_active() override;
		int m_active;
);
#endif

NETLIB_DEVICE(7486_dip,

	NETLIB_NAME(7486) m_1;
	NETLIB_NAME(7486) m_2;
	NETLIB_NAME(7486) m_3;
	NETLIB_NAME(7486) m_4;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7486_H_ */
