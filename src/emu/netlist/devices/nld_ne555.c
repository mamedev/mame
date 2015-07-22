// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_NE555.c
 *
 */

#include <solver/nld_solver.h>
#include "nld_ne555.h"
#include "../nl_setup.h"

#define R_OFF (1E20)
#define R_ON (25)   // Datasheet states a maximum discharge of 200mA, R = 5V / 0.2

NETLIB_NAMESPACE_DEVICES_START()

inline nl_double NETLIB_NAME(NE555)::clamp(const nl_double v, const nl_double a, const nl_double b)
{
	nl_double ret = v;
	nl_double vcc = TERMANALOG(m_R1.m_P);

	if (ret >  vcc - a)
		ret = vcc - a;
	if (ret < b)
		ret = b;
	return ret;
}

NETLIB_START(NE555)
{
	register_sub("R1", m_R1);
	register_sub("R2", m_R2);
	register_sub("R3", m_R3);
	register_sub("RDIS", m_RDIS);

	register_subalias("GND",  m_R3.m_N);    // Pin 1
	register_input("TRIG",    m_TRIG);      // Pin 2
	register_output("OUT",    m_OUT);       // Pin 3
	register_input("RESET",   m_RESET);     // Pin 4
	register_subalias("CONT", m_R1.m_N);    // Pin 5
	register_input("THRESH",  m_THRES);     // Pin 6
	register_subalias("DISCH", m_RDIS.m_P); // Pin 7
	register_subalias("VCC",  m_R1.m_P);    // Pin 8

	connect_late(m_R1.m_N, m_R2.m_P);
	connect_late(m_R2.m_N, m_R3.m_P);
	connect_late(m_RDIS.m_N, m_R3.m_N);

	save(NLNAME(m_last_out));
	save(NLNAME(m_ff));
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

	m_last_out = true;
}

NETLIB_UPDATE(NE555)
{
	// FIXME: assumes GND is connected to 0V.

	nl_double vt = clamp(TERMANALOG(m_R2.m_P), 0.7, 1.4);
	bool bthresh = (INPANALOG(m_THRES) > vt);
	bool btrig = (INPANALOG(m_TRIG) > clamp(TERMANALOG(m_R2.m_N), 0.7, 1.4));

	if (!btrig)
	{
		m_ff = true;
	}
	else if (bthresh)
	{
		m_ff = false;
	}

	bool out = (!INPLOGIC(m_RESET) ? false : m_ff);

	if (m_last_out && !out)
	{
		m_RDIS.update_dev();
		OUTANALOG(m_OUT, TERMANALOG(m_R3.m_N));
		m_RDIS.set_R(R_ON);
	}
	else if (!m_last_out && out)
	{
		m_RDIS.update_dev();
		// FIXME: Should be delayed by 100ns
		OUTANALOG(m_OUT, TERMANALOG(m_R1.m_P));
		m_RDIS.set_R(R_OFF);
	}
	m_last_out = out;
}


NETLIB_START(NE555_dip)
{
	NETLIB_NAME(NE555)::start();

	register_subalias("1",  m_R3.m_N);      // Pin 1
	register_subalias("2",    m_TRIG);      // Pin 2
	register_subalias("3",    m_OUT);       // Pin 3
	register_subalias("4",   m_RESET);      // Pin 4
	register_subalias("5", m_R1.m_N);       // Pin 5
	register_subalias("6",  m_THRES);       // Pin 6
	register_subalias("7", m_RDIS.m_P);     // Pin 7
	register_subalias("8",  m_R1.m_P);      // Pin 8

}

NETLIB_UPDATE(NE555_dip)
{
	NETLIB_NAME(NE555)::update();
}

NETLIB_RESET(NE555_dip)
{
	NETLIB_NAME(NE555)::reset();
}

NETLIB_NAMESPACE_DEVICES_END()
