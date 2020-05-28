// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************************************************

    wafadrive.h

    Emulation of an individual drive within the Wafadrive unit
    (preliminary, no actual emulation yet)

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_WAFADRIVE_H
#define MAME_DEVICES_IMAGEDEV_WAFADRIVE_H

#include "softlist_dev.h"

#pragma once

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> microdrive_image_device

class wafadrive_image_device : public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	wafadrive_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~wafadrive_image_device();

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	virtual iodevice_t image_type() const noexcept override { return IO_MAGTAPE; } // what are these classed as? they're infinite loop tapes, in a cartridge shell that operate like discs

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return false; } // should be (although would need a way to specify size)
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return "wafadrive_cart"; }
	virtual const char *file_extensions() const noexcept override { return "wdr"; }

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override { return image_software_list_loader::instance(); }
};


// device type definition
DECLARE_DEVICE_TYPE(WAFADRIVE_IMAGE, wafadrive_image_device)

#endif // MAME_DEVICES_IMAGEDEV_WAFADRIVE_H
