// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Master Extra User Port

**********************************************************************/

#include "emu.h"
#include "meup.h"
#include "bus/bbc/userport/userport.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_MEUP, bbc_meup_device, "bbc_meup", "Master Extra User Port");


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_meup_device::device_add_mconfig(machine_config &config)
{
	MOS6522(config, m_via, DERIVED_CLOCK(1, 1));
	m_via->readpb_handler().set("userport2", FUNC(bbc_userport_slot_device::pb_r));
	m_via->writepb_handler().set("userport2", FUNC(bbc_userport_slot_device::pb_w));
	m_via->cb1_handler().set("userport2", FUNC(bbc_userport_slot_device::write_cb1));
	m_via->cb2_handler().set("userport2", FUNC(bbc_userport_slot_device::write_cb2));
	m_via->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_modem_slot_device::irq_w));

	bbc_userport_slot_device &userport2(BBC_USERPORT_SLOT(config, "userport2", bbc_userport_devices, nullptr));
	userport2.cb1_handler().set(m_via, FUNC(via6522_device::write_cb1));
	userport2.cb2_handler().set(m_via, FUNC(via6522_device::write_cb2));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_meup_device - constructor
//-------------------------------------------------

bbc_meup_device::bbc_meup_device(const machine_config &mconfig, const char* tag, device_t* owner, uint32_t clock)
	: device_t(mconfig, BBC_MEUP, tag, owner, clock)
	, device_bbc_modem_interface(mconfig, *this)
	, m_via(*this, "via")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_meup_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_meup_device::read(offs_t offset)
{
	return m_via->read(offset);
}

void bbc_meup_device::write(offs_t offset, uint8_t data)
{
	m_via->write(offset, data);
}
