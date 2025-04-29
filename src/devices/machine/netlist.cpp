// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    netlist.c

    Discrete netlist implementation.

****************************************************************************/

#include "emu.h"
#include "netlist.h"

#include "netlist/nl_setup.h"
#include "netlist/nl_factory.h"
#include "netlist/nl_parser.h"
#include "netlist/nl_interface.h"

#include "netlist/plib/pdynlib.h"
#include "netlist/plib/pstonum.h"

#include "debugger.h"
#include "romload.h"
#include "emuopts.h"

#include <cmath>
#include <memory>
#include <string>
#include <utility>

#define LOG_DEV_CALLS   (1U << 1)
#define LOG_DEBUG       (1U << 2)
#define LOG_TIMING      (1U << 3)

//#define LOG_MASK (LOG_GENERAL | LOG_DEV_CALLS | LOG_DEBUG)
//#define LOG_MASK        (LOG_TIMING)
#define LOG_MASK        (0)

#define LOGDEVCALLS(...) LOGMASKED(LOG_DEV_CALLS, __VA_ARGS__)
#define LOGDEBUG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)
#define LOGTIMING(...) LOGMASKED(LOG_TIMING, __VA_ARGS__)

#define LOG_OUTPUT_FUNC printf

#define LOGMASKED(mask, ...) do { if (LOG_MASK & (mask)) (LOG_OUTPUT_FUNC)(__VA_ARGS__); } while (false)

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

extern const plib::static_library::symbol nl_static_solver_syms[];

static netlist::netlist_time_ext nltime_from_attotime(attotime t)
{
	netlist::netlist_time_ext nlmtime = netlist::netlist_time_ext::from_sec(t.seconds());
	nlmtime += netlist::netlist_time_ext::from_raw(t.attoseconds() / (ATTOSECONDS_PER_SECOND / netlist::netlist_time_ext::resolution()));
	return nlmtime;
}

void netlist_log_csv<1>::open(running_machine &machine, const std::string &name)
{
	m_csv_file = fopen(name.c_str(), "wb");
	m_machine = &machine;
}

void netlist_log_csv<1>::close()
{
	if (m_csv_file != nullptr)
	{
		log_flush();
		fclose(m_csv_file);
	}
}

void netlist_log_csv<1>::log_add(char const* param, double value, bool isfloat)
{
	// skip if no file
	if (m_csv_file == nullptr)
		return;

	// make a new entry
	buffer_entry entry = { (*m_machine).scheduler().time(), isfloat, value, param };

	// flush out half of the old entries if we hit the buffer limit
	if (m_buffer.size() >= MAX_BUFFER_ENTRIES)
		log_flush(MAX_BUFFER_ENTRIES / 2);

	// fast common case: if we go at the end, just push_back
	if (m_buffer.size() == 0 || entry.time >= m_buffer.back().time)
	{
		m_buffer.push_back(entry);
		return;
	}

	// find our place in the queue
	for (auto cur = m_buffer.rbegin(); cur != m_buffer.rend(); cur++)
		if (entry.time >= cur->time)
		{
			m_buffer.insert(cur.base(), entry);
			return;
		}

	// if we're too early, drop this entry rather than risk putting an out-of-order
	// entry after the last one we flushed
}

void netlist_log_csv<1>::log_flush(int count)
{
	if (m_csv_file == nullptr)
		return;
	if (count > m_buffer.size())
		count = m_buffer.size();
	while (count--)
	{
		auto &entry = m_buffer.front();
		if (entry.isfloat)
			fprintf(m_csv_file, "%s,%s,%f\n", entry.time.as_string(), entry.string, entry.value);
		else
			fprintf(m_csv_file, "%s,%s,%d\n", entry.time.as_string(), entry.string, int(entry.value));
		m_buffer.pop_front();
	}
}


class netlist_mame_device::netlist_mame_t : public netlist::netlist_state_t
{
public:

	netlist_mame_t(netlist_mame_device &parent, const pstring &name)
		: netlist::netlist_state_t(name, plib::plog_delegate(&netlist_mame_t::logger, this))
		, m_parent(parent)
	{
	}

	running_machine &machine() { return m_parent.machine(); }
	netlist_mame_device &parent() const { return m_parent; }

private:
	void logger(plib::plog_level l, const pstring &ls)
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

	netlist_mame_device &m_parent;
};


