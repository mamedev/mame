/**********************************************************************

    COMX-35 Serial/Parallel Printer Card emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "comx_prn.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMX_PRN = &device_creator<comx_prn_device>;


//-------------------------------------------------
//  ROM( comx_prn )
//-------------------------------------------------

ROM_START( comx_prn )
	ROM_REGION( 0x2000, "c000", 0 )
	ROM_LOAD( "printer.bin",			0x0000, 0x0800, CRC(3bbc2b2e) SHA1(08bf7ea4174713ab24969c553affd5c1401876b8) )

	ROM_REGION( 0x2000, "printer_fm", 0 )
	ROM_LOAD( "f&m.printer.1.2.bin",	0x0000, 0x1000, CRC(2feb997d) SHA1(ee9cb91042696c88ff5f2f44d2f702dc93369ba0) )

	ROM_REGION( 0x2000, "rs232", 0 )
	ROM_LOAD( "rs232.bin",				0x0000, 0x0800, CRC(926ff2d1) SHA1(be02bd388bba0211ea72d4868264a63308e4318d) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *comx_prn_device::device_rom_region() const
{
	return ROM_NAME( comx_prn );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_prn_device - constructor
//-------------------------------------------------

comx_prn_device::comx_prn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, COMX_PRN, "COMX-35 F&M Printer Card", tag, owner, clock),
	device_comx_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void comx_prn_device::device_start()
{
	m_rom = memregion("c000")->base();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void comx_prn_device::device_reset()
{
}


//-------------------------------------------------
//  comx_mrd_r - memory read
//-------------------------------------------------

UINT8 comx_prn_device::comx_mrd_r(offs_t offset, int *extrom)
{
	UINT8 data = 0;

	if (offset >= 0xc000 && offset < 0xd000)
	{
		data = m_rom[offset & 0xfff];
	}

	return data;
}


//-------------------------------------------------
//  comx_io_r - I/O read
//-------------------------------------------------

UINT8 comx_prn_device::comx_io_r(offs_t offset)
{
	/*
        Parallel:

        INP 2 for the printer status, where:
        b0=1: Acknowledge Fault
        b1=0: Device Busy
        b2=0: Paper Empty
        b3=1: Device Not Selected

        Serial:

        INP 2 for the printer status and to start a new range of bits for the next byte.
    */

	return 0;
}


//-------------------------------------------------
//  comx_io_w - I/O write
//-------------------------------------------------

void comx_prn_device::comx_io_w(offs_t offset, UINT8 data)
{
	/*
        Parallel:

        OUT 2 is used to send a byte to the printer

        Serial:

        OUT 2 is used to send a bit to the printer
    */
}
