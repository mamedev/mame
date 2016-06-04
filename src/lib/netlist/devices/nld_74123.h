// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74123.h
 *
 *  74123: Dual Retriggerable One-Shot with Clear and Complementary Outputs
 *
 *           +--------------+
 *        A1 |1     ++    16| VCC
 *        B1 |2           15| RC1
 *      CLR1 |3           14| C1
 *       Q1Q |4   74123   13| Q1
 *        Q2 |5           12| Q2Q
 *        C2 |6           11| CLR2
 *       RC2 |7           10| B2
 *       GND |8            9| A2
 *           +--------------+
 *
 *  Naming conventions follow Fairchild Semiconductor datasheet
 *
 *  DM9602: Dual Retriggerable, Resettable One Shots
 *
 *           +--------------+
 *        C1 |1     ++    16| VCC
 *       RC1 |2           15| C2
 *      CLR1 |3           14| RC2
 *        B1 |4    9602   13| CLR2
 *        A1 |5           12| B2
 *        Q1 |6           11| A2
 *       Q1Q |7           10| Q2
 *       GND |8            9| Q2Q
 *           +--------------+
 *
 *  CD4538: Dual Retriggerable, Resettable One Shots
 *
 *           +--------------+
 *        C1 |1     ++    16| VCC
 *       RC1 |2           15| C2
 *      CLR1 |3           14| RC2
 *        A1 |4    4538   13| CLR2
 *        B1 |5           12| A2
 *        Q1 |6           11| B2
 *       Q1Q |7           10| Q2
 *       GND |8            9| Q2Q
 *           +--------------+
 *
 */

#ifndef NLD_74123_H_
#define NLD_74123_H_

#include "nl_base.h"
#include "nld_system.h"
#include "analog/nld_twoterm.h"

