/***************************************************************************

    devintrf.c

    Device interface functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "cpuintrf.h"



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

INLINE int device_matches_type(const device_config *device, device_type type)
{
	return (type == DEVICE_TYPE_WILDCARD) ? TRUE : (device->type == type);
}



/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    device_list_add - add a new device to the
    end of a device list
-------------------------------------------------*/

device_config *device_list_add(device_config **listheadptr, const device_config *owner, device_type type, const char *tag, UINT32 clock)
{
	device_config **devptr, **tempdevptr;
	device_config *device, *tempdevice;
	UINT32 configlen;

	assert(listheadptr != NULL);
	assert(type != NULL);
	assert(tag != NULL);

	/* find the end of the list, and ensure no duplicates along the way */
	for (devptr = listheadptr; *devptr != NULL; devptr = &(*devptr)->next)
		if (strcmp(tag, (*devptr)->tag) == 0)
			fatalerror("Attempted to add duplicate device: type=%s tag=%s\n", device_get_name(*devptr), tag);

	/* get the size of the inline config */
	configlen = (UINT32)devtype_get_info_int(type, DEVINFO_INT_INLINE_CONFIG_BYTES);

	/* allocate a new device */
	device = (device_config *)alloc_array_or_die(UINT8, sizeof(*device) + strlen(tag) + configlen);

	/* populate device relationships */
	device->next = NULL;
	device->owner = (device_config *)owner;
	device->typenext = NULL;
	device->classnext = NULL;

	/* populate device properties */
	device->type = type;
	device->devclass = (device_class)(INT32)devtype_get_info_int(type, DEVINFO_INT_CLASS);

	/* populate device configuration */
	device->clock = clock;
	memset((void *)device->address_map, 0, sizeof(device->address_map));
	if ((device->clock & 0xff000000) == 0xff000000)
	{
		assert(device->owner != NULL);
		device->clock = device->owner->clock * ((device->clock >> 12) & 0xfff) / ((device->clock >> 0) & 0xfff);
	}
	device->static_config = NULL;
	device->inline_config = (configlen == 0) ? NULL : (device->tag + strlen(tag) + 1);

	/* ensure live fields are all cleared */
	device->machine = NULL;
	device->started = DEVICE_STOPPED;
	device->token = NULL;
	device->tokenbytes = 0;
	device->region = NULL;
	device->regionbytes = 0;
	memset((void *)device->space, 0, sizeof(device->space));
	device->execute = NULL;

	/* append the tag */
	strcpy(device->tag, tag);

	/* reset the inline_config to 0 */
	if (configlen > 0)
		memset(device->inline_config, 0, configlen);

	/* fetch function pointers to the core functions */

	/* before adding us to the global list, add us to the end of the type list */
	tempdevice = (device_config *)device_list_first(*listheadptr, type);
	for (tempdevptr = &tempdevice; *tempdevptr != NULL; tempdevptr = &(*tempdevptr)->typenext) ;
	*tempdevptr = device;

	/* and to the end of the class list */
	tempdevice = (device_config *)device_list_class_first(*listheadptr, device->devclass);
	for (tempdevptr = &tempdevice; *tempdevptr != NULL; tempdevptr = &(*tempdevptr)->classnext) ;
	*tempdevptr = device;

	/* link us to the end of the master list and return */
	*devptr = device;
	return device;
}


/*-------------------------------------------------
    device_list_remove - remove a device from a
    device list
-------------------------------------------------*/

void device_list_remove(device_config **listheadptr, const char *tag)
{
	device_config **devptr, **tempdevptr;
	device_config *device, *tempdevice;

	assert(listheadptr != NULL);
	assert(tag != NULL);

	/* find the device in the list */
	for (devptr = listheadptr; *devptr != NULL; devptr = &(*devptr)->next)
		if (strcmp(tag, (*devptr)->tag) == 0)
			break;
	device = *devptr;
	if (device == NULL)
		fatalerror("Attempted to remove non-existant device: tag=%s\n", tag);

	/* before removing us from the global list, remove us from the type list */
	tempdevice = (device_config *)device_list_first(*listheadptr, device->type);
	for (tempdevptr = &tempdevice; *tempdevptr != device; tempdevptr = &(*tempdevptr)->typenext) ;
	assert(*tempdevptr == device);
	*tempdevptr = device->typenext;

	/* and from the class list */
	tempdevice = (device_config *)device_list_class_first(*listheadptr, device->devclass);
	for (tempdevptr = &tempdevice; *tempdevptr != device; tempdevptr = &(*tempdevptr)->classnext) ;
	assert(*tempdevptr == device);
	*tempdevptr = device->classnext;

	/* remove the device from the list */
	*devptr = device->next;

	/* free the device object */
	free(device);
}


