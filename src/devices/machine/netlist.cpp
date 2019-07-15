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

#include "netlist/plib/palloc.h"

#include "debugger.h"
#include "romload.h"
#include "emuopts.h"

#include <cmath>
#include <memory>
#include <string>
#include <utility>



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

class netlist_mame_device::netlist_mame_callbacks_t : public netlist::callbacks_t
{
public:

	netlist_mame_callbacks_t(const netlist_mame_device &parent)
		: netlist::callbacks_t()
		, m_parent(parent)
	{
	}

protected:
	void vlog(const plib::plog_level &l, const pstring &ls) const override
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
			throw emu_fatalerror(1, "netlist FATAL: %s\n", ls.c_str());
		}
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
	void vlog(const plib::plog_level &l, const pstring &ls) const override
	{
		switch (l)
		{
		case plib::plog_level::DEBUG:
			break;
		case plib::plog_level::VERBOSE:
			break;
		case plib::plog_level::INFO:
			osd_printf_verbose("netlist INFO: %s\n", ls.c_str());
			break;
		case plib::plog_level::WARNING:
			osd_printf_warning("netlist WARNING: %s\n", ls.c_str());
			break;
		case plib::plog_level::ERROR:
			osd_printf_error("netlist ERROR: %s\n", ls.c_str());
			break;
		case plib::plog_level::FATAL:
			throw emu_fatalerror(1, "netlist FATAL: %s\n", ls.c_str());
		}
	}

private:
};


class netlist_mame_device::netlist_mame_t : public netlist::netlist_t
{
public:

	netlist_mame_t(netlist_mame_device &parent, const pstring &aname)
		: netlist::netlist_t(aname, plib::make_unique<netlist_mame_device::netlist_mame_callbacks_t>(parent))
		, m_parent(parent)
	{
	}

	netlist_mame_t(netlist_mame_device &parent, const pstring &aname, plib::unique_ptr<netlist::callbacks_t> cbs)
		: netlist::netlist_t(aname, std::move(cbs))
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
		auto *nl = dynamic_cast<netlist_mame_device::netlist_mame_t *>(&exec());
		if (nl != nullptr)
			m_cpu_device = downcast<netlist_mame_cpu_device *>(&nl->parent());
	}

	ATTR_COLD void reset() override
	{
		m_last = 0.0;
	}

	ATTR_COLD void register_callback(netlist_mame_analog_output_device::output_delegate &&callback)
	{
		m_callback = std::move(callback);
	}

	NETLIB_UPDATEI()
	{
		nl_double cur = m_in();

		// FIXME: make this a parameter
		// avoid calls due to noise
		if (std::fabs(cur - m_last) > 1e-6)
		{
			m_cpu_device->update_icount(exec().time());
			m_callback(cur, m_cpu_device->local_time());
			m_cpu_device->check_mame_abort_slice();
			m_last = cur;
		}
	}

private:
	netlist::analog_input_t m_in;
	netlist_mame_analog_output_device::output_delegate m_callback;
	netlist_mame_cpu_device *m_cpu_device;
	netlist::state_var<nl_double> m_last;
};

