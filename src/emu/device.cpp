// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    device.cpp

    Device interface functions.

***************************************************************************/

#include "emu.h"
#include "romload.h"
#include "speaker.h"
#include "debug/debugcpu.h"

#include <cstring>


//**************************************************************************
//  DEVICE TYPE REGISTRATION
//**************************************************************************

namespace emu::detail {

namespace {

struct device_registrations
{
	device_type_impl_base *first = nullptr;
	device_type_impl_base *last = nullptr;
	device_type_impl_base *unsorted = nullptr;
};

device_registrations &device_registration_data()
{
	// this is necessary to avoid issues with static initialisation order across units being indeterminate
	// thread safety issues are avoided by always calling this function during static initialisation before the app can go threaded
	static device_registrations instance;
	return instance;
}

} // anonymous namespace


device_registrar::const_iterator device_registrar::cbegin() const
{
	return const_iterator_helper(device_registration_data().first);
}


device_registrar::const_iterator device_registrar::cend() const
{
	return const_iterator_helper(nullptr);
}


device_type_impl_base *device_registrar::register_device(device_type_impl_base &type)
{
	device_registrations &data(device_registration_data());

	if (!data.first) data.first = &type;
	if (data.last) data.last->m_next = &type;
	if (!data.unsorted) data.unsorted = &type;
	data.last = &type;

	return nullptr;
}

} // namespace emu::detail

emu::detail::device_registrar const registered_device_types;



//**************************************************************************
//  LIVE DEVICE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  device_t - constructor for a new
//  running device; initial state is derived
//  from the provided config
//-------------------------------------------------

