/***************************************************************************

    devintrf.c

    Device interface functions.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "debug/debugcpu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define TEMP_STRING_POOL_ENTRIES		16
#define MAX_STRING_LENGTH				256



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

static char temp_string_pool[TEMP_STRING_POOL_ENTRIES][MAX_STRING_LENGTH];
static int temp_string_pool_index;



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  get_temp_string_buffer - return a pointer to
//  a temporary string buffer
//-------------------------------------------------

char *get_temp_string_buffer(void)
{
	char *string = &temp_string_pool[temp_string_pool_index++ % TEMP_STRING_POOL_ENTRIES][0];
	string[0] = 0;
	return string;
}


resource_pool &machine_get_pool(running_machine &machine)
{
	// temporary to get around include dependencies, until CPUs
	// get a proper device class
	return machine.m_respool;
}



//**************************************************************************
//  DEVICE LIST MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  device_list - device list constructor
//-------------------------------------------------

device_list::device_list(resource_pool &pool)
	: tagged_device_list<device_t>(pool),
	  m_machine(NULL)
{
}


//-------------------------------------------------
//  import_config_list - import a list of device
//  configs and allocate new devices
//-------------------------------------------------

void device_list::import_config_list(const device_config_list &list, running_machine &machine)
{
	// remember the machine for later use
	m_machine = &machine;

	// append each device from the configuration list
	for (const device_config *devconfig = list.first(); devconfig != NULL; devconfig = devconfig->next())
	{
		device_t *newdevice = devconfig->alloc_device(*m_machine);
		append(devconfig->tag(), newdevice);
		newdevice->find_interfaces();
	}
}


//-------------------------------------------------
//  start_all - start all the devices in the
//  list
//-------------------------------------------------

void device_list::start_all()
{
	// add exit and reset callbacks
	assert(m_machine != NULL);
	m_machine->add_notifier(MACHINE_NOTIFY_RESET, static_reset);
	m_machine->add_notifier(MACHINE_NOTIFY_EXIT, static_exit);

	// add pre-save and post-load callbacks
	state_save_register_presave(m_machine, static_pre_save, this);
	state_save_register_postload(m_machine, static_post_load, this);

	// iterate over devices to start them
	device_t *nextdevice;
	for (device_t *device = first(); device != NULL; device = nextdevice)
	{
		// attempt to start the device, catching any expected exceptions
		nextdevice = device->next();
		try
		{
			mame_printf_verbose("Starting %s '%s'\n", device->name(), device->tag());
			device->start();
		}
		
		// handle missing dependencies by moving the device to the end
		catch (device_missing_dependencies &)
		{
			// if we're the end, fail
			mame_printf_verbose("  (missing dependencies; rescheduling)\n");
			if (nextdevice == NULL)
				throw emu_fatalerror("Circular dependency in device startup; unable to start %s '%s'\n", device->name(), device->tag());
			detach(device);
			append(device->tag(), device);
		}
	}
}


//-------------------------------------------------
//  reset_all - reset all devices in the list
//-------------------------------------------------

void device_list::reset_all()
{
	// iterate over devices and stop them
	for (device_t *device = first(); device != NULL; device = device->next())
		device->reset();
}


void device_list::static_reset(running_machine &machine)
{
	machine.m_devicelist.reset_all();
}


//-------------------------------------------------
//  static_exit - tear down all the devices
//-------------------------------------------------

void device_list::static_exit(running_machine &machine)
{
	// first let the debugger save comments
	if ((machine.debug_flags & DEBUG_FLAG_ENABLED) != 0)
		debug_comment_save(&machine);
	
	// then nuke the devices
	machine.m_devicelist.reset();
}


//-------------------------------------------------
//  static_pre_save - tell all the devices we are
//  about to save
//-------------------------------------------------

void device_list::static_pre_save(running_machine *machine, void *param)
{
	device_list *list = reinterpret_cast<device_list *>(param);
	for (device_t *device = list->first(); device != NULL; device = device->next())
		device->pre_save();
}


//-------------------------------------------------
//  static_post_load - tell all the devices we just
//  completed a load
//-------------------------------------------------

void device_list::static_post_load(running_machine *machine, void *param)
{
	device_list *list = reinterpret_cast<device_list *>(param);
	for (device_t *device = list->first(); device != NULL; device = device->next())
		device->post_load();
}



//**************************************************************************
//  DEVICE INTERFACE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  device_config_interface - constructor
//-------------------------------------------------

device_config_interface::device_config_interface(const machine_config &mconfig, device_config &devconfig)
	: m_device_config(devconfig),
	  m_machine_config(mconfig),
	  m_interface_next(NULL)
{
	device_config_interface **tailptr;
	for (tailptr = &devconfig.m_interface_list; *tailptr != NULL; tailptr = &(*tailptr)->m_interface_next) ;
	*tailptr = this;
}


//-------------------------------------------------
//  ~device_config_interface - destructor
//-------------------------------------------------

device_config_interface::~device_config_interface()
{
}


//-------------------------------------------------
//  interface_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void device_config_interface::interface_config_complete()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_validity_check - default validation
//  for a device after the configuration has been
//  constructed
//-------------------------------------------------

bool device_config_interface::interface_validity_check(const game_driver &driver) const
{
	return false;
}



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  device_config - constructor for a new
//  device configuration
//-------------------------------------------------

device_config::device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock)
	: m_next(NULL),
	  m_owner(const_cast<device_config *>(owner)),
	  m_interface_list(NULL),
	  m_type(type),
	  m_clock(clock),
	  m_machine_config(mconfig),
	  m_static_config(NULL),
	  m_name(name),
	  m_tag(tag),
	  m_config_complete(false)
{
	// derive the clock from our owner if requested
	if ((m_clock & 0xff000000) == 0xff000000)
	{
		assert(m_owner != NULL);
		m_clock = m_owner->m_clock * ((m_clock >> 12) & 0xfff) / ((m_clock >> 0) & 0xfff);
	}
}


//-------------------------------------------------
//  ~device_config - destructor
//-------------------------------------------------

device_config::~device_config()
{
}


//-------------------------------------------------
//  config_complete - called when the
//  configuration of a device is complete
//-------------------------------------------------

void device_config::config_complete()
{
	// first notify the interfaces
	for (device_config_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_config_complete();

	// then notify the device itself
	device_config_complete();
}


//-------------------------------------------------
//  validity_check - validate a device after the
//  configuration has been constructed
//-------------------------------------------------

bool device_config::validity_check(const game_driver &driver) const
{
	bool error = false;

	// validate via the interfaces
	for (device_config_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		if (intf->interface_validity_check(driver))
			error = true;

	// let the device itself validate
	if (device_validity_check(driver))
		error = true;

	return error;
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void device_config::device_config_complete()
{
	// do nothing by default
}


//-------------------------------------------------
//  device_validity_check - validate a device after
//  the configuration has been constructed
//-------------------------------------------------

bool device_config::device_validity_check(const game_driver &driver) const
{
	// indicate no error by default
	return false;
}


//-------------------------------------------------
//  rom_region - return a pointer to the implicit
//  rom region description for this device
//-------------------------------------------------

const rom_entry *device_config::rom_region() const
{
	return NULL;
}


//-------------------------------------------------
//  machine_config - return a pointer to a machine
//  config constructor describing sub-devices for
//  this device
//-------------------------------------------------

machine_config_constructor device_config::machine_config_additions() const
{
	return NULL;
}



//**************************************************************************
//  LIVE DEVICE INTERFACES
//**************************************************************************

//-------------------------------------------------
//  device_interface - constructor
//-------------------------------------------------

device_interface::device_interface(running_machine &machine, const device_config &config, device_t &device)
	: m_interface_next(NULL),
	  m_device(device)
{
	device_interface **tailptr;
	for (tailptr = &device.m_interface_list; *tailptr != NULL; tailptr = &(*tailptr)->m_interface_next) ;
	*tailptr = this;
}


//-------------------------------------------------
//  ~device_interface - destructor
//-------------------------------------------------

device_interface::~device_interface()
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
//  interface_pre_save - called prior to saving the
//  state, so that registered variables can be
//  properly normalized
//-------------------------------------------------

void device_interface::interface_pre_save()
{
	// do nothing by default
}


//-------------------------------------------------
//  interface_post_load - called after the loading a
//  saved state, so that registered variables can
//  be expaneded as necessary
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

void device_interface::interface_clock_changed()
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



//**************************************************************************
//  LIVE DEVICE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  device_t - constructor for a new
//  running device; initial state is derived
//  from the provided config
//-------------------------------------------------

device_t::device_t(running_machine &_machine, const device_config &config)
	: machine(&_machine),
	  m_machine(_machine),
	  m_debug(NULL),
	  m_execute(NULL),
	  m_memory(NULL),
	  m_state(NULL),
	  m_next(NULL),
	  m_owner((config.m_owner != NULL) ? _machine.m_devicelist.find(config.m_owner->tag()) : NULL),
	  m_interface_list(NULL),
	  m_started(false),
	  m_clock(config.m_clock),
	  m_region(NULL),
	  m_baseconfig(config),
	  m_unscaled_clock(config.m_clock),
	  m_clock_scale(1.0),
	  m_attoseconds_per_clock((config.m_clock == 0) ? 0 : HZ_TO_ATTOSECONDS(config.m_clock))
{
}


//-------------------------------------------------
//  ~device_t - destructor for a device_t
//-------------------------------------------------

device_t::~device_t()
{
	auto_free(&m_machine, m_debug);
}


//-------------------------------------------------
//  subregion - return a pointer to the region
//  info for a given region
//-------------------------------------------------

const region_info *device_t::subregion(const char *_tag) const
{
	// safety first
	if (this == NULL)
		return NULL;

	// build a fully-qualified name
	astring tempstring;
	return m_machine.region(subtag(tempstring, _tag));
}


//-------------------------------------------------
//  subdevice - return a pointer to the given
//  device that is owned by us
//-------------------------------------------------

device_t *device_t::subdevice(const char *_tag) const
{
	// safety first
	if (this == NULL)
		return NULL;

	// build a fully-qualified name
	astring tempstring;
	return m_machine.device(subtag(tempstring, _tag));
}


//-------------------------------------------------
//  siblingdevice - return a pointer to the given
//  device that is owned by our same owner
//-------------------------------------------------

device_t *device_t::siblingdevice(const char *_tag) const
{
	// safety first
	if (this == NULL)
		return NULL;

	// build a fully-qualified name
	astring tempstring;
	return m_machine.device(siblingtag(tempstring, _tag));
}


//-------------------------------------------------
//  set_clock - sets the given device's raw clock
//-------------------------------------------------

void device_t::set_unscaled_clock(UINT32 clock)
{
	m_unscaled_clock = clock;
	m_clock = m_unscaled_clock * m_clock_scale;
	m_attoseconds_per_clock = HZ_TO_ATTOSECONDS(m_clock);
	notify_clock_changed();
}


//-------------------------------------------------
//  set_clockscale - sets a scale factor for the
//  device's clock
//-------------------------------------------------

void device_t::set_clock_scale(double clockscale)
{
	m_clock_scale = clockscale;
	m_clock = m_unscaled_clock * m_clock_scale;
	m_attoseconds_per_clock = HZ_TO_ATTOSECONDS(m_clock);
	notify_clock_changed();
}


//-------------------------------------------------
//  clocks_to_attotime - converts a number of
//  clock ticks to an attotime
//-------------------------------------------------

attotime device_t::clocks_to_attotime(UINT64 numclocks) const
{
	if (numclocks < m_clock)
		return attotime_make(0, numclocks * m_attoseconds_per_clock);
	else
	{
		UINT32 remainder;
		UINT32 quotient = divu_64x32_rem(numclocks, m_clock, &remainder);
		return attotime_make(quotient, (UINT64)remainder * (UINT64)m_attoseconds_per_clock);
	}
}


//-------------------------------------------------
//  attotime_to_clocks - converts a duration as
//  attotime to CPU clock ticks
//-------------------------------------------------

UINT64 device_t::attotime_to_clocks(attotime duration) const
{
	return mulu_32x32(duration.seconds, m_clock) + (UINT64)duration.attoseconds / (UINT64)m_attoseconds_per_clock;
}


//-------------------------------------------------
//  find_interfaces - locate fast interfaces
//-------------------------------------------------

void device_t::find_interfaces()
{
	// look up the common interfaces
	m_execute = dynamic_cast<device_execute_interface *>(this);
	m_memory = dynamic_cast<device_memory_interface *>(this);
	m_state = dynamic_cast<device_state_interface *>(this);
}


//-------------------------------------------------
//  start - start a device
//-------------------------------------------------

void device_t::start()
{
	// populate the region field
	m_region = m_machine.region(tag());

	// let the interfaces do their pre-work
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_pre_start();

	// remember the number of state registrations
	int state_registrations = state_save_get_reg_count(machine);

	// start the device
	device_start();

	// complain if nothing was registered by the device
	state_registrations = state_save_get_reg_count(machine) - state_registrations;
	device_execute_interface *exec;
	device_sound_interface *sound;
	if (state_registrations == 0 && (interface(exec) || interface(sound)))
	{
		logerror("Device '%s' did not register any state to save!\n", tag());
		if ((m_machine.gamedrv->flags & GAME_SUPPORTS_SAVE) != 0)
			fatalerror("Device '%s' did not register any state to save!", tag());
	}

	// let the interfaces do their post-work
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_post_start();

	// force an update of the clock
	notify_clock_changed();
	
	// if we're debugging, create a device_debug object
	if ((m_machine.debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		m_debug = auto_alloc(&m_machine, device_debug(*this));
		debug_setup();
	}

	// register our save states
	state_save_register_device_item(this, 0, m_clock);
	state_save_register_device_item(this, 0, m_unscaled_clock);
	state_save_register_device_item(this, 0, m_clock_scale);

	// we're now officially started
	m_started = true;
}


//-------------------------------------------------
//  debug_setup - set up for debugging
//-------------------------------------------------

void device_t::debug_setup()
{
	// notify the interface
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_debug_setup();

	// notify the device
	device_debug_setup();
}


//-------------------------------------------------
//  reset - reset a device
//-------------------------------------------------

void device_t::reset()
{
	// let the interfaces do their pre-work
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_pre_reset();

	// reset the device
	device_reset();

	// let the interfaces do their post-work
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_post_reset();
}


//-------------------------------------------------
//  pre_save - tell the device and its interfaces
//  that we are about to save
//-------------------------------------------------

void device_t::pre_save()
{
	// notify the interface
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_pre_save();

	// notify the device
	device_pre_save();
}


//-------------------------------------------------
//  post_load - tell the device and its interfaces
//  that we just completed a load
//-------------------------------------------------

void device_t::post_load()
{
	// notify the interface
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_post_load();

	// notify the device
	device_post_load();
}


//-------------------------------------------------
//  device_reset - actually handle resetting of
//  a device; designed to be overriden by the
//  actual device implementation
//-------------------------------------------------

void device_t::device_reset()
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
//  device_post_load - called after the loading a
//  saved state, so that registered variables can
//  be expaneded as necessary
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
//  notify_clock_changed - notify all interfaces
//  that the clock has changed
//-------------------------------------------------

void device_t::notify_clock_changed()
{
	// first notify interfaces
	for (device_interface *intf = m_interface_list; intf != NULL; intf = intf->interface_next())
		intf->interface_clock_changed();

	// then notify the device
	device_clock_changed();
}
