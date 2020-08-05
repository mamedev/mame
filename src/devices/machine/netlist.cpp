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
#include "netlist/nl_interface.h"
//#include "netlist/devices/nlid_system.h"

#include "netlist/plib/palloc.h"
#include "netlist/plib/pmempool.h"
#include "netlist/plib/pdynlib.h"
#include "netlist/plib/pstonum.h"

#include "debugger.h"
#include "romload.h"
#include "emuopts.h"

#include <cmath>
#include <memory>
#include <string>
#include <utility>

#define LOG_GENERAL     (1U << 0)
#define LOG_DEV_CALLS   (1U << 1)
#define LOG_DEBUG       (1U << 2)

//#define LOG_MASK (LOG_GENERAL | LOG_DEV_CALLS | LOG_DEBUG)
#define LOG_MASK        (0)

#define LOGDEVCALLS(...) LOGMASKED(LOG_DEV_CALLS, __VA_ARGS__)
#define LOGDEBUG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)

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

extern const plib::dynlib_static_sym nl_static_solver_syms[];

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

	std::unique_ptr<plib::dynlib_base> static_solver_lib() const noexcept override
	{
		//return plib::make_unique<plib::dynlib_static>(nullptr);
		return std::make_unique<plib::dynlib_static>(nl_static_solver_syms);
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

	std::unique_ptr<plib::dynlib_base> static_solver_lib() const noexcept override
	{
		return std::make_unique<plib::dynlib_static>(nullptr);
	}

private:
};


class netlist_mame_device::netlist_mame_t : public netlist::netlist_state_t
{
public:

	netlist_mame_t(netlist_mame_device &parent, const pstring &name)
		: netlist::netlist_state_t(name, plib::make_unique<netlist_mame_device::netlist_mame_callbacks_t, netlist::host_arena>(parent))
		, m_parent(parent)
	{
	}

	netlist_mame_t(netlist_mame_device &parent, const pstring &name, netlist::host_arena::unique_ptr<netlist::callbacks_t> cbs)
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

	virtual stream_ptr stream(const pstring &name) override;
private:
	device_t &m_dev;
	pstring m_name;
};

class netlist_data_memregions_t : public netlist::source_data_t
{
public:
	netlist_data_memregions_t(const device_t &dev);

	virtual stream_ptr stream(const pstring &name) override;

private:
	const device_t &m_dev;
};


// ----------------------------------------------------------------------------------------
// memregion source support
// ----------------------------------------------------------------------------------------

