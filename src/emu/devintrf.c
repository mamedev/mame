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
	deviceinfo info;
	
	assert(listheadptr != NULL);
	assert(type != NULL);
	assert(tag != NULL);
	
	/* find the end of the list, and ensure no duplicates along the way */
	for (devptr = listheadptr; *devptr != NULL; devptr = &(*devptr)->next)
		if (type == (*devptr)->type && strcmp(tag, (*devptr)->tag) == 0)
			fatalerror("Attempted to add duplicate device: type=%s tag=%s\n", devtype_name(type), tag);
	
	/* allocate a new device */
	device = malloc_or_die(sizeof(*device) + strlen(tag));
	
	/* populate all fields */
	device->next = NULL;
	device->type = type;
	device->flags = 0;
	device->clock = 0;
	device->config = NULL;
	device->token = NULL;
	strcpy(device->tag, tag);

	/* fetch function pointers to the core functions */
	info.set_info = NULL;
	(*type)(NULL, NULL, DEVINFO_FCT_SET_INFO, &info);
	device->set_info = info.set_info;
	
	info.start = NULL;
	(*type)(NULL, NULL, DEVINFO_FCT_START, &info);
	device->start = info.start;
	
	info.stop = NULL;
	(*type)(NULL, NULL, DEVINFO_FCT_STOP, &info);
	device->stop = info.stop;
	
	info.reset = NULL;
	(*type)(NULL, NULL, DEVINFO_FCT_RESET, &info);
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


/*-------------------------------------------------
    device_list_find_by_tag - retrieve a device
    configuration based on a type and tag
-------------------------------------------------*/

const device_config *device_list_find_by_tag(const device_config *listhead, device_type type, const char *tag)
{
	const device_config *device;

	assert(type != NULL);
	assert(tag != NULL);

	/* find the device in the list */
	for (device = listhead; device != NULL; device = device->next)
		if (type == device->type && strcmp(tag, device->tag) == 0)
			return device;
	
	/* fail */
	return NULL;
}


/*-------------------------------------------------
    device_list_find_by_index - retrieve a device
    configuration based on a type and index
-------------------------------------------------*/

const device_config *device_list_find_by_index(const device_config *listhead, device_type type, int index)
{
	const device_config *device;

	assert(listhead != NULL);
	assert(type != NULL);

	/* find the device in the list */
	for (device = listhead; device != NULL; device = device->next)
		if (type == device->type && index-- == 0)
			return device;
	
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
	device_config *config;
	
	/* add an exit callback for cleanup */
	add_reset_callback(machine, device_list_reset);
	add_exit_callback(machine, device_list_stop);
	
	/* iterate over devices and start them */
	for (config = (device_config *)machine->config->devicelist; config != NULL; config = config->next)
	{
		assert(config->token == NULL);
		assert(config->type != NULL);
		assert(config->start != NULL);

		/* call the start function */	
		config->token = (*config->start)(machine, config->clock, config->flags, config->config);
		assert(config->token != NULL);
			
		/* fatal error if this fails */
		if (config->token == NULL)
			fatalerror("Error starting device: type=%s tag=%s\n", devtype_name(config->type), config->tag);
	}
}


/*-------------------------------------------------
    device_list_stop - stop the configured list 
    of devices for a machine
-------------------------------------------------*/

static void device_list_stop(running_machine *machine)
{
	device_config *config;
	
	/* iterate over devices and stop them */
	for (config = (device_config *)machine->config->devicelist; config != NULL; config = config->next)
	{
		assert(config->token != NULL);
		assert(config->type != NULL);
		
		/* if we have a stop function, call it */
		if (config->stop != NULL)
			(*config->stop)(machine, config->token);
			
		/* clear the token to indicate we are finished */
		config->token = NULL;
	}
}


/*-------------------------------------------------
    device_list_reset - reset the configured list 
    of devices for a machine
-------------------------------------------------*/

static void device_list_reset(running_machine *machine)
{
	const device_config *config;
	
	/* iterate over devices and stop them */
	for (config = (device_config *)machine->config->devicelist; config != NULL; config = config->next)
		device_reset(machine, config);
}


/*-------------------------------------------------
    device_reset - reset a device based on an 
    allocated device_config
-------------------------------------------------*/

void device_reset(running_machine *machine, const device_config *config)
{
	assert(config->token != NULL);
	assert(config->type != NULL);
	
	/* if we have a reset function, call it */
	if (config->reset != NULL)
		(*config->reset)(machine, config->token);
}


