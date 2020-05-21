// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    netlist.c

    Discrete netlist implementation.

****************************************************************************/

#include "emu.h"
#include "netlist.h"

#include "netlist/nl_base.h"
#include "netlist/nl_setup.h"
#include "netlist/nl_factory.h"
#include "netlist/nl_parser.h"
#include "netlist/devices/net_lib.h"
#include "netlist/devices/nlid_system.h"

#include "netlist/plib/palloc.h"

#include "debugger.h"
#include "romload.h"
#include "emuopts.h"

#include <cmath>
#include <memory>
#include <string>
#include <utility>

// Workaround for return value optimization failure in some older versions of clang
#if defined(__APPLE__) && defined(__clang__) && __clang_major__ < 8
#define MOVE_UNIQUE_PTR(x) (std::move(x))
#else
#define MOVE_UNIQUE_PTR(x) (x)
#endif

DEFINE_DEVICE_TYPE(NETLIST_CORE,  netlist_mame_device,       "netlist_core",  "Netlist Core Device")
DEFINE_DEVICE_TYPE(NETLIST_CPU,   netlist_mame_cpu_device,   "netlist_cpu",   "Netlist CPU Device")
DEFINE_DEVICE_TYPE(NETLIST_SOUND, netlist_mame_sound_device, "netlist_sound", "Netlist Sound Device")

/* subdevices */

DEFINE_DEVICE_TYPE(NETLIST_ANALOG_INPUT,  netlist_mame_analog_input_device,  "nl_analog_in",  "Netlist Analog Input")
DEFINE_DEVICE_TYPE(NETLIST_INT_INPUT,     netlist_mame_int_input_device,     "nl_int_out",    "Netlist Integer Output")
DEFINE_DEVICE_TYPE(NETLIST_RAM_POINTER,   netlist_mame_ram_pointer_device,   "nl_ram_ptr",    "Netlist RAM Pointer")
DEFINE_DEVICE_TYPE(NETLIST_LOGIC_INPUT,   netlist_mame_logic_input_device,   "nl_logic_in",   "Netlist Logic Input")
DEFINE_DEVICE_TYPE(NETLIST_STREAM_INPUT,  netlist_mame_stream_input_device,  "nl_stream_in",  "Netlist Stream Input")

DEFINE_DEVICE_TYPE(NETLIST_LOGIC_OUTPUT,  netlist_mame_logic_output_device,  "nl_logic_out",  "Netlist Logic Output")
DEFINE_DEVICE_TYPE(NETLIST_ANALOG_OUTPUT, netlist_mame_analog_output_device, "nl_analog_out", "Netlist Analog Output")
DEFINE_DEVICE_TYPE(NETLIST_STREAM_OUTPUT, netlist_mame_stream_output_device, "nl_stream_out", "Netlist Stream Output")

// ----------------------------------------------------------------------------------------
// Special netlist extension devices  ....
// ----------------------------------------------------------------------------------------

extern plib::dynlib_static_sym nl_static_solver_syms[];

static netlist::netlist_time_ext nltime_from_attotime(attotime t)
{
	netlist::netlist_time_ext nlmtime = netlist::netlist_time_ext::from_sec(t.seconds());
	nlmtime += netlist::netlist_time_ext::from_raw(t.attoseconds() / (ATTOSECONDS_PER_SECOND / netlist::netlist_time_ext::resolution()));
	return nlmtime;
}

#if 0
static attotime attotime_from_nltime(netlist::netlist_time_ext t)
{
	return attotime(t.as_raw() / netlist::netlist_time_ext::resolution(),
		(t.as_raw() % netlist::netlist_time_ext::resolution()) * (ATTOSECONDS_PER_SECOND / netlist::netlist_time_ext::resolution()));
}
#endif

class netlist_mame_device::netlist_mame_callbacks_t : public netlist::callbacks_t
{
public:

	netlist_mame_callbacks_t(const netlist_mame_device &parent)
		: netlist::callbacks_t()
		, m_parent(parent)
	{
	}

protected:
	void vlog(const plib::plog_level &l, const pstring &ls) const noexcept override
	{
		switch (l)
		{
		case plib::plog_level::DEBUG:
			m_parent.logerror("netlist DEBUG: %s\n", ls.c_str());
			break;
		case plib::plog_level::VERBOSE:
			m_parent.logerror("netlist VERBOSE: %s\n", ls.c_str());
			break;
		case plib::plog_level::INFO:
			m_parent.logerror("netlist INFO: %s\n", ls.c_str());
			break;
		case plib::plog_level::WARNING:
			m_parent.logerror("netlist WARNING: %s\n", ls.c_str());
			break;
		case plib::plog_level::ERROR:
			m_parent.logerror("netlist ERROR: %s\n", ls.c_str());
			break;
		case plib::plog_level::FATAL:
			m_parent.logerror("netlist FATAL: %s\n", ls.c_str());
			break;
		}
	}

	plib::unique_ptr<plib::dynlib_base> static_solver_lib() const noexcept override
	{
		//return plib::make_unique<plib::dynlib_static>(nullptr);
		return plib::make_unique<plib::dynlib_static>(nl_static_solver_syms);
	}

private:
	const netlist_mame_device &m_parent;
};

class netlist_validate_callbacks_t : public netlist::callbacks_t
{
public:

	netlist_validate_callbacks_t()
		: netlist::callbacks_t()
	{
	}

protected:
	void vlog(const plib::plog_level &l, const pstring &ls) const noexcept override
	{
		switch (l)
		{
		case plib::plog_level::DEBUG:
			break;
		case plib::plog_level::VERBOSE:
			break;
		case plib::plog_level::INFO:
			osd_printf_verbose("netlist INFO: %s\n", ls);
			break;
		case plib::plog_level::WARNING:
			osd_printf_warning("netlist WARNING: %s\n", ls);
			break;
		case plib::plog_level::ERROR:
			osd_printf_error("netlist ERROR: %s\n", ls);
			break;
		case plib::plog_level::FATAL:
			osd_printf_error("netlist FATAL: %s\n", ls);
			break;
		}
	}

	plib::unique_ptr<plib::dynlib_base> static_solver_lib() const noexcept override
	{
		return plib::make_unique<plib::dynlib_static>(nullptr);
	}

private:
};