// ----------------------------------------------------------------------------------------
// logic_callback
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(logic_callback) : public netlist::device_t
{
public:
	NETLIB_NAME(logic_callback)(netlist::netlist_state_t &anetlist, const pstring &name)
		: device_t(anetlist, name)
		, m_in(*this, "IN")
		, m_cpu_device(nullptr)
		, m_last(*this, "m_last", 0)
	{
		auto *nl = dynamic_cast<netlist_mame_device::netlist_mame_t *>(&exec());
		if (nl != nullptr)
			m_cpu_device = downcast<netlist_mame_cpu_device *>(&nl->parent());
	}

	ATTR_COLD void reset() override
	{
		m_last = 0;
	}

	ATTR_COLD void register_callback(netlist_mame_logic_output_device::output_delegate &&callback)
	{
		m_callback = std::move(callback);
	}

	NETLIB_UPDATEI()
	{
		netlist::netlist_sig_t cur = m_in();

		// FIXME: make this a parameter
		// avoid calls due to noise
		if (cur != m_last)
		{
			m_cpu_device->update_icount(exec().time());
			m_callback(cur, m_cpu_device->local_time());
			m_cpu_device->check_mame_abort_slice();
			m_last = cur;
		}
	}

private:
	netlist::logic_input_t m_in;
	netlist_mame_logic_output_device::output_delegate m_callback;
	netlist_mame_cpu_device *m_cpu_device;
	netlist::state_var<netlist::netlist_sig_t> m_last;
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

class netlist_source_memregion_t : public netlist::source_t
{
public:


	netlist_source_memregion_t(device_t &dev, pstring name)
	: netlist::source_t(), m_dev(dev), m_name(name)
	{
	}

	virtual plib::unique_ptr<plib::pistream> stream(const pstring &name) override;
private:
	device_t &m_dev;
	pstring m_name;
};

class netlist_data_memregions_t : public netlist::source_t
{
public:
	netlist_data_memregions_t(const device_t &dev);

	virtual plib::unique_ptr<plib::pistream> stream(const pstring &name) override;

private:
	const device_t &m_dev;
};


// ----------------------------------------------------------------------------------------
// memregion source support
// ----------------------------------------------------------------------------------------

plib::unique_ptr<plib::pistream> netlist_source_memregion_t::stream(const pstring &name)
{
	if (m_dev.has_running_machine())
	{
		memory_region *mem = m_dev.memregion(m_name.c_str());
		return plib::make_unique<plib::pimemstream>(mem->base(), mem->bytes());
	}
	else
		throw memregion_not_set("memregion unavailable for {1} in source {2}", name, m_name);
		//return plib::unique_ptr<plib::pimemstream>(nullptr);
}

netlist_data_memregions_t::netlist_data_memregions_t(const device_t &dev)
	: netlist::source_t(netlist::source_t::DATA), m_dev(dev)
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
				char const *const basetag = romp->name;
				if (name == pstring(":") + basetag)
					return true;
			}
		}
	}
	return false;
}

