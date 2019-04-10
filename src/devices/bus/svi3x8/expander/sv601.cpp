// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-601 Super Expander for SVI-318/328

***************************************************************************/

#include "emu.h"
#include "sv601.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SV601, sv601_device, "sv601", "SV-601 Super Expander")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sv601_device::device_add_mconfig(machine_config &config)
{
	SVI_SLOT_BUS(config, m_slotbus, 0);
	m_slotbus->int_handler().set(FUNC(sv601_device::int_w));
	m_slotbus->romdis_handler().set(FUNC(sv601_device::romdis_w));
	m_slotbus->ramdis_handler().set(FUNC(sv601_device::ramdis_w));
	SVI_SLOT(config, "0", svi_slot_cards, nullptr);
	SVI_SLOT(config, "1", svi_slot_cards, nullptr);
	SVI_SLOT(config, "2", svi_slot_cards, nullptr);
	SVI_SLOT(config, "3", svi_slot_cards, nullptr);
	SVI_SLOT(config, "4", svi_slot_cards, nullptr);
	SVI_SLOT(config, "5", svi_slot_cards, nullptr);
	SVI_SLOT(config, "6", svi_slot_cards, nullptr);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv601_device - constructor
//-------------------------------------------------

sv601_device::sv601_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SV601, tag, owner, clock),
	device_svi_expander_interface(mconfig, *this),
	m_slotbus(*this, "slotbus")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sv601_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER( sv601_device::int_w ) { m_expander->int_w(state); }
WRITE_LINE_MEMBER( sv601_device::romdis_w ) { m_expander->romdis_w(state); }
WRITE_LINE_MEMBER( sv601_device::ramdis_w ) { m_expander->ramdis_w(state); }

uint8_t sv601_device::mreq_r(offs_t offset) { return m_slotbus->mreq_r(offset); }
void sv601_device::mreq_w(offs_t offset, uint8_t data) { m_slotbus->mreq_w(offset, data); }
uint8_t sv601_device::iorq_r(offs_t offset) { return m_slotbus->iorq_r(offset); }
void sv601_device::iorq_w(offs_t offset, uint8_t data) { m_slotbus->iorq_w(offset, data); }

void sv601_device::bk21_w(int state) { m_slotbus->bk21_w(state); }
void sv601_device::bk22_w(int state) { m_slotbus->bk22_w(state); }
void sv601_device::bk31_w(int state) { m_slotbus->bk31_w(state); }
void sv601_device::bk32_w(int state) { m_slotbus->bk32_w(state); }
