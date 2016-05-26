// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7450.h
 *
 *  DM7450: DUAL 2-WIDE 2-INPUT AND-OR-INVERT GATES (ONE GATE EXPANDABLE)
 *
 *          +--------------+
 *       1A |1     ++    14| VCC
 *       2A |2           13| 1B
 *       2B |3           12| 1XQ
 *       2C |4    7450   11| 1X
 *       2D |5           10| 1D
 *       2Y |6            9| 1C
 *      GND |7            8| 1Y
 *          +--------------+
 *                  _________________
 *              Y = (A & B) | (C & D)
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 */

#ifndef NLD_7450_H_
#define NLD_7450_H_

#include "nl_base.h"

#define TTL_7450_ANDORINVERT(name, cI1, cI2, cI3, cI4)                             \
		NET_REGISTER_DEV(TTL_7450_ANDORINVERT, name)                               \
		NET_CONNECT(name, A, cI1)                                                  \
		NET_CONNECT(name, B, cI2)                                                  \
		NET_CONNECT(name, C, cI3)                                                  \
		NET_CONNECT(name, D, cI4)

#define TTL_7450_DIP(name)                                                         \
		NET_REGISTER_DEV(TTL_7450_DIP, name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(7450)
{
	NETLIB_CONSTRUCTOR(7450)
	{
		enregister("A", m_A);
		enregister("B", m_B);
		enregister("C", m_C);
		enregister("D", m_D);
		enregister("Q", m_Q);
	}
	//NETLIB_RESETI();
	NETLIB_UPDATEI();

public:
	logic_input_t m_A;
	logic_input_t m_B;
	logic_input_t m_C;
	logic_input_t m_D;
	logic_output_t m_Q;
};

NETLIB_OBJECT(7450_dip)
{
	NETLIB_CONSTRUCTOR(7450_dip)
	, m_1(*this, "1")
	, m_2(*this, "2")
	{

		register_subalias("1", m_1.m_A);
		register_subalias("2", m_2.m_A);
		register_subalias("3", m_2.m_B);
		register_subalias("4", m_2.m_C);
		register_subalias("5", m_2.m_D);
		register_subalias("6", m_2.m_Q);
		//register_subalias("7",);  GND

		register_subalias("8", m_1.m_Q);
		register_subalias("9", m_1.m_C);
		register_subalias("10", m_1.m_D);
		//register_subalias("11", m_1.m_X1);
		//register_subalias("12", m_1.m_X1Q);
		register_subalias("13", m_1.m_B);
		//register_subalias("14",);  VCC
	}
	//NETLIB_RESETI();
	//NETLIB_UPDATEI();

	NETLIB_SUB(7450) m_1;
	NETLIB_SUB(7450) m_2;
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7450_H_ */
