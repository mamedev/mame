/*
 * nld_legacy.c
 *
 */

#include "nld_legacy.h"

NETLIB_START(nicMultiSwitch)
{
	static const char *sIN[8] = { "i1", "i2", "i3", "i4", "i5", "i6", "i7", "i8" };
	int i;

	m_position = 0;
	m_low.initial(0);

	for (i=0; i<8; i++)
	{
		register_input(sIN[i], m_I[i]);
		m_low.net().register_con(m_I[i]);
		//m_I[i].set_net(m_low.m_net);
	}
	register_param("POS", m_POS, 0);
	register_output("Q", m_Q);

	save(NAME(m_position));

}

NETLIB_UPDATE(nicMultiSwitch)
{
	assert(m_position<8);
	OUTANALOG(m_Q, INPANALOG(m_I[m_position]), NLTIME_FROM_NS(1));
}

NETLIB_UPDATE_PARAM(nicMultiSwitch)
{
	m_position = m_POS.Value();
	//update();
}

NETLIB_START(nicMixer8)
{
	static const char *sI[8] = { "I1", "I2", "I3", "I4", "I5", "I6", "I7", "I8" };
	static const char *sR[8] = { "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8" };
	int i;

	m_low.initial(0);

	for (i=0; i<8; i++)
	{
		register_input(sI[i], m_I[i]);
		m_low.net().register_con(m_I[i]);
		//m_I[i].set_output(m_low);
		register_param(sR[i], m_R[i], 1e12);
	}
	register_output("Q", m_Q);
}

NETLIB_UPDATE(nicMixer8)
{
	int i;
	double r = 0;

	for (i=0; i<8; i++)
	{
		r += m_w[i] * INPANALOG(m_I[i]);
	}
	OUTANALOG(m_Q, r, NLTIME_IMMEDIATE);
}

NETLIB_UPDATE_PARAM(nicMixer8)
{
	double t = 0;
	int i;

	for (i=0; i<8; i++)
		t += 1.0 / m_R[i].Value();
	t = 1.0 / t;

	for (i=0; i<8; i++)
		m_w[i] = t / m_R[i].Value();
}



NETLIB_START(nicRSFF)
{
	register_input("S", m_S);
	register_input("R", m_R);
	register_output("Q", m_Q);
	register_output("QQ", m_QQ);
	m_Q.initial(0);
	m_QQ.initial(1);
}

NETLIB_UPDATE(nicRSFF)
{
	if (INPLOGIC(m_S))
	{
		OUTLOGIC(m_Q,  1, NLTIME_FROM_NS(10));
		OUTLOGIC(m_QQ, 0, NLTIME_FROM_NS(10));
	}
	else if (INPLOGIC(m_R))
	{
		OUTLOGIC(m_Q,  0, NLTIME_FROM_NS(10));
		OUTLOGIC(m_QQ, 1, NLTIME_FROM_NS(10));
	}
}


NETLIB_START(nicNE555N_MSTABLE)
{
	register_input("TRIG", m_trigger);
	register_input("CV", m_CV);

	register_output("Q", m_Q);
	register_param("R", m_R, 0.0);
	register_param("C", m_C, 0.0);
	register_param("VS", m_VS, 5.0);
	register_param("VL", m_VL, 0.0 *5.0);

	m_THRESHOLD_OUT.init_object(*this, name() + "THRESHOLD");
	register_link_internal(m_THRESHOLD, m_THRESHOLD_OUT, netlist_input_t::STATE_INP_ACTIVE);

	m_Q.initial(5.0 * 0.4);
	m_last = false;

	save(NAME(m_last));

}

inline double NETLIB_NAME(nicNE555N_MSTABLE)::nicNE555N_cv()
{
	return (m_CV.is_highz() ? 0.67 * m_VS.Value() : INPANALOG(m_CV));
}

inline double NETLIB_NAME(nicNE555N_MSTABLE)::nicNE555N_clamp(const double v, const double a, const double b)
{
	double ret = v;
	if (ret >  m_VS.Value() - a)
		ret = m_VS.Value() - a;
	if (ret < b)
		ret = b;
	return ret;
}

NETLIB_UPDATE_PARAM(nicNE555N_MSTABLE)
{
}

NETLIB_UPDATE(nicNE555N_MSTABLE)
{
	update_param(); // FIXME : m_CV should be on a sub device ...

	double vt = nicNE555N_clamp(nicNE555N_cv(), 0.7, 1.4);
	bool bthresh = (INPANALOG(m_THRESHOLD) > vt);
	bool btrig = (INPANALOG(m_trigger) > nicNE555N_clamp(nicNE555N_cv() * 0.5, 0.7, 1.4));
	bool out = m_last;

	if (!btrig)
	{
		out = true;
	}
	else if (bthresh)
	{
		out = false;
	}

	if (!m_last && out)
	{
		double vl = m_VL.Value();
		double time;

		// FIXME : m_CV should be on a sub device ...

		// TI datasheet states minimum pulse of 10 us
		if (vt<vl)
			time = 10;
		else
		{
			time = - log((m_VS.Value()-vt)/(m_VS.Value()-vl)) * m_R.Value() * m_C.Value() * 1.0e6; // in us
			if (time < 10.0)
				time = 10.0;
		}

		OUTANALOG(m_Q, m_VS.Value() * 0.7, NLTIME_FROM_NS(100));
		OUTANALOG(m_THRESHOLD_OUT, m_VS.Value(), NLTIME_FROM_US(time ));
	}
	else if (m_last && !out)
	{
		OUTANALOG(m_Q, 0.25, NLTIME_FROM_NS(100));
		OUTANALOG(m_THRESHOLD_OUT, 0.0, NLTIME_FROM_NS(1));
	}
	m_last = out;
}