#define TTL_74123(_name)                                                        \
		NET_REGISTER_DEV(TTL_74123, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(74123)
{
	NETLIB_CONSTRUCTOR_EX(74123, int dev_type = 74123)
	, m_dev_type(dev_type)
	, m_RP(*this, "RP")
	, m_RN(*this, "RN")
	{
		if ((m_dev_type != 9602) && (m_dev_type != 4538) )
			m_dev_type = 74123;

		enregister("A", m_A);
		enregister("B", m_B);
		enregister("CLRQ", m_CLRQ);
		enregister("Q", m_Q);
		enregister("QQ", m_QQ);

		enregister("_RP_Q", m_RP_Q); // internal
		enregister("_RN_Q", m_RN_Q); // internal

		enregister("_CV", m_CV); // internal

		register_subalias("GND", m_RN.m_R.m_N);
		register_subalias("VCC", m_RP.m_R.m_P);
		register_subalias("C",   m_RN.m_R.m_N);
		register_subalias("RC",  m_RN.m_R.m_P);

		if (m_dev_type == 4538)
			register_param("K", m_K, 0.4);
		else
			register_param("K", m_K, 0.4);

		register_param("RI", m_RI, 400.0); // around 250 for HC series, 400 on LS/TTL, estimated from datasheets

		connect_late(m_RP_Q, m_RP.m_I);
		connect_late(m_RN_Q, m_RN.m_I);

		connect_late(m_RN.m_R.m_P, m_RP.m_R.m_N);
		connect_late(m_CV, m_RN.m_R.m_P);

		save(NLNAME(m_last_trig));
		save(NLNAME(m_state));
		save(NLNAME(m_KP));
	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();

private:
	int m_dev_type;
public:
	NETLIB_SUB(res_sw) m_RP;
	NETLIB_SUB(res_sw) m_RN;

	logic_output_t m_RP_Q;
	logic_output_t m_RN_Q;

	logic_input_t m_A;
	logic_input_t m_B;
	logic_input_t m_CLRQ;
	logic_output_t m_Q;
	logic_output_t m_QQ;

	analog_input_t m_CV;

	netlist_sig_t m_last_trig;
	UINT8         m_state;
	double        m_KP;

	param_double_t m_K;
	param_double_t m_RI;
};

#define TTL_74123_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_74123_DIP, _name)

NETLIB_OBJECT(74123_dip)
{
	NETLIB_CONSTRUCTOR(74123_dip)
	, m_1(*this, "1", 74123)
	, m_2(*this, "2", 74123)
	{
		register_subalias("1", m_1.m_A);
		register_subalias("2", m_1.m_B);
		register_subalias("3", m_1.m_CLRQ);
		register_subalias("4", m_1.m_QQ);
		register_subalias("5", m_2.m_Q);
		register_subalias("6", m_2.m_RN.m_R.m_N);
		register_subalias("7", m_2.m_RN.m_R.m_P);
		register_subalias("8", m_1.m_RN.m_R.m_N);
		connect_late(m_1.m_RN.m_R.m_N, m_2.m_RN.m_R.m_N);

		register_subalias("9", m_2.m_A);
		register_subalias("10", m_2.m_B);
		register_subalias("11", m_2.m_CLRQ);
		register_subalias("12", m_2.m_QQ);
		register_subalias("13", m_1.m_Q);
		register_subalias("14", m_1.m_RN.m_R.m_N);
		register_subalias("15", m_1.m_RN.m_R.m_P);
		register_subalias("16", m_1.m_RP.m_R.m_P);
		connect_late(m_1.m_RP.m_R.m_P, m_2.m_RP.m_R.m_P);
	}
	NETLIB_RESETI();
	NETLIB_UPDATEI();
private:
	NETLIB_SUB(74123) m_1;
	NETLIB_SUB(74123) m_2;
};

/* The 9602 is very similar to the 123. Input triggering is slightly different
 * THe 9602 uses an OR gate instead of an AND gate.
 */

#define TTL_9602_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_9602_DIP, _name)

NETLIB_OBJECT(9602_dip)
{
	NETLIB_CONSTRUCTOR(9602_dip)
	, m_1(*this, "1", 9602)
	, m_2(*this, "2", 9602)
	{
		register_subalias("1", m_1.m_RN.m_R.m_N); // C1
		register_subalias("2", m_1.m_RN.m_R.m_P); // RC1
		register_subalias("3", m_1.m_CLRQ);
		register_subalias("4", m_1.m_B);
		register_subalias("5", m_1.m_A);
		register_subalias("6", m_1.m_Q);
		register_subalias("7", m_1.m_QQ);
		register_subalias("8", m_1.m_RN.m_R.m_N);
		connect_late(m_1.m_RN.m_R.m_N, m_2.m_RN.m_R.m_N);

		register_subalias("9", m_2.m_QQ);
		register_subalias("10", m_2.m_Q);
		register_subalias("11", m_2.m_A);
		register_subalias("12", m_2.m_B);
		register_subalias("13", m_2.m_CLRQ);
		register_subalias("14", m_2.m_RN.m_R.m_P); // RC2
		register_subalias("15", m_2.m_RN.m_R.m_N); // C2
		register_subalias("16", m_1.m_RP.m_R.m_P);
		connect_late(m_1.m_RP.m_R.m_P, m_2.m_RP.m_R.m_P);
	}
	NETLIB_RESETI();
	NETLIB_UPDATEI();
private:
	NETLIB_SUB(74123) m_1;
	NETLIB_SUB(74123) m_2;
};

/*
 * The CD4538 is pretty similar to the 9602
 */

#define CD4538_DIP(_name)                                                         \
		NET_REGISTER_DEV(CD4538_DIP, _name)

NETLIB_OBJECT(4538_dip)
{
	NETLIB_CONSTRUCTOR(4538_dip)
	NETLIB_FAMILY("CD4XXX")
	, m_1(*this, "1", 4538)
	, m_2(*this, "2", 4538)
	{
		register_subalias("1", m_1.m_RN.m_R.m_N); // C1
		register_subalias("2", m_1.m_RN.m_R.m_P); // RC1
		register_subalias("3", m_1.m_CLRQ);
		register_subalias("4", m_1.m_A);
		register_subalias("5", m_1.m_B);
		register_subalias("6", m_1.m_Q);
		register_subalias("7", m_1.m_QQ);
		register_subalias("8", m_1.m_RN.m_R.m_N);
		connect_late(m_1.m_RN.m_R.m_N, m_2.m_RN.m_R.m_N);

		register_subalias("9", m_2.m_QQ);
		register_subalias("10", m_2.m_Q);
		register_subalias("11", m_2.m_B);
		register_subalias("12", m_2.m_A);
		register_subalias("13", m_2.m_CLRQ);
		register_subalias("14", m_2.m_RN.m_R.m_P); // RC2
		register_subalias("15", m_2.m_RN.m_R.m_N); // C2
		register_subalias("16", m_1.m_RP.m_R.m_P);
		connect_late(m_1.m_RP.m_R.m_P, m_2.m_RP.m_R.m_P);
	}
	NETLIB_RESETI();
	NETLIB_UPDATEI();
private:
	NETLIB_SUB(74123) m_1;
	NETLIB_SUB(74123) m_2;
};

NETLIB_NAMESPACE_DEVICES_END()


#endif /* NLD_74123_H_ */
