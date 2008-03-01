/***************************************************************************

    devintrf.h

    Device interface functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __DEVINTRF_H__
#define __DEVINTRF_H__

#include "mamecore.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* device classes */
enum _device_class
{
	DEVICE_CLASS_GENERAL = 0,			/* default class for general devices */
	DEVICE_CLASS_PERIPHERAL,			/* peripheral devices: PIAs, timers, etc. */
	DEVICE_CLASS_AUDIO,					/* audio devices (not sound chips), including speakers */
	DEVICE_CLASS_VIDEO,					/* video devices, including screens */
	DEVICE_CLASS_CPU_CHIP,				/* CPU chips; only CPU cores should return this class */
	DEVICE_CLASS_SOUND_CHIP,			/* sound chips; only sound cores should return this class */
	DEVICE_CLASS_OTHER					/* anything else (the list may expand in the future) */
};
typedef enum _device_class device_class;


/* useful in device_list functions for scanning through all devices */
#define DEVICE_TYPE_WILDCARD			NULL


/* state constants passed to the device_get_info_func */
enum
{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	DEVINFO_INT_FIRST = 0x00000,

	DEVINFO_INT_INLINE_CONFIG_BYTES = DEVINFO_INT_FIRST,/* R/O: bytes of inline configuration */
	DEVINFO_INT_CLASS,									/* R/O: the device's class */

	DEVINFO_INT_DEVICE_SPECIFIC = 0x08000,				/* R/W: device-specific values start here */

	DEVINFO_INT_LAST = 0x0ffff,

	/* --- the following bits of info are returned as pointers --- */
	DEVINFO_PTR_FIRST = 0x10000,

	DEVINFO_PTR_DEVICE_SPECIFIC = 0x18000,				/* R/W: device-specific values start here */

	DEVINFO_PTR_LAST = 0x1ffff,

	/* --- the following bits of info are returned as pointers to functions --- */
	DEVINFO_FCT_FIRST = 0x20000,

	DEVINFO_FCT_SET_INFO = DEVINFO_FCT_FIRST,			/* R/O: device_set_info_func */
	DEVINFO_FCT_START,									/* R/O: device_start_func */
	DEVINFO_FCT_STOP,									/* R/O: device_stop_func */
	DEVINFO_FCT_RESET,									/* R/O: device_reset_func */

	DEVINFO_FCT_DEVICE_SPECIFIC = 0x28000,				/* R/W: device-specific values start here */

	DEVINFO_FCT_LAST = 0x1ffff,

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	DEVINFO_STR_FIRST = 0x30000,

	DEVINFO_STR_NAME = DEVINFO_STR_FIRST,				/* R/O: name of the device */
	DEVINFO_STR_FAMILY,									/* R/O: family of the device */
	DEVINFO_STR_VERSION,								/* R/O: version of the device */
	DEVINFO_STR_SOURCE_FILE,							/* R/O: file containing the device implementation */
	DEVINFO_STR_CREDITS,								/* R/O: credits for the device implementation */

	DEVINFO_STR_DEVICE_SPECIFIC = 0x38000,				/* R/W: device-specific values start here */

	DEVINFO_STR_LAST = 0x3ffff
};



/***************************************************************************
    MACROS
***************************************************************************/

#define DEVICE_GET_INFO_NAME(name)	device_get_info_##name
#define DEVICE_GET_INFO(name)		void DEVICE_GET_INFO_NAME(name)(running_machine *machine, void *token, UINT32 state, deviceinfo *info)
#define DEVICE_GET_INFO_CALL(name)	DEVICE_GET_INFO_NAME(name)(machine, token, state, info)

#define DEVICE_SET_INFO_NAME(name)	device_set_info_##name
#define DEVICE_SET_INFO(name)		void DEVICE_SET_INFO_NAME(name)(running_machine *machine, void *token, UINT32 state, const deviceinfo *info)
#define DEVICE_SET_INFO_CALL(name)	DEVICE_SET_INFO_NAME(name)(machine, token, state, info)