namespace {



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

	virtual plib::istream_uptr stream(const pstring &name) override;
private:
	device_t &m_dev;
	pstring m_name;
};

class netlist_data_memregions_t : public netlist::source_data_t
{
public:
	netlist_data_memregions_t(const device_t &dev);

	virtual plib::istream_uptr stream(const pstring &name) override;

private:
	const device_t &m_dev;
};


// ----------------------------------------------------------------------------------------
// memregion source support
// ----------------------------------------------------------------------------------------

plib::istream_uptr netlist_source_memregion_t::stream(const pstring &name)
{
	if (m_dev.has_running_machine())
	{
		memory_region *mem = m_dev.memregion(putf8string(m_name).c_str());
		plib::istream_uptr ret(std::make_unique<std::istringstream>(putf8string(reinterpret_cast<char *>(mem->base()), mem->bytes())), name);
		ret->imbue(std::locale::classic());
		return ret;
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
	for (device_t &device : device_enumerator(root))
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

plib::istream_uptr netlist_data_memregions_t::stream(const pstring &name)
{
	//memory_region *mem = static_cast<netlist_mame_device::netlist_mame_t &>(setup().setup().exec()).parent().memregion(name.c_str());
	if (m_dev.has_running_machine())
	{
		memory_region *mem = m_dev.memregion(putf8string(name).c_str());
		if (mem != nullptr)
		{
			plib::istream_uptr ret(std::make_unique<std::istringstream>(std::string(reinterpret_cast<char *>(mem->base()), mem->bytes()), std::ios_base::binary), name);
			ret->imbue(std::locale::classic());
			return ret;
		}
		else
			return plib::istream_uptr();
	}
	else
	{
		/* validation */
		if (rom_exists(m_dev.mconfig().root_device(), pstring(m_dev.tag()) + ":" + name))
		{
			// Create an empty stream.
			plib::istream_uptr ret(std::make_unique<std::istringstream>(std::ios_base::binary), name);
			ret->imbue(std::locale::classic());
			return ret;
		}
		else
			return plib::istream_uptr();
	}
}

} // anonymous namespace

// ----------------------------------------------------------------------------------------
// sound_in
// ----------------------------------------------------------------------------------------

using sound_in_type = netlist::interface::NETLIB_NAME(buffered_param_setter)<netlist_mame_sound_input_buffer>;

class NETLIB_NAME(sound_in) : public sound_in_type
{
public:
	using base_type = sound_in_type;
	using base_type::base_type;
};

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
	m_value_to_sync = val * m_mult + m_offset;
	if (m_value_to_sync != (*m_param)())
	{
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(netlist_mame_analog_input_device::sync_callback), this));
	}
}

TIMER_CALLBACK_MEMBER(netlist_mame_analog_input_device::sync_callback)
{
	update_to_current_time();
	nl_owner().log_csv().log_add(m_param_name, m_value_to_sync, true);
	m_param->set(m_value_to_sync);
}

void netlist_mame_int_input_device::write(const uint32_t val)
{
	const uint32_t v = (val >> m_shift) & m_mask;
	if (v != (*m_param)())
	{
		LOGDEBUG("write %s\n", this->tag());
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(netlist_mame_int_input_device::sync_callback), this), v);
}
}

void netlist_mame_logic_input_device::write(const uint32_t val)
{
	const uint32_t v = (val >> m_shift) & 1;
	if (v != (*m_param)())
	{
		LOGDEBUG("write %s\n", this->tag());
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(netlist_mame_logic_input_device::sync_callback), this), v);
	}
}

TIMER_CALLBACK_MEMBER(netlist_mame_int_input_device::sync_callback)
{
	update_to_current_time();
	nl_owner().log_csv().log_add(m_param_name, param, false);
	m_param->set(param);
}

TIMER_CALLBACK_MEMBER(netlist_mame_logic_input_device::sync_callback)
{
	update_to_current_time();
	nl_owner().log_csv().log_add(m_param_name, param, false);
	m_param->set(param);
}

TIMER_CALLBACK_MEMBER(netlist_mame_ram_pointer_device::sync_callback)
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
	, m_value_to_sync(0)
{
}

netlist_mame_analog_input_device::netlist_mame_analog_input_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_ANALOG_INPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_param(nullptr)
	, m_auto_port(true)
	, m_param_name("")
	, m_value_to_sync(0)
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



