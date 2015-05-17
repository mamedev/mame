// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7408.h
 *
 *  DM7408: Quad 2-Input AND Gates
 *
 *          +--------------+
 *       A1 |1     ++    14| VCC
 *       B1 |2           13| B4
 *       Y1 |3           12| A4
 *       A2 |4    7408   11| Y4
 *       B2 |5           10| B3
 *       Y2 |6            9| A3
 *      GND |7            8| Y3
 *          +--------------+
 *                  __
 *              Y = AB
 *          +---+---++---+
 *          | A | B || Y |
 *          +===+===++===+
 *          | 0 | 0 || 0 |
 *          | 0 | 1 || 0 |
 *          | 1 | 0 || 0 |
 *          | 1 | 1 || 1 |
 *          +---+---++---+
 *
 *  Naming conventions follow Fairchild Semiconductor datasheet
 *
 */

#ifndef NLD_7408_H_
#define NLD_7408_H_

#include "nld_signal.h"

#define TTL_7408_AND(_name, _A, _B)                                                 \
		NET_REGISTER_DEV(7408, _name)                                               \
		NET_CONNECT(_name, A, _A)                                                   \
		NET_CONNECT(_name, B, _B)

#if (USE_TRUTHTABLE)
#include "nld_truthtable.h"
NETLIB_TRUTHTABLE(7408, 2, 1, 0);
#else
NETLIB_SIGNAL(7408, 2, 0, 1);
#endif

#define TTL_7408_DIP(_name)                                                         \
		NET_REGISTER_DEV(7408_dip, _name)

NETLIB_DEVICE(7408_dip,

	NETLIB_NAME(7408) m_1;
	NETLIB_NAME(7408) m_2;
	NETLIB_NAME(7408) m_3;
	NETLIB_NAME(7408) m_4;
);

#endif /* NLD_7408_H_ */
