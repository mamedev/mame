// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Personal Peripheral Products Speakeasy 64 cartridge emulation

**********************************************************************/

#include "emu.h"
#include "speakeasy.h"
#include "speaker.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define SC01A_TAG       "sc01a"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_SPEAKEASY, c64_speakeasy_cartridge_device, "c64_speakeasy", "PPP Speakeasy 64")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_speakeasy_cartridge_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	VOTRAX_SC01(config, m_votrax, 720000).add_route(ALL_OUTPUTS, "mono", 0.85);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_speakeasy_cartridge_device - constructor
//-------------------------------------------------

c64_speakeasy_cartridge_device::c64_speakeasy_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_SPEAKEASY, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_votrax(*this, SC01A_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_speakeasy_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_speakeasy_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		return m_votrax->request() << 7;
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_speakeasy_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		m_votrax->write(data & 0x3f);
		m_votrax->inflection_w(data >> 6);
	}
}