void netlist_mame_analog_output_device::custom_netlist_additions(netlist::nlparse_t &parser)
{
	/* ignore if no running machine -> called within device_validity_check context */
	if (owner()->has_running_machine())
		m_delegate.resolve();

	const pstring pin(m_in);
	pstring dname = pstring("OUT_") + pin;

	parser.register_dev(dname, dname);
	parser.register_connection(dname + ".IN", pin);
}

void netlist_mame_analog_output_device::pre_parse_action(netlist::nlparse_t &parser)
{
	const pstring pin(m_in);
	pstring dname = pstring("OUT_") + pin;

	const auto lambda = [this](auto &in, netlist::nl_fptype val)
	{
		this->cpu()->update_icount(in.exec().time());
		this->m_delegate(val, this->cpu()->local_time());
		this->cpu()->check_mame_abort_slice();
	};

	using lb_t = decltype(lambda);
	using cb_t = netlist::interface::NETLIB_NAME(analog_callback)<lb_t>;

	parser.factory().add<cb_t, netlist::nl_fptype, lb_t>(dname,
		netlist::factory::properties("-", PSOURCELOC()), 1e-6, std::forward<lb_t>(lambda));
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

void netlist_mame_logic_output_device::custom_netlist_additions(netlist::nlparse_t &parser)
{
	/* ignore if no running machine -> called within device_validity_check context */
	if (owner()->has_running_machine())
		m_delegate.resolve();

	const pstring pin(m_in);
	pstring dname = pstring("OUT_") + pin;

	parser.register_dev(dname, dname);
	parser.register_connection(dname + ".IN", pin);
}

void netlist_mame_logic_output_device::pre_parse_action(netlist::nlparse_t &parser)
{
	const pstring pin(m_in);
	pstring dname = pstring("OUT_") + pin;

	const auto lambda = [this](auto &in, netlist::netlist_sig_t val)
	{
		this->cpu()->update_icount(in.exec().time());
		this->m_delegate(val, this->cpu()->local_time());
		this->cpu()->check_mame_abort_slice();
	};

	using lb_t = decltype(lambda);
	using cb_t = netlist::interface::NETLIB_NAME(logic_callback)<lb_t>;

	parser.factory().add<cb_t, lb_t>(dname,
		netlist::factory::properties("-", PSOURCELOC()), std::forward<lb_t>(lambda));
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

void netlist_mame_stream_input_device::custom_netlist_additions(netlist::nlparse_t &parser)
{
	pstring name = plib::pfmt("STREAM_INPUT_{}")(m_channel);
	parser.register_dev("NETDEV_SOUND_IN", name);

	parser.register_param(name + ".CHAN", pstring(m_param_name));
	parser.register_param(name + ".MULT", m_mult);
	parser.register_param(name + ".OFFSET", m_offset);
	parser.register_param(name + ".ID", m_channel);
}


// ----------------------------------------------------------------------------------------
// netlist_mame_stream_output_device
// ----------------------------------------------------------------------------------------

netlist_mame_stream_output_device::netlist_mame_stream_output_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NETLIST_STREAM_OUTPUT, tag, owner, clock)
	, netlist_mame_sub_interface(*owner)
	, m_channel(0)
	, m_out_name("")
	, m_cur(0.0)
	, m_sample_time(netlist::netlist_time::from_hz(1))
	, m_last_buffer_time(netlist::netlist_time_ext::zero())
{
}

void netlist_mame_stream_output_device::set_params(int channel, const char *out_name)
{
	m_out_name = out_name;
	m_channel = channel;
	sound()->register_stream_output(channel, this);
}

/// \brief save state helper for plib classes supporting the save_state interface
///
struct save_helper
{
	save_helper(device_t *dev, const std::string &prefix)
	: m_device(dev), m_prefix(prefix)
	{}

	template<typename T, typename X = void *>
	void save_item(T &&item, const std::string &name, X = nullptr)
	{
		m_device->save_item(item, (m_prefix + "_" + name).c_str());
	}

	template <typename X = void *>
	std::enable_if_t<plib::compile_info::has_int128::value && std::is_pointer<X>::value, void>
	save_item(INT128 &item, const std::string &name, X = nullptr)
	{
		auto *p = reinterpret_cast<std::uint64_t *>(&item);
		m_device->save_item(p[0], (m_prefix + "_" + name + "_1").c_str());
		m_device->save_item(p[1], (m_prefix + "_" + name + "_2").c_str());
	}

private:
	device_t *m_device;
	std::string m_prefix;
};

void netlist_mame_stream_output_device::device_start()
{
	LOGDEVCALLS("start %s\n", name());
	m_cur = 0.0;
	m_last_buffer_time = netlist::netlist_time_ext::zero();

	save_item(NAME(m_cur));
	m_sample_time.save_state(save_helper(this, "m_sample_time"));
	m_last_buffer_time.save_state(save_helper(this, "m_last_buffer_time"));
}

void netlist_mame_stream_output_device::device_reset()
{
	LOGDEVCALLS("reset %s\n", name());
}

void netlist_mame_stream_output_device::sound_update_fill(sound_stream &stream, int output)
{
	if (stream.samples() < m_buffer.size())
		osd_printf_warning("sound %s: samples %d less bufsize %d\n", name(), stream.samples(), m_buffer.size());

	int sampindex;
	for (sampindex = 0; sampindex < m_buffer.size(); sampindex++)
		stream.put(output, sampindex, m_buffer[sampindex]);
	if (sampindex < stream.samples())
		stream.fill(output, m_cur, sampindex);
}


void netlist_mame_stream_output_device::pre_parse_action(netlist::nlparse_t &parser)
{
	pstring dname = plib::pfmt("STREAM_OUT_{1}")(m_channel);

	const auto lambda = [this](auto &in, netlist::nl_fptype val)
	{
		this->process(in.exec().time(), val);
	};

	using lb_t = decltype(lambda);
	using cb_t = netlist::interface::NETLIB_NAME(analog_callback)<lb_t>;

	parser.factory().add<cb_t, netlist::nl_fptype, lb_t>(dname,
		netlist::factory::properties("-", PSOURCELOC()), 1e-9, std::forward<lb_t>(lambda));
}


void netlist_mame_stream_output_device::custom_netlist_additions(netlist::nlparse_t &parser)
{
	pstring dname = plib::pfmt("STREAM_OUT_{1}")(m_channel);

	parser.register_dev(dname, dname);
	parser.register_connection(dname + ".IN", pstring(m_out_name));
}

void netlist_mame_stream_output_device::process(netlist::netlist_time_ext tim, netlist::nl_fptype val)
{
	val = val * m_mult + m_offset;

	int pos = (tim - m_last_buffer_time) / m_sample_time;
	//if (pos > m_bufsize)
	//  throw emu_fatalerror("sound %s: pos %d exceeded bufsize %d\n", name().c_str(), pos, m_bufsize);
	while (m_buffer.size() < pos )
	{
		m_buffer.push_back(static_cast<sound_stream::sample_t>(m_cur));
	}

	m_cur = val;
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
	, m_setup_func(nullptr)
	, m_device_reset_called(false)
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

	// add default data provider for roms - if not in validity check
	lsetup.parser().register_source<netlist_data_memregions_t>(*this);

	// Read the netlist
	m_setup_func(lsetup.parser());

	// let sub-devices tweak the netlist
	for (device_t &d : subdevices())
	{
		netlist_mame_sub_interface *sdev = dynamic_cast<netlist_mame_sub_interface *>(&d);
		if( sdev != nullptr )
		{
			LOGDEVCALLS("Found subdevice %s/%s\n", d.name(), d.shortname());
			sdev->custom_netlist_additions(lsetup.parser());
		}
	}
}

struct validity_logger
{
	void log(plib::plog_level l, const pstring &ls)
	{
		putf8string ls8(ls);

		switch (l)
		{
		case plib::plog_level::DEBUG:
			break;
		case plib::plog_level::VERBOSE:
			break;
		case plib::plog_level::INFO:
			osd_printf_verbose("netlist INFO: %s\n", ls8);
			break;
		case plib::plog_level::WARNING:
			osd_printf_warning("netlist WARNING: %s\n", ls8);
			break;
		case plib::plog_level::ERROR:
			osd_printf_error("netlist ERROR: %s\n", ls8);
			break;
		case plib::plog_level::FATAL:
			osd_printf_error("netlist FATAL: %s\n", ls8);
			break;
		}
	}
};

std::unique_ptr<netlist::netlist_state_t> netlist_mame_device::base_validity_check(validity_checker &valid) const
{
	try
	{
		validity_logger logger;
		plib::chrono::timer<plib::chrono::system_ticks> t;
		t.start();
		auto lnetlist = std::make_unique<netlist::netlist_state_t>("netlist",
			plib::plog_delegate(&validity_logger::log, &logger));
		// enable validation mode

		lnetlist->set_static_solver_lib(std::make_unique<plib::static_library>(nullptr));

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

		t.stop();
		//printf("time %s %f\n", this->mconfig().gamedrv().name, t.as_seconds<double>());
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
	return std::unique_ptr<netlist::netlist_state_t>(nullptr);
}

void netlist_mame_device::device_validity_check(validity_checker &valid) const
{

	base_validity_check(valid);
	//rom_exists(mconfig().root_device());
	LOGDEVCALLS("device_validity_check %s\n", this->mconfig().gamedrv().name);
}


void netlist_mame_device::device_start_common()
{
	m_netlist = std::make_unique<netlist_mame_t>(*this, "netlist");

	m_netlist->set_static_solver_lib(std::make_unique<plib::static_library>(nl_static_solver_syms));

	if (!machine().options().verbose())
	{
		m_netlist->log().verbose.set_enabled(false);
		m_netlist->log().debug.set_enabled(false);
	}

	common_dev_start(m_netlist.get());
	m_netlist->setup().prepare_to_run();


	m_device_reset_called = false;

	std::string name = machine().system().name;
	name += tag();
	for (int index = 0; index < name.size(); index++)
		if (name[index] == ':')
			name[index] = '_';
	name += ".csv";
	log_csv().open(machine(), name);

	LOGDEVCALLS("device_start exit\n");
}


void netlist_mame_device::device_start()
{
	LOGDEVCALLS("device_start entry\n");

	device_start_common();
	save_state();

	LOGDEVCALLS("device_start exit\n");
}


void netlist_mame_device::device_reset()
{
	LOGDEVCALLS("device_reset\n");
	if (!m_device_reset_called)
	{
		// netlists don't have a reset line, doing a soft-reset is pointless
		// the only reason we call these here once after device_start
		// is that netlist input devices may be started after the netlist device
		// and because the startup code may trigger actions which need all
		// devices set up.
		netlist().free_setup_resources();
		netlist().exec().reset();
		m_device_reset_called = true;
	}
}

void netlist_mame_device::device_stop()
{
	LOGDEVCALLS("device_stop\n");
	if (m_netlist)
		netlist().exec().stop();
	log_csv().close();
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

void netlist_mame_cpu_device::update_icount(netlist::netlist_time_ext time) noexcept
{
	const netlist::netlist_time_ext delta = (time - m_old).shl(MDIV_SHIFT) + m_rem;
	const uint64_t d = delta / m_div;
	m_old = time;
	m_rem = (delta - (m_div * d));
	//printf("d %d m_rem %d\n", (int) d, (int) m_rem.as_raw());
	m_icount -= d;
}

void netlist_mame_cpu_device::check_mame_abort_slice() noexcept
{
	if (m_icount <= 0)
		netlist().exec().abort_current_queue_slice();
}

void netlist_mame_device::save_state()
{
	for (auto const & s : netlist().run_state_manager().save_list())
	{
		putf8string u8name(s->name());

		netlist().log().debug("saving state for {1}\n", u8name.c_str());
		if (s->dt().is_float())
		{
			if (s->dt().size() == sizeof(double))
				save_pointer((double *) s->ptr(), u8name.c_str(), s->count());
			else if (s->dt().size() == sizeof(float))
				save_pointer((float *) s->ptr(), u8name.c_str(), s->count());
			else
				netlist().log().fatal("Unknown floating type for {1}\n", u8name.c_str());
		}
		else if (s->dt().is_integral())
		{
			if (s->dt().size() == sizeof(int64_t))
				save_pointer((int64_t *) s->ptr(), u8name.c_str(), s->count());
			else if (s->dt().size() == sizeof(int32_t))
				save_pointer((int32_t *) s->ptr(), u8name.c_str(), s->count());
			else if (s->dt().size() == sizeof(int16_t))
				save_pointer((int16_t *) s->ptr(), u8name.c_str(), s->count());
			else if (s->dt().size() == sizeof(int8_t))
				save_pointer((int8_t *) s->ptr(), u8name.c_str(), s->count());
			else if (plib::compile_info::has_int128::value && s->dt().size() == sizeof(INT128))
				save_pointer((int64_t *) s->ptr(), u8name.c_str(), s->count() * 2);
			else
				netlist().log().fatal("Unknown integral type size {1} for {2}\n", s->dt().size(), u8name.c_str());
		}
		else if (s->dt().is_custom())
		{
			/* do nothing */
		}
		else
			netlist().log().fatal("found unsupported save element {1}\n", u8name);
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
	, m_icount(0)
	, m_old(netlist::netlist_time_ext::zero())
	, m_genPC(0)
{
}


void netlist_mame_cpu_device::device_start()
{
	LOGDEVCALLS("device_start entry\n");

	device_start_common();
	// FIXME: use save_helper
	netlist().save(*this, m_rem, pstring(this->name()), "m_rem");
	netlist().save(*this, m_div, pstring(this->name()), "m_div");
	netlist().save(*this, m_old, pstring(this->name()), "m_old");

	m_old = netlist::netlist_time_ext::zero();
	m_rem = netlist::netlist_time_ext::zero();

	save_state();

	state_add(STATE_GENPC, "GENPC", m_genPC).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_genPC).noshow();

	int index = 0;
	for (auto &n : netlist().nets())
	{
		putf8string name(n->name()); //plib::replace_all(n->name(), ".", "_");
		if (n->is_logic())
		{
			auto nl = downcast<netlist::logic_net_t *>(n.get());
			state_add<netlist::netlist_sig_t>(index++, name.c_str(),
				[nl]() { return nl->Q(); },
				[nl](netlist::netlist_sig_t data) { nl->set_Q_and_push(data, netlist::netlist_time::quantum()); });
		}
		else
		{
			auto nl = downcast<netlist::analog_net_t *>(n.get());
			state_add<double>(
				index++,
				name.c_str(),
				[nl]() { return nl->Q_Analog(); },
				[nl](double data) { nl->set_Q_Analog(data); });
		}
	}

	// set our instruction counter
	set_icountptr(m_icount);

	LOGDEVCALLS("device_start exit\n");
}

void netlist_mame_cpu_device::device_clock_changed()
{
	m_div = static_cast<netlist::netlist_time_ext>(
		(netlist::netlist_time_ext::resolution() << MDIV_SHIFT) / clock());
	//printf("m_div %d\n", (int) m_div.as_raw());
	netlist().log().debug("Setting clock {1} and divisor {2}\n", clock(), m_div.as_double());
}

void netlist_mame_cpu_device::nl_register_devices(netlist::nlparse_t &parser) const
{
}

uint64_t netlist_mame_cpu_device::execute_clocks_to_cycles(uint64_t clocks) const noexcept
{
	return clocks;
}

uint64_t netlist_mame_cpu_device::execute_cycles_to_clocks(uint64_t cycles) const noexcept
{
	return cycles;
}

netlist::netlist_time_ext netlist_mame_cpu_device::nltime_ext_from_clocks(unsigned c) const noexcept
{
	return (m_div * c).shr(MDIV_SHIFT);
}

netlist::netlist_time netlist_mame_cpu_device::nltime_from_clocks(unsigned c) const noexcept
{
	return static_cast<netlist::netlist_time>((m_div * c).shr(MDIV_SHIFT));
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
	: netlist_mame_device(mconfig, NETLIST_SOUND, tag, owner, 0)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_cur_time(attotime::zero)
	, m_sound_clock(clock)
	, m_attotime_per_clock(attotime::zero)
	, m_last_update_to_current_time(attotime::zero)
{
}

void netlist_mame_sound_device::device_validity_check(validity_checker &valid) const
{
	LOGDEVCALLS("sound device_validity check\n");
	auto lnetlist = base_validity_check(valid);
	if (lnetlist)
	{
		/* Ok - do some more checks */
		if (m_out.size() == 0)
			osd_printf_error("No output devices\n");
		else
		{
			for (auto &outdev : m_out)
			{
				if (outdev.first < 0 || outdev.first >= m_out.size())
					osd_printf_error("illegal output channel number %d\n", outdev.first);
			}
		}
		std::vector<nld_sound_in *> indevs = lnetlist->get_device_list<nld_sound_in>();
		for (auto &e : indevs)
		{
			if (e->id() >= indevs.size())
				osd_printf_error("illegal input channel number %d\n", e->id());
		}
	}
}

void netlist_mame_sound_device::device_start()
{
	LOGDEVCALLS("sound device_start\n");

	m_attotime_per_clock = attotime::from_hz(m_sound_clock);

	save_item(NAME(m_cur_time));
	save_item(NAME(m_attotime_per_clock));

	device_start_common();
	save_state();

	m_cur_time = attotime::zero;

	// Configure outputs

	if (m_out.size() == 0)
		fatalerror("No output devices");

	/* resort channels */
	for (auto &outdev : m_out)
	{
		if (outdev.first < 0 || outdev.first >= m_out.size())
			fatalerror("illegal output channel number %d", outdev.first);
		outdev.second->set_sample_time(netlist::netlist_time::from_hz(m_sound_clock));
		outdev.second->buffer_reset(netlist::netlist_time_ext::zero());
	}

	// Configure inputs

	m_in.clear();

	std::vector<nld_sound_in *> indevs = netlist().get_device_list<nld_sound_in>();
	for (auto &e : indevs)
	{
		m_in.emplace(e->id(), e);
		const auto sample_time = netlist::netlist_time::from_raw(static_cast<netlist::netlist_time::internal_type>(nltime_from_attotime(m_attotime_per_clock).as_raw()));
		e->resolve_params(sample_time);
	}
	for (auto &e : m_in)
	{
		if (e.first < 0 || e.first >= m_in.size())
			fatalerror("illegal input channel number %d", e.first);
	}
	m_inbuffer.resize(m_in.size());

	/* initialize the stream(s) */
	m_stream = stream_alloc(m_in.size(), m_out.size(), m_sound_clock);

	LOGDEVCALLS("sound device_start exit\n");
}


void netlist_mame_sound_device::nl_register_devices(netlist::nlparse_t &parser) const
{
	//parser.factory().add<nld_sound_out>("NETDEV_SOUND_OUT",
	//  netlist::factory::properties("+CHAN", PSOURCELOC()));
	parser.factory().add<nld_sound_in>("NETDEV_SOUND_IN",
		netlist::factory::properties("-", PSOURCELOC()));
}

void netlist_mame_sound_device::register_stream_output(int channel, netlist_mame_stream_output_device *so)
{
	m_out[channel] = so;
}

void netlist_mame_sound_device::update_to_current_time()
{
	LOGDEBUG("before update\n");

	get_stream()->update();

	if (machine().time() < m_last_update_to_current_time)
		LOGTIMING("machine.time() decreased 2\n");

	m_last_update_to_current_time = machine().time();

	const auto mtime = nltime_from_attotime(machine().time());
	const auto cur(netlist().exec().time());

	if (mtime > cur)
	{
		//expected don't log
		//LOGTIMING("%f us\n", (mtime - cur).as_double() * 1000000.0);
		netlist().exec().process_queue(mtime - cur);
	}
	else if (mtime < cur)
		LOGTIMING("%s : %f us before machine time\n", this->name(), (cur - mtime).as_double() * 1000000.0);
}

void netlist_mame_sound_device::sound_stream_update(sound_stream &stream)
{
	for (auto &e : m_in)
	{
		auto clock_period = stream.sample_period();
		auto sample_time = netlist::netlist_time::from_raw(static_cast<netlist::netlist_time::internal_type>(nltime_from_attotime(clock_period).as_raw()));
		m_inbuffer[e.first] = netlist_mame_sound_input_buffer(stream, e.first);
		e.second->buffer_reset(sample_time, stream.samples(), &m_inbuffer[e.first]);
	}

	int samples = stream.samples();
	LOGDEBUG("samples %d\n", samples);

	// end_time() is the time at the END of the last sample we're generating
	// however, the sample value is the value at the START of that last sample,
	// so subtract one sample period so that we only process up to the minimum
	auto nl_target_time = nltime_from_attotime(stream.end_time() - stream.sample_period());

	auto nltime(netlist().exec().time());
	if (nltime < nl_target_time)
	{
		netlist().exec().process_queue(nl_target_time - nltime);
	}

	for (auto &e : m_out)
	{
		e.second->sound_update_fill(stream, e.first);
		e.second->buffer_reset(nl_target_time);
	}

}
