// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/***************************************************************************

    QX-10 Video Slot

***************************************************************************/

#include "emu.h"
#include "video.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_QX_VIDEO_SLOT, bus::epson_qx::video::video_slot_device, "epson_qx_video_slot", "Epson QX-10 Video Slot")


namespace bus::epson_qx::video {

//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  video_slot_device - constructor
//-------------------------------------------------

video_slot_device::video_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, EPSON_QX_VIDEO_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_qx_video_interface>(mconfig, *this)
	, m_iospace(*this, finder_base::DUMMY_TAG, -1)
	, m_drq_cb(*this)
	, m_card(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void video_slot_device::device_start()
{
	m_card = get_card_device();
	if (m_card)
		m_card->install_io(*m_iospace);
}

uint8_t video_slot_device::dack_r()
{
	if (m_card)
		return m_card->dack_r();
	return 0xff;
}

void video_slot_device::dack_w(uint8_t data)
{
	if (m_card)
		m_card->dack_w(data);
}

uint32_t video_slot_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_card)
		return m_card->screen_update(screen, bitmap, cliprect);
	bitmap.fill(rgb_t::black(), cliprect);
	return 0;
}


//**************************************************************************
//  VIDEO CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_qx_video_interface - constructor
//-------------------------------------------------

device_qx_video_interface::device_qx_video_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "epson_qx_video")
{
	m_slot = dynamic_cast<video_slot_device *>(device.owner());
}

}