device_t::device_t(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: m_type(type)
	, m_owner(owner)
	, m_next(nullptr)

	, m_configured_clock(clock)
	, m_unscaled_clock(clock)
	, m_clock(clock)
	, m_clock_scale(1.0)
	, m_attoseconds_per_clock((clock == 0) ? 0 : HZ_TO_ATTOSECONDS(clock))

	, m_machine_config(mconfig)
	, m_input_defaults(nullptr)
	, m_system_bios(0)
	, m_default_bios(0)
	, m_default_bios_tag("")

	, m_machine(nullptr)
	, m_save(nullptr)
	, m_basetag(tag)
	, m_config_complete(false)
	, m_started(false)
	, m_auto_finder_list(nullptr)
{
	if (owner != nullptr)
		m_tag.assign((owner->owner() == nullptr) ? "" : owner->tag()).append(":").append(tag);
	else
		m_tag.assign(":");
	set_clock(clock);
}


//-------------------------------------------------
//  ~device_t - destructor for a device_t
//-------------------------------------------------

device_t::~device_t()
{
}


//-------------------------------------------------
//  searchpath - get the media search path for a
//  device
//-------------------------------------------------

std::vector<std::string> device_t::searchpath() const
{
	std::vector<std::string> result;
	device_t const *system(owner());
	while (system && !dynamic_cast<driver_device const *>(system))
		system = system->owner();
	if (system)
		result = system->searchpath();
	if (type().parent_rom_device_type())
		result.emplace(result.begin(), type().parent_rom_device_type()->shortname());
	result.emplace(result.begin(), shortname());
	return result;
}


//-------------------------------------------------
//  memregion - return a pointer to the region
//  info for a given region
//-------------------------------------------------

memory_region *device_t::memregion(std::string_view tag) const
{
	// build a fully-qualified name and look it up
	auto search = machine().memory().regions().find(subtag(tag));
	if (search != machine().memory().regions().end())
		return search->second.get();
	else
		return nullptr;
}


//-------------------------------------------------
//  memshare - return a pointer to the memory share
//  info for a given share
//-------------------------------------------------

memory_share *device_t::memshare(std::string_view tag) const
{
	// build a fully-qualified name and look it up
	auto search = machine().memory().shares().find(subtag(tag));
	if (search != machine().memory().shares().end())
		return search->second.get();
	else
		return nullptr;
}


//-------------------------------------------------
//  membank - return a pointer to the memory
//  bank info for a given bank
//-------------------------------------------------

memory_bank *device_t::membank(std::string_view tag) const
{
	auto search = machine().memory().banks().find(subtag(tag));
	if (search != machine().memory().banks().end())
		return search->second.get();
	else
		return nullptr;
}


//-------------------------------------------------
//  ioport - return a pointer to the I/O port
//  object for a given port name
//-------------------------------------------------

ioport_port *device_t::ioport(std::string_view tag) const
{
	// build a fully-qualified name and look it up
	return machine().ioport().port(subtag(tag));
}


//-------------------------------------------------
//  parameter - return a pointer to a given
//  parameter
//-------------------------------------------------

std::string device_t::parameter(std::string_view tag) const
{
	// build a fully-qualified name and look it up
	return machine().parameters().lookup(subtag(tag));
}


//-------------------------------------------------
//  add_machine_configuration - add device-
//  specific machine configuration
//-------------------------------------------------

void device_t::add_machine_configuration(machine_config &config)
{
	assert(&config == &m_machine_config);
	machine_config::token const tok(config.begin_configuration(*this));
	device_add_mconfig(config);
	for (auto *autodev = m_auto_finder_list; autodev; autodev = autodev->next())
		autodev->end_configuration();
}


//-------------------------------------------------
//  set_clock - set/change the clock on
//  a device
//-------------------------------------------------

void device_t::set_clock(u32 clock)
{
	m_configured_clock = clock;

	// derive the clock from our owner if requested
	if ((clock & 0xff000000) == 0xff000000)
		calculate_derived_clock();
	else
		set_unscaled_clock(clock);
}


//-------------------------------------------------
//  config_complete - called when the
//  configuration of a device is complete
//-------------------------------------------------

void device_t::config_complete()
{
	// resolve default BIOS
	tiny_rom_entry const *const roms(rom_region());
	if (roms)
	{
		// first pass: try to find default BIOS from ROM region or machine configuration
		char const *defbios(m_default_bios_tag.empty() ? nullptr : m_default_bios_tag.c_str());
		bool twopass(false), havebios(false);
		u8 firstbios(0);
		for (const tiny_rom_entry *rom = roms; !m_default_bios && !ROMENTRY_ISEND(rom); ++rom)
		{
			if (ROMENTRY_ISSYSTEM_BIOS(rom))
			{
				if (!havebios)
				{
					havebios = true;
					firstbios = ROM_GETBIOSFLAGS(rom);
				}
				if (!defbios)
					twopass = true;
				else if (!std::strcmp(rom->name, defbios))
					m_default_bios = ROM_GETBIOSFLAGS(rom);
			}
			else if (!defbios && ROMENTRY_ISDEFAULT_BIOS(rom))
			{
				defbios = rom->name;
			}
		}

		// second pass is needed if default BIOS came after one or more system BIOSes
		if (havebios && !m_default_bios)
		{
			if (defbios && twopass)
			{
				for (const tiny_rom_entry *rom = roms; !m_default_bios && !ROMENTRY_ISEND(rom); ++rom)
				{
					if (ROMENTRY_ISSYSTEM_BIOS(rom) && !std::strcmp(rom->name, defbios))
						m_default_bios = ROM_GETBIOSFLAGS(rom);
				}
			}

			// no default BIOS declared but at least one system BIOS declared
			if (!m_default_bios)
				m_default_bios = firstbios;
		}

		// set system BIOS to the default unless something overrides it
		set_system_bios(m_default_bios);
	}

	// notify the interfaces
	for (device_interface &intf : interfaces())
		intf.interface_config_complete();

	// then notify the device itself
	device_config_complete();

	// then mark ourselves complete
	m_config_complete = true;
}


//-------------------------------------------------
//  validity_check - validate a device after the
//  configuration has been constructed
//-------------------------------------------------

void device_t::validity_check(validity_checker &valid) const
{
	// validate mixins
	for (device_interface &intf : interfaces())
		intf.interface_validity_check(valid);

	// let the device itself validate
	device_validity_check(valid);
}


//-------------------------------------------------
//  reset - reset a device
//-------------------------------------------------

void device_t::reset()
{
	// let the interfaces do their pre-work
	for (device_interface &intf : interfaces())
		intf.interface_pre_reset();

	// reset the device
	device_reset();

	// reset all child devices
	for (device_t &child : subdevices())
		child.reset();

	// now allow for some post-child reset action
	device_reset_after_children();

	// let the interfaces do their post-work
	for (device_interface &intf : interfaces())
		intf.interface_post_reset();
}


//-------------------------------------------------
//  set_unscaled_clock - sets the given device's
//  unscaled clock
//-------------------------------------------------

void device_t::set_unscaled_clock(u32 clock, bool sync_on_new_clock_domain)
{
	// do nothing if no actual change
	if (clock == m_unscaled_clock)
		return;

	m_unscaled_clock = clock;
	m_clock = m_unscaled_clock * m_clock_scale + 0.5;
	m_attoseconds_per_clock = (m_clock == 0) ? 0 : HZ_TO_ATTOSECONDS(m_clock);

	// recalculate all derived clocks
	for (device_t &child : subdevices())
		child.calculate_derived_clock();

	// if the device has already started, make sure it knows about the new clock
	if (m_started)
		notify_clock_changed(sync_on_new_clock_domain);
}


//-------------------------------------------------
//  set_clock_scale - sets a scale factor for the
//  device's clock
//-------------------------------------------------

void device_t::set_clock_scale(double clockscale)
{
	// do nothing if no actual change
	if (clockscale == m_clock_scale)
		return;

	m_clock_scale = clockscale;
	m_clock = m_unscaled_clock * m_clock_scale + 0.5;
	m_attoseconds_per_clock = (m_clock == 0) ? 0 : HZ_TO_ATTOSECONDS(m_clock);

	// recalculate all derived clocks
	for (device_t &child : subdevices())
		child.calculate_derived_clock();

	// if the device has already started, make sure it knows about the new clock
	if (m_started)
		notify_clock_changed();
}


//-------------------------------------------------
//  calculate_derived_clock - derive the device's
//  clock from its owner, if so configured
//-------------------------------------------------

void device_t::calculate_derived_clock()
{
	if ((m_configured_clock & 0xff000000) == 0xff000000)
	{
		assert(m_owner != nullptr);
		set_unscaled_clock(m_owner->m_clock * ((m_configured_clock >> 12) & 0xfff) / ((m_configured_clock >> 0) & 0xfff));
	}
}


//-------------------------------------------------
//  clocks_to_attotime - converts a number of
//  clock ticks to an attotime
//-------------------------------------------------

attotime device_t::clocks_to_attotime(u64 numclocks) const noexcept
{
	if (m_clock == 0)
		return attotime::never;
	else if (numclocks < m_clock)
		return attotime(0, numclocks * m_attoseconds_per_clock);
	else
	{
		u32 remainder;
		u32 quotient = divu_64x32_rem(numclocks, m_clock, remainder);
		return attotime(quotient, u64(remainder) * u64(m_attoseconds_per_clock));
	}
}


//-------------------------------------------------
//  attotime_to_clocks - converts a duration as
//  attotime to CPU clock ticks
//-------------------------------------------------

u64 device_t::attotime_to_clocks(const attotime &duration) const noexcept
{
	if (m_clock == 0)
		return 0;
	else
		return mulu_32x32(duration.seconds(), m_clock) + u64(duration.attoseconds()) / u64(m_attoseconds_per_clock);
}


//-------------------------------------------------
//  set_machine - notify that the machine now
//  exists
//-------------------------------------------------

void device_t::set_machine(running_machine &machine)
{
	m_machine = &machine;
	m_save = &machine.save();
}

//-------------------------------------------------
//  findit - search for all objects in auto finder
//  list and return status
//-------------------------------------------------

bool device_t::findit(validity_checker *valid) const
{
	bool allfound = true;
	for (auto *autodev = m_auto_finder_list; autodev; autodev = autodev->next())
	{
		if (!autodev->findit(valid))
			allfound = false;
	}
	return allfound;
}


//-------------------------------------------------
//  resolve_pre_map - find objects that may be used
//  in memory maps
//-------------------------------------------------

void device_t::resolve_pre_map()
{
	// prepare the logerror buffer
	if (m_machine->allow_logging())
		m_string_buffer.reserve(1024);
}


//-------------------------------------------------
//  resolve - find objects
//-------------------------------------------------

void device_t::resolve_post_map()
{
	// find all the registered post-map objects
	if (!findit(nullptr))
		throw emu_fatalerror("Missing some required objects, unable to proceed");

	// allow implementation to do additional setup
	device_resolve_objects();
}


//-------------------------------------------------
//  view_register - register a view for future state saving
//-------------------------------------------------

void device_t::view_register(memory_view *view)
{
	m_viewlist.push_back(view);
}


//-------------------------------------------------
//  start - start a device
//-------------------------------------------------

void device_t::start()
{
	// prepare the logerror buffer
	if (m_machine->allow_logging())
		m_string_buffer.reserve(1024);

	// let the interfaces do their pre-work
	for (device_interface &intf : interfaces())
		intf.interface_pre_start();

	// remember the number of state registrations
	int state_registrations = machine().save().registration_count();

	// start the device
	device_start();

	// complain if nothing was registered by the device
	state_registrations = machine().save().registration_count() - state_registrations;
	device_execute_interface *exec;
	device_sound_interface *sound;
	if (state_registrations == 0 && (interface(exec) || interface(sound)) && type() != SPEAKER && type() != MICROPHONE)
	{
		logerror("Device did not register any state to save!\n");
		if ((machine().system().flags & MACHINE_SUPPORTS_SAVE) != 0)
			fatalerror("Device '%s' did not register any state to save!\n", tag());
	}

	// let the interfaces do their post-work
	for (device_interface &intf : interfaces())
		intf.interface_post_start();

	// force an update of the clock
	notify_clock_changed();

	// if we're debugging, create a device_debug object
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		m_debug = std::make_unique<device_debug>(*this);
		debug_setup();
	}

	// register our save states
	save_item(NAME(m_unscaled_clock));
	save_item(NAME(m_clock_scale));

	// have the views register their state
	if (!m_viewlist.empty())
		osd_printf_verbose("%s: Registering %d views\n", m_tag, int(m_viewlist.size()));
	for (memory_view *view : m_viewlist)
		view->register_state();

	// we're now officially started
	m_started = true;
}