/*-------------------------------------------------
    device_build_tag - build a tag that combines
    the device's name and the given tag
-------------------------------------------------*/

const char *device_build_tag(astring *dest, const device_config *device, const char *tag)
{
	if (device != NULL)
	{
		astring_cpyc(dest, device->tag);
		astring_catc(dest, ":");
		astring_catc(dest, tag);
	}
	else
		astring_cpyc(dest, tag);
	return astring_c(dest);
}


/*-------------------------------------------------
    device_inherit_tag - build a tag with the same
    device prefix as the source tag
-------------------------------------------------*/

const char *device_inherit_tag(astring *dest, const char *sourcetag, const char *tag)
{
	const char *divider = strrchr(sourcetag, ':');
	if (divider != NULL)
	{
		astring_cpych(dest, sourcetag, divider + 1 - sourcetag);
		astring_catc(dest, tag);
	}
	else
		astring_cpyc(dest, tag);
	return astring_c(dest);
}


/*-------------------------------------------------
    device_get_contract - find a given contract
    on a device
-------------------------------------------------*/

const device_contract *device_get_contract(const device_config *device, const char *name)
{
	const device_contract *contract = (const device_contract *)device_get_info_ptr(device, DEVINFO_PTR_CONTRACT_LIST);

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
    TYPE-BASED DEVICE ACCESS
***************************************************************************/

/*-------------------------------------------------
    device_list_items - return the number of
    items of a given type; DEVICE_TYPE_WILDCARD
    is allowed
-------------------------------------------------*/

int device_list_items(const device_config *listhead, device_type type)
{
	const device_config *curdev;
	int count = 0;

	/* locate all devices */
	if (type == DEVICE_TYPE_WILDCARD)
	{
		for (curdev = listhead; curdev != NULL; curdev = curdev->next)
			count++;
	}

	/* locate all devices of a given type */
	else
	{
		for (curdev = listhead; curdev != NULL && curdev->type != type; curdev = curdev->next) ;
		for ( ; curdev != NULL; curdev = curdev->typenext)
			count++;
	}

	return count;
}


/*-------------------------------------------------
    device_list_first - return the first device
    in the list of a given type;
    DEVICE_TYPE_WILDCARD is allowed
-------------------------------------------------*/

const device_config *device_list_first(const device_config *listhead, device_type type)
{
	const device_config *curdev;

	/* first of any device type */
	if (type == DEVICE_TYPE_WILDCARD)
		return listhead;

	/* first of a given type */
	for (curdev = listhead; curdev != NULL && curdev->type != type; curdev = curdev->next) ;
	return curdev;
}


/*-------------------------------------------------
    device_list_next - return the next device
    in the list of a given type;
    DEVICE_TYPE_WILDCARD is allowed
-------------------------------------------------*/

const device_config *device_list_next(const device_config *prevdevice, device_type type)
{
	assert(prevdevice != NULL);
	return (type == DEVICE_TYPE_WILDCARD) ? prevdevice->next : prevdevice->typenext;
}


/*-------------------------------------------------
    device_list_find_by_tag - retrieve a device
    configuration based on a type and tag;
    DEVICE_TYPE_WILDCARD is allowed
-------------------------------------------------*/

const device_config *device_list_find_by_tag(const device_config *listhead, const char *tag)
{
	const device_config *curdev;

	assert(tag != NULL);

	/* locate among all devices */
	for (curdev = listhead; curdev != NULL; curdev = curdev->next)
		if (strcmp(tag, curdev->tag) == 0)
			return curdev;

	/* fail */
	return NULL;
}


/*-------------------------------------------------
    device_list_index - return the index of a
    device based on its type and tag;
    DEVICE_TYPE_WILDCARD is allowed
-------------------------------------------------*/

int device_list_index(const device_config *listhead, device_type type, const char *tag)
{
	const device_config *curdev;
	int index = 0;

	assert(tag != NULL);

	/* locate among all devices */
	if (type == DEVICE_TYPE_WILDCARD)
	{
		for (curdev = listhead; curdev != NULL; curdev = curdev->next)
		{
			if (strcmp(tag, curdev->tag) == 0)
				return index;
			index++;
		}
	}

	/* locate among all devices of a given type */
	else
	{
		for (curdev = listhead; curdev != NULL && curdev->type != type; curdev = curdev->next) ;
		for ( ; curdev != NULL; curdev = curdev->typenext)
		{
			if (strcmp(tag, curdev->tag) == 0)
				return index;
			index++;
		}
	}

	return -1;
}


/*-------------------------------------------------
    device_list_find_by_index - retrieve a device
    configuration based on a type and index
-------------------------------------------------*/

const device_config *device_list_find_by_index(const device_config *listhead, device_type type, int index)
{
	const device_config *curdev;

	/* locate among all devices */
	if (type == DEVICE_TYPE_WILDCARD)
	{
		for (curdev = listhead; curdev != NULL; curdev = curdev->next)
			if (index-- == 0)
				return curdev;
	}

	/* locate among all devices of a given type */
	else
	{
		for (curdev = listhead; curdev != NULL && curdev->type != type; curdev = curdev->next) ;
		for ( ; curdev != NULL; curdev = curdev->typenext)
			if (index-- == 0)
				return curdev;
	}

	/* fail */
	return NULL;
}



/***************************************************************************
    CLASS-BASED DEVICE ACCESS
***************************************************************************/

/*-------------------------------------------------
    device_list_class_items - return the number of
    items of a given class
-------------------------------------------------*/

int device_list_class_items(const device_config *listhead, device_class devclass)
{
	const device_config *curdev;
	int count = 0;

	/* locate all devices of a given class */
	for (curdev = listhead; curdev != NULL && curdev->devclass != devclass; curdev = curdev->next) ;
	for ( ; curdev != NULL; curdev = curdev->classnext)
		count++;

	return count;
}


/*-------------------------------------------------
    device_list_class_first - return the first
    device in the list of a given class
-------------------------------------------------*/

const device_config *device_list_class_first(const device_config *listhead, device_class devclass)
{
	const device_config *curdev;

	/* first of a given class */
	for (curdev = listhead; curdev != NULL && curdev->devclass != devclass; curdev = curdev->next) ;
	return curdev;
}


/*-------------------------------------------------
    device_list_class_next - return the next
    device in the list of a given class
-------------------------------------------------*/

const device_config *device_list_class_next(const device_config *prevdevice, device_class devclass)
{
	assert(prevdevice != NULL);
	return prevdevice->classnext;
}


/*-------------------------------------------------
    device_list_class_index - return the index of a
    device based on its class and tag
-------------------------------------------------*/

int device_list_class_index(const device_config *listhead, device_class devclass, const char *tag)
{
	const device_config *curdev;
	int index = 0;

	assert(tag != NULL);

	/* locate among all devices of a given class */
	for (curdev = listhead; curdev != NULL && curdev->devclass != devclass; curdev = curdev->next) ;
	for ( ; curdev != NULL; curdev = curdev->classnext)
	{
		if (strcmp(tag, curdev->tag) == 0)
			return index;
		index++;
	}

	return -1;
}


/*-------------------------------------------------
    device_list_class_find_by_index - retrieve a
    device configuration based on a class and
    index
-------------------------------------------------*/

const device_config *device_list_class_find_by_index(const device_config *listhead, device_class devclass, int index)
{
	const device_config *curdev;

	/* locate among all devices of a given class */
	for (curdev = listhead; curdev != NULL && curdev->devclass != devclass; curdev = curdev->next) ;
	for ( ; curdev != NULL; curdev = curdev->classnext)
		if (index-- == 0)
			return curdev;

	/* fail */
	return NULL;
}



/***************************************************************************
    LIVE DEVICE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    device_list_attach_machine - "attach" a
    running_machine to its list of devices
-------------------------------------------------*/

void device_list_attach_machine(running_machine *machine)
{
	device_config *device;

	assert(machine != NULL);

	/* iterate over devices and assign the machine to them */
	for (device = (device_config *)machine->config->devicelist; device != NULL; device = device->next)
		device->machine = machine;
}


/*-------------------------------------------------
    device_list_start - start the configured list
    of devices for a machine
-------------------------------------------------*/

void device_list_start(running_machine *machine)
{
	device_config *device;
	int numstarted = 0;
	int devcount = 0;

	assert(machine != NULL);

	/* add an exit callback for cleanup */
	add_reset_callback(machine, device_list_reset);
	add_exit_callback(machine, device_list_stop);

	/* iterate over devices and allocate memory for them */
	for (device = (device_config *)machine->config->devicelist; device != NULL; device = device->next)
	{
		int spacenum;

		assert(!device->started);
		assert(device->machine == machine);
		assert(device->token == NULL);
		assert(device->type != NULL);

		devcount++;

		/* get the size of the token data; we do it directly because we can't call device_get_info_* with no token */
		device->tokenbytes = device_get_info_int(device, DEVINFO_INT_TOKEN_BYTES);
		if (device->tokenbytes == 0)
			fatalerror("Device %s specifies a 0 token length!\n", device_get_name(device));

		/* allocate memory for the token */
		device->token = alloc_array_clear_or_die(UINT8, device->tokenbytes);

		/* fill in the remaining runtime fields */
		device->region = memory_region(machine, device->tag);
		device->regionbytes = memory_region_length(machine, device->tag);
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			device->space[spacenum] = memory_find_address_space(device, spacenum);
		device->execute = (device_execute_func)device_get_info_fct(device, DEVINFO_FCT_EXECUTE);
	}

	/* iterate until we've started everything */
	while (numstarted < devcount)
	{
		int prevstarted = numstarted;

		/* iterate over devices and start them */
		for (device = (device_config *)machine->config->devicelist; device != NULL; device = device->next)
		{
			device_start_func start = (device_start_func)device_get_info_fct(device, DEVINFO_FCT_START);
			assert(start != NULL);

			if (!device->started)
			{
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

void device_delay_init(const device_config *device)
{
	if (device->started != DEVICE_STARTING && device->started != DEVICE_DELAYED)
		fatalerror("Error: Calling device_delay_init on a device not in the process of starting.");
	((device_config *)device)->started = DEVICE_DELAYED;
}


/*-------------------------------------------------
    device_list_stop - stop the configured list
    of devices for a machine
-------------------------------------------------*/

static void device_list_stop(running_machine *machine)
{
	device_config *device;

	assert(machine != NULL);

	/* iterate over devices and stop them */
	for (device = (device_config *)machine->config->devicelist; device != NULL; device = device->next)
	{
		device_stop_func stop = (device_stop_func)device_get_info_fct(device, DEVINFO_FCT_STOP);

		assert(device->token != NULL);
		assert(device->type != NULL);

		/* if we have a stop function, call it */
		if (stop != NULL)
			(*stop)(device);

		/* free allocated memory for the token */
		if (device->token != NULL)
			free(device->token);

		/* reset all runtime fields */
		device->token = NULL;
		device->tokenbytes = 0;
		device->machine = NULL;
		device->region = NULL;
		device->regionbytes = 0;
	}
}


/*-------------------------------------------------
    device_list_reset - reset the configured list
    of devices for a machine
-------------------------------------------------*/

static void device_list_reset(running_machine *machine)
{
	const device_config *device;

	assert(machine != NULL);

	/* iterate over devices and stop them */
	for (device = (device_config *)machine->config->devicelist; device != NULL; device = device->next)
		device_reset(device);
}


/*-------------------------------------------------
    device_reset - reset a device based on an
    allocated device_config
-------------------------------------------------*/

void device_reset(const device_config *device)
{
	device_reset_func reset;

	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type != NULL);

	/* if we have a reset function, call it */
	reset = (device_reset_func)device_get_info_fct(device, DEVINFO_FCT_RESET);
	if (reset != NULL)
		(*reset)(device);
}


/*-------------------------------------------------
    device_set_clock - change the clock on a
    device
-------------------------------------------------*/

void device_set_clock(const device_config *device, UINT32 clock)
{
	device_config *devicerw = (device_config *)device;

	/* not much for now */
	devicerw->clock = clock;
}



/***************************************************************************
    DEVICE INFORMATION GETTERS
***************************************************************************/

/*-------------------------------------------------
    device_get_info_int - return an integer state
    value from an allocated device
-------------------------------------------------*/

INT64 device_get_info_int(const device_config *device, UINT32 state)
{
	deviceinfo info;

	assert(device != NULL);
	assert(device->type != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	/* retrieve the value */
	info.i = 0;
	(*device->type)(device, state, &info);
	return info.i;
}


/*-------------------------------------------------
    device_get_info_ptr - return a pointer state
    value from an allocated device
-------------------------------------------------*/

void *device_get_info_ptr(const device_config *device, UINT32 state)
{
	deviceinfo info;

	assert(device != NULL);
	assert(device->type != NULL);
	assert(state >= DEVINFO_PTR_FIRST && state <= DEVINFO_PTR_LAST);

	/* retrieve the value */
	info.p = NULL;
	(*device->type)(device, state, &info);
	return info.p;
}


/*-------------------------------------------------
    device_get_info_fct - return a function
    pointer state value from an allocated device
-------------------------------------------------*/

genf *device_get_info_fct(const device_config *device, UINT32 state)
{
	deviceinfo info;

	assert(device != NULL);
	assert(device->type != NULL);
	assert(state >= DEVINFO_FCT_FIRST && state <= DEVINFO_FCT_LAST);

	/* retrieve the value */
	info.f = 0;
	(*device->type)(device, state, &info);
	return info.f;
}


/*-------------------------------------------------
    device_get_info_string - return a string value
    from an allocated device
-------------------------------------------------*/

const char *device_get_info_string(const device_config *device, UINT32 state)
{
	deviceinfo info;

	assert(device != NULL);
	assert(device->type != NULL);
	assert(state >= DEVINFO_STR_FIRST && state <= DEVINFO_STR_LAST);

	/* retrieve the value */
	info.s = get_temp_string_buffer();
	(*device->type)(device, state, &info);
	if (info.s[0] == 0)
		set_default_string(state, info.s);
	return info.s;
}



/***************************************************************************
    DEVICE TYPE INFORMATION SETTERS
***************************************************************************/

/*-------------------------------------------------
    devtype_get_info_int - return an integer value
    from a device type (does not need to be
    allocated)
-------------------------------------------------*/

INT64 devtype_get_info_int(device_type type, UINT32 state)
{
	deviceinfo info;

	assert(type != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	/* retrieve the value */
	info.i = 0;
	(*type)(NULL, state, &info);
	return info.i;
}


/*-------------------------------------------------
    devtype_get_info_int - return a function
    pointer from a device type (does not need to
    be allocated)
-------------------------------------------------*/

genf *devtype_get_info_fct(device_type type, UINT32 state)
{
	deviceinfo info;

	assert(type != NULL);
	assert(state >= DEVINFO_FCT_FIRST && state <= DEVINFO_FCT_LAST);

	/* retrieve the value */
	info.f = 0;
	(*type)(NULL, state, &info);
	return info.f;
}


/*-------------------------------------------------
    devtype_get_info_string - return a string value
    from a device type (does not need to be
    allocated)
-------------------------------------------------*/

const char *devtype_get_info_string(device_type type, UINT32 state)
{
	deviceinfo info;

	assert(type != NULL);
	assert(state >= DEVINFO_STR_FIRST && state <= DEVINFO_STR_LAST);

	/* retrieve the value */
	info.s = get_temp_string_buffer();
	(*type)(NULL, state, &info);
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
