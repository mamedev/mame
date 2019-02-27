// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_log.c
 *
 */

#include "netlist/nl_base.h"
#include "nld_log.h"
#include "plib/pfmtlog.h"
#include "plib/pstream.h"
//#include "sound/wavwrite.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(log)
	{
		NETLIB_CONSTRUCTOR(log)
		, m_I(*this, "I")
		, m_strm(pstring(plib::pfmt("{1}.log")(this->name())))
		, m_writer(&m_strm)
		{
		}

		NETLIB_UPDATEI()
		{
			/* use pstring::sprintf, it is a LOT faster */
			m_writer.writeline(plib::pfmt("{1:.9} {2}").e(exec().time().as_double()).e(static_cast<double>(m_I())));
		}

		NETLIB_RESETI() { }
	protected:
		analog_input_t m_I;
		plib::pofilestream m_strm;
		plib::putf8_writer m_writer;
	};

	NETLIB_OBJECT_DERIVED(logD, log)
	{
		NETLIB_CONSTRUCTOR_DERIVED(logD, log)
		, m_I2(*this, "I2")
		{
		}

		NETLIB_UPDATEI()
		{
			m_writer.writeline(plib::pfmt("{1:.9} {2}").e(exec().time().as_double()).e(static_cast<double>(m_I() - m_I2())));
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
		fprintf(m_file, "%e %e\n", netlist().time().as_double(), m_I());
	}

	NETLIB_NAME(log)::~NETLIB_NAME(wav)()
	{
		fclose(m_file);
	}
	#endif


	NETLIB_DEVICE_IMPL(log,  "LOG",  "+I")
	NETLIB_DEVICE_IMPL(logD, "LOGD", "+I,+I2")

	} //namespace devices
} // namespace netlist
