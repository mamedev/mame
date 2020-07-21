// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SID Soundchip Interface for SAM Coupe

***************************************************************************/

#include "emu.h"
#include "sid.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAM_SID6581, sam_sid6581_device, "sid6581", "SID Soundchip Interface (6581)")
DEFINE_DEVICE_TYPE(SAM_SID8580, sam_sid8580_device, "sid8580", "SID Soundchip Interface (8580)")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sam_sid6581_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	MOS6581(config, m_sid, 1_MHz_XTAL);
	m_sid->add_route(ALL_OUTPUTS, "mono", 1.00);
}

void sam_sid8580_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	MOS8580(config, m_sid, 1_MHz_XTAL);
	m_sid->add_route(ALL_OUTPUTS, "mono", 1.00);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sam_sid_device - constructor
//-------------------------------------------------

sam_sid_device::sam_sid_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_samcoupe_expansion_interface(mconfig, *this),
	m_sid(*this, "sid")
{
}

//-------------------------------------------------
//  sam_sid6581_device - constructor
//-------------------------------------------------

sam_sid6581_device::sam_sid6581_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sam_sid_device(mconfig, SAM_SID6581, tag, owner, clock)
{
}

//-------------------------------------------------
//  sam_sid8580_device - constructor
//-------------------------------------------------

sam_sid8580_device::sam_sid8580_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sam_sid_device(mconfig, SAM_SID8580, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam_sid_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void sam_sid_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xff) == 0xd4)
		m_sid->write(offset >> 8, data);
}