//-------------------------------------------------
//  stop - stop a device
//-------------------------------------------------

void device_t::stop()
{
	// let the interfaces do their pre-work
	for (device_interface &intf : interfaces())
		intf.interface_pre_stop();

	// stop the device
	device_stop();

	// let the interfaces do their post-work
	for (device_interface &intf : interfaces())
		intf.interface_post_stop();

	// free any debugging info
	m_debug.reset();

	// we're now officially stopped, and the machine is off-limits
	m_started = false;
	m_machine = nullptr;
}


//-------------------------------------------------
//  debug_setup - set up for debugging
//-------------------------------------------------

void device_t::debug_setup()
{
	// notify the interface
	for (device_interface &intf : interfaces())
		intf.interface_debug_setup();

	// notify the device
	device_debug_setup();
}


//-------------------------------------------------
//  pre_save - tell the device and its interfaces
//  that we are about to save
//-------------------------------------------------

void device_t::pre_save()
{
	// notify the interface
	for (device_interface &intf : interfaces())
		intf.interface_pre_save();

	// notify the device
	device_pre_save();
}


//-------------------------------------------------
//  post_load - tell the device and its interfaces
//  that we just completed a load
//-------------------------------------------------

void device_t::post_load()
{
	// recompute clock-related parameters if something changed
	u32 const scaled_clock = m_unscaled_clock * m_clock_scale + 0.5;
	if (m_clock != scaled_clock)
	{
		m_clock = scaled_clock;
		m_attoseconds_per_clock = (scaled_clock == 0) ? 0 : HZ_TO_ATTOSECONDS(scaled_clock);

		// recalculate all derived clocks
		for (device_t &child : subdevices())
			child.calculate_derived_clock();

		// make sure the device knows about the new clock
		notify_clock_changed();
	}

	// notify the interface
	for (device_interface &intf : interfaces())
		intf.interface_post_load();

	// notify the device
	device_post_load();
}