class netlist_mame_device::netlist_mame_t : public netlist::netlist_state_t
{
public:

	netlist_mame_t(netlist_mame_device &parent, const pstring &name)
		: netlist::netlist_state_t(name, plib::make_unique<netlist_mame_device::netlist_mame_callbacks_t>(parent))
		, m_parent(parent)
	{
	}

	netlist_mame_t(netlist_mame_device &parent, const pstring &name, plib::unique_ptr<netlist::callbacks_t> cbs)
		: netlist::netlist_state_t(name, std::move(cbs))
		, m_parent(parent)
	{
	}

	running_machine &machine() { return m_parent.machine(); }
	netlist_mame_device &parent() const { return m_parent; }

private:
	netlist_mame_device &m_parent;
};


namespace {

// ----------------------------------------------------------------------------------------
// analog_callback
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(analog_callback) : public netlist::device_t
{
public:
	NETLIB_NAME(analog_callback)(netlist::netlist_state_t &anetlist, const pstring &name)
		: device_t(anetlist, name)
		, m_in(*this, "IN")
		, m_cpu_device(nullptr)
		, m_last(*this, "m_last", 0)
	{
		auto *nl = dynamic_cast<netlist_mame_device::netlist_mame_t *>(&state());
		if (nl != nullptr)
			m_cpu_device = downcast<netlist_mame_cpu_device *>(&nl->parent());
	}

	void reset() override
	{
		m_last = 0.0;
	}

	void register_callback(netlist_mame_analog_output_device::output_delegate &&callback)
	{
		m_callback.reset(new netlist_mame_analog_output_device::output_delegate(std::move(callback)));
	}

	NETLIB_UPDATEI()
	{
		nl_fptype cur = m_in();

		// FIXME: make this a parameter
		// avoid calls due to noise
		if (plib::abs(cur - m_last) > 1e-6)
		{
			m_cpu_device->update_icount(exec().time());
			(*m_callback)(cur, m_cpu_device->local_time());
			m_cpu_device->check_mame_abort_slice();
			m_last = cur;
		}
	}

private:
	netlist::analog_input_t m_in;
	std::unique_ptr<netlist_mame_analog_output_device::output_delegate> m_callback; // TODO: change to std::optional for C++17
	netlist_mame_cpu_device *m_cpu_device;
	netlist::state_var<nl_fptype> m_last;
};

// ----------------------------------------------------------------------------------------
// logic_callback
//
// This device must be connected to a logic net. It has no power terminals
// and conversion with proxies will not work.
//
// Background: This is inserted later into the driver and should not modify
// the resulting analog representation of the netlist.
//
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(logic_callback) : public netlist::device_t
{
public:
	NETLIB_NAME(logic_callback)(netlist::netlist_state_t &anetlist, const pstring &name)
		: device_t(anetlist, name)
		, m_in(*this, "IN")
		, m_cpu_device(nullptr)
		, m_last(*this, "m_last", 0)
//      , m_supply(*this)
	{
		auto *nl = dynamic_cast<netlist_mame_device::netlist_mame_t *>(&state());
		if (nl != nullptr)
			m_cpu_device = downcast<netlist_mame_cpu_device *>(&nl->parent());
	}

	void reset() override
	{
		m_last = 0;
	}

	void register_callback(netlist_mame_logic_output_device::output_delegate &&callback)
	{
		m_callback.reset(new netlist_mame_logic_output_device::output_delegate(std::move(callback)));
	}

	NETLIB_UPDATEI()
	{
		netlist::netlist_sig_t cur = m_in();

		// FIXME: make this a parameter
		// avoid calls due to noise
		if (cur != m_last)
		{
			m_cpu_device->update_icount(exec().time());
			(*m_callback)(cur, m_cpu_device->local_time());
			m_cpu_device->check_mame_abort_slice();
			m_last = cur;
		}
	}

private:
	netlist::logic_input_t m_in;
	std::unique_ptr<netlist_mame_logic_output_device::output_delegate> m_callback; // TODO: change to std::optional for C++17
	netlist_mame_cpu_device *m_cpu_device;
	netlist::state_var<netlist::netlist_sig_t> m_last;
	//netlist::devices::NETLIB_NAME(power_pins) m_supply;
};


// ----------------------------------------------------------------------------------------
// Extensions to interface netlist with MAME code ....
// ----------------------------------------------------------------------------------------

/*! Specific exception if memregion is not available.
 *  The exception is thrown if the memregions are not available.
 *  This may be the case in device_validity_check and needs
 *  to be ignored.
 */
class memregion_not_set : public netlist::nl_exception
{
public:
	/*! Constructor.
	 *  Allows a descriptive text to be assed to the exception
	 */
	explicit memregion_not_set(const pstring &text //!< text to be passed
			)
	: netlist::nl_exception(text) { }

	template<typename... Args>
	explicit memregion_not_set(const pstring &fmt //!< format to be used
		, Args&&... args //!< arguments to be passed
		)
	: netlist::nl_exception(plib::pfmt(fmt)(std::forward<Args>(args)...)) { }
};

class netlist_source_memregion_t : public netlist::source_netlist_t
{
public:


	netlist_source_memregion_t(device_t &dev, pstring name)
	: netlist::source_netlist_t(), m_dev(dev), m_name(name)
	{
	}

	virtual plib::unique_ptr<std::istream> stream(const pstring &name) override;
private:
	device_t &m_dev;
	pstring m_name;
};

class netlist_data_memregions_t : public netlist::source_data_t
{
public:
	netlist_data_memregions_t(const device_t &dev);

