// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    picture.h

    Image device that loads still pictures (currently PNG)

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_PICTURE_H
#define MAME_DEVICES_IMAGEDEV_PICTURE_H

#pragma once

#include "bitmap.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> picture_image_device

class picture_image_device : public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	picture_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~picture_image_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "png,jpg,jpeg,jfif,bmp,dib"; }
	virtual const char *image_type_name() const noexcept override { return "picture"; }
	virtual const char *image_brief_type_name() const noexcept override { return "pic"; }

	const bitmap_argb32 &get_bitmap() { return m_picture; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	bitmap_argb32 m_picture;
};


// device type definition
DECLARE_DEVICE_TYPE(IMAGE_PICTURE, picture_image_device)

#endif // MAME_DEVICES_IMAGEDEV_PICTURE_H
