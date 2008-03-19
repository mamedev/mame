/***************************************************************************

    devintrf.c

    Device interface functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"



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
    FUNCTION PROTOTYPES
***************************************************************************/

static void device_list_stop(running_machine *machine);
static void device_list_reset(running_machine *machine);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_temp_string_buffer - return a pointer to
    a temporary string buffer
-------------------------------------------------*/

INLINE char *get_temp_string_buffer(void)
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

device_config *device_list_add(device_config **listheadptr, device_type type, const char *tag)
{
	device_config **devptr;
	device_config *device;
	UINT32 configlen;
	deviceinfo info;

	assert(listheadptr != NULL);
	assert(type != NULL);
	assert(tag != NULL);

	/* find the end of the list, and ensure no duplicates along the way */
	for (devptr = listheadptr; *devptr != NULL; devptr = &(*devptr)->next)
		if (type == (*devptr)->type && strcmp(tag, (*devptr)->tag) == 0)
			fatalerror("Attempted to add duplicate device: type=%s tag=%s\n", devtype_name(type), tag);

	/* get the size of the inline config */
	configlen = (UINT32)devtype_get_info_int(type, DEVINFO_INT_INLINE_CONFIG_BYTES);

	/* allocate a new device */
	device = malloc_or_die(sizeof(*device) + strlen(tag) + configlen);

	/* populate all fields */
	device->next = NULL;
	device->type = type;
	device->class = devtype_get_info_int(type, DEVINFO_INT_CLASS);
	device->static_config = NULL;
	device->inline_config = (configlen == 0) ? NULL : (device->tag + strlen(tag) + 1);
	device->token = NULL;
	strcpy(device->tag, tag);

	/* reset the inline_config to 0 */
	if (configlen > 0)
		memset(device->inline_config, 0, configlen);

	/* fetch function pointers to the core functions */
	info.set_info = NULL;
	(*type)(NULL, DEVINFO_FCT_SET_INFO, &info);
	device->set_info = info.set_info;

	info.start = NULL;
	(*type)(NULL, DEVINFO_FCT_START, &info);
	device->start = info.start;

	info.stop = NULL;
	(*type)(NULL, DEVINFO_FCT_STOP, &info);
	device->stop = info.stop;

	info.reset = NULL;
	(*type)(NULL, DEVINFO_FCT_RESET, &info);
	device->reset = info.reset;

	/* link us to the end and return */
	*devptr = device;
	return device;
}


/*-------------------------------------------------
    device_list_remove - remove a device from a
    device list
-------------------------------------------------*/

void device_list_remove(device_config **listheadptr, device_type type, const char *tag)
{
	device_config **devptr;
	device_config *device;

	assert(listheadptr != NULL);
	assert(type != NULL);
	assert(tag != NULL);

	/* find the device in the list */
	for (devptr = listheadptr; *devptr != NULL; devptr = &(*devptr)->next)
		if (type == (*devptr)->type && strcmp(tag, (*devptr)->tag) == 0)
			break;
	if (*devptr == NULL)
		fatalerror("Attempted to remove non-existant device: type=%s tag=%s\n", devtype_name(type), tag);

	/* remove the device from the list */
	device = *devptr;
	*devptr = device->next;

	/* free the device object */
	free(device);
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

	assert(type != NULL);

	/* locate all devices */
	for (curdev = listhead; curdev != NULL; curdev = curdev->next)
		count += device_matches_type(curdev, type);

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

	assert(type != NULL);

	/* scan forward starting with the list head */
	for (curdev = listhead; curdev != NULL; curdev = curdev->next)
		if (device_matches_type(curdev, type))
			return curdev;

	return NULL;
}


/*-------------------------------------------------
    device_list_next - return the next device
    in the list of a given type;
    DEVICE_TYPE_WILDCARD is allowed
-------------------------------------------------*/

const device_config *device_list_next(const device_config *prevdevice, device_type type)
{
	const device_config *curdev;

	assert(prevdevice != NULL);
	assert(type != NULL);

	/* scan forward starting with the item after the previous one */
	for (curdev = prevdevice->next; curdev != NULL; curdev = curdev->next)
		if (device_matches_type(curdev, type))
			return curdev;

	return NULL;
}


