// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    PS-64 speech cartridge emulation

**********************************************************************/

/*

    PCB Layout
    ----------

    |===========================|
    |=|                         |
    |=|                      SW1|
    |=|       SC02              |
    |=|                         |
    |=|                      CN1|
    |=|       ROM        LF347  |
    |=|                         |
    |=|                         |
    |===========================|

    SC02  - Votrax SSI-263AP Speech Synthesizer
    ROM   - Hynix Semiconductor HY27C64D-20 8Kx8 EPROM
    LF347 - National Instruments LF347N JFET Operational Amplifier
    SW1   - Module on/off switch
    CN1   - connector to C64 video/audio port

*/

/*

    TODO:

    - Votrax SC02 emulation
    - route sound to SID audio input
    - on/off switch

*/

#include "emu.h"
#include "ps64.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define SSI263_TAG      "ssi263"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_PS64, c64_ps64_cartridge_device, "c64_ps64", "C64 PS-64")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_ps64_cartridge_device::device_add_mconfig(machine_config &config)
{
	//SPEAKER(config, "speaker").front_center();
	//VOTRAX_SC02(config, SSI263_TAG, 2000000).add_route(ALL_OUTPUTS, "mono", 1.00);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_ps64_cartridge_device - constructor
//-------------------------------------------------

c64_ps64_cartridge_device::c64_ps64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_PS64, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_ps64_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_ps64_cartridge_device::device_reset()
{
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_ps64_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = m_roml[offset & 0x1fff];
	}
	else if (!io1)
	{
		//sc02->read(offset & 0x07);
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_ps64_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		// sc02->write(offset & 0x07, data);
	}
}
