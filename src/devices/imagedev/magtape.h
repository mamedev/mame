// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    magtape.h

    Base classes for magnetic tape image devices.

*********************************************************************/

#ifndef MAME_IMAGEDEV_MAGTAPE_H
#define MAME_IMAGEDEV_MAGTAPE_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> magtape_image_device

class magtape_image_device : public device_t, public device_image_interface
{
public:
	// image-level overrides
	virtual bool is_readable() const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual bool support_command_line_image_creation() const noexcept override { return true; }
	virtual const char *image_type_name() const noexcept override { return "magtape"; }
	virtual const char *image_brief_type_name() const noexcept override { return "mtap"; }

protected:
	// construction/destruction
	magtape_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override;
};

// ======================> microtape_image_device

class microtape_image_device : public magtape_image_device
{
protected:
	// construction/destruction
	microtape_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// image-level overrides
	virtual const char *image_type_name() const noexcept override { return "microtape"; }
	virtual const char *image_brief_type_name() const noexcept override { return "utap"; }
};

#endif // MAME_IMAGEDEV_MAGTAPE_H
