/*********************************************************************

    harddriv.h

    Interface to the CHD code

*********************************************************************/

#ifndef HARDDRIV_H
#define HARDDRIV_H

#include "harddisk.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> harddisk_interface

struct harddisk_interface
{
	const char *                    m_interface;
	device_image_display_info_func  m_device_displayinfo;
};

// ======================> harddisk_image_device

class harddisk_image_device :   public device_t,
								public harddisk_interface,
								public device_image_interface
{
public:
	// construction/destruction
	harddisk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~harddisk_image_device();

	void set_device_load(device_image_load_delegate _load) { m_device_image_load = _load; }
	void set_device_unload(device_image_func_delegate _unload) { m_device_image_unload = _unload; }

	// image-level overrides
	virtual bool call_load();
	virtual bool call_create(int create_format, option_resolution *create_args);
	virtual void call_unload();
	virtual void call_display_info() { if (m_device_displayinfo) m_device_displayinfo(*this); }
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) { load_software_part_region(*this, swlist, swname, start_entry ); return TRUE; }

	virtual iodevice_t image_type() const { return IO_HARDDISK; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return m_interface; }
	virtual const char *file_extensions() const { return "chd,hd"; }
	virtual const option_guide *create_option_guide() const;

	// specific implementation
	hard_disk_file *get_hard_disk_file() { return m_hard_disk_handle; }
	chd_file *get_chd_file();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();

	int internal_load_hd();

	chd_file        *m_chd;
	chd_file        m_origchd;              /* handle to the original CHD */
	chd_file        m_diffchd;              /* handle to the diff CHD */
	hard_disk_file  *m_hard_disk_handle;

	device_image_load_delegate      m_device_image_load;
	device_image_func_delegate      m_device_image_unload;
};

// device type definition
extern const device_type HARDDISK;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_HARDDISK_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, HARDDISK, 0)
#define MCFG_HARDDISK_CONFIG_ADD(_tag,_config) \
	MCFG_DEVICE_ADD(_tag, HARDDISK, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_HARDDISK_LOAD(_class,_load)                                \
	static_cast<harddisk_image_device *>(device)->set_device_load( DEVICE_IMAGE_LOAD_DELEGATE(_class,_load));

#define MCFG_HARDDISK_UNLOAD(_class,_unload)                            \
	static_cast<harddisk_image_device *>(device)->set_device_unload( DEVICE_IMAGE_UNLOAD_DELEGATE(_class,_unload));

#endif /* HARDDRIV_H */