	virtual plib::unique_ptr<std::istream> stream(const pstring &name) override;

private:
	const device_t &m_dev;
};


// ----------------------------------------------------------------------------------------
// memregion source support
// ----------------------------------------------------------------------------------------

plib::unique_ptr<std::istream> netlist_source_memregion_t::stream(const pstring &name)
{
	if (m_dev.has_running_machine())
	{
		memory_region *mem = m_dev.memregion(m_name.c_str());
		auto ret(plib::make_unique<std::istringstream>(pstring(reinterpret_cast<char *>(mem->base()), mem->bytes())));
		ret->imbue(std::locale::classic());
		return MOVE_UNIQUE_PTR(ret);
	}
	else
		throw memregion_not_set("memregion unavailable for {1} in source {2}", name, m_name);
		//return plib::unique_ptr<plib::pimemstream>(nullptr);
}

netlist_data_memregions_t::netlist_data_memregions_t(const device_t &dev)
	: netlist::source_data_t(), m_dev(dev)
{
}

static bool rom_exists(device_t &root, pstring name)
{
	// iterate, starting with the driver's ROMs and continuing with device ROMs
	for (device_t &device : device_iterator(root))
	{
		// scan the ROM entries for this device
		for (tiny_rom_entry const *romp = device.rom_region(); romp && !ROMENTRY_ISEND(romp); ++romp)
		{
			if (ROMENTRY_ISREGION(romp)) // if this is a region, check for rom
			{
				auto basetag(pstring(romp->name));
				if (name == pstring(":") + basetag)
					return true;
			}
		}
	}
	return false;
}

plib::unique_ptr<std::istream> netlist_data_memregions_t::stream(const pstring &name)
{
	//memory_region *mem = static_cast<netlist_mame_device::netlist_mame_t &>(setup().setup().exec()).parent().memregion(name.c_str());
	if (m_dev.has_running_machine())
	{
		memory_region *mem = m_dev.memregion(name.c_str());
		if (mem != nullptr)
		{
			auto ret(plib::make_unique<std::istringstream>(std::string(reinterpret_cast<char *>(mem->base()), mem->bytes()), std::ios_base::binary));
			ret->imbue(std::locale::classic());
			return MOVE_UNIQUE_PTR(ret);
		}
		else
			return plib::unique_ptr<std::istream>(nullptr);
	}
	else
	{
		/* validation */
		if (rom_exists(m_dev.mconfig().root_device(), pstring(m_dev.tag()) + ":" + name))
		{
			// Create an empty stream.
			auto ret(plib::make_unique<std::istringstream>(std::ios_base::binary));
			ret->imbue(std::locale::classic());
			return MOVE_UNIQUE_PTR(ret);
		}
		else
			return plib::unique_ptr<std::istream>(nullptr);
	}
}

} // anonymous namespace



// ----------------------------------------------------------------------------------------
// sound_out
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(sound_out) : public netlist::device_t
{
public:
	NETLIB_NAME(sound_out)(netlist::netlist_state_t &anetlist, const pstring &name)
		: netlist::device_t(anetlist, name)
		, m_channel(*this, "CHAN", 0)
		, m_mult(*this, "MULT", 1000.0)
		, m_offset(*this, "OFFSET", 0.0)
		, m_sample_time(netlist::netlist_time::from_hz(1))
		, m_in(*this, "IN")
		, m_cur(0.0)
		, m_last_buffer_time(*this, "m_last_buffer", netlist::netlist_time_ext::zero())
	{
	}

protected:

	void reset() override
	{
		m_cur = 0.0;
		m_last_buffer_time = netlist::netlist_time_ext::zero();
	}

	NETLIB_UPDATEI()
	{
		nl_fptype val = m_in() * m_mult() + m_offset();
		sound_update(exec().time());
		/* ignore spikes */
		if (plib::abs(val) < 32767.0)
			m_cur = val;
		else if (val > 0.0)
			m_cur = 32767.0;
		else
			m_cur = -32767.0;

	}

public:
	void buffer_reset(const netlist::netlist_time_ext &upto)
	{
		m_last_buffer_time = upto;
		m_buffer.clear();
	}

	void sound_update(const netlist::netlist_time_ext &upto)
	{
		int pos = (upto - m_last_buffer_time()) / m_sample_time;
		//if (pos > m_bufsize)
		//	throw emu_fatalerror("sound %s: pos %d exceeded bufsize %d\n", name().c_str(), pos, m_bufsize);
		while (m_buffer.size() < pos )
		{
			m_buffer.push_back(static_cast<stream_sample_t>(m_cur));
		}
	}

	void sound_update_fill(int samples, stream_sample_t *target)
	{
		if (samples < m_buffer.size())
			throw emu_fatalerror("sound %s: samples %d less bufsize %d\n", name().c_str(), samples, m_buffer.size());
		std::copy(m_buffer.begin(), m_buffer.end(), target);
		std::size_t pos = m_buffer.size();
		while (pos < samples )
		{
			target[pos++] = static_cast<stream_sample_t>(m_cur);
		}
	}


	netlist::param_int_t m_channel;
	netlist::param_fp_t m_mult;
	netlist::param_fp_t m_offset;
	std::vector<stream_sample_t> m_buffer;

	netlist::netlist_time m_sample_time;

private:
	netlist::analog_input_t m_in;
	double m_cur;
	netlist::state_var<netlist::netlist_time_ext> m_last_buffer_time;
};

// ----------------------------------------------------------------------------------------
// sound_in
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(sound_in) : public netlist::device_t
{
public:

	static const int MAX_INPUT_CHANNELS = 16;

	NETLIB_NAME(sound_in)(netlist::netlist_state_t &anetlist, const pstring &name)
	: netlist::device_t(anetlist, name)
	, m_sample_time(attotime::zero)
	, m_feedback(*this, "FB") // clock part
	, m_Q(*this, "Q")
	, m_pos(0)
	, m_samples(0)
	, m_num_channels(0)
	{
		connect(m_feedback, m_Q);

		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			m_channels[i].m_param_name = anetlist.make_object<netlist::param_str_t>(*this, plib::pfmt("CHAN{1}")(i), "");
			m_channels[i].m_param_mult = anetlist.make_object<netlist::param_fp_t>(*this, plib::pfmt("MULT{1}")(i), 1.0);
			m_channels[i].m_param_offset = anetlist.make_object<netlist::param_fp_t>(*this, plib::pfmt("OFFSET{1}")(i), 0.0);
		}
	}

protected:
	void reset() override
	{
		m_pos = 0;
		for (auto & elem : m_channels)
			elem.m_buffer = nullptr;
	}

	NETLIB_UPDATEI()
	{
		if (m_pos < m_samples)
		{
			for (int i=0; i<m_num_channels; i++)
			{
				if (m_channels[i].m_buffer == nullptr)
					break; // stop, called outside of stream_update
				const nl_fptype v = m_channels[i].m_buffer[m_pos];
				m_channels[i].m_param->set(v * (*m_channels[i].m_param_mult)() + (*m_channels[i].m_param_offset)());
			}
		}
		else
		{
			// FIXME: The logic has a rounding issue because time resolution divided
			//        by 48,000 is not a natural number. The fractional part
			//        adds up to one samples every 13 seconds for 100 ps resolution.
			//        Fixing this is possible but complicated and expensive.
		}
		m_pos++;

		m_Q.net().toggle_and_push_to_queue(nltime_from_attotime(m_sample_time));
	}

