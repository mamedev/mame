// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/***************************************************************************

    QX-10 Video Slot

***************************************************************************/

#ifndef MAME_BUS_EPSON_QX_VIDEO_VIDEO_H
#define MAME_BUS_EPSON_QX_VIDEO_VIDEO_H

#pragma once

#include "screen.h"

namespace bus::epson_qx::video {

//**************************************************************************
//  FORWARD DECLARATIONS
//**************************************************************************

class device_qx_video_interface;

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class video_slot_device : public device_t, public device_single_card_slot_interface<device_qx_video_interface>
{
public:
	// construction/destruction
	template <typename T>
	video_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: video_slot_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	video_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_iospace(T &&tag, int spacenum) { m_iospace.set_tag(std::forward<T>(tag), spacenum); }

	// callbacks
	auto drq_callback() { return m_drq_cb.bind(); }

	// called from card
	void drq_w(int state) { m_drq_cb(state); }

	// called from host
	uint8_t dack_r();
	void dack_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	required_address_space m_iospace;
	devcb_write_line m_drq_cb;

	device_qx_video_interface *m_card;
};


class device_qx_video_interface : public device_interface
{
public:
	virtual void install_io(address_space &space) ATTR_COLD {}

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) = 0;
	virtual uint8_t dack_r() { return 0xff; }
	virtual void dack_w(uint8_t data) {}

protected:
	device_qx_video_interface(const machine_config &mconfig, device_t &device);

	video_slot_device *m_slot;
};

} // namespace bus::epson_qx::video


DECLARE_DEVICE_TYPE_NS(EPSON_QX_VIDEO_SLOT, bus::epson_qx::video, video_slot_device)

#endif // MAME_BUS_EPSON_QX_VIDEO_VIDEO_H
