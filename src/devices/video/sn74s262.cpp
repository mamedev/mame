// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Texas Instruments SN74S262N Row Output Character Generator emulation

**********************************************************************/

#include "emu.h"
#include "sn74s262.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SN74S262, sn74s262_device, "sn74s262", "SN74S262N Row Output Character Generator")
DEFINE_DEVICE_TYPE(SN74S263, sn74s263_device, "sn74s263", "SN74S263N Row Output Character Generator")


//-------------------------------------------------
//  ROM( sn74s262 )
//-------------------------------------------------

ROM_START( sn74s262 )
	ROM_REGION( 0xa00, "chargen", 0 )
	ROM_LOAD( "sn74s262.h2", 0x000, 0xa00, NO_DUMP )
ROM_END


//-------------------------------------------------
//  ROM( sn74s263 )
//-------------------------------------------------

ROM_START( sn74s263 )
	ROM_REGION( 0xa00, "chargen", 0 )
	ROM_LOAD( "sn74s263.h2", 0x000, 0xa00, BAD_DUMP CRC(9e064e91) SHA1(354783c8f2865f73dc55918c9810c66f3aca751f) ) // created by hand
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *sn74s262_device::device_rom_region() const
{
	return ROM_NAME( sn74s262 );
}

const tiny_rom_entry *sn74s263_device::device_rom_region() const
{
	return ROM_NAME( sn74s263 );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sn74s262_device::device_start()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sn74s262_device - constructor
//-------------------------------------------------

sn74s262_device::sn74s262_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_char_rom(*this, "chargen")
{
}

sn74s262_device::sn74s262_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sn74s262_device(mconfig, SN74S262, tag, owner, clock)
{
}

sn74s263_device::sn74s263_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
    sn74s262_device(mconfig, SN74S263, tag, owner, clock)
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

u8 sn74s262_device::read(u8 character, u8 row)
{
    if ((row & 0xf) > 8)
    {
        return 0;
    }

    return m_char_rom[((character & 0x7f) * 10) + (row & 0xf)];
}
