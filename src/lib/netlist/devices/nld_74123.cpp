// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74123.c
 *
 */

#include "nld_74123.h"

#include "nlid_system.h"
#include "analog/nld_twoterm.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(74123)
	{
		NETLIB_CONSTRUCTOR_EX(74123, int dev_type = 74123)
		, m_dev_type(dev_type)
		, m_RP(*this, "RP")
		, m_RN(*this, "RN")
		, m_RP_Q(*this, "_RP_Q")
		, m_RN_Q(*this, "_RN_Q")
		, m_A(*this, "A")
		, m_B(*this, "B")
		, m_CLRQ(*this, "CLRQ")
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_CV(*this, "_CV") // internal
		, m_last_trig(*this, "m_last_trig", 0)
		, m_state(*this, "m_state", 0)
		, m_KP(*this, "m_KP", 0)
		, m_K(*this, "K", (m_dev_type == 4538) ? 0.4 : 0.4)
		, m_RI(*this, "RI", 400.0) // around 250 for HC series, 400 on LS/TTL, estimated from datasheets
		{
			if ((m_dev_type != 9602) && (m_dev_type != 4538) )
				m_dev_type = 74123;

			register_subalias("GND", m_RN.m_R.m_N);
			register_subalias("VCC", m_RP.m_R.m_P);
			register_subalias("C",   m_RN.m_R.m_N);
			register_subalias("RC",  m_RN.m_R.m_P);


			connect_late(m_RP_Q, m_RP.m_I);
			connect_late(m_RN_Q, m_RN.m_I);

			connect_late(m_RN.m_R.m_P, m_RP.m_R.m_N);
			connect_late(m_CV, m_RN.m_R.m_P);
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

		state_var<netlist_sig_t> m_last_trig;
		state_var<unsigned>      m_state;
		state_var<double>        m_KP;

		param_double_t m_K;
		param_double_t m_RI;
	};

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

	NETLIB_DEVICE_IMPL(74123)
	NETLIB_DEVICE_IMPL(74123_dip)
	NETLIB_DEVICE_IMPL(4538_dip)
	NETLIB_DEVICE_IMPL(9602_dip)

	} //namespace devices
} // namespace netlist
