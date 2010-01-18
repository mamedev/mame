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

// ->started states
//
// Only 0 and 1 are externally visible in practice, making it a
// boolean //for external users

enum {
	DEVICE_STOPPED = 0,
	DEVICE_STARTED = 1,
	DEVICE_STARTING = 2,
	DEVICE_DELAYED = 3
};


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static char temp_string_pool[TEMP_STRING_POOL_ENTRIES][MAX_STRING_LENGTH];
static int temp_string_pool_index;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void device_list_stop(running_machine *machine);
static void device_list_reset(running_machine *machine);
static void set_default_string(UINT32 state, char *buffer);



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


/*-------------------------------------------------
    device_matches_type - does a device match
    the provided type, taking wildcards into
    effect?
-------------------------------------------------*/

INLINE int device_matches_type(running_device *device, device_type type)
{
	return (type == DEVICE_TYPE_WILDCARD) ? TRUE : (device->type == type);
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
	astring tempstring(tag, ":", _tag);
	return machine->region(tempstring);
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
	astring tempstring(tag, ":", _tag);
	return machine->device(tempstring);
}



/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    device_list_add - add a new device to the
    end of a device list
-------------------------------------------------*/

device_config::device_config(const device_config *_owner, device_type _type, const char *_tag, UINT32 _clock)
	: next(NULL),
	  owner(const_cast<device_config *>(_owner)),
	  tag(_tag),
	  type(_type),
	  devclass(DEVICE_CLASS_GENERAL),
	  clock(_clock),
	  static_config(NULL),
	  inline_config(NULL)
{
	memset(address_map, 0, sizeof(address_map));

	devclass = (device_class)get_config_int(DEVINFO_INT_CLASS);

	if ((clock & 0xff000000) == 0xff000000)
	{
		assert(owner != NULL);
		clock = owner->clock * ((clock >> 12) & 0xfff) / ((clock >> 0) & 0xfff);
	}

	/* populate device configuration */
	UINT32 configlen = (UINT32)get_config_int(DEVINFO_INT_INLINE_CONFIG_BYTES);
	inline_config = (configlen == 0) ? NULL : global_alloc_array_clear(UINT8, configlen);
}


device_config::~device_config()
{
	/* call the custom config free function first */
	device_custom_config_func custom = reinterpret_cast<device_custom_config_func>(get_config_fct(DEVINFO_FCT_CUSTOM_CONFIG));
	if (custom != NULL)
		(*custom)(this, MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_FREE, NULL);

	global_free(inline_config);
}


running_device::running_device(running_machine &_machine, const device_config &_config)
	: _baseconfig(_config),
	  machine(&_machine),
	  next(NULL),
	  owner((_config.owner != NULL) ? _machine.devicelist.find(_config.owner->tag) : NULL),
	  tag(_config.tag),
	  type(_config.type),
	  devclass(_config.devclass),
	  clock(_config.clock),
	  started(DEVICE_STOPPED),
	  token(NULL),
	  tokenbytes(_config.get_config_int(DEVINFO_INT_TOKEN_BYTES)),
	  region(NULL),
	  execute(NULL),
	  get_runtime_info(NULL)
{
	/* get the size of the token data; we do it directly because we can't call device_get_config_* with no token */
	if (tokenbytes == 0)
		throw emu_fatalerror("Device %s specifies a 0 token length!\n", tag.cstr());

	/* allocate memory for the token */
	token = auto_alloc_array_clear(machine, UINT8, tokenbytes);

	/* fill in the remaining runtime fields */
	memset(addrspace, 0, sizeof(addrspace));
		
	// get function pointers
	execute = (device_execute_func)get_config_fct(DEVINFO_FCT_EXECUTE);
	get_runtime_info = (device_get_runtime_info_func)get_config_fct(DEVINFO_FCT_GET_RUNTIME_INFO);
}


running_device::~running_device()
{
	auto_free(machine, token);
}


void running_device::set_address_space(int spacenum, const address_space *space)
{
	addrspace[spacenum] = space;
}


/*-------------------------------------------------
    device_build_tag - build a tag that combines
    the device's name and the given tag
-------------------------------------------------*/

astring &device_build_tag(astring &dest, const device_config *devconfig, const char *tag)
{
	if (devconfig != NULL)
		dest.cpy(devconfig->tag).cat(":").cat(tag);
	else
		dest.cpy(tag);
	return dest;
}