//-------------------------------------------------
//  notify_clock_changed - notify all interfaces
//  that the clock has changed
//-------------------------------------------------

void device_t::notify_clock_changed(bool sync_on_new_clock_domain)
{
	// first notify interfaces
	for (device_interface &intf : interfaces())
		intf.interface_clock_changed(sync_on_new_clock_domain);

	// then notify the device
	device_clock_changed();
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void device_t::device_config_complete()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_validity_check - validate a device after
//  the configuration has been constructed
//-------------------------------------------------

void device_t::device_validity_check(validity_checker &valid) const
{
	// do nothing by default
}


//-------------------------------------------------
//  rom_region - return a pointer to the implicit
//  rom region description for this device
//-------------------------------------------------

const tiny_rom_entry *device_t::device_rom_region() const
{
	// none by default
	return nullptr;
}


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void device_t::device_add_mconfig(machine_config &config)
{
	// do nothing by default
}


//-------------------------------------------------
//  input_ports - return a pointer to the implicit
//  input ports description for this device
//-------------------------------------------------

ioport_constructor device_t::device_input_ports() const
{
	// none by default
	return nullptr;
}


//-------------------------------------------------
//  device_reset - actually handle resetting of
//  a device; designed to be overridden by the
//  actual device implementation
//-------------------------------------------------

void device_t::device_reset()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_reset_after_children - hook to do
//  reset logic that must happen after the children
//  are reset; designed to be overridden by the
//  actual device implementation
//-------------------------------------------------

void device_t::device_reset_after_children()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void device_t::device_resolve_objects()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_stop - clean up anything that needs to
//  happen before the running_machine goes away
//-------------------------------------------------

void device_t::device_stop()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_pre_save - called prior to saving the
//  state, so that registered variables can be
//  properly normalized
//-------------------------------------------------

void device_t::device_pre_save()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_post_load - called after loading a
//  saved state, so that registered variables can
//  be expanded as necessary
//-------------------------------------------------

void device_t::device_post_load()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_clock_changed - called when the
//  device clock is altered in any way; designed
//  to be overridden by the actual device
//  implementation
//-------------------------------------------------

void device_t::device_clock_changed()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_debug_setup - called when the debugger
//  is active to allow for device-specific setup
//-------------------------------------------------

void device_t::device_debug_setup()
{
	// do nothing by default
}


//-------------------------------------------------
//  subdevice_slow - perform a slow name lookup,
//  caching the results
//-------------------------------------------------

device_t *device_t::subdevice_slow(std::string_view tag) const
{
	// resolve the full path
	std::string fulltag = subtag(tag);

	// we presume the result is a rooted path; also doubled colons mess up our
	// tree walk, so catch them early
	assert(fulltag[0] == ':');
	assert(fulltag.find("::") == std::string::npos);

	// walk the device list to the final path
	device_t *curdevice = &mconfig().root_device();
	std::string_view part(std::string_view(fulltag).substr(1));
	while (!part.empty() && curdevice != nullptr)
	{
		std::string_view::size_type end = part.find_first_of(':');
		if (end == std::string::npos)
		{
			curdevice = curdevice->subdevices().find(part);
			part = std::string_view();
		}
		else
		{
			curdevice = curdevice->subdevices().find(part.substr(0, end));
			part.remove_prefix(end + 1);
		}
	}

	return curdevice;
}


//-------------------------------------------------
//  subtag - create a fully resolved path relative
//  to our device based on the provided tag
//-------------------------------------------------

std::string device_t::subtag(std::string_view tag) const
{
	std::string result;
	if (!tag.empty() && (tag[0] == ':'))
	{
		// if the tag begins with a colon, ignore our path and start from the root
		tag.remove_prefix(1);
		result.assign(":");
	}
	else
	{
		// otherwise, start with our path
		result.assign(m_tag);
		if (result != ":")
			result.append(1, ':');
	}

	// iterate over the tag, look for special path characters to resolve
	std::string_view::size_type delimiter;
	while ((delimiter = tag.find_first_of("^:")) != std::string_view::npos)
	{
		// copy everything up to there
		bool const parent = tag[delimiter] == '^';
		result.append(tag, 0, delimiter);
		tag.remove_prefix(delimiter + 1);

		if (parent)
		{
			// strip trailing colons
			std::string::size_type len = result.length();
			while ((len > 1) && (result[--len] == ':'))
				result.resize(len);

			// remove the last path part, leaving the last colon
			if (result != ":")
			{
				std::string::size_type lastcolon = result.find_last_of(':');
				if (lastcolon != std::string::npos)
					result.resize(lastcolon + 1);
			}
		}
		else
		{
			// collapse successive colons
			if (result.back() != ':')
				result.append(1, ':');
			delimiter = tag.find_first_not_of(':');
			if (delimiter != std::string_view::npos)
				tag.remove_prefix(delimiter);
		}
	}

	// copy everything else
	result.append(tag);

	// strip trailing colons up to the root
	std::string::size_type len = result.length();
	while ((len > 1) && (result[--len] == ':'))
		result.resize(len);
	return result;
}


//-------------------------------------------------
//  append - add a new subdevice to the list
//-------------------------------------------------

device_t &device_t::subdevice_list::append(std::unique_ptr<device_t> &&device)
{
	device_t &result(m_list.append(*device.release()));
	m_tagmap.emplace(result.m_basetag, std::ref(result));
	return result;
}


//-------------------------------------------------
//  replace_and_remove - add a new device to
//  replace an existing subdevice
//-------------------------------------------------

device_t &device_t::subdevice_list::replace_and_remove(std::unique_ptr<device_t> &&device, device_t &existing)
{
	m_tagmap.erase(existing.m_basetag);
	device_t &result(m_list.replace_and_remove(*device.release(), existing));
	m_tagmap.emplace(result.m_basetag, std::ref(result));
	return result;
}


//-------------------------------------------------
//  remove - remove a subdevice from the list
//-------------------------------------------------

void device_t::subdevice_list::remove(device_t &device)
{
	m_tagmap.erase(device.m_basetag);
	m_list.remove(device);
}


//-------------------------------------------------
//  register_auto_finder - add a new item to the
//  list of stuff to find after we go live
//-------------------------------------------------

device_resolver_base *device_t::register_auto_finder(device_resolver_base &autodev)
{
	return std::exchange(m_auto_finder_list, &autodev);
}


//**************************************************************************
//  LIVE DEVICE INTERFACES
//**************************************************************************

//-------------------------------------------------
//  device_interface - constructor
//-------------------------------------------------

device_interface::device_interface(device_t &device, const char *type)
	: m_interface_next(nullptr),
		m_device(device),
		m_type(type)
{
	device_interface **tailptr;
	for (tailptr = &device.interfaces().m_head; *tailptr != nullptr; tailptr = &(*tailptr)->m_interface_next) { }
	*tailptr = this;
}


//-------------------------------------------------
//  ~device_interface - destructor
//-------------------------------------------------

device_interface::~device_interface()
{
}


//-------------------------------------------------
//  interface_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void device_interface::interface_config_complete()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_validity_check - default validation
//  for a device after the configuration has been
//  constructed
//-------------------------------------------------

void device_interface::interface_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  interface_pre_start - called before the
//  device's own start function
//-------------------------------------------------

void device_interface::interface_pre_start()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_post_start - called after the
//  device's own start function
//-------------------------------------------------

void device_interface::interface_post_start()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_pre_reset - called before the
//  device's own reset function
//-------------------------------------------------

void device_interface::interface_pre_reset()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_post_reset - called after the
//  device's own reset function
//-------------------------------------------------

void device_interface::interface_post_reset()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_pre_stop - called before the
//  device's own stop function
//-------------------------------------------------

void device_interface::interface_pre_stop()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_post_stop - called after the
//  device's own stop function
//-------------------------------------------------

void device_interface::interface_post_stop()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_pre_save - called prior to saving the
//  state, so that registered variables can be
//  properly normalized
//-------------------------------------------------

void device_interface::interface_pre_save()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_post_load - called after loading a
//  saved state, so that registered variables can
//  be expanded as necessary
//-------------------------------------------------

void device_interface::interface_post_load()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_clock_changed - called when the
//  device clock is altered in any way; designed
//  to be overridden by the actual device
//  implementation
//-------------------------------------------------

void device_interface::interface_clock_changed(bool sync_on_new_clock_domain)
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_debug_setup - called to allow
//  interfaces to set up any debugging for this
//  device
//-------------------------------------------------

void device_interface::interface_debug_setup()
{
	// do nothing by default
}


//-------------------------------------------------
// rom_region_vector
//-------------------------------------------------

const std::vector<rom_entry> &device_t::rom_region_vector() const
{
	if (m_rom_entries.empty())
	{
		m_rom_entries = rom_build_entries(device_rom_region());
	}
	return m_rom_entries;
}