plib::unique_ptr<plib::pistream> netlist_data_memregions_t::stream(const pstring &name)
{
	//memory_region *mem = static_cast<netlist_mame_device::netlist_mame_t &>(setup().setup().exec()).parent().memregion(name.c_str());
	if (m_dev.has_running_machine())
	{
		memory_region *mem = m_dev.memregion(name.c_str());
		if (mem != nullptr)
			return plib::make_unique<plib::pimemstream>(mem->base(), mem->bytes());
		else
			return plib::unique_ptr<plib::pistream>(nullptr);
	}
	else
	{
		/* validation */
		if (rom_exists(m_dev.mconfig().root_device(), pstring(m_dev.tag()) + ":" + name))
			return plib::make_unique<plib::pimemstream>();
		else
			return plib::unique_ptr<plib::pistream>(nullptr);
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
		, m_buffer(nullptr)
		, m_bufsize(0)
		, m_sample_time(netlist::netlist_time::from_hz(1)) //sufficiently big enough
		, m_in(*this, "IN")
		, m_cur(0.0)
		, m_last_pos(0)
		, m_last_buffer_time(*this, "m_last_buffer", netlist::netlist_time::zero())
	{
	}

	//static const int BUFSIZE = 2048;

	ATTR_COLD void reset() override
	{
		m_cur = 0.0;
		m_last_pos = 0;
		m_last_buffer_time = netlist::netlist_time::zero();
	}

	ATTR_HOT void sound_update(const netlist::netlist_time &upto)
	{
		int pos = (upto - m_last_buffer_time) / m_sample_time;
		if (pos > m_bufsize)
			throw emu_fatalerror("sound %s: pos %d exceeded bufsize %d\n", name().c_str(), pos, m_bufsize);
		while (m_last_pos < pos )
		{
			m_buffer[m_last_pos++] = (stream_sample_t) m_cur;
		}
	}

	ATTR_HOT void sound_update_fill(int samples)
	{
		if (samples > m_bufsize)
			throw emu_fatalerror("sound %s: pos %d exceeded bufsize %d\n", name().c_str(), samples, m_bufsize);
		while (m_last_pos < samples )
		{
			m_buffer[m_last_pos++] = (stream_sample_t) m_cur;
		}
	}

	NETLIB_UPDATEI()
	{
		nl_double val = m_in() * m_mult() + m_offset();
		sound_update(exec().time());
		/* ignore spikes */
		if (std::abs(val) < 32767.0)
			m_cur = val;
		else if (val > 0.0)
			m_cur = 32767.0;
		else
			m_cur = -32767.0;

	}

public:
	ATTR_HOT void buffer_reset(const netlist::netlist_time &upto)
	{
		m_last_pos = 0;
		m_last_buffer_time = upto;
	}

	netlist::param_int_t m_channel;
	netlist::param_double_t m_mult;
	netlist::param_double_t m_offset;
	stream_sample_t *m_buffer;
	int m_bufsize;

	netlist::netlist_time m_sample_time;

private:
	netlist::analog_input_t m_in;
	double m_cur;
	int m_last_pos;
	netlist::state_var<netlist::netlist_time> m_last_buffer_time;
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
	, m_inc(netlist::netlist_time::from_nsec(1))
	, m_feedback(*this, "FB") // clock part
	, m_Q(*this, "Q")
	, m_pos(0)
	, m_num_channels(0)
	{
		connect(m_feedback, m_Q);

		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			m_channels[i].m_param_name = netlist::pool().make_poolptr<netlist::param_str_t>(*this, plib::pfmt("CHAN{1}")(i), "");
			m_channels[i].m_param_mult = netlist::pool().make_poolptr<netlist::param_double_t>(*this, plib::pfmt("MULT{1}")(i), 1.0);
			m_channels[i].m_param_offset = netlist::pool().make_poolptr<netlist::param_double_t>(*this, plib::pfmt("OFFSET{1}")(i), 0.0);
		}
	}

	ATTR_COLD void reset() override
	{
		m_pos = 0;
		for (auto & elem : m_channels)
			elem.m_buffer = nullptr;
	}

	ATTR_COLD void resolve()
	{
		m_pos = 0;
		for (int i = 0; i < MAX_INPUT_CHANNELS; i++)
		{
			if ((*m_channels[i].m_param_name)() != pstring(""))
			{
				if (i != m_num_channels)
					state().log().fatal("sound input numbering has to be sequential!");
				m_num_channels++;
				m_channels[i].m_param = dynamic_cast<netlist::param_double_t *>(setup().find_param((*m_channels[i].m_param_name)(), true));
			}
		}
	}

	NETLIB_UPDATEI()
	{
		for (int i=0; i<m_num_channels; i++)
		{
			if (m_channels[i].m_buffer == nullptr)
				break; // stop, called outside of stream_update
			const nl_double v = m_channels[i].m_buffer[m_pos];
			m_channels[i].m_param->setTo(v * (*m_channels[i].m_param_mult)() + (*m_channels[i].m_param_offset)());
		}
		m_pos++;
		m_Q.net().toggle_and_push_to_queue(m_inc);
	}

public:
	ATTR_HOT void buffer_reset()
	{
		m_pos = 0;
	}

	struct channel
	{
		netlist::pool_owned_ptr<netlist::param_str_t> m_param_name;
		netlist::param_double_t *m_param;
		stream_sample_t *m_buffer;
		netlist::pool_owned_ptr<netlist::param_double_t> m_param_mult;
		netlist::pool_owned_ptr<netlist::param_double_t> m_param_offset;
	};
	channel m_channels[MAX_INPUT_CHANNELS];
	netlist::netlist_time m_inc;

	int num_channels() { return m_num_channels; }

private:
	netlist::logic_input_t m_feedback;
	netlist::logic_output_t m_Q;

	int m_pos;
	int m_num_channels;
};