void devtag_reset(running_machine *machine, device_type type, const char *tag)
{
	const device_config *config = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (config == NULL)
		fatalerror("devtag_reset failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	device_reset(machine, config);
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
	const device_config *config = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (config == NULL)
		fatalerror("devtag_get_token failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	return config->token;
}


/*-------------------------------------------------
    device_get_info_int - return an integer state 
    value from an allocated device
-------------------------------------------------*/

INT64 device_get_info_int(running_machine *machine, const device_config *config, UINT32 state)
{
	deviceinfo info;
	
	assert(config->token != NULL);
	assert(config->type != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);
	
	/* retrieve the value */
	info.i = 0;
	(*config->type)(machine, config->token, state, &info);
	return info.i;
}


INT64 devtag_get_info_int(running_machine *machine, device_type type, const char *tag, UINT32 state)
{
	const device_config *config = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (config == NULL)
		fatalerror("devtag_get_info_int failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	return device_get_info_int(machine, config, state);
}


/*-------------------------------------------------
    device_get_info_ptr - return a pointer state 
    value from an allocated device
-------------------------------------------------*/

void *device_get_info_ptr(running_machine *machine, const device_config *config, UINT32 state)
{
	deviceinfo info;
	
	assert(config->token != NULL);
	assert(config->type != NULL);
	assert(state >= DEVINFO_PTR_FIRST && state <= DEVINFO_PTR_LAST);
	
	/* retrieve the value */
	info.p = NULL;
	(*config->type)(machine, config->token, state, &info);
	return info.p;
}


void *devtag_get_info_ptr(running_machine *machine, device_type type, const char *tag, UINT32 state)
{
	const device_config *config = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (config == NULL)
		fatalerror("devtag_get_info_ptr failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	return device_get_info_ptr(machine, config, state);
}


/*-------------------------------------------------
    device_get_info_fct - return a function 
    pointer state value from an allocated device
-------------------------------------------------*/

genf *device_get_info_fct(running_machine *machine, const device_config *config, UINT32 state)
{
	deviceinfo info;
	
	assert(config->token != NULL);
	assert(config->type != NULL);
	assert(state >= DEVINFO_FCT_FIRST && state <= DEVINFO_FCT_LAST);
	
	/* retrieve the value */
	info.f = 0;
	(*config->type)(machine, config->token, state, &info);
	return info.f;
}


genf *devtag_get_info_fct(running_machine *machine, device_type type, const char *tag, UINT32 state)
{
	const device_config *config = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (config == NULL)
		fatalerror("device_get_info_fct failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	return device_get_info_fct(machine, config, state);
}


/*-------------------------------------------------
    device_get_info_string - return a string value 
    from an allocated device
-------------------------------------------------*/

const char *device_get_info_string(running_machine *machine, const device_config *config, UINT32 state)
{
	deviceinfo info;
	
	assert(config->token != NULL);
	assert(config->type != NULL);
	assert(state >= DEVINFO_STR_FIRST && state <= DEVINFO_STR_LAST);
	
	/* retrieve the value */
	info.s = get_temp_string_buffer();
	(*config->type)(machine, config->token, state, &info);
	return info.s;
}


const char *devtag_get_info_string(running_machine *machine, device_type type, const char *tag, UINT32 state)
{
	const device_config *config = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (config == NULL)
		fatalerror("device_get_info_string failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	return device_get_info_string(machine, config, state);
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
	(*type)(NULL, NULL, state, &info);
	return info.s;
}



/***************************************************************************
    DEVICE INFORMATION SETTERS
***************************************************************************/

/*-------------------------------------------------
    device_set_info_int - set an integer state 
    value for an allocated device
-------------------------------------------------*/

void device_set_info_int(running_machine *machine, const device_config *config, UINT32 state, INT64 data)
{
	deviceinfo info;
	
	assert(config->token != NULL);
	assert(config->type != NULL);
	assert(state >= DEVINFO_INT_FIRST && state <= DEVINFO_INT_LAST);
	
	/* set the value */
	info.i = data;
	(*config->set_info)(machine, config->token, state, &info);
}


void devtag_set_info_int(running_machine *machine, device_type type, const char *tag, UINT32 state, INT64 data)
{
	const device_config *config = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (config == NULL)
		fatalerror("devtag_set_info_int failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	device_set_info_int(machine, config, state, data);
}


/*-------------------------------------------------
    device_set_info_ptr - set a pointer state 
    value for an allocated device
-------------------------------------------------*/

void device_set_info_ptr(running_machine *machine, const device_config *config, UINT32 state, void *data)
{
	deviceinfo info;
	
	assert(config->token != NULL);
	assert(config->type != NULL);
	assert(state >= DEVINFO_PTR_FIRST && state <= DEVINFO_PTR_LAST);
	
	/* set the value */
	info.p = data;
	(*config->set_info)(machine, config->token, state, &info);
}


void devtag_set_info_ptr(running_machine *machine, device_type type, const char *tag, UINT32 state, void *data)
{
	const device_config *config = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (config == NULL)
		fatalerror("devtag_set_info_ptr failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	device_set_info_ptr(machine, config, state, data);
}


/*-------------------------------------------------
    device_set_info_fct - set a function pointer 
    state value for an allocated device
-------------------------------------------------*/

void device_set_info_fct(running_machine *machine, const device_config *config, UINT32 state, genf *data)
{
	deviceinfo info;
	
	assert(config->token != NULL);
	assert(config->type != NULL);
	assert(state >= DEVINFO_FCT_FIRST && state <= DEVINFO_FCT_LAST);
	
	/* set the value */
	info.f = data;
	(*config->set_info)(machine, config->token, state, &info);
}


void devtag_set_info_fct(running_machine *machine, device_type type, const char *tag, UINT32 state, genf *data)
{
	const device_config *config = device_list_find_by_tag(machine->config->devicelist, type, tag);
	if (config == NULL)
		fatalerror("devtag_set_info_fct failed to find device: type=%s tag=%s\n", devtype_name(type), tag);
	device_set_info_fct(machine, config, state, data);
}
