// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9310.h
 *
 *  DM9310: Synchronous 4-Bit Counters
 *
 *  FIXME: This should be merged with the 9316. The only difference is MAXCNT!
 *
 *          +--------------+
 *    CLEAR |1     ++    16| VCC
 *    CLOCK |2           15| RC (Ripple Carry)
 *        A |3           14| QA
 *        B |4    9310   13| QB
 *        C |5           12| QC
 *        D |6           11| QD
 * Enable P |7           10| Enable T
 *      GND |8            9| LOAD
 *          +--------------+
 *
 *          Counter Sequence
 *
 *          +-------++----+----+----+----+----+
 *          | COUNT || QD | QC | QB | QA | RC |
 *          +=======++====+====+====+====+====+
 *          |    0  ||  0 |  0 |  0 |  0 |  0 |
 *          |    1  ||  0 |  0 |  0 |  1 |  0 |
 *          |    2  ||  0 |  0 |  1 |  0 |  0 |
 *          |    3  ||  0 |  0 |  1 |  1 |  0 |
 *          |    4  ||  0 |  1 |  0 |  0 |  0 |
 *          |    5  ||  0 |  1 |  0 |  1 |  0 |
 *          |    6  ||  0 |  1 |  1 |  0 |  0 |
 *          |    7  ||  0 |  1 |  1 |  1 |  0 |
 *          |    8  ||  1 |  0 |  0 |  0 |  0 |
 *          |    9  ||  1 |  0 |  0 |  1 |  0 |
 *          +-------++----+----+----+----+----+
 *
 *          Reset count function: Please refer to
 *          National Semiconductor datasheet (timing diagramm)
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_9310_H_
#define NLD_9310_H_

#include "nl_base.h"

#define TTL_9310(name, cCLK, cENP, cENT, cCLRQ, cLOADQ, cA, cB, cC, cD)            \
		NET_REGISTER_DEV(TTL_9310, name)                                               \
		NET_CONNECT(name, CLK, cCLK)                                               \
		NET_CONNECT(name, ENP,  cENP)                                              \
		NET_CONNECT(name, ENT,  cENT)                                              \
		NET_CONNECT(name, CLRQ, cCLRQ)                                             \
		NET_CONNECT(name, LOADQ,_LOADQ)                                            \
		NET_CONNECT(name, A,    cA)                                                \
		NET_CONNECT(name, B,    cB)                                                \
		NET_CONNECT(name, C,    cC)                                                \
		NET_CONNECT(name, D,    cD)

#define TTL_9310_DIP(name)                                                         \
		NET_REGISTER_DEV(TTL_9310_DIP, name)

namespace netlist
{
	namespace devices
	{

NETLIB_OBJECT(9310_subABCD)
{
	NETLIB_CONSTRUCTOR(9310_subABCD)
	{
		enregister("A", m_A);
		enregister("B", m_B);
		enregister("C", m_C);
		enregister("D", m_D);
	}

	NETLIB_RESETI();
	//NETLIB_UPDATEI();

public:
	logic_input_t m_A;
	logic_input_t m_B;
	logic_input_t m_C;
	logic_input_t m_D;

	ATTR_HOT inline UINT8 read_ABCD() const
	{
		//return (INPLOGIC_PASSIVE(m_D) << 3) | (INPLOGIC_PASSIVE(m_C) << 2) | (INPLOGIC_PASSIVE(m_B) << 1) | (INPLOGIC_PASSIVE(m_A) << 0);
		return (INPLOGIC(m_D) << 3) | (INPLOGIC(m_C) << 2) | (INPLOGIC(m_B) << 1) | (INPLOGIC(m_A) << 0);
	}
};

NETLIB_OBJECT(9310_sub)
{
	NETLIB_CONSTRUCTOR(9310_sub)
	, m_cnt(0)
	, m_ABCD(nullptr)
	, m_loadq(0)
	, m_ent(0)
	{
		enregister("CLK", m_CLK);

		enregister("QA", m_QA);
		enregister("QB", m_QB);
		enregister("QC", m_QC);
		enregister("QD", m_QD);
		enregister("RC", m_RC);

		save(NLNAME(m_cnt));
		save(NLNAME(m_loadq));
		save(NLNAME(m_ent));
	}
	NETLIB_RESETI();
	NETLIB_UPDATEI();
public:
	ATTR_HOT inline void update_outputs_all(const UINT8 cnt, const netlist_time out_delay);
	ATTR_HOT inline void update_outputs(const UINT8 cnt);

	logic_input_t m_CLK;

	UINT8 m_cnt;
	NETLIB_NAME(9310_subABCD) *m_ABCD;
	netlist_sig_t m_loadq;
	netlist_sig_t m_ent;

	logic_output_t m_QA;
	logic_output_t m_QB;
	logic_output_t m_QC;
	logic_output_t m_QD;
	logic_output_t m_RC;
};

NETLIB_OBJECT(9310)
{
	NETLIB_CONSTRUCTOR(9310)
	, subABCD(*this, "subABCD")
	, sub(*this, "sub")
	{
		sub.m_ABCD = &(subABCD);

		register_subalias("CLK", sub.m_CLK);

		enregister("ENP", m_ENP);
		enregister("ENT", m_ENT);
		enregister("CLRQ", m_CLRQ);
		enregister("LOADQ", m_LOADQ);

		register_subalias("A", subABCD.m_A);
		register_subalias("B", subABCD.m_B);
		register_subalias("C", subABCD.m_C);
		register_subalias("D", subABCD.m_D);

		register_subalias("QA", sub.m_QA);
		register_subalias("QB", sub.m_QB);
		register_subalias("QC", sub.m_QC);
		register_subalias("QD", sub.m_QD);
		register_subalias("RC", sub.m_RC);
	}
	NETLIB_RESETI();
	NETLIB_UPDATEI();

public:
	NETLIB_SUB(9310_subABCD) subABCD;
	NETLIB_SUB(9310_sub) sub;
	logic_input_t m_ENP;
	logic_input_t m_ENT;
	logic_input_t m_CLRQ;
	logic_input_t m_LOADQ;
};

NETLIB_OBJECT_DERIVED(9310_dip, 9310)
{
	NETLIB_CONSTRUCTOR_DERIVED(9310_dip, 9310)
	{
		register_subalias("1", m_CLRQ);
		register_subalias("2", sub.m_CLK);
		register_subalias("3", subABCD.m_A);
		register_subalias("4", subABCD.m_B);
		register_subalias("5", subABCD.m_C);
		register_subalias("6", subABCD.m_D);
		register_subalias("7", m_ENP);
		// register_subalias("8", ); -. GND

		register_subalias("9", m_LOADQ);
		register_subalias("10", m_ENT);
		register_subalias("11", sub.m_QD);
		register_subalias("12", sub.m_QC);
		register_subalias("13", sub.m_QB);
		register_subalias("14", sub.m_QA);
		register_subalias("15", sub.m_RC);
		// register_subalias("16", ); -. VCC
	}
};
	} //namespace devices
} // namespace netlist

#endif /* NLD_9310_H_ */