// netlib #defines this and it fights with logmacro.h
#undef LOG

#define LOG_GENERAL     (1U << 0)
#define LOG_DEV_CALLS   (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_DEV_CALLS)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGDEVCALLS(...) LOGMASKED(LOG_DEV_CALLS, __VA_ARGS__)


netlist::setup_t &netlist_mame_device::setup()
{
	if (!m_netlist)
		throw device_missing_dependencies();
	return m_netlist->nlstate().setup();
}

void netlist_mame_device::register_memregion_source(netlist::nlparse_t &setup, device_t &dev, const char *name)
{
	setup.register_source(plib::make_unique<netlist_source_memregion_t>(dev, pstring(name)));
}

void netlist_mame_analog_input_device::write(const double val)
{
	m_value_for_device_timer = val * m_mult + m_offset;
	if (m_value_for_device_timer != (*m_param)())
		synchronize(0, 0, &m_value_for_device_timer);
}

void netlist_mame_analog_input_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	update_to_current_time();
	m_param->setTo(*((double *) ptr));
}

void netlist_mame_int_input_device::write(const uint32_t val)
{
	const uint32_t v = (val >> m_shift) & m_mask;
	if (v != (*m_param)())
		synchronize(0, v);
}

void netlist_mame_logic_input_device::write(const uint32_t val)
{
	const uint32_t v = (val >> m_shift) & 1;
	if (v != (*m_param)())
		synchronize(0, v);
}

void netlist_mame_int_input_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	update_to_current_time();
	m_param->setTo(param);
}

void netlist_mame_logic_input_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	update_to_current_time();
	m_param->setTo(param);
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
{
}

netlist_mame_analog_input_device::netlist_mame_analog_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_ANALOG_INPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_param(nullptr)
	, m_auto_port(true)
	, m_param_name("")
{
}

