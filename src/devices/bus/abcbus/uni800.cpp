// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*
UNI-800

PCB Layout
----------

8120 821025 REV.3

|-------------------------------------------|
|                                           |
|                   4164        PROM0   CN3 |
|                   4164                    |
|                   4164                    |
|                   4164                    |
|                   4164                    |
|                   4164                    |
|CN1                4164                CN2 |
|                   4164                    |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    4164    - Hitachi HM4864P-2 64Kx1 RAM
    PROM0   - Philips 82S129 256x4 Bipolar PROM ".800 1.2"
    CN1     - 2x6 pin PCB header
    CN2     - 2x10 pin PCB header
    CN3     - 2x10 pin PCB header

*/

#include "uni800.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ABC_UNI800 = &device_creator<abc_uni800_device>;


//-------------------------------------------------
//  ROM( abc_uni800 )
//-------------------------------------------------

ROM_START( abc_uni800 )
	ROM_REGION( 0x100, "uni800", 0 )
	ROM_LOAD( ".800 1.2.bin", 0x0000, 0x0100, CRC(df4897f8) SHA1(0c641f4cf321f0003da3fbd435edb138a9b949b4) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *abc_uni800_device::device_rom_region() const
{
	return ROM_NAME( abc_uni800 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_uni800_device - constructor
//-------------------------------------------------

abc_uni800_device::abc_uni800_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ABC_UNI800, "UNI-800", tag, owner, clock, "uni800", __FILE__),
		device_abcbus_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc_uni800_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc_uni800_device::device_reset()
{
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void abc_uni800_device::abcbus_cs(UINT8 data)
{
}