public:
	void resolve(attotime sample_time)
	{
		m_pos = 0;
		m_sample_time = sample_time;

		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			if ((*m_channels[i].m_param_name)() != pstring(""))
			{
				if (i != m_num_channels)
					state().log().fatal("sound input numbering has to be sequential!");
				m_num_channels++;
				m_channels[i].m_param = dynamic_cast<netlist::param_fp_t *>(
					&state().setup().find_param((*m_channels[i].m_param_name)()).param()
				);
			}
		}
	}

	template <typename S>
	void buffer_reset(attotime sample_time, int num_samples, S **inputs)
	{
		m_samples = num_samples;
		m_sample_time = sample_time;

		m_pos = 0;
		for (int i=0; i < m_num_channels; i++)
		{
			m_channels[i].m_buffer = inputs[i];
		}
	}

	struct channel
	{
		netlist::unique_pool_ptr<netlist::param_str_t> m_param_name;
		netlist::param_fp_t *m_param;
		stream_sample_t *m_buffer;
		netlist::unique_pool_ptr<netlist::param_fp_t> m_param_mult;
		netlist::unique_pool_ptr<netlist::param_fp_t> m_param_offset;
	};

	int num_channels() { return m_num_channels; }

private:
	channel m_channels[MAX_INPUT_CHANNELS];
	attotime m_sample_time;

	netlist::logic_input_t m_feedback;
	netlist::logic_output_t m_Q;

	int m_pos;
	int m_samples;
	int m_num_channels;
};


// netlib #defines this and it fights with logmacro.h
#undef LOG

#define LOG_GENERAL     (1U << 0)
#define LOG_DEV_CALLS   (1U << 1)
#define LOG_DEBUG       (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_DEV_CALLS | LOG_DEBUG)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGDEVCALLS(...) LOGMASKED(LOG_DEV_CALLS, __VA_ARGS__)
#define LOGDEBUG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


netlist::setup_t &netlist_mame_device::setup()
{
	if (!m_netlist)
		throw device_missing_dependencies();
	return m_netlist->setup();
}

void netlist_mame_device::register_memregion_source(netlist::nlparse_t &parser, device_t &dev, const char *name)
{
	parser.register_source<netlist_source_memregion_t>(dev, pstring(name));
}

void netlist_mame_analog_input_device::write(const double val)
{
	m_value_for_device_timer = val * m_mult + m_offset;
	if (m_value_for_device_timer != (*m_param)())
	{
		LOGDEBUG("write %s\n", this->tag());
		synchronize(0, 0, &m_value_for_device_timer);
}
}

void netlist_mame_analog_input_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	update_to_current_time();
	m_param->set(*((double *) ptr));
}

void netlist_mame_int_input_device::write(const uint32_t val)
{
	const uint32_t v = (val >> m_shift) & m_mask;
	if (v != (*m_param)())
	{
		LOGDEBUG("write %s\n", this->tag());
		synchronize(0, v);
}
}

void netlist_mame_logic_input_device::write(const uint32_t val)
{
	const uint32_t v = (val >> m_shift) & 1;
	if (v != (*m_param)())
	{
		LOGDEBUG("write %s\n", this->tag());
		synchronize(0, v);
}
}

void netlist_mame_int_input_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	update_to_current_time();
	m_param->set(param);
}

void netlist_mame_logic_input_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	update_to_current_time();
	m_param->set(param);
}

void netlist_mame_ram_pointer_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_data = (*m_param)();
}

device_memory_interface::space_config_vector netlist_mame_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void netlist_mame_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	if (entry.index() >= 0)
	{
		if (entry.index() & 1)
			str = string_format("%10.6f", *((double *)entry.dataptr()));
		else
			str = string_format("%d", *((netlist::netlist_sig_t *)entry.dataptr()));
	}
}


// ----------------------------------------------------------------------------------------
// netlist_mame_analog_input_device
// ----------------------------------------------------------------------------------------

void netlist_mame_sub_interface::set_mult_offset(const double mult, const double offset)
{
	m_mult = mult;
	m_offset = offset;
}

netlist_mame_analog_input_device::netlist_mame_analog_input_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *param_name)
	: device_t(mconfig, NETLIST_ANALOG_INPUT, tag, owner, 0)
	, netlist_mame_sub_interface(*owner)
	, m_param(nullptr)
	, m_auto_port(true)
	, m_param_name(param_name)
	, m_value_for_device_timer(0)
{
}

netlist_mame_analog_input_device::netlist_mame_analog_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_ANALOG_INPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_param(nullptr)
	, m_auto_port(true)
	, m_param_name("")
	, m_value_for_device_timer(0)
{
}

