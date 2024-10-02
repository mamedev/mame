// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************************************************

    wafadrive.h

    Emulation of an individual drive within the Wafadrive unit
    (preliminary, no actual emulation yet)

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_WAFADRIVE_H
#define MAME_DEVICES_IMAGEDEV_WAFADRIVE_H

#include "magtape.h"

#pragma once

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> microdrive_image_device

class wafadrive_image_device : public microtape_image_device
{
public:
	// construction/destruction
	wafadrive_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~wafadrive_image_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_creatable() const noexcept override { return false; } // should be (although would need a way to specify size)
	virtual const char *image_interface() const noexcept override { return "wafadrive_cart"; }
	virtual const char *file_extensions() const noexcept override { return "wdr"; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(WAFADRIVE_IMAGE, wafadrive_image_device)

#endif // MAME_DEVICES_IMAGEDEV_WAFADRIVE_H
