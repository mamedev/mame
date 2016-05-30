// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74192.h
 *
 *  DM74192: Synchronous 4-Bit Binary Counter with Dual Clock
 *           Decade counter
 *
 *  FIXME: This should be merged with the 74193 which counts to 16
 *
 *          +--------------+
 *        B |1     ++    16| VCC
 *       QB |2           15| A
 *       QA |3           14| CLEAR
 *       CD |4    74192  13| BORROWQ
 *       CU |5           12| CARRYQ
 *       QC |6           11| LOADQ
 *       QD |7           10| C
 *      GND |8            9| D
 *          +--------------+
 *
 * CD: Count up
 * CU: Count down
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_74192_H_
#define NLD_74192_H_

#include "nl_base.h"
#include "nld_9316.h"

#define TTL_74192(name)                                              \
		NET_REGISTER_DEV(TTL_74192, name)

#define TTL_74192_DIP(name)                                                         \
		NET_REGISTER_DEV(TTL_74192_DIP, name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(74192)
{
	NETLIB_CONSTRUCTOR(74192)
	, m_ABCD(*this, "subABCD")
	{
		register_subalias("A", m_ABCD.m_A);
		register_subalias("B", m_ABCD.m_B);
		register_subalias("C", m_ABCD.m_C);
		register_subalias("D", m_ABCD.m_D);
		enregister("CLEAR",  m_CLEAR);
		enregister("LOADQ",  m_LOADQ);
		enregister("CU", m_CU);
		enregister("CD", m_CD);

		enregister("QA", m_Q[0]);
		enregister("QB", m_Q[1]);
		enregister("QC", m_Q[2]);
		enregister("QD", m_Q[3]);
		enregister("BORROWQ", m_BORROWQ);
		enregister("CARRYQ", m_CARRYQ);

		save(NLNAME(m_cnt));
		save(NLNAME(m_last_CU));
		save(NLNAME(m_last_CD));
	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();

protected:
	NETLIB_SUB(9316_subABCD) m_ABCD;
	logic_input_t m_CLEAR;
	logic_input_t m_LOADQ;
	logic_input_t m_CU;
	logic_input_t m_CD;

	INT8 m_cnt;
	UINT8 m_last_CU;
	UINT8 m_last_CD;

	logic_output_t m_Q[4];
	logic_output_t m_BORROWQ;
	logic_output_t m_CARRYQ;
};

NETLIB_OBJECT_DERIVED(74192_dip, 74192)
{
	NETLIB_CONSTRUCTOR_DERIVED(74192_dip, 74192)
	{
		register_subalias("1", m_ABCD.m_B);
		register_subalias("2", m_Q[1]);
		register_subalias("3", m_Q[0]);
		register_subalias("4", m_CD);
		register_subalias("5", m_CU);
		register_subalias("6", m_Q[2]);
		register_subalias("7", m_Q[3]);

		register_subalias("9", m_ABCD.m_D);
		register_subalias("10", m_ABCD.m_C);
		register_subalias("11", m_LOADQ);
		register_subalias("12", m_CARRYQ);
		register_subalias("13", m_BORROWQ);
		register_subalias("14", m_CLEAR);
		register_subalias("15", m_ABCD.m_A);
	}
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_74192_H_ */
