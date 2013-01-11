/***************************************************************************

    Cartrige loading

***************************************************************************/

#ifndef __CARTSLOT_H__
#define __CARTSLOT_H__

#include "image.h"


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

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual void call_display_info() { if (m_device_displayinfo) m_device_displayinfo(*this); }
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry) {   load_software_part_region( this, swlist, swname, start_entry ); return TRUE; }
	virtual device_image_partialhash_func get_partial_hash() const { return m_device_partialhash; }

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return m_must_be_loaded; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const char *image_interface() const { return m_interface; }
	virtual const char *file_extensions() const { return m_extensions; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	void set_extensions(const char *_extensions) { m_extensions = _extensions; }
	void set_interface(const char *_interface) { m_interface = _interface; }
	void set_must_be_loaded(bool _must_be_loaded) { m_must_be_loaded = _must_be_loaded; }
	void set_device_start(device_start_func _start) { m_device_start = _start; }
	void set_device_load(device_image_load_func _load) { m_device_load = _load; }
	void set_device_unload(device_image_unload_func _unload) { m_device_unload = _unload; }
	void set_partialhash(device_image_partialhash_func _partialhash) { m_device_partialhash = _partialhash; }
	void set_displayinfo(device_image_display_info_func _displayinfo) { m_device_displayinfo = _displayinfo; }

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	int load_cartridge(const rom_entry *romrgn, const rom_entry *roment, bool load);
	int process_cartridge(bool load);


	const char *                    m_extensions;
	const char *                    m_interface;
	bool                            m_must_be_loaded;
	device_start_func               m_device_start;
	device_image_load_func          m_device_load;
	device_image_unload_func        m_device_unload;
	device_image_partialhash_func   m_device_partialhash;
	device_image_display_info_func  m_device_displayinfo;
};

// device type definition
extern const device_type CARTSLOT;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_CARTSLOT_ADD(_tag)                                         \
	MCFG_DEVICE_ADD(_tag, CARTSLOT, 0)                                  \

#define MCFG_CARTSLOT_MODIFY(_tag)                                      \
	MCFG_DEVICE_MODIFY(_tag)                                    \

#define MCFG_CARTSLOT_EXTENSION_LIST(_extensions)                       \
	static_cast<cartslot_image_device *>(device)->set_extensions(_extensions);

#define MCFG_CARTSLOT_INTERFACE(_interface)                         \
	static_cast<cartslot_image_device *>(device)->set_interface(_interface);

#define MCFG_CARTSLOT_NOT_MANDATORY                                     \
	static_cast<cartslot_image_device *>(device)->set_must_be_loaded(FALSE);

#define MCFG_CARTSLOT_MANDATORY                                         \
	static_cast<cartslot_image_device *>(device)->set_must_be_loaded(TRUE);

#define MCFG_CARTSLOT_START(_start)                                     \
	static_cast<cartslot_image_device *>(device)->set_device_start(DEVICE_START_NAME(_start));

#define MCFG_CARTSLOT_LOAD(_load)                                       \
	static_cast<cartslot_image_device *>(device)->set_device_load(DEVICE_IMAGE_LOAD_NAME(_load));

#define MCFG_CARTSLOT_UNLOAD(_unload)                                   \
	static_cast<cartslot_image_device *>(device)->set_device_unload(DEVICE_IMAGE_UNLOAD_NAME(_unload));

#define MCFG_CARTSLOT_PARTIALHASH(_partialhash)                         \
	static_cast<cartslot_image_device *>(device)->set_partialhash(_partialhash);

#define MCFG_CARTSLOT_DISPLAY_INFO(_displayinfo)                                        \
	static_cast<cartslot_image_device *>(device)->set_displayinfo(DEVICE_IMAGE_DISPLAY_INFO_NAME(_displayinfo));

#endif /* __CARTSLOT_H__ */
