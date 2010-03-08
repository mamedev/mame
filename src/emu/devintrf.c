/***************************************************************************

    devintrf.c

    Device interface functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define TEMP_STRING_POOL_ENTRIES		16
#define MAX_STRING_LENGTH				256



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static char temp_string_pool[TEMP_STRING_POOL_ENTRIES][MAX_STRING_LENGTH];
static int temp_string_pool_index;



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_temp_string_buffer - return a pointer to
    a temporary string buffer
-------------------------------------------------*/

static char *get_temp_string_buffer(void)
{
	char *string = &temp_string_pool[temp_string_pool_index++ % TEMP_STRING_POOL_ENTRIES][0];
	string[0] = 0;
	return string;
}




/***************************************************************************
    LIVE DEVICE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    device_list - device list constructor
-------------------------------------------------*/

device_list::device_list()
	: machine(NULL)
{
}


/*-------------------------------------------------
    import_config_list - import a list of device
    configs and allocate new devices
-------------------------------------------------*/

void device_list::import_config_list(const device_config_list &list, running_machine &_machine)
{
	// remember the machine for later use
	machine = &_machine;

	// append each device from the configuration list
	for (const device_config *devconfig = list.first(); devconfig != NULL; devconfig = devconfig->next)
		append(devconfig->tag(), new running_device(_machine, *devconfig));
}


/*-------------------------------------------------
    start_all - start all the devices in the
    list
-------------------------------------------------*/

void device_list::start_all()
{
	// add exit and reset callbacks
	assert(machine != NULL);
	add_reset_callback(machine, static_reset);
	add_exit_callback(machine, static_stop);

	// iterate until we've started everything
	int devcount = count();
	int numstarted = 0;
	while (numstarted < devcount)
	{
		// iterate over devices and start them
		int prevstarted = numstarted;
		for (running_device *device = first(); device != NULL; device = device->next)
			if (!device->started)
			{
				// attempt to start the device, catching any expected exceptions
				try
				{
					device->start();
					numstarted++;
				}
				catch (device_missing_dependencies &)
				{
				}
			}

		// if we didn't start anything new, we're in trouble
		if (numstarted == prevstarted)
			fatalerror("Circular dependency in device startup; unable to start %d/%d devices\n", devcount - numstarted, devcount);
	}
}


/*-------------------------------------------------
    stop_all - stop all devices in the list
-------------------------------------------------*/

void device_list::stop_all()
{
	// iterate over devices and stop them
	for (running_device *device = first(); device != NULL; device = device->next)
		device->stop();
}


void device_list::static_stop(running_machine *machine)
{
	machine->devicelist.stop_all();
}


/*-------------------------------------------------
    reset_all - reset all devices in the list
-------------------------------------------------*/

void device_list::reset_all()
{
	/* iterate over devices and stop them */
	for (running_device *device = first(); device != NULL; device = device->next)
		device->reset();
}


void device_list::static_reset(running_machine *machine)
{
	machine->devicelist.reset_all();
}



/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    device_config - constructor for a new
    device configuration
-------------------------------------------------*/

device_config::device_config(const device_config *_owner, device_type _type, const char *_tag, UINT32 _clock)
	: next(NULL),
	  owner(const_cast<device_config *>(_owner)),
	  type(_type),
	  devclass(DEVICE_CLASS_GENERAL),
	  clock(_clock),
	  static_config(NULL),
	  inline_config(NULL),
	  m_tag(_tag)
{
	// initialize remaining members
	memset(address_map, 0, sizeof(address_map));
	devclass = (device_class)get_config_int(DEVINFO_INT_CLASS);

	// derive the clock from our owner if requested
	if ((clock & 0xff000000) == 0xff000000)
	{
		assert(owner != NULL);
		clock = owner->clock * ((clock >> 12) & 0xfff) / ((clock >> 0) & 0xfff);
	}

	// allocate a buffer for the inline configuration
	UINT32 configlen = (UINT32)get_config_int(DEVINFO_INT_INLINE_CONFIG_BYTES);
	inline_config = (configlen == 0) ? NULL : global_alloc_array_clear(UINT8, configlen);
}


/*-------------------------------------------------
    ~device_config - destructor
-------------------------------------------------*/

