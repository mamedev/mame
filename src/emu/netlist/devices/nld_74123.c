// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74123.c
 *
 */

#include "nld_74123.h"

#define R_OFF (1E20)
#define R_ON (m_RI.Value())

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_START(74123)
{
	if ((m_dev_type != 9602) && (m_dev_type != 4538) )
		m_dev_type = 74123;

	register_sub("RP", m_RP);
	register_sub("RN", m_RN);

	register_input("A", m_A);
	register_input("B", m_B);
	register_input("CLRQ", m_CLRQ);
	register_output("Q", m_Q);
	register_output("QQ", m_QQ);

	register_output("_RP_Q", m_RP_Q); // internal
	register_output("_RN_Q", m_RN_Q); // internal

	register_input("_CV", m_CV); // internal

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

NETLIB_UPDATE(74123)
{
	netlist_sig_t m_trig;
	netlist_sig_t res = !INPLOGIC(m_CLRQ);
	netlist_time t_AB_to_Q = NLTIME_FROM_NS(10);
	netlist_time t_C_to_Q = NLTIME_FROM_NS(10);

	if (m_dev_type == 74123)
	{
		m_trig = (INPLOGIC(m_A) ^ 1) & INPLOGIC(m_B) & INPLOGIC(m_CLRQ);
	}
	else if (m_dev_type == 9602)
	{
		m_trig = (INPLOGIC(m_A) ^ 1) | INPLOGIC(m_B);
	}
	else // 4538
	{
		m_trig = (INPLOGIC(m_B) ^ 1) | INPLOGIC(m_A);
		// The line below is from the datasheet truthtable ... doesn't make sense at all
		//res = res | INPLOGIC(m_A) | (INPLOGIC(m_B) ^ 1);
		t_AB_to_Q = NLTIME_FROM_NS(300);
		t_C_to_Q = NLTIME_FROM_NS(250);
	}

	if (res)
	{
		OUTLOGIC(m_Q, 0, t_C_to_Q);
		OUTLOGIC(m_QQ, 1, t_C_to_Q);
		/* quick charge until trigger */
		/* FIXME: SGS datasheet shows quick charge to 5V,
		 * though schematics indicate quick charge to Vhigh only.
		 */
		OUTLOGIC(m_RP_Q, 1, t_C_to_Q); // R_ON
		OUTLOGIC(m_RN_Q, 0, t_C_to_Q); // R_OFF
		m_state = 2; //charging (quick)
	}
	else if (!m_last_trig && m_trig)
	{
		// FIXME: Timing!
		OUTLOGIC(m_Q, 1, t_AB_to_Q);
		OUTLOGIC(m_QQ, 0,t_AB_to_Q);

		OUTLOGIC(m_RN_Q, 1, t_AB_to_Q); // R_ON
		OUTLOGIC(m_RP_Q, 0, t_AB_to_Q); // R_OFF

		m_state = 1; // discharging
	}

	m_last_trig = m_trig;

	if (m_state == 1)
	{
		const nl_double vLow = m_KP * TERMANALOG(m_RP.m_R.m_P);
		if (INPANALOG(m_CV) < vLow)
		{
			OUTLOGIC(m_RN_Q, 0, NLTIME_FROM_NS(10)); // R_OFF
			m_state = 2; // charging
		}
	}
	if (m_state == 2)
	{
		const nl_double vHigh = TERMANALOG(m_RP.m_R.m_P) * (1.0 - m_KP);
		if (INPANALOG(m_CV) > vHigh)
		{
			OUTLOGIC(m_RP_Q, 0, NLTIME_FROM_NS(10)); // R_OFF

			OUTLOGIC(m_Q, 0, NLTIME_FROM_NS(10));
			OUTLOGIC(m_QQ, 1, NLTIME_FROM_NS(10));
			m_state = 0; // waiting
		}
	}
}

NETLIB_RESET(74123)
{
	m_KP = 1.0 / (1.0 + exp(m_K.Value()));

	m_RP.do_reset();
	m_RN.do_reset();

	//m_RP.set_R(R_OFF);
	//m_RN.set_R(R_OFF);

	m_last_trig = 0;
	m_state = 0;
}

NETLIB_START(74123_dip)
{
	register_sub("1", m_1);
	register_sub("2", m_2);

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

NETLIB_UPDATE(74123_dip)
{
	/* only called during startup */
	m_1.update_dev();
	m_2.update_dev();
}

NETLIB_RESET(74123_dip)
{
	m_1.do_reset();
	m_2.do_reset();
}

NETLIB_START(9602_dip)
{
	m_1.m_dev_type = 9602;
	m_2.m_dev_type = 9602;

	register_sub("1", m_1);
	register_sub("2", m_2);

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

NETLIB_UPDATE(9602_dip)
{
	/* only called during startup */
	m_1.update_dev();
	m_2.update_dev();
}

NETLIB_RESET(9602_dip)
{
	m_1.do_reset();
	m_2.do_reset();
}

NETLIB_START(4538_dip)
{
	m_1.m_dev_type = 4538;
	m_2.m_dev_type = 4538;

	register_sub("1", m_1);
	register_sub("2", m_2);

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

NETLIB_UPDATE(4538_dip)
{
	/* only called during startup */
	m_1.update_dev();
	m_2.update_dev();
}

NETLIB_RESET(4538_dip)
{
	m_1.do_reset();
	m_2.do_reset();
}

NETLIB_NAMESPACE_DEVICES_END()
