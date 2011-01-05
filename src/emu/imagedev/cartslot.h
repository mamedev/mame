/***************************************************************************

    Cartrige loading

***************************************************************************/

#ifndef __CARTSLOT_H__
#define __CARTSLOT_H__

#include "image.h"
#include "multcart.h"


/***************************************************************************
    MACROS
***************************************************************************/
#define TAG_PCB		"pcb"

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

typedef struct _cartslot_t cartslot_t;
struct _cartslot_t
{
	device_t *pcb_device;
	multicart_t *mc;
};


typedef struct _cartslot_pcb_type cartslot_pcb_type;
struct _cartslot_pcb_type
{
	const char *					name;
	device_type						devtype;
};


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
	cartslot_pcb_type				pcb_types[16];
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* accesses the PCB associated with this cartslot */
device_t *cartslot_get_pcb(device_t *device);

/* accesses a particular socket */
void *cartslot_get_socket(device_t *device, const char *socket_name);

/* accesses a particular socket; gets the length of the associated resource */
int cartslot_get_resource_length(device_t *device, const char *socket_name);


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

#define MCFG_CARTSLOT_PARTIALHASH(_partialhash)							\
	MCFG_DEVICE_CONFIG_DATAPTR(cartslot_config, device_partialhash, _partialhash)

#define MCFG_CARTSLOT_PCBTYPE(_index, _pcb_type_name, _pcb_devtype)			\
	MCFG_DEVICE_CONFIG_DATAPTR_ARRAY_MEMBER(cartslot_config, pcb_types, _index, cartslot_pcb_type, name, _pcb_type_name) \
	MCFG_DEVICE_CONFIG_DATAPTR_ARRAY_MEMBER(cartslot_config, pcb_types, _index, cartslot_pcb_type, devtype, _pcb_devtype)

#define MCFG_CARTSLOT_INTERFACE(_interface)							\
	MCFG_DEVICE_CONFIG_DATAPTR(cartslot_config, interface, _interface )

#define DECLARE_LEGACY_CART_SLOT_DEVICE(name, basename) _DECLARE_LEGACY_DEVICE(name, basename, basename##_device_config, basename##_device, legacy_cart_slot_device_config_base, legacy_cart_slot_device_base)
#define DEFINE_LEGACY_CART_SLOT_DEVICE(name, basename) _DEFINE_LEGACY_DEVICE(name, basename, basename##_device_config, basename##_device, legacy_cart_slot_device_config_base, legacy_cart_slot_device_base)

// ======================> device_config_cart_slot_interface

// class representing interface-specific configuration cart_slot
class device_config_cart_slot_interface : public device_config_interface
{
public:
	// construction/destruction
	device_config_cart_slot_interface(const machine_config &mconfig, device_config &device);
	virtual ~device_config_cart_slot_interface();
};



// ======================> device_cart_slot_interface

// class representing interface-specific live cart_slot
class device_cart_slot_interface : public device_interface
{
public:
	// construction/destruction
	device_cart_slot_interface(running_machine &machine, const device_config &config, device_t &device);
	virtual ~device_cart_slot_interface();

	// configuration access
	const device_config_cart_slot_interface &cart_slot_config() const { return m_cart_slot_config; }
protected:

	// configuration
	const device_config_cart_slot_interface &m_cart_slot_config;	// reference to our device_config_execute_interface
};
// ======================> legacy_cart_slot_device_config

// legacy_cart_slot_device_config is a device_config with a cart_slot interface
class legacy_cart_slot_device_config_base : 	public legacy_device_config_base,
											public device_config_cart_slot_interface
{
protected:
	// construction/destruction
	legacy_cart_slot_device_config_base(const machine_config &mconfig, device_type type, const char *tag, const device_config *owner, UINT32 clock, device_get_config_func get_config);
public:
	INT64 get_config_int(UINT32 state) const { return get_legacy_config_int(state); }
	genf *get_config_fct(UINT32 state) const { return get_legacy_config_fct(state); }
	void *get_config_ptr(UINT32 state) const { return get_legacy_config_ptr(state); }
};



// ======================> legacy_cart_slot_device

// legacy_cart_slot_device is a legacy_device_base with a cart_slot interface
class legacy_cart_slot_device_base :	public legacy_device_base,
									public device_cart_slot_interface
{
protected:
	// construction/destruction
	legacy_cart_slot_device_base(running_machine &machine, const device_config &config);

	// device_cart_slot_interface overrides
};
#endif /* __cart_slot_H__ */
