// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/****************************************************************************

    printer.h

    Code for handling printer devices

****************************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_PRINTER_H
#define MAME_DEVICES_IMAGEDEV_PRINTER_H

#pragma once


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> printer_image_device

class printer_image_device : public device_t,
	public device_image_interface
{
public:
	// construction/destruction
	printer_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto online_callback() { return m_online_cb.bind(); }

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

	// image device
	virtual iodevice_t image_type() const override { return IO_PRINTER; }
	virtual bool is_readable()  const override { return 0; }
	virtual bool is_writeable() const override { return 1; }
	virtual bool is_creatable() const override { return 1; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 0; }
	virtual const char *file_extensions() const override { return "prn"; }

	// specific implementation

	/* checks to see if a printer is ready */
	int is_ready();
	/* outputs data to a printer */
	void output(uint8_t data);
protected:
	// device-level overrides
	virtual void device_start() override;

	devcb_write_line m_online_cb;
};


// device type definition
DECLARE_DEVICE_TYPE(PRINTER, printer_image_device)

#endif // MAME_DEVICES_IMAGEDEV_PRINTER_H
