/****************************************************************************

    printer.h

    Code for handling printer devices

****************************************************************************/

#ifndef __PRINTER_H__
#define __PRINTER_H__

#include "diimage.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> printer_interface

struct printer_interface
{
	devcb_write_line m_online;
};

// ======================> printer_image_device

class printer_image_device :    public device_t,
								public printer_interface,
								public device_image_interface
{
public:
	// construction/destruction
	printer_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~printer_image_device();

	// image-level overrides
	virtual bool call_load();
	virtual bool call_create(int format_type, option_resolution *format_options);
	virtual void call_unload();

	virtual iodevice_t image_type() const { return IO_PRINTER; }

	virtual bool is_readable()  const { return 0; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 1; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return NULL; }
	virtual const char *file_extensions() const { return "prn"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	// specific implementation

	/* checks to see if a printer is ready */
	int is_ready();
	/* outputs data to a printer */
	void output(UINT8 data);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	devcb_resolved_write_line m_online_func;
};


// device type definition
extern const device_type PRINTER;


#define MCFG_PRINTER_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PRINTER, 0)
#endif /* __PRINTER_H__ */
