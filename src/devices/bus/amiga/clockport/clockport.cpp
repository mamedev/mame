// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Amiga 1200 clock port

***************************************************************************/

#include "emu.h"
#include "clockport.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_CLOCKPORT, amiga_clockport_device, "amiga_clockport", "Amiga 1200 clock port")

template class device_finder<device_amiga_clockport_interface, false>;
template class device_finder<device_amiga_clockport_interface, true>;


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

amiga_clockport_device::amiga_clockport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_CLOCKPORT, tag, owner, clock),
	device_single_card_slot_interface<device_amiga_clockport_interface>(mconfig, *this),
	m_card(nullptr),
	m_int6_handler(*this)
{
}

amiga_clockport_device::~amiga_clockport_device()
{
}

void amiga_clockport_device::device_start()
{
	m_card = get_card_device();
}

uint8_t amiga_clockport_device::spare_r(offs_t offset)
{
	return m_card ? m_card->spare_r(offset & 0x0f) : 0xff;
}

void amiga_clockport_device::spare_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->spare_w(offset & 0x0f, data);
}

uint8_t amiga_clockport_device::rtc_r(offs_t offset)
{
	return m_card ? m_card->rtc_r(offset & 0x0f) : 0xff;
}

void amiga_clockport_device::rtc_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->rtc_w(offset & 0x0f, data);
}


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

device_amiga_clockport_interface::device_amiga_clockport_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "amigaclockport"),
	m_host(dynamic_cast<amiga_clockport_device *>(device.owner()))
{
}

device_amiga_clockport_interface::~device_amiga_clockport_interface()
{
}

uint8_t device_amiga_clockport_interface::spare_r(offs_t offset)
{
	return 0xff;
}

void device_amiga_clockport_interface::spare_w(offs_t offset, uint8_t data)
{
}

uint8_t device_amiga_clockport_interface::rtc_r(offs_t offset)
{
	return 0xff;
}

void device_amiga_clockport_interface::rtc_w(offs_t offset, uint8_t data)
{
}
