// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-601 Super Expander for SVI-318/328

***************************************************************************/

#include "sv601.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SV601 = &device_creator<sv601_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( sv601 )
	MCFG_SVI_SLOT_BUS_ADD
	MCFG_SVI_SLOT_INT_HANDLER(WRITELINE(sv601_device, int_w))
	MCFG_SVI_SLOT_ROMDIS_HANDLER(WRITELINE(sv601_device, romdis_w))
	MCFG_SVI_SLOT_RAMDIS_HANDLER(WRITELINE(sv601_device, ramdis_w))
	MCFG_SVI_SLOT_ADD("0", svi_slot_cards, NULL)
	MCFG_SVI_SLOT_ADD("1", svi_slot_cards, NULL)
	MCFG_SVI_SLOT_ADD("2", svi_slot_cards, NULL)
	MCFG_SVI_SLOT_ADD("3", svi_slot_cards, NULL)
	MCFG_SVI_SLOT_ADD("4", svi_slot_cards, NULL)
	MCFG_SVI_SLOT_ADD("5", svi_slot_cards, NULL)
	MCFG_SVI_SLOT_ADD("6", svi_slot_cards, NULL)
MACHINE_CONFIG_END

machine_config_constructor sv601_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sv601 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv601_device - constructor
//-------------------------------------------------

sv601_device::sv601_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SV601, "SV-601 Super Expander", tag, owner, clock, "sv601", __FILE__),
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

READ8_MEMBER( sv601_device::mreq_r ) { return m_slotbus->mreq_r(space, offset); }
WRITE8_MEMBER( sv601_device::mreq_w ) { m_slotbus->mreq_w(space, offset, data); }
READ8_MEMBER( sv601_device::iorq_r ) { return m_slotbus->iorq_r(space, offset); }
WRITE8_MEMBER( sv601_device::iorq_w ) { m_slotbus->iorq_w(space, offset, data); }

void sv601_device::bk21_w(int state) { m_slotbus->bk21_w(state); }
void sv601_device::bk22_w(int state) { m_slotbus->bk22_w(state); }
void sv601_device::bk31_w(int state) { m_slotbus->bk31_w(state); }
void sv601_device::bk32_w(int state) { m_slotbus->bk32_w(state); }
