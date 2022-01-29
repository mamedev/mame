// license:BSD-3-Clause
// copyright-holders:Bavarese
/**********************************************************************

    VS Systems LBA Enhancer (ISA; 1995).

***********************************************************************

    Generic BIOS extension card. Adds 28 bit LBA mode to any pre-1994
       board with an empty ISA slot.

    Supports 1 to 4 large hard disks. Bios boot remains possible.

    Check date code of Ami BIOS (at bottom of boot screen):
    40-0100-001139-00101111-111192-486ABC-F  (111192 will not support LBA)
    50-0100-001292-00101111-072594-ABCDEF-F  (072594 will support LBA)

    See: ftp://ami.com/archive/utility/LBA.TXT

    Usage:
      select type 47 for each LBA drive and set C,H,S (params) to '8',
      then repartition and reformat (Fdisk and Format). Data is lost.

    Other notes: ROM bank should be excluded in EMM386.
      C8000 - CBFFF is sometimes occupied (by graphics).
      Changes to the ROM location require a hard reset!

    Requirements:
      Compatible OS (DOS >= 6.22) and LBA capable drive(s).
***********************************************************************

PCB Layout
----------

|-------------------------------------------|
|                                           |
|                                           |
|                                           |
|            EPROM     CD74LS245            |
|                                  J3,J2,J1 |
|                      PAL16V8              |
|                                           |
|                                           |
|---|                                   |---|
    |-----------------------------------|

Notes:

J3, J2, J1 set BIOS (start) address to C800, CC00, D000, D400, D800 or DC00

- one CD74LS245
- one PALCE16V8H-25
- one 32 K EPROM : 27C256 (STM) (mirrored after 16 K)

*/

#include "emu.h"
#include "lbaenhancer.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_LBA_ENHANCER, lba_enhancer_device, "lba_enhancer", "VS Systems LBA Enhancer BIOS 1995")

//-------------------------------------------------
//  ROM( lba_enhancer )
//-------------------------------------------------

ROM_START( lbabios )
	ROM_REGION( 0xc8000, "lbabios", 0 )
	ROM_LOAD( "lbaenhancer.bin", 0x0000, 0x04000, CRC(39d4566d) SHA1(d275193a870250f212dc29768d4e68fb43770e95) )
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *lba_enhancer_device::device_rom_region() const
{
	return ROM_NAME( lbabios );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void lba_enhancer_device::device_add_mconfig(machine_config &config)
{
//  lba_enhancer(config, "lba_enhancer", 0);
}


//-------------------------------------------------
//  jumpers
//-------------------------------------------------
static INPUT_PORTS_START( lba_enhancer_dsw )

	PORT_START("ROM_ADDRESS")
	PORT_CONFNAME( 0x07, 0x00, "16 K ROM bank")
	PORT_CONFSETTING(    0, "C8000 - CBFFF" )
	PORT_CONFSETTING(    1, "CC000 - CFFFF" )
	PORT_CONFSETTING(    2, "D0000 - D3FFF" )
	PORT_CONFSETTING(    3, "D4000 - D7FFF" )
	PORT_CONFSETTING(    4, "D8000 - DBFFF" )
	PORT_CONFSETTING(    5, "DC000 - DFFFF" )
INPUT_PORTS_END

//-------------------------------------------------
//  isa8_areplay_device - constructor
//-------------------------------------------------
ioport_constructor lba_enhancer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( lba_enhancer_dsw );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

// Byte 3 in extra BIOS specifies 8 blocks of 512 byte (4K)
void lba_enhancer_device::device_start()
{
	set_isa_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void lba_enhancer_device::device_reset()
{
	if(m_current_rom_start == 0)
	{
		m_current_rom_start = 0xc8000 + (ioport("ROM_ADDRESS")->read()* 0x4000);
		uint32_t current_rom_end   = m_current_rom_start + 0x04000 - 1;

		m_isa->install_rom(this, m_current_rom_start, current_rom_end, "lbabios");

		logerror("LBA enhancer (for 28 bit LBA) located at BIOS address %x - %x\n", m_current_rom_start, current_rom_end);
	}
}

//-------------------------------------------------
//  lba_enhancer_device - constructor
//-------------------------------------------------

lba_enhancer_device::lba_enhancer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_LBA_ENHANCER, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
{
	m_current_rom_start = 0;
}
