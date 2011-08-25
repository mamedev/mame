/***************************************************************************

    Cartrige loading

***************************************************************************/

#ifndef __CARTSLOT_H__
#define __CARTSLOT_H__

#include "image.h"


/***************************************************************************
    MACROS
***************************************************************************/
#define ROM_CART_LOAD(tag,offset,length,flags)	\
	{ NULL, tag, offset, length, ROMENTRYTYPE_CARTRIDGE | (flags) },

#define ROM_MIRROR		0x01000000
#define ROM_NOMIRROR	0x00000000
#define ROM_FULLSIZE	0x02000000
#define ROM_FILL_FF		0x04000000
#define ROM_NOCLEAR		0x08000000

DECLARE_LEGACY_IMAGE_DEVICE(CARTSLOT, cartslot);



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _cartslot_config cartslot_config;
struct _cartslot_config
{
	const char *					extensions;
	const char *					interface;
	int								must_be_loaded;
	device_start_func				device_start;
	device_image_load_func			device_load;
	device_image_unload_func		device_unload;
	device_image_partialhash_func	device_partialhash;
	device_image_display_info_func	device_displayinfo;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_CARTSLOT_ADD(_tag) 										\
	MCFG_DEVICE_ADD(_tag, CARTSLOT, 0)									\

#define MCFG_CARTSLOT_MODIFY(_tag)										\
	MCFG_DEVICE_MODIFY(_tag)									\

#define MCFG_CARTSLOT_EXTENSION_LIST(_extensions)						\
	MCFG_DEVICE_CONFIG_DATAPTR(cartslot_config, extensions, _extensions)

#define MCFG_CARTSLOT_NOT_MANDATORY										\
	MCFG_DEVICE_CONFIG_DATA32(cartslot_config, must_be_loaded, FALSE)

#define MCFG_CARTSLOT_MANDATORY											\
	MCFG_DEVICE_CONFIG_DATA32(cartslot_config, must_be_loaded, TRUE)

#define MCFG_CARTSLOT_START(_start)										\
	MCFG_DEVICE_CONFIG_DATAPTR(cartslot_config, device_start, DEVICE_START_NAME(_start))

#define MCFG_CARTSLOT_LOAD(_load)										\
	MCFG_DEVICE_CONFIG_DATAPTR(cartslot_config, device_load, DEVICE_IMAGE_LOAD_NAME(_load))

#define MCFG_CARTSLOT_UNLOAD(_unload)									\
	MCFG_DEVICE_CONFIG_DATAPTR(cartslot_config, device_unload, DEVICE_IMAGE_UNLOAD_NAME(_unload))

#define MCFG_CARTSLOT_DISPLAY_INFO(_displayinfo)										\
	MCFG_DEVICE_CONFIG_DATAPTR(cartslot_config, device_displayinfo, DEVICE_IMAGE_DISPLAY_INFO_NAME(_displayinfo))

#define MCFG_CARTSLOT_PARTIALHASH(_partialhash)							\
	MCFG_DEVICE_CONFIG_DATAPTR(cartslot_config, device_partialhash, _partialhash)

#define MCFG_CARTSLOT_INTERFACE(_interface)							\
	MCFG_DEVICE_CONFIG_DATAPTR(cartslot_config, interface, _interface )

#endif /* __cart_slot_H__ */