void netlist_mame_analog_input_device::device_start()
{
	LOGDEVCALLS("start\n");
	netlist::param_ref_t p = this->nl_owner().setup().find_param(pstring(m_param_name));
	// FIXME: m_param should be param_ref_t
	m_param = dynamic_cast<netlist::param_fp_t *>(&p.param());
	if (m_param == nullptr)
	{
		fatalerror("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}
	if (m_mult != 1.0 || m_offset != 0.0)
	{
		// disable automatic scaling for ioports
		m_auto_port = false;
	}
}

void netlist_mame_analog_input_device::validity_helper(validity_checker &valid,
	netlist::netlist_state_t &nlstate) const
{
	netlist::param_ref_t p = nlstate.setup().find_param(pstring(m_param_name));
	auto *param = dynamic_cast<netlist::param_fp_t *>(&p.param());
	if (param == nullptr)
	{
		osd_printf_warning("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}
}

// ----------------------------------------------------------------------------------------
// netlist_mame_analog_output_device
// ----------------------------------------------------------------------------------------

netlist_mame_analog_output_device::netlist_mame_analog_output_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_ANALOG_OUTPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_in("")
	, m_delegate(*this)
{
}

void netlist_mame_analog_output_device::custom_netlist_additions(netlist::netlist_state_t &nlstate)
{
	const pstring pin(m_in);
	pstring dname = pstring("OUT_") + pin;

	/* ignore if no running machine -> called within device_validity_check context */
	if (owner()->has_running_machine())
		m_delegate.resolve();

	auto dev = nlstate.make_object<NETLIB_NAME(analog_callback)>(nlstate, dname);
	dev->register_callback(std::move(m_delegate));
	nlstate.register_device(dname, std::move(dev));
	nlstate.parser().register_link(dname + ".IN", pin);
}

void netlist_mame_analog_output_device::device_start()
{
	LOGDEVCALLS("start\n");
}


// ----------------------------------------------------------------------------------------
// netlist_mame_logic_output_device
// ----------------------------------------------------------------------------------------

netlist_mame_logic_output_device::netlist_mame_logic_output_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_LOGIC_OUTPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_in("")
	, m_delegate(*this)
{
}

void netlist_mame_logic_output_device::custom_netlist_additions(netlist::netlist_state_t &nlstate)
{
	pstring pin(m_in);
	pstring dname = "OUT_" + pin;

	/* ignore if no running machine -> called within device_validity_check context */
	if (owner()->has_running_machine())
		m_delegate.resolve();

	auto dev = nlstate.make_object<NETLIB_NAME(logic_callback)>(nlstate, dname);
	dev->register_callback(std::move(m_delegate));
	nlstate.register_device(dname, std::move(dev));
	nlstate.parser().register_link(dname + ".IN", pin);
}

void netlist_mame_logic_output_device::device_start()
{
	LOGDEVCALLS("start\n");
}


// ----------------------------------------------------------------------------------------
// netlist_mame_int_input_device
// ----------------------------------------------------------------------------------------

netlist_mame_int_input_device::netlist_mame_int_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_INT_INPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_param(nullptr)
	, m_mask(0xffffffff)
	, m_shift(0)
	, m_param_name("")
{
}

void netlist_mame_int_input_device::set_params(const char *param_name, const uint32_t mask, const uint32_t shift)
{
	LOGDEVCALLS("set_params\n");
	m_param_name = param_name;
	m_shift = shift;
	m_mask = mask;
}

void netlist_mame_int_input_device::device_start()
{
	LOGDEVCALLS("start\n");
	netlist::param_ref_t p = downcast<netlist_mame_device *>(this->owner())->setup().find_param(pstring(m_param_name));
	m_param = dynamic_cast<netlist::param_int_t *>(&p.param());
	if (m_param == nullptr)
	{
		fatalerror("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}
}

void netlist_mame_int_input_device::validity_helper(validity_checker &valid,
	netlist::netlist_state_t &nlstate) const
{
	netlist::param_ref_t p = nlstate.setup().find_param(pstring(m_param_name));
	auto *param = dynamic_cast<netlist::param_int_t *>(&p.param());
	if (param == nullptr)
	{
		osd_printf_warning("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}
}

// ----------------------------------------------------------------------------------------
// netlist_mame_logic_input_device
// ----------------------------------------------------------------------------------------

netlist_mame_logic_input_device::netlist_mame_logic_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_LOGIC_INPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_param(nullptr)
	, m_shift(0)
	, m_param_name("")
{
}

void netlist_mame_logic_input_device::set_params(const char *param_name, const uint32_t shift)
{
	LOGDEVCALLS("set_params\n");
	m_param_name = param_name;
	m_shift = shift;
}

void netlist_mame_logic_input_device::device_start()
{
	LOGDEVCALLS("start\n");
	netlist::param_ref_t p = downcast<netlist_mame_device *>(this->owner())->setup().find_param(pstring(m_param_name));
	m_param = dynamic_cast<netlist::param_logic_t *>(&p.param());
	if (m_param == nullptr)
	{
		fatalerror("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}
}

void netlist_mame_logic_input_device::validity_helper(validity_checker &valid,
	netlist::netlist_state_t &nlstate) const
{
	netlist::param_ref_t p = nlstate.setup().find_param(pstring(m_param_name));
	auto *param = dynamic_cast<netlist::param_logic_t *>(&p.param());
	if (param == nullptr)
	{
		osd_printf_warning("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}
}


// ----------------------------------------------------------------------------------------
// netlist_mame_ram_pointer_device
// ----------------------------------------------------------------------------------------

netlist_mame_ram_pointer_device::netlist_mame_ram_pointer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_RAM_POINTER, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_param(nullptr)
	, m_param_name("")
	, m_data(nullptr)
{
}

netlist_mame_ram_pointer_device::netlist_mame_ram_pointer_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *pname)
	: device_t(mconfig, NETLIST_RAM_POINTER, tag, owner, 0)
	, netlist_mame_sub_interface(*owner)
	, m_param(nullptr)
	, m_param_name(pname)
	, m_data(nullptr)
{
}

void netlist_mame_ram_pointer_device::set_params(const char *param_name)
{
	LOGDEVCALLS("set_params\n");
	m_param_name = param_name;
}

void netlist_mame_ram_pointer_device::device_start()
{
	LOGDEVCALLS("start\n");
	netlist::param_ref_t p = downcast<netlist_mame_device *>(this->owner())->setup().find_param(pstring(m_param_name));
	m_param = dynamic_cast<netlist::param_ptr_t *>(&p.param());
	if (m_param == nullptr)
	{
		fatalerror("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}

	m_data = (*m_param)();
}

void netlist_mame_ram_pointer_device::validity_helper(validity_checker &valid,
	netlist::netlist_state_t &nlstate) const
{
	netlist::param_ref_t p = nlstate.setup().find_param(pstring(m_param_name));
	auto *param = dynamic_cast<netlist::param_ptr_t *>(&p.param());
	if (param == nullptr)
	{
		osd_printf_warning("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}
}

// ----------------------------------------------------------------------------------------
// netlist_mame_stream_input_device
// ----------------------------------------------------------------------------------------

netlist_mame_stream_input_device::netlist_mame_stream_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_STREAM_INPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_channel(0)
	, m_param_name("")
{
}

void netlist_mame_stream_input_device::set_params(int channel, const char *param_name)
{
	m_param_name = param_name;
	m_channel = channel;
}

void netlist_mame_stream_input_device::device_start()
{
	LOGDEVCALLS("start\n");
}

void netlist_mame_stream_input_device::custom_netlist_additions(netlist::netlist_state_t &nlstate)
{
	if (!nlstate.parser().device_exists("STREAM_INPUT"))
		nlstate.parser().register_dev("NETDEV_SOUND_IN", "STREAM_INPUT");

	pstring sparam = plib::pfmt("STREAM_INPUT.CHAN{1}")(m_channel);
	nlstate.parser().register_param(sparam, pstring(m_param_name));
	sparam = plib::pfmt("STREAM_INPUT.MULT{1}")(m_channel);
	nlstate.parser().register_param_val(sparam, m_mult);
	sparam = plib::pfmt("STREAM_INPUT.OFFSET{1}")(m_channel);
	nlstate.parser().register_param_val(sparam, m_offset);
}

// ----------------------------------------------------------------------------------------
// netlist_mame_stream_output_device
// ----------------------------------------------------------------------------------------

netlist_mame_stream_output_device::netlist_mame_stream_output_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_STREAM_OUTPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_channel(0)
	, m_out_name("")
{
}

void netlist_mame_stream_output_device::set_params(int channel, const char *out_name)
{
	m_out_name = out_name;
	m_channel = channel;
}

void netlist_mame_stream_output_device::device_start()
{
	LOGDEVCALLS("start\n");
}

void netlist_mame_stream_output_device::custom_netlist_additions(netlist::netlist_state_t &nlstate)
{
	//NETLIB_NAME(sound_out) *snd_out;
	pstring sname = plib::pfmt("STREAM_OUT_{1}")(m_channel);

	//snd_out = dynamic_cast<NETLIB_NAME(sound_out) *>(setup.register_dev("nld_sound_out", sname));
	nlstate.parser().register_dev("NETDEV_SOUND_OUT", sname);

	nlstate.parser().register_param_val(sname + ".CHAN" , m_channel);
	nlstate.parser().register_param_val(sname + ".MULT",  m_mult);
	nlstate.parser().register_param_val(sname + ".OFFSET",  m_offset);
	nlstate.parser().register_link(sname + ".IN", pstring(m_out_name));
}


// ----------------------------------------------------------------------------------------
// netlist_mame_device
// ----------------------------------------------------------------------------------------

netlist_mame_device::netlist_mame_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: netlist_mame_device(mconfig, NETLIST_CORE, tag, owner, clock)
{
}

netlist_mame_device::netlist_mame_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_icount(0)
	, m_cur_time(attotime::zero)
	, m_attotime_per_clock(attotime::zero)
	, m_old(netlist::netlist_time_ext::zero())
	, m_setup_func(nullptr)
{
}

netlist_mame_device::~netlist_mame_device()
{
	LOGDEVCALLS("~netlist_mame_device\n");
}

void netlist_mame_device::device_config_complete()
{
	LOGDEVCALLS("device_config_complete %s\n", this->mconfig().gamedrv().name);

}

void netlist_mame_device::common_dev_start(netlist::netlist_state_t *lnetlist) const
{
	auto &lsetup = lnetlist->setup();

	// Override log statistics
	pstring p = plib::util::environment("NL_STATS", "");
	if (p != "")
	{
		bool err=false;
		bool v = plib::pstonum_ne<bool>(p, err);
		if (err)
			lsetup.log().warning("NL_STATS: invalid value {1}", p);
		else
			lnetlist->exec().enable_stats(v);
	}

	// register additional devices

	nl_register_devices(lsetup.parser());

	/* let sub-devices add sources and do stuff prior to parsing */
	for (device_t &d : subdevices())
	{
		netlist_mame_sub_interface *sdev = dynamic_cast<netlist_mame_sub_interface *>(&d);
		if( sdev != nullptr )
		{
			LOGDEVCALLS("Preparse subdevice %s/%s\n", d.name(), d.shortname());
			sdev->pre_parse_action(lsetup.parser());
		}
	}

	/* add default data provider for roms - if not in validity check*/
	//if (has_running_machine())
		lsetup.parser().register_source<netlist_data_memregions_t>(*this);

	m_setup_func(lsetup.parser());

	/* let sub-devices tweak the netlist */
	for (device_t &d : subdevices())
	{
		netlist_mame_sub_interface *sdev = dynamic_cast<netlist_mame_sub_interface *>(&d);
		if( sdev != nullptr )
		{
			LOGDEVCALLS("Found subdevice %s/%s\n", d.name(), d.shortname());
			sdev->custom_netlist_additions(*lnetlist);
		}
	}
}

plib::unique_ptr<netlist::netlist_state_t> netlist_mame_device::base_validity_check(validity_checker &valid) const
{
	try
	{
		auto lnetlist = plib::make_unique<netlist::netlist_state_t>("netlist", plib::make_unique<netlist_validate_callbacks_t>());
		// enable validation mode
		lnetlist->set_extended_validation(true);
		common_dev_start(lnetlist.get());
		lnetlist->setup().prepare_to_run();

		for (device_t &d : subdevices())
		{
			netlist_mame_sub_interface *sdev = dynamic_cast<netlist_mame_sub_interface *>(&d);
			if( sdev != nullptr )
			{
				LOGDEVCALLS("Validity check on subdevice %s/%s\n", d.name(), d.shortname());
				sdev->validity_helper(valid, *lnetlist);
			}
		}

		return lnetlist;
	}
	catch (memregion_not_set &err)
	{
		// Do not report an error. Validity check has no access to ROM area.
		osd_printf_verbose("%s\n", err.what());
	}
	catch (emu_fatalerror &err)
	{
		osd_printf_error("%s\n", err.what());
	}
	catch (std::exception &err)
	{
		osd_printf_error("%s\n", err.what());
	}
	return plib::unique_ptr<netlist::netlist_state_t>(nullptr);
}

void netlist_mame_device::device_validity_check(validity_checker &valid) const
{
	base_validity_check(valid);
	//rom_exists(mconfig().root_device());
	LOGDEVCALLS("device_validity_check %s\n", this->mconfig().gamedrv().name);
}


void netlist_mame_device::device_start()
{
	LOGDEVCALLS("device_start entry\n");

	m_attotime_per_clock = attotime(0, m_attoseconds_per_clock);

	//netlist().save(*this, m_cur_time, pstring(this->name()), "m_cur_time");
	save_item(NAME(m_cur_time));
	save_item(NAME(m_attotime_per_clock));

	m_netlist = std::make_unique<netlist_mame_t>(*this, "netlist");
	if (!machine().options().verbose())
	{
		m_netlist->log().verbose.set_enabled(false);
		m_netlist->log().debug.set_enabled(false);
	}

	common_dev_start(m_netlist.get());
	m_netlist->setup().prepare_to_run();

	m_netlist->save(*this, m_rem, pstring(this->name()), "m_rem");
	m_netlist->save(*this, m_div, pstring(this->name()), "m_div");
	m_netlist->save(*this, m_old, pstring(this->name()), "m_old");

	save_state();

	m_old = netlist::netlist_time_ext::zero();
	m_rem = netlist::netlist_time_ext::zero();
	LOGDEVCALLS("device_start exit\n");
}

void netlist_mame_device::device_clock_changed()
{
	m_div = static_cast<netlist::netlist_time_ext>(
		(netlist::netlist_time_ext::resolution() << MDIV_SHIFT) / clock());
	//printf("m_div %d\n", (int) m_div.as_raw());
	netlist().log().debug("Setting clock {1} and divisor {2}\n", clock(), m_div.as_double());
	m_attotime_per_clock = attotime(0, m_attoseconds_per_clock);
}


void netlist_mame_device::device_reset()
{
	LOGDEVCALLS("device_reset\n");
	m_cur_time = attotime::zero;
	m_old = netlist::netlist_time_ext::zero();
	m_rem = netlist::netlist_time_ext::zero();
	netlist().exec().reset();
}

void netlist_mame_device::device_stop()
{
	LOGDEVCALLS("device_stop\n");
	netlist().exec().stop();
}

void netlist_mame_device::device_post_load()
{
	LOGDEVCALLS("device_post_load\n");

	netlist().run_state_manager().post_load();
	netlist().rebuild_lists();
}

void netlist_mame_device::device_pre_save()
{
	LOGDEVCALLS("device_pre_save\n");

	netlist().run_state_manager().pre_save();
}

void netlist_mame_device::update_icount(netlist::netlist_time_ext time) noexcept
{
	const netlist::netlist_time_ext delta = (time - m_old).shl(MDIV_SHIFT) + m_rem;
	const uint64_t d = delta / m_div;
	m_old = time;
	m_rem = (delta - (m_div * d));
	//printf("d %d m_rem %d\n", (int) d, (int) m_rem.as_raw());
	m_icount -= d;
}

void netlist_mame_device::check_mame_abort_slice() noexcept
{
	if (m_icount <= 0)
		netlist().exec().abort_current_queue_slice();
}

void netlist_mame_device::save_state()
{
	for (auto const & s : netlist().run_state_manager().save_list())
	{
		netlist().log().debug("saving state for {1}\n", s->name().c_str());
		if (s->dt().is_float())
		{
			if (s->dt().size() == sizeof(double))
				save_pointer((double *) s->ptr(), s->name().c_str(), s->count());
			else if (s->dt().size() == sizeof(float))
				save_pointer((float *) s->ptr(), s->name().c_str(), s->count());
			else
				netlist().log().fatal("Unknown floating type for {1}\n", s->name().c_str());
		}
		else if (s->dt().is_integral())
		{
			if (s->dt().size() == sizeof(int64_t))
				save_pointer((int64_t *) s->ptr(), s->name().c_str(), s->count());
			else if (s->dt().size() == sizeof(int32_t))
				save_pointer((int32_t *) s->ptr(), s->name().c_str(), s->count());
			else if (s->dt().size() == sizeof(int16_t))
				save_pointer((int16_t *) s->ptr(), s->name().c_str(), s->count());
			else if (s->dt().size() == sizeof(int8_t))
				save_pointer((int8_t *) s->ptr(), s->name().c_str(), s->count());
#if (PHAS_INT128)
			else if (s->dt().size() == sizeof(INT128))
				save_pointer((int64_t *) s->ptr(), s->name().c_str(), s->count() * 2);
#endif
			else
				netlist().log().fatal("Unknown integral type size {1} for {2}\n", s->dt().size(), s->name().c_str());
		}
		else if (s->dt().is_custom())
		{
			/* do nothing */
		}
		else
			netlist().log().fatal("found unsupported save element {1}\n", s->name());
	}
}

// ----------------------------------------------------------------------------------------
// netlist_mame_cpu_device
// ----------------------------------------------------------------------------------------

netlist_mame_cpu_device::netlist_mame_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: netlist_mame_device(mconfig, NETLIST_CPU, tag, owner, clock)
	, device_execute_interface(mconfig, *this)
	, device_state_interface(mconfig, *this)
	, device_disasm_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 12) // Interface is needed to keep debugger happy
	, m_genPC(0)
{
}


void netlist_mame_cpu_device::device_start()
{
	netlist_mame_device::device_start();

	// State support

	state_add(STATE_GENPC, "GENPC", m_genPC).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_genPC).noshow();

	int index = 0;
	for (auto &n : netlist().nets())
	{
		if (n->is_logic())
		{
			state_add(index++, n->name().c_str(), *(downcast<netlist::logic_net_t &>(*n).Q_state_ptr()));
		}
		else
		{
			//state_add(index++, n->name().c_str(), *(downcast<netlist::analog_net_t &>(*n).Q_Analog_state_ptr()));
		}
	}

	// set our instruction counter
	set_icountptr(m_icount);
}


void netlist_mame_cpu_device::nl_register_devices(netlist::nlparse_t &parser) const
{
	parser.factory().add<nld_analog_callback>( "NETDEV_CALLBACK",
		netlist::factory::properties("-", PSOURCELOC()));
}

uint64_t netlist_mame_cpu_device::execute_clocks_to_cycles(uint64_t clocks) const noexcept
{
	return clocks;
}

uint64_t netlist_mame_cpu_device::execute_cycles_to_clocks(uint64_t cycles) const noexcept
{
	return cycles;
}

void netlist_mame_cpu_device::execute_run()
{
	//m_ppc = m_pc; // copy PC to previous PC
	if (debugger_enabled())
	{
		while (m_icount > 0)
		{
			m_genPC++;
			m_genPC &= 255;
			debugger_instruction_hook(m_genPC);
			netlist().exec().process_queue(nltime_ext_from_clocks(1));
			update_icount(netlist().exec().time());
		}
	}
	else
	{
		netlist().exec().process_queue(nltime_ext_from_clocks(m_icount));
		update_icount(netlist().exec().time());
	}
}

std::unique_ptr<util::disasm_interface> netlist_mame_cpu_device::create_disassembler()
{
	return std::make_unique<netlist_disassembler>(this);
}

netlist_disassembler::netlist_disassembler(netlist_mame_cpu_device *dev) : m_dev(dev)
{
}

u32 netlist_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t netlist_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	unsigned startpc = pc;
	int relpc = pc - m_dev->genPC();
	if (relpc >= 0 && relpc < m_dev->netlist().exec().queue().size())
	{
		int dpc = m_dev->netlist().exec().queue().size() - relpc - 1;
		util::stream_format(stream, "%c %s @%10.7f", (relpc == 0) ? '*' : ' ', m_dev->netlist().exec().queue()[dpc].object()->name().c_str(),
				m_dev->netlist().exec().queue()[dpc].exec_time().as_double());
	}

	pc+=1;
	return (pc - startpc);
}

// ----------------------------------------------------------------------------------------
// netlist_mame_sound_device
// ----------------------------------------------------------------------------------------

netlist_mame_sound_device::netlist_mame_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: netlist_mame_device(mconfig, NETLIST_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_in(nullptr)
	, m_stream(nullptr)
	, m_is_device_call(false)
{
}

void netlist_mame_sound_device::device_validity_check(validity_checker &valid) const
{
	LOGDEVCALLS("sound device_validity check\n");
	auto lnetlist = base_validity_check(valid);
	if (lnetlist)
	{
		/*Ok - do some more checks */
		std::vector<nld_sound_out *> outdevs = lnetlist->get_device_list<nld_sound_out>();
		if (outdevs.size() == 0)
			osd_printf_error("No output devices\n");
		else
		{
			for (auto &outdev : outdevs)
			{
				int chan = outdev->m_channel();
				if (chan < 0 || chan >= outdevs.size())
					osd_printf_error("illegal channel number %d\n", chan);
			}
		}
		std::vector<nld_sound_in *> indevs = lnetlist->get_device_list<nld_sound_in>();
		if (indevs.size() > 1)
			osd_printf_error("A maximum of one input device is allowed but found %d!\n", (int)indevs.size());
	}

}


void netlist_mame_sound_device::device_reset()
{
	netlist_mame_device::device_reset();
}

void netlist_mame_sound_device::device_start()
{
	netlist_mame_device::device_start();

	LOGDEVCALLS("sound device_start\n");

	// Configure outputs

	std::vector<nld_sound_out *> outdevs = netlist().get_device_list<nld_sound_out>();
	if (outdevs.size() == 0)
		fatalerror("No output devices");

	//m_num_outputs = outdevs.size();

	/* resort channels */
	for (auto &outdev : outdevs)
	{
		int chan = outdev->m_channel();

		netlist().log().verbose("Output %s on channel %d", outdev->name(), chan);

		if (chan < 0 || chan >= outdevs.size())
			fatalerror("illegal channel number");
		m_out[chan] = outdev;
		m_out[chan]->m_sample_time = netlist::netlist_time::from_hz(clock());
		m_out[chan]->buffer_reset(netlist::netlist_time::zero());
	}

	// Configure inputs
	// FIXME: The limitation to one input device seems artificial.
	//        We should allow multiple devices with one channel each.

	m_in = nullptr;

	std::vector<nld_sound_in *> indevs = netlist().get_device_list<nld_sound_in>();
	if (indevs.size() > 1)
		fatalerror("A maximum of one input device is allowed!");
	if (indevs.size() == 1)
	{
		m_in = indevs[0];
		m_in->resolve(clocks_to_attotime(1));
	}

	/* initialize the stream(s) */
	m_is_device_call = false;
	m_stream = machine().sound().stream_alloc(*this, m_in ? m_in->num_channels() : 0, m_out.size(), clock());
}


void netlist_mame_sound_device::nl_register_devices(netlist::nlparse_t &parser) const
{
	parser.factory().add<nld_sound_out>("NETDEV_SOUND_OUT",
		netlist::factory::properties("+CHAN", PSOURCELOC()));
	parser.factory().add<nld_sound_in>("NETDEV_SOUND_IN",
		netlist::factory::properties("-", PSOURCELOC()));
}

void netlist_mame_sound_device::device_clock_changed()
{
	netlist_mame_device::device_clock_changed();

	for (auto &e : m_out)
	{
		e.second->m_sample_time = nltime_from_clocks(1);
	}
}

static attotime last;

void netlist_mame_sound_device::update_to_current_time()
{
	LOGDEBUG("before update\n");
	m_is_device_call = true;
	get_stream()->update();
	m_is_device_call = false;

	if (machine().time() < last)
		LOGDEBUG("machine.time() decreased 2\n");

	last = machine().time();

	const auto mtime = nltime_from_attotime(machine().time());
	const auto cur(netlist().exec().time());

	if (mtime > cur)
	{
		if ((mtime - cur) >= nltime_from_clocks(1))
			LOGDEBUG("%f us\n", (mtime - cur).as_double() * 1000000.0);
		netlist().exec().process_queue(mtime - cur);
	}
	else if (mtime < cur)
		LOGDEBUG("%s : %f ns before machine time\n", this->name(), (cur - mtime).as_double() * 1000000000.0);
}

void netlist_mame_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	if (machine().time() < last)
		LOGDEBUG("machine.time() decreased 1\n");
	last = machine().time();
	LOGDEBUG("samples %d %d\n", (int) m_is_device_call, samples);

	if (m_in)
	{
		m_in->buffer_reset(m_attotime_per_clock, samples, inputs);
	}

	m_cur_time += (samples * m_attotime_per_clock);
	auto nl_target_time = nltime_from_attotime(m_cur_time);

	if (!m_is_device_call)
		nl_target_time -= netlist::netlist_time_ext::from_usec(2); // FIXME make adjustment a parameter

	auto nltime(netlist().exec().time());

	if (nltime < nl_target_time)
	{
		netlist().exec().process_queue(nl_target_time - nltime);
	}

	for (auto &e : m_out)
	{
		e.second->sound_update_fill(samples, outputs[e.first]);
		e.second->buffer_reset(nl_target_time);
	}

}
