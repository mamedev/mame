// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_log.c
 *
 */

#include "nld_log.h"
//#include "sound/wavwrite.h"

namespace netlist
{
	namespace devices
	{

//FIXME: what to do with save states?

NETLIB_UPDATE(log)
{
	/* use pstring::sprintf, it is a LOT faster */
	m_strm->writeline(plib::pfmt("{1} {2}").e(netlist().time().as_double(),".9").e((nl_double) INPANALOG(m_I)));
}

NETLIB_NAME(log)::~NETLIB_NAME(log)()
{
	m_strm->close();
}

NETLIB_UPDATE(logD)
{
	m_strm->writeline(plib::pfmt("{1} {2}").e(netlist().time().as_double(),".9").e((nl_double) (INPANALOG(m_I) - INPANALOG(m_I2))));
}

// FIXME: Implement wav later, this must be clock triggered device where the input to be written
//        is on a subdevice ..
#if 0
NETLIB_START(wav)
{
	enregister("I", m_I);

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

	} //namespace devices
} // namespace netlist
