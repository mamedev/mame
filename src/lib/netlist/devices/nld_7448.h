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

#define TTL_7448(_name, _A0, _A1, _A2, _A3, _LTQ, _BIQ, _RBIQ)                      \
		NET_REGISTER_DEV(TTL_7448, _name)                                               \
		NET_CONNECT(_name, A, _A0)                                                  \
		NET_CONNECT(_name, B, _A1)                                                  \
		NET_CONNECT(_name, C, _A2)                                                  \
		NET_CONNECT(_name, D, _A3)                                                  \
		NET_CONNECT(_name, LTQ, _LTQ)                                               \
		NET_CONNECT(_name, BIQ, _BIQ)                                               \
		NET_CONNECT(_name, RBIQ, _RBIQ)

#define TTL_7448_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7448_DIP, _name)

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

NETLIB_DEVICE_DERIVED_PURE(7448_dip, 7448);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7448_H_ */
