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

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

	virtual bool is_readable()  const noexcept override { return false; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual bool support_command_line_image_creation() const noexcept override { return true; }
	virtual const char *file_extensions() const noexcept override { return "prn"; }
	virtual const char *image_type_name() const noexcept override { return "printout"; }
	virtual const char *image_brief_type_name() const noexcept override { return "prin"; }

	// specific implementation

	/* checks to see if a printer is ready */
	int is_ready();
	/* outputs data to a printer */
	void output(uint8_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	devcb_write_line m_online_cb;
};


// device type definition
DECLARE_DEVICE_TYPE(PRINTER, printer_image_device)

#endif // MAME_DEVICES_IMAGEDEV_PRINTER_H
