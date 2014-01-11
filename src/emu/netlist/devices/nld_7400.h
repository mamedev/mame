// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7400.h
 *
 *  DM7400: Quad 2-Input NAND Gates
 *
 *          +--------------+
 *       A1 |1     ++    14| VCC
 *       B1 |2           13| B4
 *       Y1 |3           12| A4
 *       A2 |4    7400   11| Y4
 *       B2 |5           10| B3
 *       Y2 |6            9| A3
 *      GND |7            8| Y3
 *          +--------------+
 *                  __
 *              Y = AB
 *          +---+---++---+
 *          | A | B || Y |
 *          +===+===++===+
 *          | 0 | 0 || 1 |
 *          | 0 | 1 || 1 |
 *          | 1 | 0 || 1 |
 *          | 1 | 1 || 0 |
 *          +---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7400_H_
#define NLD_7400_H_

#include "nld_signal.h"

#define TTL_7400_NAND(_name, _A, _B)                                                \
		NET_REGISTER_DEV(7400, _name)                                               \
		NET_CONNECT(_name, A, _A)                                                   \
		NET_CONNECT(_name, B, _B)

NETLIB_SIGNAL(7400, 2, 0, 0);

NETLIB_DEVICE(7400pin,

	NETLIB_NAME(7400) m_1;
	NETLIB_NAME(7400) m_2;
	NETLIB_NAME(7400) m_3;
	NETLIB_NAME(7400) m_4;
);

inline NETLIB_START(7400pin)
{
	register_sub(m_1, "1");
	register_sub(m_2, "2");
	register_sub(m_3, "3");
	register_sub(m_4, "4");

	register_subalias("1", m_1.m_i[0]);
	register_subalias("2", m_1.m_i[1]);
	register_subalias("3", m_1.m_Q);

	register_subalias("4", m_2.m_i[0]);
	register_subalias("5", m_2.m_i[1]);
	register_subalias("6", m_2.m_Q);

	register_subalias("9", m_3.m_i[0]);
	register_subalias("10", m_3.m_i[1]);
	register_subalias("8", m_3.m_Q);

	register_subalias("12", m_4.m_i[0]);
	register_subalias("13", m_4.m_i[1]);
	register_subalias("11", m_4.m_Q);
}

inline NETLIB_RESET(7400pin)
{
    m_1.do_reset();
    m_2.do_reset();
    m_3.do_reset();
    m_4.do_reset();
}

#endif /* NLD_7400_H_ */
