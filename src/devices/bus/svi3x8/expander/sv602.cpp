// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-602 Single Slot Expander for SVI-318/328

***************************************************************************/

#include "sv602.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SV602 = &device_creator<sv602_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( sv602 )
	MCFG_SVI_SLOT_BUS_ADD
	MCFG_SVI_SLOT_INT_HANDLER(WRITELINE(sv602_device, int_w))
	MCFG_SVI_SLOT_ROMDIS_HANDLER(WRITELINE(sv602_device, romdis_w))
	MCFG_SVI_SLOT_RAMDIS_HANDLER(WRITELINE(sv602_device, ramdis_w))
	MCFG_SVI_SLOT_ADD("0", sv602_slot_cards, NULL)
MACHINE_CONFIG_END

machine_config_constructor sv602_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sv602 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv602_device - constructor
//-------------------------------------------------

sv602_device::sv602_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SV602, "SV-602 Single Slot Expander", tag, owner, clock, "sv602", __FILE__),
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
