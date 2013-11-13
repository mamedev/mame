/**********************************************************
 *   DIABLO drive image to hard disk interface
 *
 *   Copyright: Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 **********************************************************/

#ifndef _IMAGEDEV_DIABLO_H_
#define _IMAGEDEV_DIABLO_H_

#include "harddisk.h"

#define	DIABLO_TAG(_id) "diablo"#_id

/***************************************************************************
	TYPE DEFINITIONS
***************************************************************************/

// ======================> diablo_interface

struct diablo_interface
{
	device_image_load_func      m_device_image_load;
	device_image_unload_func    m_device_image_unload;
	const char *                    m_interface;
	device_image_display_info_func  m_device_displayinfo;
};

// ======================> diablo_image_device

class diablo_image_device :   public device_t,
								public diablo_interface,
								public device_image_interface
{
public:
	// construction/destruction
	diablo_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~diablo_image_device();

	// image-level overrides
	virtual bool call_load();
	virtual bool call_create(int create_format, option_resolution *create_args);
	virtual void call_unload();
	virtual void call_display_info() { if (m_device_displayinfo) m_device_displayinfo(*this); }
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry) { load_software_part_region(this, swlist, swname, start_entry ); return TRUE; }

	virtual iodevice_t image_type() const { return IO_HARDDISK; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return m_interface; }
	virtual const char *file_extensions() const { return "chd,dsk"; }
	virtual const option_guide *create_option_guide() const;

	// specific implementation
	hard_disk_file *get_hard_disk_file() { return m_hard_disk_handle; }
	chd_file *get_chd_file();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();

	int internal_load_dsk();

	chd_file        *m_chd;
	chd_file        m_origchd;              /* handle to the original CHD */
	chd_file        m_diffchd;              /* handle to the diff CHD */
	hard_disk_file  *m_hard_disk_handle;

	image_device_format m_format;
};

// device type definition
extern const device_type DIABLO;

/***************************************************************************
	DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DIABLO_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DIABLO, 0)
#define MCFG_DIABLO_CONFIG_ADD(_tag,_config) \
	MCFG_DEVICE_ADD(_tag, DIABLO, 0) \
	MCFG_DEVICE_CONFIG(_config)
#endif /* _IMAGEDEV_DIABLO_H_ */
