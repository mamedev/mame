/*********************************************************************

    floppy.h

*********************************************************************/

#ifndef FLOPPY_H
#define FLOPPY_H

#include "formats/flopimg.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/
// ======================> floppy_interface

struct floppy_interface
{
	const struct floppy_format_def *m_formats;
	const char *					m_interface;
	device_image_display_info_func	m_device_displayinfo;
	device_image_load_func			m_load_func;
	device_image_unload_func		m_unload_func;
};

// ======================> cdrom_image_device

class floppy_image_device :	public device_t,
							public floppy_interface,
							public device_image_interface
{
public:
	// construction/destruction
	floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~floppy_image_device();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual void call_display_info() { if (m_device_displayinfo) m_device_displayinfo(*this); }
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry) { return load_software(swlist, swname, start_entry); }

	virtual iodevice_t image_type() const { return IO_FLOPPY; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return m_interface; }
	virtual const char *file_extensions() const { return m_extension_list; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	UINT8* get_buffer(UINT16 track, UINT8 side) { return m_image->get_buffer(track,side); }
protected:
	// device-level overrides
    virtual void device_config_complete();
	virtual void device_start();

	image_device_format m_format;
	floppy_image 		*m_image;
	char				m_extension_list[256];
};

// device type definition
extern const device_type FLOPPY;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/
#define FLOPPY_0 "floppy0"
#define FLOPPY_1 "floppy1"
#define FLOPPY_2 "floppy2"
#define FLOPPY_3 "floppy3"

#define MCFG_FLOPPY_DRIVE_ADD(_config) \
	MCFG_DEVICE_ADD(FLOPPY_0, FLOPPY, 0) \
	MCFG_DEVICE_CONFIG(_config)	\

#define MCFG_FLOPPY_2_DRIVES_ADD(_config)	\
	MCFG_DEVICE_ADD(FLOPPY_0, FLOPPY, 0)		\
	MCFG_DEVICE_CONFIG(_config)	\
	MCFG_DEVICE_ADD(FLOPPY_1, FLOPPY, 0)		\
	MCFG_DEVICE_CONFIG(_config)

#endif /* FLOPPY_H */
