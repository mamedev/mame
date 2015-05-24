// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_log.c
 *
 */

#include <cstdio>

#include "nld_log.h"
//#include "sound/wavwrite.h"

//FIXME: what to do with save states?

NETLIB_START(log)
{
	register_input("I", m_I);

	pstring filename = pstring::sprintf("%s.log", name().cstr());
	m_file = fopen(filename, "w");
}

NETLIB_RESET(log)
{
}

NETLIB_UPDATE(log)
{
	std::fprintf(static_cast<FILE *>(m_file), "%20.9e %e\n", netlist().time().as_double(), (nl_double) INPANALOG(m_I));
}

NETLIB_NAME(log)::~NETLIB_NAME(log)()
{
	std::fclose(static_cast<FILE *>(m_file));
}

NETLIB_START(logD)
{
	NETLIB_NAME(log)::start();
	register_input("I2", m_I2);
}

NETLIB_RESET(logD)
{
}

NETLIB_UPDATE(logD)
{
	std::fprintf(static_cast<FILE *>(m_file), "%e %e\n", netlist().time().as_double(), (nl_double) (INPANALOG(m_I) - INPANALOG(m_I2)));
}

// FIXME: Implement wav later, this must be clock triggered device where the input to be written
//        is on a subdevice ...
#if 0
NETLIB_START(wav)
{
	register_input("I", m_I);

	pstring filename = "netlist_" + name() + ".wav";
	m_file = wav_open(filename, sample_rate(), active_inputs()/2)
}

NETLIB_UPDATE(wav)
{
	fprintf(m_file, "%e %e\n", netlist().time().as_double(), INPANALOG(m_I));
}

NETLIB_NAME(log)::~NETLIB_NAME(wav)()
{
	fclose(m_file);
}
#endif
