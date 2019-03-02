// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-602 Single Slot Expander for SVI-318/328

***************************************************************************/

#include "emu.h"
#include "sv602.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SV602, sv602_device, "sv602", "SV-602 Single Slot Expander")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sv602_device::device_add_mconfig(machine_config &config)
{
	SVI_SLOT_BUS(config, m_slotbus, 0);
	m_slotbus->int_handler().set(FUNC(sv602_device::int_w));
	m_slotbus->romdis_handler().set(FUNC(sv602_device::romdis_w));
	m_slotbus->ramdis_handler().set(FUNC(sv602_device::ramdis_w));
	SVI_SLOT(config, "0", sv602_slot_cards, nullptr);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv602_device - constructor
//-------------------------------------------------

sv602_device::sv602_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SV602, tag, owner, clock),
	device_svi_expander_interface(mconfig, *this),
	m_slotbus(*this, "slotbus")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sv602_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER( sv602_device::int_w ) { m_expander->int_w(state); }
WRITE_LINE_MEMBER( sv602_device::romdis_w ) { m_expander->romdis_w(state); }
WRITE_LINE_MEMBER( sv602_device::ramdis_w ) { m_expander->ramdis_w(state); }

READ8_MEMBER( sv602_device::mreq_r ) { return m_slotbus->mreq_r(space, offset); }
WRITE8_MEMBER( sv602_device::mreq_w ) { m_slotbus->mreq_w(space, offset, data); }
READ8_MEMBER( sv602_device::iorq_r ) { return m_slotbus->iorq_r(space, offset); }
WRITE8_MEMBER( sv602_device::iorq_w ) { m_slotbus->iorq_w(space, offset, data); }

void sv602_device::bk21_w(int state) { m_slotbus->bk21_w(state); }
void sv602_device::bk22_w(int state) { m_slotbus->bk22_w(state); }
void sv602_device::bk31_w(int state) { m_slotbus->bk31_w(state); }
void sv602_device::bk32_w(int state) { m_slotbus->bk32_w(state); }
