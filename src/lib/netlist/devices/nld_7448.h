// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7448.h
 *
 *  DM7448: BCD to 7-Segment decoders/drivers
 *
 *           +--------------+
 *         B |1     ++    16| VCC
 *         C |2           15| f
 * LAMP TEST |3           14| g
 *    BI/RBQ |4    7448   13| a
 *       RBI |5           12| b
 *         D |6           11| c
 *         A |7           10| d
 *       GND |8            9| e
 *           +--------------+
 *
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7448_H_
#define NLD_7448_H_

#include "nl_base.h"
#include "nld_truthtable.h"

#define TTL_7448(name, cA0, cA1, cA2, cA3, cLTQ, cBIQ, cRBIQ)                      \
		NET_REGISTER_DEV(TTL_7448, name)                                               \
		NET_CONNECT(name, A, cA0)                                                  \
		NET_CONNECT(name, B, cA1)                                                  \
		NET_CONNECT(name, C, cA2)                                                  \
		NET_CONNECT(name, D, cA3)                                                  \
		NET_CONNECT(name, LTQ, cLTQ)                                               \
		NET_CONNECT(name, BIQ, cBIQ)                                               \
		NET_CONNECT(name, RBIQ, cRBIQ)

#define TTL_7448_DIP(name)                                                         \
		NET_REGISTER_DEV(TTL_7448_DIP, name)

NETLIB_NAMESPACE_DEVICES_START()

/*
 * FIXME: Using truthtable is a lot slower than the explicit device
 */

#define USE_TRUTHTABLE_7448	(0)

#if (USE_TRUTHTABLE_7448 && USE_TRUTHTABLE)

NETLIB_TRUTHTABLE(7448, 7, 7, 0);

#else

NETLIB_OBJECT(7448_sub)
{
	NETLIB_CONSTRUCTOR(7448_sub)
	, m_state(0)
	{
		enregister("A0", m_A);
		enregister("A1", m_B);
		enregister("A2", m_C);
		enregister("A3", m_D);
		enregister("RBIQ", m_RBIQ);

		enregister("a", m_Q[0]);
		enregister("b", m_Q[1]);
		enregister("c", m_Q[2]);
		enregister("d", m_Q[3]);
		enregister("e", m_Q[4]);
		enregister("f", m_Q[5]);
		enregister("g", m_Q[6]);

		save(NLNAME(m_state));
	}

	NETLIB_RESETI() { m_state = 0; }
	NETLIB_UPDATEI();

public:
	ATTR_HOT void update_outputs(UINT8 v);
	static const UINT8 tab7448[16][7];

	logic_input_t m_A;
	logic_input_t m_B;
	logic_input_t m_C;
	logic_input_t m_D;
	logic_input_t m_RBIQ;

	UINT8 m_state;

	logic_output_t m_Q[7];  /* a .. g */

};

NETLIB_OBJECT(7448)
{
	NETLIB_CONSTRUCTOR(7448)
	, m_sub(*this, "sub")
	{

		register_subalias("A", m_sub.m_A);
		register_subalias("B", m_sub.m_B);
		register_subalias("C", m_sub.m_C);
		register_subalias("D", m_sub.m_D);
		enregister("LTQ", m_LTQ);
		enregister("BIQ", m_BIQ);
		register_subalias("RBIQ",m_sub.m_RBIQ);

		register_subalias("a", m_sub.m_Q[0]);
		register_subalias("b", m_sub.m_Q[1]);
		register_subalias("c", m_sub.m_Q[2]);
		register_subalias("d", m_sub.m_Q[3]);
		register_subalias("e", m_sub.m_Q[4]);
		register_subalias("f", m_sub.m_Q[5]);
		register_subalias("g", m_sub.m_Q[6]);
	}

	NETLIB_RESETI() { m_sub.do_reset(); }
	NETLIB_UPDATEI();

public:
	NETLIB_SUB(7448_sub) m_sub;

	logic_input_t m_LTQ;
	logic_input_t m_BIQ;
};
#endif

NETLIB_OBJECT_DERIVED(7448_dip, 7448)
{
	NETLIB_CONSTRUCTOR_DERIVED(7448_dip, 7448)
	{
		register_subalias("1", m_sub.m_B);
		register_subalias("2", m_sub.m_C);
		register_subalias("3", m_LTQ);
		register_subalias("4", m_BIQ);
		register_subalias("5",m_sub.m_RBIQ);
		register_subalias("6", m_sub.m_D);
		register_subalias("7", m_sub.m_A);

		register_subalias("9",  m_sub.m_Q[4]); // e
		register_subalias("10", m_sub.m_Q[3]); // d
		register_subalias("11", m_sub.m_Q[2]); // c
		register_subalias("12", m_sub.m_Q[1]); // b
		register_subalias("13", m_sub.m_Q[0]); // a
		register_subalias("14", m_sub.m_Q[6]); // g
		register_subalias("15", m_sub.m_Q[5]); // f
	}
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7448_H_ */
