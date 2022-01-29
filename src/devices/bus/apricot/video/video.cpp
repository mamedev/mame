// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    ACT Apricot XEN Video Expansion Slot

***************************************************************************/

#include "emu.h"
#include "video.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(APRICOT_VIDEO_SLOT, apricot_video_slot_device, "apricot_video", "Apricot XEN Video Slot")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  apricot_video_slot_device - constructor
//-------------------------------------------------

apricot_video_slot_device::apricot_video_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, APRICOT_VIDEO_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_apricot_video_interface>(mconfig, *this),
	m_apvid_handler(*this),
	m_card(nullptr)
{
}

//-------------------------------------------------
//  apricot_video_slot_device - destructor
//-------------------------------------------------

apricot_video_slot_device::~apricot_video_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apricot_video_slot_device::device_start()
{
	// get inserted module
	m_card = get_card_device();

	// resolve callbacks
	m_apvid_handler.resolve_safe();
}

//-------------------------------------------------
//  host to module interface
//-------------------------------------------------

bool apricot_video_slot_device::mem_r(offs_t offset, uint16_t &data, uint16_t mem_mask)
{
	if (m_card)
		return m_card->mem_r(offset, data, mem_mask);

	return false;
}

bool apricot_video_slot_device::mem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_card)
		return m_card->mem_w(offset, data, mem_mask);

	return false;
}

bool apricot_video_slot_device::io_r(offs_t offset, uint16_t &data, uint16_t mem_mask)
{
	if (m_card)
		return m_card->io_r(offset, data, mem_mask);

	return false;
}

bool apricot_video_slot_device::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_card)
		return m_card->io_w(offset, data, mem_mask);

	return false;
}


//**************************************************************************
//  MODULE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_apricot_video_interface - constructor
//-------------------------------------------------

device_apricot_video_interface::device_apricot_video_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "apricotvid")
{
	m_slot = dynamic_cast<apricot_video_slot_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_apricot_video_interface - destructor
//-------------------------------------------------

device_apricot_video_interface::~device_apricot_video_interface()
{
}