/*-------------------------------------------------
    device_list_find_by_tag - retrieve a device
    configuration based on a type and tag
-------------------------------------------------*/

const device_config *device_list_find_by_tag(const device_config *listhead, device_type type, const char *tag)
{
	const device_config *curdev;

	assert(type != NULL);
	assert(tag != NULL);

	/* find the device in the list */
	for (curdev = listhead; curdev != NULL; curdev = curdev->next)
		if (device_matches_type(curdev, type) && strcmp(tag, curdev->tag) == 0)
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

	assert(type != NULL);
	assert(tag != NULL);

	/* locate all devices */
	for (curdev = listhead; curdev != NULL; curdev = curdev->next)
		if (device_matches_type(curdev, type))
		{
			if (strcmp(tag, curdev->tag) == 0)
				return index;
			index++;
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

	assert(type != NULL);
	assert(index >= 0);

	/* find the device in the list */
	for (curdev = listhead; curdev != NULL; curdev = curdev->next)
		if (device_matches_type(curdev, type) && index-- == 0)
			return curdev;

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

int device_list_class_items(const device_config *listhead, device_class class)
{
	const device_config *curdev;
	int count = 0;

	/* locate all devices */
	for (curdev = listhead; curdev != NULL; curdev = curdev->next)
		count += (curdev->class == class);

	return count;
}


/*-------------------------------------------------
    device_list_class_first - return the first
    device in the list of a given class
-------------------------------------------------*/

const device_config *device_list_class_first(const device_config *listhead, device_class class)
{
	const device_config *curdev;

	/* scan forward starting with the list head */
	for (curdev = listhead; curdev != NULL; curdev = curdev->next)
		if (curdev->class == class)
			return curdev;

	return NULL;
}


/*-------------------------------------------------
    device_list_class_next - return the next
    device in the list of a given class
-------------------------------------------------*/

const device_config *device_list_class_next(const device_config *prevdevice, device_class class)
{
	const device_config *curdev;

	assert(prevdevice != NULL);

	/* scan forward starting with the item after the previous one */
	for (curdev = prevdevice->next; curdev != NULL; curdev = curdev->next)
		if (curdev->class == class)
			return curdev;

	return NULL;
}


/*-------------------------------------------------
    device_list_class_find_by_tag - retrieve a
    device configuration based on a class and tag
-------------------------------------------------*/

const device_config *device_list_class_find_by_tag(const device_config *listhead, device_class class, const char *tag)
{
	const device_config *curdev;

	assert(tag != NULL);

	/* find the device in the list */
	for (curdev = listhead; curdev != NULL; curdev = curdev->next)
		if (curdev->class == class && strcmp(tag, curdev->tag) == 0)
			return curdev;

	/* fail */
	return NULL;
}


/*-------------------------------------------------
    device_list_class_index - return the index of a
    device based on its class and tag
-------------------------------------------------*/

int device_list_class_index(const device_config *listhead, device_class class, const char *tag)
{
	const device_config *curdev;
	int index = 0;

	assert(tag != NULL);

	/* locate all devices */
	for (curdev = listhead; curdev != NULL; curdev = curdev->next)
		if (curdev->class == class)
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

const device_config *device_list_class_find_by_index(const device_config *listhead, device_class class, int index)
{
	const device_config *curdev;

	assert(index >= 0);

	/* find the device in the list */
	for (curdev = listhead; curdev != NULL; curdev = curdev->next)
		if (curdev->class == class && index-- == 0)
			return curdev;

	/* fail */
	return NULL;
}



/***************************************************************************
    LIVE DEVICE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    device_list_start - start the configured list
    of devices for a machine
-------------------------------------------------*/

void device_list_start(running_machine *machine)
{
	device_config *device;

	assert(machine != NULL);

	/* add an exit callback for cleanup */
	add_reset_callback(machine, device_list_reset);
	add_exit_callback(machine, device_list_stop);

	/* iterate over devices and start them */
	for (device = (device_config *)machine->config->devicelist; device != NULL; device = device->next)
	{
		UINT32 tokenlen;
		
		assert(device->token == NULL);
		assert(device->type != NULL);
		assert(device->start != NULL);

		/* get the size of the token data */
		tokenlen = (UINT32)devtype_get_info_int(device->type, DEVINFO_INT_TOKEN_BYTES);
		if (tokenlen == 0)
			fatalerror("Device %s specifies a 0 token length!\n", devtype_name(device->type));

		/* allocate memory for the token */
		device->token = malloc_or_die(tokenlen);
		memset(device->token, 0, tokenlen);

		/* call the start function */
		device->machine = machine;
		(*device->start)(device);
	}
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
		assert(device->token != NULL);
		assert(device->type != NULL);

		/* if we have a stop function, call it */
		if (device->stop != NULL)
			(*device->stop)(device);
		
		/* free allocated memory for the token */
		if (device->token != NULL)
			free(device->token);
		device->token = NULL;
		device->machine = NULL;
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
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type != NULL);

	/* if we have a reset function, call it */
	if (device->reset != NULL)
		(*device->reset)(device);
}


void devtag_reset(running_machine *machine, device_type type, const char *tag)
{
	const device_config *device;

	assert(machine != NULL);
	assert(type != NULL);
	assert(tag != NULL);

	device = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (device == NULL)
		fatalerror("devtag_reset failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	device_reset(device);
}



/***************************************************************************
    DEVICE INFORMATION GETTERS
***************************************************************************/

/*-------------------------------------------------
    devtag_get_token - return the token associated
    with an allocated device
-------------------------------------------------*/

void *devtag_get_token(running_machine *machine, device_type type, const char *tag)
{
	const device_config *device;

	assert(machine != NULL);
	assert(type != NULL);
	assert(tag != NULL);

	device = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (device == NULL)
		fatalerror("devtag_get_token failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	return device->token;
}


/*-------------------------------------------------
    devtag_get_static_config - return a pointer to
    the static configuration for a device based on
    type and tag
-------------------------------------------------*/

const void *devtag_get_static_config(running_machine *machine, device_type type, const char *tag)
{
	const device_config *device;

	assert(machine != NULL);
	assert(type != NULL);
	assert(tag != NULL);

	device = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (device == NULL)
		fatalerror("devtag_get_static_config failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	return device->static_config;
}


/*-------------------------------------------------
    devtag_get_inline_config - return a pointer to
    the inline configuration for a device based on
    type and tag
-------------------------------------------------*/

const void *devtag_get_inline_config(running_machine *machine, device_type type, const char *tag)
{
	const device_config *device;

	assert(machine != NULL);
	assert(type != NULL);
	assert(tag != NULL);

	device = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (device == NULL)
		fatalerror("devtag_get_inline_config failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	return device->inline_config;
}


/*-------------------------------------------------
    device_get_info_int - return an integer state
    value from an allocated device
-------------------------------------------------*/

INT64 device_get_info_int(const device_config *device, UINT32 state)
{
	deviceinfo info;

	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	/* retrieve the value */
	info.i = 0;
	(*device->type)(device, state, &info);
	return info.i;
}


INT64 devtag_get_info_int(running_machine *machine, device_type type, const char *tag, UINT32 state)
{
	const device_config *device;

	assert(machine != NULL);
	assert(type != NULL);
	assert(tag != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	device = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (device == NULL)
		fatalerror("devtag_get_info_int failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	return device_get_info_int(device, state);
}


/*-------------------------------------------------
    device_get_info_ptr - return a pointer state
    value from an allocated device
-------------------------------------------------*/

void *device_get_info_ptr(const device_config *device, UINT32 state)
{
	deviceinfo info;

	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type != NULL);
	assert(state >= DEVINFO_PTR_FIRST && state <= DEVINFO_PTR_LAST);

	/* retrieve the value */
	info.p = NULL;
	(*device->type)(device, state, &info);
	return info.p;
}


void *devtag_get_info_ptr(running_machine *machine, device_type type, const char *tag, UINT32 state)
{
	const device_config *device;

	assert(machine != NULL);
	assert(type != NULL);
	assert(tag != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	device = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (device == NULL)
		fatalerror("devtag_get_info_ptr failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	return device_get_info_ptr(device, state);
}


/*-------------------------------------------------
    device_get_info_fct - return a function
    pointer state value from an allocated device
-------------------------------------------------*/

genf *device_get_info_fct(const device_config *device, UINT32 state)
{
	deviceinfo info;

	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type != NULL);
	assert(state >= DEVINFO_FCT_FIRST && state <= DEVINFO_FCT_LAST);

	/* retrieve the value */
	info.f = 0;
	(*device->type)(device, state, &info);
	return info.f;
}


genf *devtag_get_info_fct(running_machine *machine, device_type type, const char *tag, UINT32 state)
{
	const device_config *device;

	assert(machine != NULL);
	assert(type != NULL);
	assert(tag != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	device = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (device == NULL)
		fatalerror("device_get_info_fct failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	return device_get_info_fct(device, state);
}


/*-------------------------------------------------
    device_get_info_string - return a string value
    from an allocated device
-------------------------------------------------*/

const char *device_get_info_string(const device_config *device, UINT32 state)
{
	deviceinfo info;

	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type != NULL);
	assert(state >= DEVINFO_STR_FIRST && state <= DEVINFO_STR_LAST);

	/* retrieve the value */
	info.s = get_temp_string_buffer();
	(*device->type)(device, state, &info);
	return info.s;
}


const char *devtag_get_info_string(running_machine *machine, device_type type, const char *tag, UINT32 state)
{
	const device_config *device;

	assert(machine != NULL);
	assert(type != NULL);
	assert(tag != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	device = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (device == NULL)
		fatalerror("device_get_info_string failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	return device_get_info_string(device, state);
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
	return info.s;
}



/***************************************************************************
    DEVICE INFORMATION SETTERS
***************************************************************************/

/*-------------------------------------------------
    device_set_info_int - set an integer state
    value for an allocated device
-------------------------------------------------*/

void device_set_info_int(const device_config *device, UINT32 state, INT64 data)
{
	deviceinfo info;

	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	/* set the value */
	info.i = data;
	(*device->set_info)(device, state, &info);
}


void devtag_set_info_int(running_machine *machine, device_type type, const char *tag, UINT32 state, INT64 data)
{
	const device_config *device;

	assert(machine != NULL);
	assert(type != NULL);
	assert(tag != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	device = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (device == NULL)
		fatalerror("devtag_set_info_int failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	device_set_info_int(device, state, data);
}


/*-------------------------------------------------
    device_set_info_ptr - set a pointer state
    value for an allocated device
-------------------------------------------------*/

void device_set_info_ptr(const device_config *device, UINT32 state, void *data)
{
	deviceinfo info;

	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type != NULL);
	assert(state >= DEVINFO_PTR_FIRST && state <= DEVINFO_PTR_LAST);

	/* set the value */
	info.p = data;
	(*device->set_info)(device, state, &info);
}


void devtag_set_info_ptr(running_machine *machine, device_type type, const char *tag, UINT32 state, void *data)
{
	const device_config *device;

	assert(machine != NULL);
	assert(type != NULL);
	assert(tag != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	device = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (device == NULL)
		fatalerror("devtag_set_info_ptr failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	device_set_info_ptr(device, state, data);
}


/*-------------------------------------------------
    device_set_info_fct - set a function pointer
    state value for an allocated device
-------------------------------------------------*/

void device_set_info_fct(const device_config *device, UINT32 state, genf *data)
{
	deviceinfo info;

	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type != NULL);
	assert(state >= DEVINFO_FCT_FIRST && state <= DEVINFO_FCT_LAST);

	/* set the value */
	info.f = data;
	(*device->set_info)(device, state, &info);
}


void devtag_set_info_fct(running_machine *machine, device_type type, const char *tag, UINT32 state, genf *data)
{
	const device_config *device;

	assert(machine != NULL);
	assert(type != NULL);
	assert(tag != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);

	device = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (device == NULL)
		fatalerror("devtag_set_info_fct failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	device_set_info_fct(device, state, data);
}
