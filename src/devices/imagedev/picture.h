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

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual iodevice_t image_type() const noexcept override { return IO_PICTURE; }

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "png"; }

	bitmap_argb32 &get_bitmap() { return *m_picture; }

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	bitmap_argb32 *m_picture;
};


// device type definition
DECLARE_DEVICE_TYPE(IMAGE_PICTURE, picture_image_device)

#endif // MAME_DEVICES_IMAGEDEV_PICTURE_H
