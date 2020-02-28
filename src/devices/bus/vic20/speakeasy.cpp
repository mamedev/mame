// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Personal Peripheral Products Speakeasy cartridge emulation
    (aka Protecto Enterprizes VIC-20 Voice Synthesizer)

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

DEFINE_DEVICE_TYPE(VIC20_SPEAKEASY, vic20_speakeasy_device, "vic20_speakeasy", "PPP Speakeasy VIC-20")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vic20_speakeasy_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	VOTRAX_SC01(config, m_votrax, 720000).add_route(ALL_OUTPUTS, "mono", 0.85);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic20_speakeasy_device - constructor
//-------------------------------------------------

vic20_speakeasy_device::vic20_speakeasy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIC20_SPEAKEASY, tag, owner, clock),
	device_vic20_expansion_card_interface(mconfig, *this),
	m_votrax(*this, SC01A_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic20_speakeasy_device::device_start()
{
}


//-------------------------------------------------
//  vic20_cd_r - cartridge data read
//-------------------------------------------------

uint8_t vic20_speakeasy_device::vic20_cd_r(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (!io2)
	{
		return m_votrax->request() << 7;
	}

	return data;
}


//-------------------------------------------------
//  vic20_cd_w - cartridge data write
//-------------------------------------------------

void vic20_speakeasy_device::vic20_cd_w(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3)
{
	if (!io2)
	{
		m_votrax->write(data & 0x3f);
		m_votrax->inflection_w(data >> 6);
	}
}
