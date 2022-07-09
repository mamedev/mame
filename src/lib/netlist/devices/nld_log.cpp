// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_log.cpp
 *
 *  Devices supporting analysis and logging
 *
 *  nld_log:
 *
 *          +---------+
 *          |    ++   |
 *        I |         | ==> Log to file "netlist_" + name() + ".log"
 *          |         |
 *          +---------+
 *
 */

#include "nl_base.h"
#include "plib/pfmtlog.h"
#include "plib/pmulti_threading.h"
#include "plib/pstream.h"
//#include "sound/wavwrite.h"

#include <array>
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace netlist::devices {

	NETLIB_OBJECT(log)
	{
		NETLIB_CONSTRUCTOR(log)
		, m_I(*this, "I", NETLIB_DELEGATE(input))
		, m_strm(plib::filesystem::u8path(plib::pfmt("{1}.log")(this->name())))
		, m_writer(&m_strm)
		, m_reset(false)
		, m_done(false)
		, m_sem_w(0)
		, m_sem_r(0)
		, m_w(0)
		, m_r(0)
		{
			if (m_strm.fail())
				throw plib::file_open_e(plib::pfmt("{1}.log")(this->name()));

			m_strm.imbue(std::locale::classic());

			//m_write_thread = std::thread(std::bind(&nld_log::thread_writer, this));
			m_write_thread = std::thread([this]{this->thread_writer(); });
			m_sem_r.acquire();
		}

		PCOPYASSIGNMOVE(NETLIB_NAME(log), delete)

		NETLIB_DESTRUCTOR(log)
		{
			if (m_reset)
				log_value(m_I());
			m_sem_w.release();
			m_done = true;
			m_write_thread.join();
		}

		NETLIB_HANDLERI(input)
		{
			log_value(static_cast<nl_fptype>(m_I()));
		}

		void log_value(nl_fptype val)
		{
			if (m_buffers[m_w].size() == BUF_SIZE)
			{
				m_sem_w.release();
				m_sem_r.acquire();
				m_w++;
				if (m_w >= BUFFERS)
					m_w = 0;
			}
			m_buffers[m_w].push_back({exec().time(), val});
		}

		NETLIB_RESETI() { m_reset = true; }
	protected:

		void thread_writer()
		{
			m_sem_r.release(BUFFERS);
			while (true)
			{
				if (!m_sem_w.try_acquire())
				{
					if (m_done)
						break;
					m_sem_w.acquire();
				}
				auto &b = m_buffers[m_r];

				for (auto &e : b)
					/* use pstring::sprintf, it is a LOT faster */
					m_writer.write_line(plib::pfmt("{1:.9} {2}").e(e.t.as_fp<nl_fptype>()).e(e.v));
				b.clear();
				m_sem_r.release();
				m_r++;
				if (m_r >= BUFFERS)
					m_r = 0;
			}
		}

		struct entry
		{
			netlist_time_ext t;
			nl_fptype v;
		};
		static constexpr std::size_t BUF_SIZE=16384;
		static constexpr std::size_t BUFFERS=4;
		analog_input_t m_I;
	private:
		plib::ofstream m_strm;
		plib::putf8_writer m_writer;
		bool m_reset;
		std::array<std::vector<entry>, BUFFERS> m_buffers;
		std::atomic<bool> m_done;
		plib::psemaphore m_sem_w;
		plib::psemaphore m_sem_r;
		std::size_t m_w;
		std::size_t m_r;
		std::thread m_write_thread;
	};

	class NETLIB_NAME(logD) : public NETLIB_NAME(log)
	{
	public:
		NETLIB_NAME(logD)(constructor_param_t data)
		: NETLIB_NAME(log)(data)
		, m_I2(*this, "I2", nl_delegate(&NETLIB_NAME(logD)::input, this))
		{
			m_I.set_delegate(nl_delegate(&NETLIB_NAME(logD)::input, this));
		}

	private:
		NETLIB_HANDLERI(input)
		{
			log_value(static_cast<nl_fptype>(m_I() - m_I2()));
		}

		//NETLIB_RESETI() {}
		analog_input_t m_I2;
	};

	#if 0
	NETLIB_DEVICE(wav,
		~NETLIB_NAME(wav)();
		analog_input_t m_I;
	private:
		// FIXME: rewrite `sound/wavwrite.h` to be an object ...
		void *m_file;
	);
	#endif

	//FIXME: what to do with save states?


	// FIXME: Implement wav later, this must be clock triggered device where the input to be written
	//        is on a sub device ..
	#if 0
	NETLIB_START(wav)
	{
		enregister("I", m_I);

		pstring filename = "netlist_" + name() + ".wav";
		m_file = wav_open(filename, sample_rate(), active_inputs()/2)
	}

	NETLIB_UPDATE(wav)
	{
		fprintf(m_file, "%e %e\n", netlist().time().as_fp<nl_fptype>(), m_I());
	}

	NETLIB_NAME(log)::~NETLIB_NAME(wav)()
	{
		fclose(m_file);
	}
	#endif


	NETLIB_DEVICE_IMPL(log,  "LOG",  "+I")
	NETLIB_DEVICE_IMPL(logD, "LOGD", "+I,+I2")

} // namespace netlist::devices