#define DEVICE_START_NAME(name)		device_start_##name
#define DEVICE_START(name)			void *DEVICE_START_NAME(name)(running_machine *machine, const char *tag, const void *static_config, const void *inline_config)
#define DEVICE_START_CALL(name)		DEVICE_START_NAME(name)(machine, tag, static_config, inline_config)

#define DEVICE_STOP_NAME(name)		device_stop_##name
#define DEVICE_STOP(name)			void DEVICE_STOP_NAME(name)(running_machine *machine, void *token)
#define DEVICE_STOP_CALL(name)		DEVICE_STOP_NAME(name)(machine, token)

#define DEVICE_RESET_NAME(name)		device_reset_##name
#define DEVICE_RESET(name)			void DEVICE_RESET_NAME(name)(running_machine *machine, void *token)
#define DEVICE_RESET_CALL(name)		DEVICE_RESET_NAME(name)(machine, token)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* forward-declare this type */
typedef union _deviceinfo deviceinfo;


/* device interface function types */
typedef void (*device_get_info_func)(running_machine *machine, void *token, UINT32 state, deviceinfo *info);
typedef void (*device_set_info_func)(running_machine *machine, void *token, UINT32 state, const deviceinfo *info);
typedef void *(*device_start_func)(running_machine *machine, const char *tag, const void *static_config, const void *inline_config);
typedef void (*device_stop_func)(running_machine *machine, void *token);
typedef void (*device_reset_func)(running_machine *machine, void *token);


/* a device_type is simply a pointer to its get_info function */
typedef device_get_info_func device_type;


/* the actual deviceinfo union */
union _deviceinfo
{
	INT64					i;						/* generic integers */
	void *					p;						/* generic pointers */
	genf *  				f;						/* generic function pointers */
	const char *			s;						/* generic strings */

	device_set_info_func 	set_info;				/* DEVINFO_FCT_SET_INFO */
	device_start_func		start;					/* DEVINFO_FCT_START */
	device_stop_func		stop;					/* DEVINFO_FCT_STOP */
	device_reset_func		reset;					/* DEVINFO_FCT_RESET */
};


