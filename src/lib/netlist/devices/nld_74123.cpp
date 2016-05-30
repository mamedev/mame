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

NETLIB_UPDATE(74123_dip)
{
	/* only called during startup */
	//_1.update_dev();
	//m_2.update_dev();
}

NETLIB_RESET(74123_dip)
{
	//m_1.do_reset();
	//m_2.do_reset();
}

NETLIB_UPDATE(9602_dip)
{
	/* only called during startup */
	//m_1.update_dev();
	//m_2.update_dev();
}

NETLIB_RESET(9602_dip)
{
	//m_1.do_reset();
	//m_2.do_reset();
}

NETLIB_UPDATE(4538_dip)
{
	/* only called during startup */
	//m_1.update_dev();
	//m_2.update_dev();
}

NETLIB_RESET(4538_dip)
{
	m_1.do_reset();
	m_2.do_reset();
}

NETLIB_NAMESPACE_DEVICES_END()
