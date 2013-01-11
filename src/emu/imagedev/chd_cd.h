/*********************************************************************

    chd_cd.h

    Interface to the CHD CDROM code

*********************************************************************/

#ifndef CHD_CD_H
#define CHD_CD_H

#include "cdrom.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/
// ======================> cdrom_interface

struct cdrom_interface
{
	const char *                    m_interface;
	device_image_display_info_func  m_device_displayinfo;
};

// ======================> cdrom_image_device

class cdrom_image_device :  public device_t,
								public cdrom_interface,
								public device_image_interface
{
public:
	// construction/destruction
	cdrom_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~cdrom_image_device();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual void call_display_info() { if (m_device_displayinfo) m_device_displayinfo(*this); }
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry) { load_software_part_region(this, swlist, swname, start_entry ); return TRUE; }

	virtual iodevice_t image_type() const { return IO_CDROM; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return m_interface; }
	virtual const char *file_extensions() const { return m_extension_list; }
	virtual const option_guide *create_option_guide() const;

	// specific implementation
	cdrom_file *get_cdrom_file() { return m_cdrom_handle; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();

	chd_file    m_self_chd;
	cdrom_file  *m_cdrom_handle;
	image_device_format m_format;
	const char  *m_extension_list;
};

// device type definition
extern const device_type CDROM;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/


#define MCFG_CDROM_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, CDROM, 0) \
	MCFG_DEVICE_CONFIG(_config)
#endif /* CHD_CD_H */
