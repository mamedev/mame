/***************************************************************************

    Cartrige loading

***************************************************************************/

#ifndef __CARTSLOT_H__
#define __CARTSLOT_H__



/***************************************************************************
    MACROS
***************************************************************************/
#define ROM_CART_LOAD(tag,offset,length,flags)  \
	{ NULL, tag, offset, length, ROMENTRYTYPE_CARTRIDGE | (flags) },

#define ROM_MIRROR      0x01000000
#define ROM_NOMIRROR    0x00000000
#define ROM_FULLSIZE    0x02000000
#define ROM_FILL_FF     0x04000000
#define ROM_NOCLEAR     0x08000000

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> cartslot_image_device

class cartslot_image_device :   public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	cartslot_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~cartslot_image_device();

	static void static_set_device_load(device_t &device, device_image_load_delegate callback) { downcast<cartslot_image_device &>(device).m_device_image_load = callback; }
	static void static_set_device_unload(device_t &device, device_image_func_delegate callback) { downcast<cartslot_image_device &>(device).m_device_image_unload = callback; }
	static void static_set_partialhash(device_t &device, device_image_partialhash_func callback) { downcast<cartslot_image_device &>(device).m_device_image_partialhash = callback; }

	static void static_set_extensions(device_t &device, const char *_extensions) { downcast<cartslot_image_device &>(device).m_extensions = _extensions; }
	static void static_set_interface(device_t &device, const char *_interface) { downcast<cartslot_image_device &>(device).m_interface = _interface; }
	static void static_set_must_be_loaded(device_t &device, bool _must_be_loaded) { downcast<cartslot_image_device &>(device).m_must_be_loaded = _must_be_loaded; }
	
	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) {   load_software_part_region( *this, swlist, swname, start_entry ); return TRUE; }
	virtual device_image_partialhash_func get_partial_hash() const { return m_device_image_partialhash; }

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return m_must_be_loaded; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const char *image_interface() const { return m_interface; }
	virtual const char *file_extensions() const { return m_extensions; }
	virtual const option_guide *create_option_guide() const { return NULL; }

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	int load_cartridge(const rom_entry *romrgn, const rom_entry *roment, bool load);
	int process_cartridge(bool load);


	const char *                    m_extensions;
	const char *                    m_interface;
	bool                            m_must_be_loaded;
	device_image_load_delegate      m_device_image_load;
	device_image_func_delegate      m_device_image_unload;
	device_image_partialhash_func   m_device_image_partialhash;
};

// device type definition
extern const device_type CARTSLOT;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_CARTSLOT_ADD(_tag)                                         \
	MCFG_DEVICE_ADD(_tag, CARTSLOT, 0)
#define MCFG_CARTSLOT_MODIFY(_tag)                                      \
	MCFG_DEVICE_MODIFY(_tag)

#define MCFG_CARTSLOT_EXTENSION_LIST(_extensions)                       \
	cartslot_image_device::static_set_extensions(*device, _extensions);

#define MCFG_CARTSLOT_INTERFACE(_interface)                         \
	cartslot_image_device::static_set_interface(*device, _interface);

#define MCFG_CARTSLOT_NOT_MANDATORY                                     \
	cartslot_image_device::static_set_must_be_loaded(*device, FALSE);

#define MCFG_CARTSLOT_MANDATORY                                         \
	cartslot_image_device::static_set_must_be_loaded(*device, TRUE);

#define MCFG_CARTSLOT_LOAD(_class,_method)                                \
    cartslot_image_device::static_set_device_load(*device, device_image_load_delegate(&DEVICE_IMAGE_LOAD_NAME(_class,_method), #_class "::device_image_load_" #_method, downcast<_class *>(owner)));

#define MCFG_CARTSLOT_UNLOAD(_class,_method)                            \
    cartslot_image_device::static_set_device_unload(*device, device_image_func_delegate(&DEVICE_IMAGE_UNLOAD_NAME(_class,_method), #_class "::device_image_unload_" #_method, downcast<_class *>(owner)));

#define MCFG_CARTSLOT_PARTIALHASH(_partialhash)                  \
	cartslot_image_device::static_set_partialhash(*device, _partialhash);

#endif /* __CARTSLOT_H__ */
