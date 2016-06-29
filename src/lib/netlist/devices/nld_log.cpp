// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_log.c
 *
 */

#include <memory>
#include "nl_base.h"
#include "plib/pstream.h"
#include "plib/pfmtlog.h"
#include "nld_log.h"
//#include "sound/wavwrite.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(log)
	{
		NETLIB_CONSTRUCTOR(log)
		, m_I(*this, "I")
		{
			pstring filename = plib::pfmt("{1}.log")(this->name());
			m_strm = plib::make_unique<plib::pofilestream>(filename);
		}

		NETLIB_UPDATEI()
		{
			/* use pstring::sprintf, it is a LOT faster */
			m_strm->writeline(plib::pfmt("{1} {2}").e(netlist().time().as_double(),".9").e((nl_double) INPANALOG(m_I)));
		}

		NETLIB_RESETI() { }
	protected:
		analog_input_t m_I;
		std::unique_ptr<plib::pofilestream> m_strm;
	};

	NETLIB_OBJECT_DERIVED(logD, log)
	{
		NETLIB_CONSTRUCTOR_DERIVED(logD, log)
		, m_I2(*this, "I2")
		{
		}

		NETLIB_UPDATEI()
		{
			m_strm->writeline(plib::pfmt("{1} {2}").e(netlist().time().as_double(),".9").e((nl_double) (INPANALOG(m_I) - INPANALOG(m_I2))));
		}

		NETLIB_RESETI() { }
		analog_input_t m_I2;
	};

	#if 0
	NETLIB_DEVICE(wav,
		~NETLIB_NAME(wav)();
		analog_input_t m_I;
	private:
		// FIXME: rewrite sound/wavwrite.h to be an object ...
		void *m_file;
	);
	#endif

	//FIXME: what to do with save states?


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


	NETLIB_DEVICE_IMPL(log)
	NETLIB_DEVICE_IMPL(logD)

	} //namespace devices
} // namespace netlist
