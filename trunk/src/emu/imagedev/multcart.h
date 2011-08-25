/*********************************************************************

    multcart.h

    Multi-cartridge handling code

*********************************************************************/

#ifndef __MULTCART_H__
#define __MULTCART_H__

#include "osdcore.h"

#define TAG_PCB		"pcb"

DECLARE_LEGACY_IMAGE_DEVICE(MULTICARTSLOT, multicartslot);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

enum _multicart_load_flags
{
	MULTICART_FLAGS_DONT_LOAD_RESOURCES	= 0x00,
	MULTICART_FLAGS_LOAD_RESOURCES		= 0x01
};
typedef enum _multicart_load_flags multicart_load_flags;

enum _multicart_resource_type
{
	MULTICART_RESOURCE_TYPE_INVALID,
	MULTICART_RESOURCE_TYPE_ROM,
	MULTICART_RESOURCE_TYPE_RAM
};
typedef enum _multicart_resource_type multicart_resource_type;

typedef struct _multicart_resource multicart_resource;
struct _multicart_resource
{
	const char *				id;
	const char *				filename;
	multicart_resource *		next;
	multicart_resource_type		type;
	UINT32						length;
	void *						ptr;
};


typedef struct _multicart_socket multicart_socket;
struct _multicart_socket
{
	const char *				id;
	multicart_socket *			next;
	const multicart_resource *	resource;
	void *						ptr;
};


typedef struct _multicart_private multicart_private;

typedef struct _multicart_t multicart_t;
struct _multicart_t
{
	const multicart_resource *	resources;
	const multicart_socket *	sockets;
	const char *				pcb_type;
	const char *				gamedrv_name; /* need this to find the path to the nvram files */
	multicart_private *			data;
};

typedef struct _multicartslot_t multicartslot_t;
struct _multicartslot_t
{
	device_t *pcb_device;
	multicart_t *mc;
};


typedef struct _multicartslot_pcb_type multicartslot_pcb_type;
struct _multicartslot_pcb_type
{
	const char *					name;
	device_type						devtype;
};


typedef struct _multicartslot_config multicartslot_config;
struct _multicartslot_config
{
	const char *					extensions;
	device_start_func				device_start;
	device_image_load_func			device_load;
	device_image_unload_func		device_unload;
	multicartslot_pcb_type			pcb_types[16];
};

enum _multicart_open_error
{
	MCERR_NONE,
	MCERR_NOT_MULTICART,
	MCERR_CORRUPT,
	MCERR_OUT_OF_MEMORY,
	MCERR_XML_ERROR,
	MCERR_INVALID_FILE_REF,
	MCERR_ZIP_ERROR,
	MCERR_MISSING_RAM_LENGTH,
	MCERR_INVALID_RAM_SPEC,
	MCERR_UNKNOWN_RESOURCE_TYPE,
	MCERR_INVALID_RESOURCE_REF,
	MCERR_INVALID_FILE_FORMAT,
	MCERR_MISSING_LAYOUT,
	MCERR_NO_PCB_OR_RESOURCES
};
typedef enum _multicart_open_error multicart_open_error;

const char *multicart_error_text(multicart_open_error error);

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* opens a multicart */
multicart_open_error multicart_open(emu_options &options, const char *filename, const char *drvname, multicart_load_flags load_flags, multicart_t **cart);

/* closes a multicart */
void multicart_close(emu_options &options, multicart_t *cart);

/* accesses the PCB associated with this cartslot */
device_t *cartslot_get_pcb(device_t *device);

/* accesses a particular socket */
void *cartslot_get_socket(device_t *device, const char *socket_name);

/* accesses a particular socket; gets the length of the associated resource */
int cartslot_get_resource_length(device_t *device, const char *socket_name);

#define DECLARE_LEGACY_CART_SLOT_DEVICE(name, basename) _DECLARE_LEGACY_DEVICE(name, basename, basename##_device, legacy_cart_slot_device_base)
#define DEFINE_LEGACY_CART_SLOT_DEVICE(name, basename) _DEFINE_LEGACY_DEVICE(name, basename, basename##_device, legacy_cart_slot_device_base)

#define MCFG_MULTICARTSLOT_ADD(_tag)										\
	MCFG_DEVICE_ADD(_tag, MULTICARTSLOT, 0)									\

#define MCFG_MULTICARTSLOT_MODIFY(_tag)										\
	MCFG_DEVICE_MODIFY(_tag)									\

#define MCFG_MULTICARTSLOT_PCBTYPE(_index, _pcb_type_name, _pcb_devtype)			\
	MCFG_DEVICE_CONFIG_DATAPTR_ARRAY_MEMBER(multicartslot_config, pcb_types, _index, multicartslot_pcb_type, name, _pcb_type_name) \
	MCFG_DEVICE_CONFIG_DATAPTR_ARRAY_MEMBER(multicartslot_config, pcb_types, _index, multicartslot_pcb_type, devtype, _pcb_devtype)

#define MCFG_MULTICARTSLOT_START(_start)										\
	MCFG_DEVICE_CONFIG_DATAPTR(multicartslot_config, device_start, DEVICE_START_NAME(_start))

#define MCFG_MULTICARTSLOT_LOAD(_load)										\
	MCFG_DEVICE_CONFIG_DATAPTR(multicartslot_config, device_load, DEVICE_IMAGE_LOAD_NAME(_load))

#define MCFG_MULTICARTSLOT_UNLOAD(_unload)									\
	MCFG_DEVICE_CONFIG_DATAPTR(multicartslot_config, device_unload, DEVICE_IMAGE_UNLOAD_NAME(_unload))

#define MCFG_MULTICARTSLOT_EXTENSION_LIST(_extensions)						\
	MCFG_DEVICE_CONFIG_DATAPTR(multicartslot_config, extensions, _extensions)


// ======================> device_cart_slot_interface

// class representing interface-specific live cart_slot
class device_cart_slot_interface : public device_interface
{
public:
	// construction/destruction
	device_cart_slot_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_cart_slot_interface();
};


// ======================> legacy_cart_slot_device

// legacy_cart_slot_device is a legacy_device_base with a cart_slot interface
class legacy_cart_slot_device_base :	public legacy_device_base,
										public device_cart_slot_interface
{
protected:
	// construction/destruction
	legacy_cart_slot_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, device_get_config_func get_config);

public:
	using legacy_device_base::get_legacy_int;
	using legacy_device_base::get_legacy_fct;
	using legacy_device_base::get_legacy_ptr;

	// device_cart_slot_interface overrides
};


#endif /* __MULTCART_H__ */
