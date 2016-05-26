// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4020.h
 *
 *  CD4020: 14-Stage Ripple Carry Binary Counters
 *
 *          +--------------+
 *      Q12 |1     ++    16| VDD
 *      Q13 |2           15| Q11
 *      Q14 |3           14| Q10
 *       Q6 |4    4020   13| Q8
 *       Q5 |5           12| Q9
 *       Q7 |6           11| RESET
 *       Q4 |7           10| IP (Input pulses)
 *      VSS |8            9| Q1
 *          +--------------+
 *
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 *  FIXME: Timing depends on VDD-VSS
 *         This needs a cmos d-a/a-d proxy implementation.
 *
 */

#ifndef NLD_4020_H_
#define NLD_4020_H_

#include "nl_base.h"
#include "nld_cmos.h"

/* FIXME: only used in mario.c */
#define CD4020_WI(name, cIP, cRESET, cVDD, cVSS)                              \
		NET_REGISTER_DEV(CD4020_WI, name)                                        \
		NET_CONNECT(name, IP, cIP)                                            \
		NET_CONNECT(name, RESET,  cRESET)                                     \
		NET_CONNECT(name, VDD,  cVDD)                                         \
		NET_CONNECT(name, VSS,  cVSS)

#define CD4020(name)                                                          \
		NET_REGISTER_DEV(CD4020, name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(CD4020_sub)
{
	NETLIB_CONSTRUCTOR(CD4020_sub)
	NETLIB_FAMILY("CD4XXX")
	, m_cnt(0)
	{
		enregister("IP", m_IP);

		enregister("Q1", m_Q[0]);
		enregister("Q4", m_Q[3]);
		enregister("Q5", m_Q[4]);
		enregister("Q6", m_Q[5]);
		enregister("Q7", m_Q[6]);
		enregister("Q8", m_Q[7]);
		enregister("Q9", m_Q[8]);
		enregister("Q10", m_Q[9]);
		enregister("Q11", m_Q[10]);
		enregister("Q12", m_Q[11]);
		enregister("Q13", m_Q[12]);
		enregister("Q14", m_Q[13]);

		save(NLNAME(m_cnt));
	}

	NETLIB_RESETI()
	{
		m_IP.set_state(logic_t::STATE_INP_HL);
		m_cnt = 0;
	}

	NETLIB_UPDATEI();

public:
	ATTR_HOT void update_outputs(const UINT16 cnt);

	logic_input_t m_IP;
	logic_output_t m_Q[14];

	UINT16 m_cnt;
};

NETLIB_OBJECT(CD4020)
{
	NETLIB_CONSTRUCTOR(CD4020)
	NETLIB_FAMILY("CD4XXX")
	, m_sub(*this, "sub")
	, m_supply(*this, "supply")
	{

		enregister("RESET", m_RESET);
		register_subalias("IP", m_sub.m_IP);
		register_subalias("Q1", m_sub.m_Q[0]);
		register_subalias("Q4", m_sub.m_Q[3]);
		register_subalias("Q5", m_sub.m_Q[4]);
		register_subalias("Q6", m_sub.m_Q[5]);
		register_subalias("Q7", m_sub.m_Q[6]);
		register_subalias("Q8", m_sub.m_Q[7]);
		register_subalias("Q9", m_sub.m_Q[8]);
		register_subalias("Q10", m_sub.m_Q[9]);
		register_subalias("Q11", m_sub.m_Q[10]);
		register_subalias("Q12", m_sub.m_Q[11]);
		register_subalias("Q13", m_sub.m_Q[12]);
		register_subalias("Q14", m_sub.m_Q[13]);
		register_subalias("VDD", m_supply.m_vdd);
		register_subalias("VSS", m_supply.m_vss);
	}
	NETLIB_RESETI() { }
	NETLIB_UPDATEI();

private:
	NETLIB_SUB(CD4020_sub) m_sub;
	NETLIB_SUB(vdd_vss) m_supply;
	logic_input_t m_RESET;
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_4020_H_ */
