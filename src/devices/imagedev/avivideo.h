// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    avivideo.h

    Image device for AVI video.

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_AVIVIDEO_H
#define MAME_DEVICES_IMAGEDEV_AVIVIDEO_H

#pragma once

#include "aviio.h"
#include "bitmap.h"

#include <memory>
#include <string>
#include <system_error>
#include <utility>


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> avivideo_image_device

class avivideo_image_device : public device_t, public device_image_interface
{
public:
	// construction/destruction
	avivideo_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~avivideo_image_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "avi"; }
	virtual const char *image_type_name() const noexcept override { return "vidfile"; }
	virtual const char *image_brief_type_name() const noexcept override { return "vid"; }

	bitmap_argb32 &get_frame() { return *m_frame; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(frame_timer);

private:
	std::unique_ptr<bitmap_argb32> m_frame;
	avi_file::ptr m_avi;

	emu_timer *m_frame_timer;
	uint32_t m_frame_count;
	uint32_t m_frame_num;
};


// device type definition
DECLARE_DEVICE_TYPE(IMAGE_AVIVIDEO, avivideo_image_device)

#endif // MAME_DEVICES_IMAGEDEV_AVIVIDEO_H
