/*
 * nld_NE555.c
 *
 */

#include "nld_ne555.h"
#include "../nl_setup.h"

#define R_OFF (1E20)
#define R_ON (1)

inline double NETLIB_NAME(NE555)::clamp(const double v, const double a, const double b)
{
	double ret = v;
	double vcc = TERMANALOG(m_R1.m_P);

	if (ret >  vcc - a)
		ret = vcc - a;
	if (ret < b)
		ret = b;
	return ret;
}

NETLIB_START(NE555)
{
	register_sub(m_R1, "R1");
	register_sub(m_R2, "R2");
	register_sub(m_R3, "R3");
	register_sub(m_RDIS, "RDIS");

	register_subalias("GND",  m_R3.m_N);    // Pin 1
	register_input("TRIG",    m_TRIG);      // Pin 2
	register_output("OUT",    m_OUT);       // Pin 3
	register_input("RESET",   m_RESET);     // Pin 4
	register_subalias("CONT", m_R1.m_N);    // Pin 5
	register_input("THRESH",  m_THRES);     // Pin 6
	register_subalias("DISCH", m_RDIS.m_P); // Pin 7
	register_subalias("VCC",  m_R1.m_P);    // Pin 8

	connect(m_R1.m_N, m_R2.m_P);
	connect(m_R2.m_N, m_R3.m_P);
	connect(m_RDIS.m_N, m_R3.m_N);

	save(NAME(m_last_out));
}

NETLIB_RESET(NE555)
{
    m_R1.do_reset();
    m_R2.do_reset();
    m_R3.do_reset();
    m_RDIS.do_reset();

    m_R1.set_R(5000);
    m_R2.set_R(5000);
    m_R3.set_R(5000);
    m_RDIS.set_R(R_OFF);

    m_last_out = false;
}

NETLIB_UPDATE(NE555)
{
	// FIXME: assumes GND is connected to 0V.

	double vt = clamp(TERMANALOG(m_R2.m_P), 0.7, 1.4);
	bool bthresh = (INPANALOG(m_THRES) > vt);
	bool btrig = (INPANALOG(m_TRIG) > clamp(TERMANALOG(m_R2.m_N), 0.7, 1.4));
	bool out = m_last_out;

	if (!btrig)
	{
		out = true;
	}
	else if (bthresh)
	{
		out = false;
	}

	if (!m_last_out && out)
	{
		OUTANALOG(m_OUT, TERMANALOG(m_R1.m_P), NLTIME_FROM_NS(100));
		m_RDIS.set_R(R_OFF);
	}
	else if (m_last_out && !out)
	{
		OUTANALOG(m_OUT, TERMANALOG(m_R3.m_N), NLTIME_FROM_NS(100));
		m_RDIS.set_R(R_ON);
	}
	m_last_out = out;
}