/*-------------------------------------------------
    device_inherit_tag - build a tag with the same
    device prefix as the source tag
-------------------------------------------------*/

astring &device_inherit_tag(astring &dest, const char *sourcetag, const char *tag)
{
	const char *divider = strrchr(sourcetag, ':');
	if (divider != NULL)
		dest.cpy(sourcetag, divider + 1 - sourcetag).cat(tag);
	else
		dest.cpy(tag);
	return dest;
}


/*-------------------------------------------------
    device_get_contract - find a given contract
    on a device
-------------------------------------------------*/

const device_contract *device_get_contract(running_device *device, const char *name)
{
	const device_contract *contract = (const device_contract *)device->get_config_ptr(DEVINFO_PTR_CONTRACT_LIST);

	/* if no contracts, obviously we don't have it */
	if (contract == NULL)
		return NULL;

	/* scan forward through the array looking for a match */
	for ( ; contract->name != NULL; contract++)
		if (strcmp(name, contract->name) == 0)
			return contract;

	return NULL;
}



/***************************************************************************
    LIVE DEVICE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    device_list_attach_machine - "attach" a
    running_machine to its list of devices
-------------------------------------------------*/

void device_list::import_config_list(const device_config_list &list, running_machine &machine)
{
	for (const device_config *devconfig = list.first(); devconfig != NULL; devconfig = devconfig->next)
		append(devconfig->tag, new running_device(machine, *devconfig));
}


/*-------------------------------------------------
    device_list_start - start the configured list
    of devices for a machine
-------------------------------------------------*/

void device_list_start(running_machine *machine)
{
	running_device *device;
	int numstarted = 0;
	int devcount = machine->devicelist.count();

	assert(machine != NULL);

	/* add an exit callback for cleanup */
	add_reset_callback(machine, device_list_reset);
	add_exit_callback(machine, device_list_stop);

	/* iterate until we've started everything */
	while (numstarted < devcount)
	{
		int prevstarted = numstarted;

		/* iterate over devices and start them */
		for (device = machine->devicelist.first(); device != NULL; device = device->next)
		{
			device_start_func start = (device_start_func)device->get_config_fct(DEVINFO_FCT_START);
			assert(start != NULL);

			if (!device->started)
			{
				device->region = machine->regionlist.find(device->baseconfig().tag);
			
				device->started = DEVICE_STARTING;
				(*start)(device);

				/* if the start was delayed, move back to the stopped state, otherwise count it */
				if (device->started == DEVICE_DELAYED)
					device->started = DEVICE_STOPPED;
				else
				{
					device->started = DEVICE_STARTED;
					numstarted++;
				}
			}
		}

		/* if we didn't start anything new, we're in trouble */
		if (numstarted == prevstarted)
			fatalerror("Circular dependency in device startup; unable to start %d/%d devices\n", devcount - numstarted, devcount);
	}
}


/*-------------------------------------------------
    device_delay_init - delay the startup of a
    given device for dependency reasons
-------------------------------------------------*/

void device_delay_init(running_device *device)
{
	if (device->started != DEVICE_STARTING && device->started != DEVICE_DELAYED)
		fatalerror("Error: Calling device_delay_init on a device not in the process of starting.");
	device->started = DEVICE_DELAYED;
}


/*-------------------------------------------------
    device_list_stop - stop the configured list
    of devices for a machine
-------------------------------------------------*/

static void device_list_stop(running_machine *machine)
{
	running_device *device;

	assert(machine != NULL);

	/* iterate over devices and stop them */
	for (device = machine->devicelist.first(); device != NULL; device = device->next)
	{
		device_stop_func stop = (device_stop_func)device->get_config_fct(DEVINFO_FCT_STOP);

		assert(device->token != NULL);
		assert(device->type != NULL);

		/* if we have a stop function, call it */
		if (stop != NULL)
			(*stop)(device);

		/* free allocated memory for the token */
		auto_free(machine, device->token);

		/* reset all runtime fields */
		device->token = NULL;
		device->tokenbytes = 0;
		device->machine = NULL;
		device->region = NULL;
	}
}


/*-------------------------------------------------
    device_list_reset - reset the configured list
    of devices for a machine
-------------------------------------------------*/

