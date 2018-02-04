// license:BSD-3-Clause
// copyright-holders:Peter Bortas
/*
ABC80 CAD Mouse

Official designation unknown.

A proto-mouse in the form or a flat box with a handle sticking
out. The handle can be manipulated in one direction by turning it
around it's internal pivot point left and right. The other dimension
in handled by pulling and pushing the handle. One button is available
on the top of the handle.

The mouse is connected via the ABC bus, but also passes through the
ABC80<->Monitor power/AV cable for power and possibly other reasons.

The mouse was sold with the PCB CAD program "CAD-ABC"

PCB Layout
----------

  
|-|   CN1    |------------------------------|
|                 CN2   CN3  CN4            |
|              Z80                          |
|                                           |
|                                           |
|                                           |
|                               PROM0       |
|                               PROM1       |
|                                4801       |
|              CR1                          |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    4801    - Mostek MK4801AN-2  1KiB SRAM 150ns
    PROM0   - Intel 2764-25  8KiB EPROM "D"
    PROM1   - Intel 2764-25  8KiB EPROM "E"
    Z80     - Z80 CPU "Z 80/1C"
    CN1     - ABCBUS connector
    CN2     - ABC80 power/AV connector passthrough?
    CN3     - ABC80 power/AV connector passthrough?
    CN4     - ABC80 power/AV connector passthrough?
    CR1     - Crystal "8.000 OSI"
*/

#include "emu.h"
#include "cadmouse.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC_CADMOUSE, abc_cadmouse_device, "cadmouse", "ABC 80 CAD mouse")


//-------------------------------------------------
//  ROM( abc_cadmouse )
//-------------------------------------------------

ROM_START( abc_cadmouse )
	ROM_REGION( 0x4000, "cadmouse", 0 )
	// FIXME: The mapping of the ROMs or if the map locally or on
	// the bus in unknown. 0x0 and 0x2k are just placeholders.
	ROM_LOAD( "D.bin", 0x0000, 0x2000, CRC(c19d655d) SHA1(332ad862b77cff3ec55f0f78ac31b2b8cf93b7b3) )
	ROM_LOAD( "E.bin", 0x2000, 0x2000, CRC(e71c9141) SHA1(07a6fae4e3fff3d7a4f67ad0791e4e297c1763aa) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *abc_cadmouse_device::device_rom_region() const
{
	return ROM_NAME( abc_cadmouse );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_cadmouse_device - constructor
//-------------------------------------------------

abc_cadmouse_device::abc_cadmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ABC_CADMOUSE, tag, owner, clock),
		device_abcbus_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc_cadmouse_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc_cadmouse_device::device_reset()
{
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void abc_cadmouse_device::abcbus_cs(uint8_t data)
{
}