void netlist_mame_analog_input_device::device_start()
{
	LOGDEVCALLS("start\n");
	netlist::param_t *p = this->nl_owner().setup().find_param(pstring(m_param_name));
	m_param = dynamic_cast<netlist::param_double_t *>(p);
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
	netlist::param_t *p = nlstate.setup().find_param(pstring(m_param_name));
	auto *param = dynamic_cast<netlist::param_double_t *>(p);
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
{
}

void netlist_mame_analog_output_device::custom_netlist_additions(netlist::netlist_state_t &nlstate)
{
	const pstring pin(m_in);
	pstring dname = pstring("OUT_") + pin;
	pstring dfqn = nlstate.setup().build_fqn(dname);

	/* ignore if no running machine -> called within device_validity_check context */
	if (owner()->has_running_machine())
		m_delegate.bind_relative_to(owner()->machine().root_device());

	auto dev = netlist::pool().make_poolptr<NETLIB_NAME(analog_callback)>(nlstate, dfqn);
	//static_cast<NETLIB_NAME(analog_callback) *>(dev.get())->register_callback(std::move(m_delegate));
	dev->register_callback(std::move(m_delegate));
	nlstate.add_dev(dfqn, std::move(dev));
	nlstate.setup().register_link(dname + ".IN", pin);
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
{
}

void netlist_mame_logic_output_device::set_params(const char *in_name, output_delegate &&adelegate)
{
	m_in = in_name;
	m_delegate = std::move(adelegate);
}

void netlist_mame_logic_output_device::custom_netlist_additions(netlist::netlist_state_t &nlstate)
{
	pstring pin(m_in);
	pstring dname = "OUT_" + pin;
	pstring dfqn = nlstate.setup().build_fqn(dname);

	/* ignore if no running machine -> called within device_validity_check context */
	if (owner()->has_running_machine())
		m_delegate.bind_relative_to(owner()->machine().root_device());

	auto dev = netlist::pool().make_poolptr<NETLIB_NAME(logic_callback)>(nlstate, dfqn);
	dev->register_callback(std::move(m_delegate));
	nlstate.add_dev(dfqn, std::move(dev));
	nlstate.setup().register_link(dname + ".IN", pin);
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
	if (LOG_DEV_CALLS) logerror("set_params\n");
	m_param_name = param_name;
	m_shift = shift;
	m_mask = mask;
}

void netlist_mame_int_input_device::device_start()
{
	LOGDEVCALLS("start\n");
	netlist::param_t *p = downcast<netlist_mame_device *>(this->owner())->setup().find_param(pstring(m_param_name));
	m_param = dynamic_cast<netlist::param_int_t *>(p);
	if (m_param == nullptr)
	{
		fatalerror("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}
}

void netlist_mame_int_input_device::validity_helper(validity_checker &valid,
	netlist::netlist_state_t &nlstate) const
{
	netlist::param_t *p = nlstate.setup().find_param(pstring(m_param_name));
	auto *param = dynamic_cast<netlist::param_int_t *>(p);
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
	if (LOG_DEV_CALLS) logerror("set_params\n");
	m_param_name = param_name;
	m_shift = shift;
}

void netlist_mame_logic_input_device::device_start()
{
	LOGDEVCALLS("start\n");
	netlist::param_t *p = downcast<netlist_mame_device *>(this->owner())->setup().find_param(pstring(m_param_name));
	m_param = dynamic_cast<netlist::param_logic_t *>(p);
	if (m_param == nullptr)
	{
		fatalerror("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}
}

void netlist_mame_logic_input_device::validity_helper(validity_checker &valid,
	netlist::netlist_state_t &nlstate) const
{
	netlist::param_t *p = nlstate.setup().find_param(pstring(m_param_name));
	auto *param = dynamic_cast<netlist::param_logic_t *>(p);
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
	if (LOG_DEV_CALLS) logerror("set_params\n");
	m_param_name = param_name;
}

void netlist_mame_ram_pointer_device::device_start()
{
	LOGDEVCALLS("start\n");
	netlist::param_t *p = downcast<netlist_mame_device *>(this->owner())->setup().find_param(pstring(m_param_name));
	m_param = dynamic_cast<netlist::param_ptr_t *>(p);
	if (m_param == nullptr)
	{
		fatalerror("device %s wrong parameter type for %s\n", basetag(), m_param_name);
	}

	m_data = (*m_param)();
}

void netlist_mame_ram_pointer_device::validity_helper(validity_checker &valid,
	netlist::netlist_state_t &nlstate) const
{
	netlist::param_t *p = nlstate.setup().find_param(pstring(m_param_name));
	auto *param = dynamic_cast<netlist::param_ptr_t *>(p);
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
	if (!nlstate.setup().device_exists("STREAM_INPUT"))
		nlstate.setup().register_dev("NETDEV_SOUND_IN", "STREAM_INPUT");

	pstring sparam = plib::pfmt("STREAM_INPUT.CHAN{1}")(m_channel);
	nlstate.setup().register_param(sparam, pstring(m_param_name));
	sparam = plib::pfmt("STREAM_INPUT.MULT{1}")(m_channel);
	nlstate.setup().register_param(sparam, m_mult);
	sparam = plib::pfmt("STREAM_INPUT.OFFSET{1}")(m_channel);
	nlstate.setup().register_param(sparam, m_offset);
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
	nlstate.setup().register_dev("NETDEV_SOUND_OUT", sname);

	nlstate.setup().register_param(sname + ".CHAN" , m_channel);
	nlstate.setup().register_param(sname + ".MULT",  m_mult);
	nlstate.setup().register_param(sname + ".OFFSET",  m_offset);
	nlstate.setup().register_link(sname + ".IN", pstring(m_out_name));
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
	, m_old(netlist::netlist_time::zero())
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

void netlist_mame_device::common_dev_start(netlist::netlist_t *lnetlist) const
{
	auto &lsetup = lnetlist->nlstate().setup();

	// Override log statistics
	pstring p = plib::util::environment("NL_STATS", "");
	if (p != "")
	{
		bool err=false;
		bool v = plib::pstonum_ne<bool, true>(p, err);
		if (err)
			lsetup.log().warning("NL_STATS: invalid value {1}", p);
		else
			lnetlist->enable_stats(v);
	}

	// register additional devices

	nl_register_devices(lsetup);

	/* let sub-devices add sources and do stuff prior to parsing */
	for (device_t &d : subdevices())
	{
		netlist_mame_sub_interface *sdev = dynamic_cast<netlist_mame_sub_interface *>(&d);
		if( sdev != nullptr )
		{
			LOGDEVCALLS("Preparse subdevice %s/%s\n", d.name(), d.shortname());
			sdev->pre_parse_action(lnetlist->nlstate());
		}
	}

	/* add default data provider for roms - if not in validity check*/
	//if (has_running_machine())
		lsetup.register_source(plib::make_unique<netlist_data_memregions_t>(*this));

	m_setup_func(lsetup);

#if 1
	/* let sub-devices tweak the netlist */
	for (device_t &d : subdevices())
	{
		netlist_mame_sub_interface *sdev = dynamic_cast<netlist_mame_sub_interface *>(&d);
		if( sdev != nullptr )
		{
			LOGDEVCALLS("Found subdevice %s/%s\n", d.name(), d.shortname());
			sdev->custom_netlist_additions(lnetlist->nlstate());
		}
	}
	lsetup.prepare_to_run();
#endif
}

plib::unique_ptr<netlist::netlist_t> netlist_mame_device::base_validity_check(validity_checker &valid) const
{
	try
	{
		//netlist_mame_t lnetlist(*this, "netlist", plib::make_unique<netlist_validate_callbacks_t>());
		auto lnetlist = plib::make_unique<netlist::netlist_t>("netlist", plib::make_unique<netlist_validate_callbacks_t>());
		// enable validation mode
		lnetlist->nlstate().setup().enable_validation();
		common_dev_start(lnetlist.get());

		for (device_t &d : subdevices())
		{
			netlist_mame_sub_interface *sdev = dynamic_cast<netlist_mame_sub_interface *>(&d);
			if( sdev != nullptr )
			{
				LOGDEVCALLS("Validity check on subdevice %s/%s\n", d.name(), d.shortname());
				sdev->validity_helper(valid, lnetlist->nlstate());
			}
		}

		return lnetlist;
	}
	catch (memregion_not_set &err)
	{
		osd_printf_verbose("%s\n", err.what());
	}
	catch (emu_fatalerror &err)
	{
		osd_printf_error("%s\n", err.string());
	}
	catch (std::exception &err)
	{
		osd_printf_error("%s\n", err.what());
	}
	return plib::unique_ptr<netlist::netlist_t>(nullptr);
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

	m_netlist = netlist::pool().make_poolptr<netlist_mame_t>(*this, "netlist");
	if (!machine().options().verbose())
	{
		m_netlist->nlstate().log().verbose.set_enabled(false);
		m_netlist->nlstate().log().debug.set_enabled(false);
	}

	common_dev_start(m_netlist.get());

	m_netlist->nlstate().save(*this, m_rem, this->name(), "m_rem");
	m_netlist->nlstate().save(*this, m_div, this->name(), "m_div");
	m_netlist->nlstate().save(*this, m_old, this->name(), "m_old");

	save_state();

	m_old = netlist::netlist_time::zero();
	m_rem = netlist::netlist_time::zero();

	LOGDEVCALLS("device_start exit\n");
}

void netlist_mame_device::device_clock_changed()
{
	m_div = netlist::netlist_time::from_hz(clock());
	netlist().log().debug("Setting clock {1} and divisor {2}\n", clock(), m_div.as_double());
}


void netlist_mame_device::device_reset()
{
	LOGDEVCALLS("device_reset\n");
	m_old = netlist::netlist_time::zero();
	m_rem = netlist::netlist_time::zero();
	netlist().reset();
}

void netlist_mame_device::device_stop()
{
	LOGDEVCALLS("device_stop\n");
	netlist().stop();
}

ATTR_COLD void netlist_mame_device::device_post_load()
{
	LOGDEVCALLS("device_post_load\n");

	netlist().run_state_manager().post_load();
	netlist().nlstate().rebuild_lists();
}

ATTR_COLD void netlist_mame_device::device_pre_save()
{
	LOGDEVCALLS("device_pre_save\n");

	netlist().run_state_manager().pre_save();
}

void netlist_mame_device::update_icount(netlist::netlist_time time)
{
	const netlist::netlist_time newt(time);
	const netlist::netlist_time delta(newt - m_old + m_rem);
	const uint64_t d = delta / m_div;
	m_old = newt;
	m_rem = delta - (m_div * d);
	m_icount -= d;
}

void netlist_mame_device::check_mame_abort_slice()
{
	if (m_icount <= 0)
		netlist().abort_current_queue_slice();
}

ATTR_COLD void netlist_mame_device::save_state()
{
	for (auto const & s : netlist().run_state_manager().save_list())
	{
		netlist().log().debug("saving state for {1}\n", s->m_name.c_str());
		if (s->m_dt.is_float)
		{
			if (s->m_dt.size == sizeof(double))
				save_pointer((double *) s->m_ptr, s->m_name.c_str(), s->m_count);
			else if (s->m_dt.size == sizeof(float))
				save_pointer((float *) s->m_ptr, s->m_name.c_str(), s->m_count);
			else
				netlist().log().fatal("Unknown floating type for {1}\n", s->m_name.c_str());
		}
		else if (s->m_dt.is_integral)
		{
			if (s->m_dt.size == sizeof(int64_t))
				save_pointer((int64_t *) s->m_ptr, s->m_name.c_str(), s->m_count);
			else if (s->m_dt.size == sizeof(int32_t))
				save_pointer((int32_t *) s->m_ptr, s->m_name.c_str(), s->m_count);
			else if (s->m_dt.size == sizeof(int16_t))
				save_pointer((int16_t *) s->m_ptr, s->m_name.c_str(), s->m_count);
			else if (s->m_dt.size == sizeof(int8_t))
				save_pointer((int8_t *) s->m_ptr, s->m_name.c_str(), s->m_count);
#if (PHAS_INT128)
			else if (s->m_dt.size == sizeof(INT128))
				save_pointer((int64_t *) s->m_ptr, s->m_name.c_str(), s->m_count * 2);
#endif
			else
				netlist().log().fatal("Unknown integral type size {1} for {2}\n", s->m_dt.size, s->m_name.c_str());
		}
		else if (s->m_dt.is_custom)
		{
			/* do nothing */
		}
		else
			netlist().log().fatal("found unsupported save element {1}\n", s->m_name);
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
	for (auto &n : netlist().nlstate().nets())
	{
		if (n->is_logic())
		{
			state_add(index, n->name().c_str(), *(downcast<netlist::logic_net_t &>(*n).Q_state_ptr()));
		}
		else
		{
			state_add(index, n->name().c_str(), *(downcast<netlist::analog_net_t &>(*n).Q_Analog_state_ptr()));
		}
		index++;
	}

	// set our instruction counter
	set_icountptr(m_icount);
}


void netlist_mame_cpu_device::nl_register_devices(netlist::setup_t &lsetup) const
{
	lsetup.factory().register_device<nld_analog_callback>( "NETDEV_CALLBACK", "nld_analog_callback", "-");
}

ATTR_COLD uint64_t netlist_mame_cpu_device::execute_clocks_to_cycles(uint64_t clocks) const
{
	return clocks;
}

ATTR_COLD uint64_t netlist_mame_cpu_device::execute_cycles_to_clocks(uint64_t cycles) const
{
	return cycles;
}

ATTR_HOT void netlist_mame_cpu_device::execute_run()
{
	//m_ppc = m_pc; // copy PC to previous PC
	if (debugger_enabled())
	{
		while (m_icount > 0)
		{
			m_genPC++;
			m_genPC &= 255;
			debugger_instruction_hook(m_genPC);
			netlist().process_queue(m_div);
			update_icount(netlist().time());
		}
	}
	else
	{
		netlist().process_queue(m_div * m_icount);
		update_icount(netlist().time());
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
	if (relpc >= 0 && relpc < m_dev->netlist().queue().size())
	{
		int dpc = m_dev->netlist().queue().size() - relpc - 1;
		util::stream_format(stream, "%c %s @%10.7f", (relpc == 0) ? '*' : ' ', m_dev->netlist().queue()[dpc].m_object->name().c_str(),
				m_dev->netlist().queue()[dpc].m_exec_time.as_double());
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
{
}

void netlist_mame_sound_device::device_validity_check(validity_checker &valid) const
{
	LOGDEVCALLS("sound device_validity check\n");
	auto lnetlist = base_validity_check(valid);
	if (lnetlist)
	{
		/*Ok - do some more checks */
		std::vector<nld_sound_out *> outdevs = lnetlist->nlstate().get_device_list<nld_sound_out>();
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
		std::vector<nld_sound_in *> indevs = lnetlist->nlstate().get_device_list<nld_sound_in>();
		if (indevs.size() > 1)
			osd_printf_error("A maximum of one input device is allowed but found %d!\n", (int)indevs.size());
	}

}


void netlist_mame_sound_device::device_start()
{
	netlist_mame_device::device_start();

	LOGDEVCALLS("sound device_start\n");

	// Configure outputs

	std::vector<nld_sound_out *> outdevs = netlist().nlstate().get_device_list<nld_sound_out>();
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
		m_out[chan]->m_buffer = nullptr;
		m_out[chan]->m_bufsize = 0;
	}

	// Configure inputs
	// FIXME: The limitation to one input device seems artificial.
	//        We should allow multiple devices with one channel each.

	m_in = nullptr;

	std::vector<nld_sound_in *> indevs = netlist().nlstate().get_device_list<nld_sound_in>();
	if (indevs.size() > 1)
		fatalerror("A maximum of one input device is allowed!");
	if (indevs.size() == 1)
	{
		m_in = indevs[0];
		m_in->resolve();
		m_in->m_inc = netlist::netlist_time::from_hz(clock());
	}

	/* initialize the stream(s) */
	m_stream = machine().sound().stream_alloc(*this, m_in ? m_in->num_channels() : 0, m_out.size(), clock());

}

void netlist_mame_sound_device::nl_register_devices(netlist::setup_t &lsetup) const
{
	lsetup.factory().register_device<nld_sound_out>("NETDEV_SOUND_OUT", "nld_sound_out", "+CHAN");
	lsetup.factory().register_device<nld_sound_in>("NETDEV_SOUND_IN", "nld_sound_in", "-");
}

void netlist_mame_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{

	for (auto &e : m_out)
	{
		e.second->m_buffer = outputs[e.first];
		e.second->m_bufsize = samples;
	}


	if (m_in)
	{
		m_in->buffer_reset();
		for (int i=0; i < m_in->num_channels(); i++)
		{
			m_in->m_channels[i].m_buffer = inputs[i];
		}
	}

	netlist::netlist_time cur(netlist().time());

	netlist().process_queue(m_div * samples);

	cur += (m_div * samples);

	for (auto &e : m_out)
	{
		e.second->sound_update_fill(samples);
		e.second->buffer_reset(cur);
	}
}
