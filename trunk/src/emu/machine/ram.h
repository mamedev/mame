/*************************************************************************

    RAM device

    Provides a configurable amount of RAM to drivers

**************************************************************************/

#ifndef __RAM_H__
#define __RAM_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define RAM_DEFAULT_VALUE	0xcd
#define RAM_TAG				"ram"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ram_config ram_config;
struct _ram_config
{
	const char *default_size;
	const char *extra_options;
	UINT8 default_value;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(RAM, ram);

#define MCFG_RAM_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, RAM, 0) \
	MCFG_DEVICE_CONFIG_DATA32(ram_config, default_value, RAM_DEFAULT_VALUE)

#define MCFG_RAM_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_RAM_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)	\
	MCFG_DEVICE_CONFIG_DATAPTR(ram_config, extra_options, NULL)

#define MCFG_RAM_DEFAULT_SIZE(_default_size) \
	MCFG_DEVICE_CONFIG_DATAPTR(ram_config, default_size, _default_size)

#define MCFG_RAM_EXTRA_OPTIONS(_extra_options) \
	MCFG_DEVICE_CONFIG_DATAPTR(ram_config, extra_options, _extra_options)

#define MCFG_RAM_DEFAULT_VALUE(_default_value) \
	MCFG_DEVICE_CONFIG_DATA32(ram_config, default_value, _default_value)


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

UINT32 ram_get_size(device_t *device);
UINT8 *ram_get_ptr(device_t *device);
UINT32 ram_parse_string(const char *s);

#endif /* __RAM_H__ */