device_config::~device_config()
{
	// call the custom config free function first
	device_custom_config_func custom = reinterpret_cast<device_custom_config_func>(get_config_fct(DEVINFO_FCT_CUSTOM_CONFIG));
	if (custom != NULL)
		(*custom)(this, MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_FREE, NULL);

	// free the inline config
	global_free(inline_config);
}


/*-------------------------------------------------
    device_get_config_int - return an integer state
    value from an allocated device
-------------------------------------------------*/

INT64 device_config::get_config_int(UINT32 state) const
{
	assert(this != NULL);
	assert(type != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	// retrieve the value
	deviceinfo info;
	info.i = 0;
	(*type)(this, state, &info);
	return info.i;
}


/*-------------------------------------------------
    device_get_config_ptr - return a pointer state
    value from an allocated device
-------------------------------------------------*/

void *device_config::get_config_ptr(UINT32 state) const
{
	assert(this != NULL);
	assert(type != NULL);
	assert(state >= DEVINFO_PTR_FIRST && state <= DEVINFO_PTR_LAST);

	// retrieve the value
	deviceinfo info;
	info.p = NULL;
	(*type)(this, state, &info);
	return info.p;
}


/*-------------------------------------------------
    device_get_config_fct - return a function
    pointer state value from an allocated device
-------------------------------------------------*/

genf *device_config::get_config_fct(UINT32 state) const
{
	assert(this != NULL);
	assert(type != NULL);
	assert(state >= DEVINFO_FCT_FIRST && state <= DEVINFO_FCT_LAST);

	// retrieve the value
	deviceinfo info;
	info.f = 0;
	(*type)(this, state, &info);
	return info.f;
}


/*-------------------------------------------------
    device_get_config_string - return a string value
    from an allocated device
-------------------------------------------------*/

const char *device_config::get_config_string(UINT32 state) const
{
	assert(this != NULL);
	assert(type != NULL);
	assert(state >= DEVINFO_STR_FIRST && state <= DEVINFO_STR_LAST);

	// retrieve the value
	deviceinfo info;
	info.s = get_temp_string_buffer();
	(*type)(this, state, &info);
	if (info.s[0] == 0)
	{
		switch (state)
		{
			case DEVINFO_STR_NAME:			strcpy(info.s, "Custom");						break;
			case DEVINFO_STR_FAMILY:		strcpy(info.s, "Custom");						break;
			case DEVINFO_STR_VERSION:		strcpy(info.s, "1.0");							break;
			case DEVINFO_STR_SOURCE_FILE:	strcpy(info.s, __FILE__);						break;
			case DEVINFO_STR_CREDITS:		strcpy(info.s, "Copyright Nicola Salmoria and the MAME Team"); break;
		}
	}
	return info.s;
}


/*-------------------------------------------------
    subtag - create a tag for an object that is
    owned by this device
-------------------------------------------------*/

astring &device_config::subtag(astring &dest, const char *_tag) const
{
	return (this != NULL) ? dest.cpy(m_tag).cat(":").cat(_tag) : dest.cpy(_tag);
}


/*-------------------------------------------------
    siblingtag - create a tag for an object that
    a sibling to this device
-------------------------------------------------*/

astring &device_config::siblingtag(astring &dest, const char *_tag) const
{
	return (this != NULL && owner != NULL) ? owner->subtag(dest, _tag) : dest.cpy(_tag);
}



/***************************************************************************
    LIVE DEVICE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    running_device - constructor for a new
    running device; initial state is derived
    from the provided config
-------------------------------------------------*/

running_device::running_device(running_machine &_machine, const device_config &_config)
	: m_baseconfig(_config),
	  machine(&_machine),
	  next(NULL),
	  owner((_config.owner != NULL) ? _machine.devicelist.find(_config.owner->tag()) : NULL),
	  type(_config.type),
	  devclass(_config.devclass),
	  clock(_config.clock),
	  started(false),
	  token(NULL),
	  tokenbytes(_config.get_config_int(DEVINFO_INT_TOKEN_BYTES)),
	  region(NULL),
	  execute(NULL),
	  get_runtime_info(NULL),
	  m_tag(_config.tag())
{
	memset(addrspace, 0, sizeof(addrspace));

	if (tokenbytes == 0)
		throw emu_fatalerror("Device %s specifies a 0 token length!\n", tag());

	// allocate memory for the token
	token = auto_alloc_array_clear(machine, UINT8, tokenbytes);

	// get function pointers
	execute = (device_execute_func)get_config_fct(DEVINFO_FCT_EXECUTE);
	get_runtime_info = (device_get_runtime_info_func)get_config_fct(DEVINFO_FCT_GET_RUNTIME_INFO);
}


/*-------------------------------------------------
    ~running_device - destructor for a
    running_device
-------------------------------------------------*/

running_device::~running_device()
{
	// release token memory
	auto_free(machine, token);
}


/*-------------------------------------------------
    subregion - return a pointer to the region
    info for a given region
-------------------------------------------------*/

const region_info *running_device::subregion(const char *_tag) const
{
	// safety first
	if (this == NULL)
		return NULL;

	// build a fully-qualified name
	astring tempstring;
	return machine->region(subtag(tempstring, _tag));
}


/*-------------------------------------------------
    subdevice - return a pointer to the given
    device that is owned by us
-------------------------------------------------*/

running_device *running_device::subdevice(const char *_tag) const
{
	// safety first
	if (this == NULL)
		return NULL;

	// build a fully-qualified name
	astring tempstring;
	return machine->device(subtag(tempstring, _tag));
}


/*-------------------------------------------------
    set_address_space - connect an address space
    to a device
-------------------------------------------------*/

void running_device::set_address_space(int spacenum, const address_space *space)
{
	addrspace[spacenum] = space;
}


/*-------------------------------------------------
    set_clock - set a device's clock
-------------------------------------------------*/

void running_device::set_clock(UINT32 _clock)
{
	clock = _clock;
}


/*-------------------------------------------------
    start - start a device
-------------------------------------------------*/

void running_device::start()
{
	// find our region
	region = machine->regionlist.find(baseconfig().tag());

	// start functions are required
	device_start_func start = (device_start_func)get_config_fct(DEVINFO_FCT_START);
	assert(start != NULL);
	(*start)(this);

	// if we didn't get any exceptions then we started ok
	started = true;
}


/*-------------------------------------------------
    stop - stop a device
-------------------------------------------------*/

void running_device::stop()
{
	assert(token != NULL);
	assert(type != NULL);

	// if we have a stop function, call it
	device_stop_func stop = (device_stop_func)get_config_fct(DEVINFO_FCT_STOP);
	if (stop != NULL)
		(*stop)(this);
}


/*-------------------------------------------------
    reset - reset a device
-------------------------------------------------*/

void running_device::reset()
{
	assert(this != NULL);
	assert(this->token != NULL);
	assert(this->type != NULL);

	// if we have a reset function, call it
	device_reset_func reset = (device_reset_func)get_config_fct(DEVINFO_FCT_RESET);
	if (reset != NULL)
		(*reset)(this);
}


/*-------------------------------------------------
    get_runtime_int - return an integer state
    value from an allocated device
-------------------------------------------------*/

INT64 running_device::get_runtime_int(UINT32 state)
{
	assert(this != NULL);
	assert(get_runtime_info != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	// retrieve the value
	deviceinfo info;
	info.i = 0;
	(*get_runtime_info)(this, state, &info);
	return info.i;
}


/*-------------------------------------------------
    get_runtime_ptr - return a pointer state
    value from an allocated device
-------------------------------------------------*/

void *running_device::get_runtime_ptr(UINT32 state)
{
	assert(this != NULL);
	assert(get_runtime_info != NULL);
	assert(state >= DEVINFO_PTR_FIRST && state <= DEVINFO_PTR_LAST);

	// retrieve the value
	deviceinfo info;
	info.p = NULL;
	(*get_runtime_info)(this, state, &info);
	return info.p;
}


/*-------------------------------------------------
    get_runtime_string - return a string value
    from an allocated device
-------------------------------------------------*/

const char *running_device::get_runtime_string(UINT32 state)
{
	assert(this != NULL);
	assert(get_runtime_info != NULL);
	assert(state >= DEVINFO_STR_FIRST && state <= DEVINFO_STR_LAST);

	// retrieve the value
	deviceinfo info;
	info.s = get_temp_string_buffer();
	(*get_runtime_info)(this, state, &info);
	return info.s;
}
