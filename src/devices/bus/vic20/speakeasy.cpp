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

	VOTRAX_SC01A(config, m_votrax, 720000).add_route(ALL_OUTPUTS, "mono", 0.85);
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
	m_slot->io2().install_readwrite_handler(0x000, 0x3ff, read8smo_delegate(*this, FUNC(vic20_speakeasy_device::io2_r)), write8smo_delegate(*this, FUNC(vic20_speakeasy_device::io2_w)));
}


//-------------------------------------------------
//  io2_r - I/O 2 read
//-------------------------------------------------

uint8_t vic20_speakeasy_device::io2_r()
{
	return m_votrax->request() << 7;
}


//-------------------------------------------------
//  io2_w - I/O 2 write
//-------------------------------------------------

void vic20_speakeasy_device::io2_w(uint8_t data)
{
	m_votrax->write(data & 0x3f);
	m_votrax->inflection_w(data >> 6);
}