static void device_list_reset(running_machine *machine)
{
	running_device *device;

	assert(machine != NULL);

	/* iterate over devices and stop them */
	for (device = machine->devicelist.first(); device != NULL; device = device->next)
		device_reset(device);
}


/*-------------------------------------------------
    device_reset - reset a device based on an
    allocated device_config
-------------------------------------------------*/

void device_reset(running_device *device)
{
	device_reset_func reset;

	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type != NULL);

	/* if we have a reset function, call it */
	reset = (device_reset_func)device->get_config_fct(DEVINFO_FCT_RESET);
	if (reset != NULL)
		(*reset)(device);
}


/*-------------------------------------------------
    device_set_clock - change the clock on a
    device
-------------------------------------------------*/

void device_set_clock(running_device *device, UINT32 clock)
{
	device_config *devicerw = (device_config *)device;

	/* not much for now */
	devicerw->clock = clock;
}



/***************************************************************************
    DEVICE INFORMATION GETTERS
***************************************************************************/

/*-------------------------------------------------
    device_get_config_int - return an integer state
    value from an allocated device
-------------------------------------------------*/

INT64 device_config::get_config_int(UINT32 state) const
{
	deviceinfo info;

	assert(this != NULL);
	assert(type != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	/* retrieve the value */
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
	deviceinfo info;

	assert(this != NULL);
	assert(type != NULL);
	assert(state >= DEVINFO_PTR_FIRST && state <= DEVINFO_PTR_LAST);

	/* retrieve the value */
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
	deviceinfo info;

	assert(this != NULL);
	assert(type != NULL);
	assert(state >= DEVINFO_FCT_FIRST && state <= DEVINFO_FCT_LAST);

	/* retrieve the value */
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
	deviceinfo info;

	assert(this != NULL);
	assert(type != NULL);
	assert(state >= DEVINFO_STR_FIRST && state <= DEVINFO_STR_LAST);

	/* retrieve the value */
	info.s = get_temp_string_buffer();
	(*type)(this, state, &info);
	if (info.s[0] == 0)
		set_default_string(state, info.s);
	return info.s;
}



/*-------------------------------------------------
    device_get_runtime_int - return an integer state
    value from an allocated device
-------------------------------------------------*/

INT64 running_device::get_runtime_int(UINT32 state)
{
	deviceinfo info;

	assert(this != NULL);
	assert(get_runtime_info != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	/* retrieve the value */
	info.i = 0;
	(*get_runtime_info)(this, state, &info);
	return info.i;
}


/*-------------------------------------------------
    device_get_runtime_ptr - return a pointer state
    value from an allocated device
-------------------------------------------------*/

void *running_device::get_runtime_ptr(UINT32 state)
{
	deviceinfo info;

	assert(this != NULL);
	assert(get_runtime_info != NULL);
	assert(state >= DEVINFO_PTR_FIRST && state <= DEVINFO_PTR_LAST);

	/* retrieve the value */
	info.p = NULL;
	(*get_runtime_info)(this, state, &info);
	return info.p;
}


/*-------------------------------------------------
    device_get_runtime_string - return a string value
    from an allocated device
-------------------------------------------------*/

const char *running_device::get_runtime_string(UINT32 state)
{
	deviceinfo info;

	assert(this != NULL);
	assert(get_runtime_info != NULL);
	assert(state >= DEVINFO_STR_FIRST && state <= DEVINFO_STR_LAST);

	/* retrieve the value */
	info.s = get_temp_string_buffer();
	(*get_runtime_info)(this, state, &info);
	if (info.s[0] == 0)
		set_default_string(state, info.s);
	return info.s;
}



/***************************************************************************
    DEFAULT HANDLERS
***************************************************************************/

/*-------------------------------------------------
    set_default_string - compute a default string
    if none is provided
-------------------------------------------------*/

static void set_default_string(UINT32 state, char *buffer)
{
	switch (state)
	{
		case DEVINFO_STR_NAME:			strcpy(buffer, "Custom");						break;
		case DEVINFO_STR_FAMILY:		strcpy(buffer, "Custom");						break;
		case DEVINFO_STR_VERSION:		strcpy(buffer, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:	strcpy(buffer, __FILE__);						break;
		case DEVINFO_STR_CREDITS:		strcpy(buffer, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