/* the configuration for a general device */
typedef struct _device_config device_config;
struct _device_config
{
	device_config *			next;					/* next device */
	device_type				type;					/* device type */
	device_class			class;					/* device class */
	device_set_info_func 	set_info;				/* quick pointer to set_info callback */
	device_start_func		start;					/* quick pointer to start callback */
	device_stop_func		stop;					/* quick pointer to stop callback */
	device_reset_func		reset;					/* quick pointer to reset callback */
	const void *			static_config;			/* static device configuration */
	void *					inline_config;			/* inline device configuration */
	void *					token;					/* token if device is live */
	char					tag[1];					/* tag for this instance */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- device configuration ----- */

/* add a new device to the end of a device list */
device_config *device_list_add(device_config **listheadptr, device_type type, const char *tag);

/* remove a device from a device list */
void device_list_remove(device_config **listheadptr, device_type type, const char *tag);



/* ----- type-based device access ----- */

/* return the number of items of a given type; DEVICE_TYPE_WILDCARD is allowed */
int device_list_items(const device_config *listhead, device_type type);

/* return the first device in the list of a given type; DEVICE_TYPE_WILDCARD is allowed */
const device_config *device_list_first(const device_config *listhead, device_type type);

/* return the next device in the list of a given type; DEVICE_TYPE_WILDCARD is allowed */
const device_config *device_list_next(const device_config *prevdevice, device_type type);

/* retrieve a device configuration based on a type and tag; DEVICE_TYPE_WILDCARD is allowed */
const device_config *device_list_find_by_tag(const device_config *listhead, device_type type, const char *tag);

/* return the index of a device based on its type and tag; DEVICE_TYPE_WILDCARD is allowed */
int device_list_index(const device_config *listhead, device_type type, const char *tag);

/* retrieve a device configuration based on a type and index; DEVICE_TYPE_WILDCARD is allowed */
const device_config *device_list_find_by_index(const device_config *listhead, device_type type, int index);



/* ----- class-based device access ----- */

/* return the number of items of a given class */
int device_list_class_items(const device_config *listhead, device_class class);

/* return the first device in the list of a given class */
const device_config *device_list_class_first(const device_config *listhead, device_class class);

/* return the next device in the list of a given class */
const device_config *device_list_class_next(const device_config *prevdevice, device_class class);

/* retrieve a device configuration based on a class and tag */
const device_config *device_list_class_find_by_tag(const device_config *listhead, device_class class, const char *tag);

/* return the index of a device based on its class and tag */
int device_list_class_index(const device_config *listhead, device_class class, const char *tag);

/* retrieve a device configuration based on a class and index */
const device_config *device_list_class_find_by_index(const device_config *listhead, device_class class, int index);



/* ----- live device management ----- */

/* start the configured list of devices for a machine */
void device_list_start(running_machine *machine);

/* reset a device based on an allocated device_config */
void device_reset(running_machine *machine, const device_config *config);
void devtag_reset(running_machine *machine, device_type type, const char *tag);



/* ----- device information getters ----- */

/* return the token associated with an allocated device */
void *devtag_get_token(running_machine *machine, device_type type, const char *tag);

/* return a pointer to the static configuration for a device based on type and tag */
const void *devtag_get_static_config(running_machine *machine, device_type type, const char *tag);

/* return a pointer to the inline configuration for a device based on type and tag */
const void *devtag_get_inline_config(running_machine *machine, device_type type, const char *tag);

/* return an integer state value from an allocated device */
INT64 device_get_info_int(running_machine *machine, const device_config *config, UINT32 state);
INT64 devtag_get_info_int(running_machine *machine, device_type type, const char *tag, UINT32 state);

/* return a pointer state value from an allocated device */
void *device_get_info_ptr(running_machine *machine, const device_config *config, UINT32 state);
void *devtag_get_info_ptr(running_machine *machine, device_type type, const char *tag, UINT32 state);

/* return a function pointer state value from an allocated device */
genf *device_get_info_fct(running_machine *machine, const device_config *config, UINT32 state);
genf *devtag_get_info_fct(running_machine *machine, device_type type, const char *tag, UINT32 state);

/* return a string value from an allocated device */
const char *device_get_info_string(running_machine *machine, const device_config *config, UINT32 state);
const char *devtag_get_info_string(running_machine *machine, device_type type, const char *tag, UINT32 state);



/* ----- device type information getters ----- */

/* return an integer value from a device type (does not need to be allocated) */
INT64 devtype_get_info_int(device_type type, UINT32 state);

/* return a string value from a device type (does not need to be allocated) */
const char *devtype_get_info_string(device_type type, UINT32 state);



/* ----- device information setters ----- */

/* set an integer state value for an allocated device */
void device_set_info_int(running_machine *machine, const device_config *config, UINT32 state, INT64 data);
void devtag_set_info_int(running_machine *machine, device_type type, const char *tag, UINT32 state, INT64 data);

/* set a pointer state value for an allocated device */
void device_set_info_ptr(running_machine *machine, const device_config *config, UINT32 state, void *data);
void devtag_set_info_ptr(running_machine *machine, device_type type, const char *tag, UINT32 state, void *data);

/* set a function pointer state value for an allocated device */
void device_set_info_fct(running_machine *machine, const device_config *config, UINT32 state, genf *data);
void devtag_set_info_fct(running_machine *machine, device_type type, const char *tag, UINT32 state, genf *data);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/* return common strings based on device types */
INLINE const char *devtype_name(device_type devtype)		{ return devtype_get_info_string(devtype, DEVINFO_STR_NAME); }
INLINE const char *devtype_family(device_type devtype) 		{ return devtype_get_info_string(devtype, DEVINFO_STR_FAMILY); }
INLINE const char *devtype_version(device_type devtype) 	{ return devtype_get_info_string(devtype, DEVINFO_STR_VERSION); }
INLINE const char *devtype_source_file(device_type devtype) { return devtype_get_info_string(devtype, DEVINFO_STR_SOURCE_FILE); }
INLINE const char *devtype_credits(device_type devtype) 	{ return devtype_get_info_string(devtype, DEVINFO_STR_CREDITS); }


#endif	/* __DEVINTRF_H__ */