netlist_source_memregion_t::stream_ptr netlist_source_memregion_t::stream(const pstring &name)
{
	if (m_dev.has_running_machine())
	{
		memory_region *mem = m_dev.memregion(m_name.c_str());
		stream_ptr ret(std::make_unique<std::istringstream>(pstring(reinterpret_cast<char *>(mem->base()), mem->bytes())), name);
		ret.stream().imbue(std::locale::classic());
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

netlist_data_memregions_t::stream_ptr netlist_data_memregions_t::stream(const pstring &name)
{
	//memory_region *mem = static_cast<netlist_mame_device::netlist_mame_t &>(setup().setup().exec()).parent().memregion(name.c_str());
	if (m_dev.has_running_machine())
	{
		memory_region *mem = m_dev.memregion(name.c_str());
		if (mem != nullptr)
		{
			stream_ptr ret(std::make_unique<std::istringstream>(std::string(reinterpret_cast<char *>(mem->base()), mem->bytes()), std::ios_base::binary), name);
			ret.stream().imbue(std::locale::classic());
			return ret;
		}
		else
			return stream_ptr();
	}
	else
	{
		/* validation */
		if (rom_exists(m_dev.mconfig().root_device(), pstring(m_dev.tag()) + ":" + name))
		{
			// Create an empty stream.
			stream_ptr ret(std::make_unique<std::istringstream>(std::ios_base::binary), name);
			ret.stream().imbue(std::locale::classic());
			return ret;
		}
		else
			return stream_ptr();
	}
}

} // anonymous namespace

// ----------------------------------------------------------------------------------------
// sound_in
// ----------------------------------------------------------------------------------------

using sound_in_type = netlist::interface::NETLIB_NAME(buffered_param_setter)<stream_sample_t, 16>;

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
	m_value_for_device_timer = val * m_mult + m_offset;
	if (m_value_for_device_timer != (*m_param)())
	{
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
		printf("write %s: %d\n", this->tag(), val);
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



void netlist_mame_analog_output_device::custom_netlist_additions(netlist::nlparse_t &parser)
{
	/* ignore if no running machine -> called within device_validity_check context */
	if (owner()->has_running_machine())
		m_delegate.resolve();

	const pstring pin(m_in);
	pstring dname = pstring("OUT_") + pin;

	parser.register_dev(dname, dname);
	parser.register_link(dname + ".IN", pin);
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
	parser.register_link(dname + ".IN", pin);
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
	if (!parser.device_exists("STREAM_INPUT"))
		parser.register_dev("NETDEV_SOUND_IN", "STREAM_INPUT");

	pstring sparam = plib::pfmt("STREAM_INPUT.CHAN{1}")(m_channel);
	parser.register_param(sparam, pstring(m_param_name));
	sparam = plib::pfmt("STREAM_INPUT.MULT{1}")(m_channel);
	parser.register_param_val(sparam, m_mult);
	sparam = plib::pfmt("STREAM_INPUT.OFFSET{1}")(m_channel);
	parser.register_param_val(sparam, m_offset);
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
	save_helper(device_t *dev, const pstring &prefix)
	: m_device(dev), m_prefix(prefix)
	{}

	template<typename T, typename X = void *>
	void save_item(T &&item, const pstring &name, X = nullptr)
	{
		m_device->save_item(item, (m_prefix + "_" + name).c_str());
	}

	template <typename X = void *>
	std::enable_if_t<plib::compile_info::has_int128::value && std::is_pointer<X>::value, void>
	save_item(INT128 &item, const pstring &name, X = nullptr)
	{
		auto *p = reinterpret_cast<std::uint64_t *>(&item);
		m_device->save_item(p[0], (m_prefix + "_" + name + "_1").c_str());
		m_device->save_item(p[1], (m_prefix + "_" + name + "_2").c_str());
	}

private:
	device_t *m_device;
	pstring m_prefix;
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
#if 0
	m_cur = 0.0;
	m_last_buffer_time = netlist::netlist_time_ext::zero();
#endif
}

void netlist_mame_stream_output_device::sound_update_fill(std::size_t samples, stream_sample_t *target)
{
	if (samples < m_buffer.size())
		throw emu_fatalerror("sound %s: samples %d less bufsize %d\n", name(), samples, m_buffer.size());

	std::copy(m_buffer.begin(), m_buffer.end(), target);
	std::size_t pos = m_buffer.size();
	while (pos < samples)
	{
		target[pos++] = m_cur;
	}
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
	parser.register_link(dname + ".IN", pstring(m_out_name));
}

void netlist_mame_stream_output_device::process(netlist::netlist_time_ext tim, netlist::nl_fptype val)
{
	val = val * m_mult + m_offset;

	int pos = (tim - m_last_buffer_time) / m_sample_time;
	//if (pos > m_bufsize)
	//  throw emu_fatalerror("sound %s: pos %d exceeded bufsize %d\n", name().c_str(), pos, m_bufsize);
	while (m_buffer.size() < pos )
	{
		m_buffer.push_back(static_cast<stream_sample_t>(m_cur));
	}

	// clamp to avoid spikes, but not too hard, as downstream processing, volume
	// controls, etc may bring values above 32767 back down in range; some clamping
	// is still useful, however, as the mixing is done with integral values
	if (plib::abs(val) < 32767.0*256.0)
		m_cur = val;
	else if (val > 0.0)
		m_cur = 32767.0*256.0;
	else
		m_cur = -32767.0*256.0;

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

std::unique_ptr<netlist::netlist_state_t> netlist_mame_device::base_validity_check(validity_checker &valid) const
{
	try
	{
		plib::chrono::timer<plib::chrono::system_ticks> t;
		t.start();
		auto lnetlist = std::make_unique<netlist::netlist_state_t>("netlist",
			plib::make_unique<netlist_validate_callbacks_t, netlist::host_arena>());
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

	// FIXME: use save_helper
	m_netlist->save(*this, m_rem, pstring(this->name()), "m_rem");
	m_netlist->save(*this, m_div, pstring(this->name()), "m_div");
	m_netlist->save(*this, m_old, pstring(this->name()), "m_old");

	save_state();

	m_old = netlist::netlist_time_ext::zero();
	m_rem = netlist::netlist_time_ext::zero();
	m_cur_time = attotime::zero;

	m_device_reset_called = false;

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
		m_netlist->exec().stop();
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
			else if (plib::compile_info::has_int128::value && s->dt().size() == sizeof(INT128))
				save_pointer((int64_t *) s->ptr(), s->name().c_str(), s->count() * 2);
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


// Fixes overflow error in device_pseudo_state_register
template<>
class device_pseudo_state_register<double> : public device_state_entry
{
public:
	typedef typename std::function<double ()> getter_func;
	typedef typename std::function<void (double)> setter_func;

	// construction/destruction
	device_pseudo_state_register(int index, const char *symbol, getter_func &&getter, setter_func &&setter, device_state_interface *dev)
		: device_state_entry(index, symbol, sizeof(double), ~u64(0), DSF_FLOATING_POINT, dev),
			m_getter(std::move(getter)),
			m_setter(std::move(setter))
	{
	}

protected:
	// device_state_entry overrides
	virtual u64 entry_value() const override { return u64(m_getter()); }
	virtual void entry_set_value(u64 value) const override { m_setter(double(value)); }
	virtual double entry_dvalue() const override { return m_getter(); }
	virtual void entry_set_dvalue(double value) const override { m_setter(value); }

private:
	getter_func             m_getter;               // function to retrieve the data
	setter_func             m_setter;               // function to store the data
};


void netlist_mame_cpu_device::device_start()
{
	netlist_mame_device::device_start();

	// State support

	state_add(STATE_GENPC, "GENPC", m_genPC).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_genPC).noshow();

	int index = 0;
	for (auto &n : netlist().nets())
	{
		pstring name = n->name(); //plib::replace_all(n->name(), ".", "_");
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
			state_add(std::make_unique<device_pseudo_state_register<double>>(
				index++,
				name.c_str(),
				[nl]() { return nl->Q_Analog(); },
				[nl](double data) { nl->set_Q_Analog(data); },
				this));
		}
	}

	// set our instruction counter
	set_icountptr(m_icount);
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
		if (m_out.size() == 0)
			osd_printf_error("No output devices\n");
		else
		{
			for (auto &outdev : m_out)
			{
				if (outdev.first < 0 || outdev.first >= m_out.size())
					osd_printf_error("illegal channel number %d\n", outdev.first);
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

	if (m_out.size() == 0)
		fatalerror("No output devices");

	//m_num_outputs = outdevs.size();

	/* resort channels */
	for (auto &outdev : m_out)
	{
		if (outdev.first < 0 || outdev.first >= m_out.size())
			fatalerror("illegal channel number %d", outdev.first);
		outdev.second->set_sample_time(netlist::netlist_time::from_hz(clock()));
		outdev.second->buffer_reset(netlist::netlist_time_ext::zero());
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
		const auto sample_time = netlist::netlist_time::from_raw(static_cast<netlist::netlist_time::internal_type>(nltime_from_attotime(clocks_to_attotime(1)).as_raw()));
		m_in->resolve_params(sample_time);
	}

	/* initialize the stream(s) */
	m_is_device_call = false;
	m_stream = machine().sound().stream_alloc(*this, m_in ? m_in->num_channels() : 0, m_out.size(), clock());
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

void netlist_mame_sound_device::device_clock_changed()
{
	netlist_mame_device::device_clock_changed();

	for (auto &e : m_out)
	{
		e.second->set_sample_time(nltime_from_clocks(1));
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
		auto sample_time = netlist::netlist_time::from_raw(static_cast<netlist::netlist_time::internal_type>(nltime_from_attotime(m_attotime_per_clock).as_raw()));
		m_in->buffer_reset(sample_time, samples, inputs);
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
