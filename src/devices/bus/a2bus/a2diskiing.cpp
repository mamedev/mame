// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2diskii.c

    Implementation of the Apple II Disk II controller card

*********************************************************************/

#include "emu.h"
#include "imagedev/floppy.h"
#include "formats/ap2_dsk.h"
#include "a2diskiing.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_DISKIING, a2bus_diskiing_device, "a2diskiing", "Apple Disk II NG controller (16-sector)")
DEFINE_DEVICE_TYPE(A2BUS_DISKIING13, a2bus_diskiing13_device, "diskii13", "Apple Disk II NG controller (13-sector)")

#define WOZFDC_TAG         "wozfdc"
#define DISKII_ROM_REGION  "diskii_rom"

static void a2_floppies(device_slot_interface &device)
{
	device.option_add("525", FLOPPY_525_SD);
}

ROM_START( diskiing )
	ROM_REGION(0x100, DISKII_ROM_REGION, 0)
	ROM_LOAD( "341-0027-a.p5",  0x0000, 0x0100, CRC(ce7144f6) SHA1(d4181c9f046aafc3fb326b381baac809d9e38d16) ) /* 341-0027-a: 16-sector disk drive (older version), PROM P5 */
ROM_END

ROM_START( diskiing13 )
	ROM_REGION(0x100, DISKII_ROM_REGION, 0)
	ROM_LOAD( "341-0009.bin", 0x000000, 0x000100, CRC(d34eb2ff) SHA1(afd060e6f35faf3bb0146fa889fc787adf56330a) )
ROM_END

FLOPPY_FORMATS_MEMBER( diskiing_device::floppy_formats )
	FLOPPY_A216S_FORMAT, FLOPPY_RWTS18_FORMAT, FLOPPY_EDD_FORMAT, FLOPPY_WOZ_FORMAT
FLOPPY_FORMATS_END

FLOPPY_FORMATS_MEMBER( a2bus_diskiing13_device::floppy_formats )
	FLOPPY_EDD_FORMAT, FLOPPY_WOZ_FORMAT
FLOPPY_FORMATS_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void diskiing_device::device_add_mconfig(machine_config &config)
{
	DISKII_FDC(config, m_wozfdc, 1021800*2);
	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, a2_floppies, "525", diskiing_device::floppy_formats);
}

void a2bus_diskiing13_device::device_add_mconfig(machine_config &config)
{
	DISKII_FDC(config, m_wozfdc, 1021800*2);
	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, a2_floppies, "525", a2bus_diskiing13_device::floppy_formats);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *diskiing_device::device_rom_region() const
{
	return ROM_NAME( diskiing );
}

const tiny_rom_entry *a2bus_diskiing13_device::device_rom_region() const
{
	return ROM_NAME( diskiing13 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

diskiing_device::diskiing_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_wozfdc(*this, WOZFDC_TAG),
	m_floppy(*this, "%u", 0U),
	m_rom(nullptr)
{
}

a2bus_diskiing_device::a2bus_diskiing_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	diskiing_device(mconfig, A2BUS_DISKIING, tag, owner, clock)
{
}

a2bus_diskiing13_device::a2bus_diskiing13_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	diskiing_device(mconfig, A2BUS_DISKIING13, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void diskiing_device::device_start()
{
	m_rom = device().machine().root_device().memregion(this->subtag(DISKII_ROM_REGION).c_str())->base();
}

void diskiing_device::device_reset()
{
	m_wozfdc->set_floppies(m_floppy[0], m_floppy[1]);
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t diskiing_device::read_c0nx(uint8_t offset)
{
	return m_wozfdc->read(offset);
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void diskiing_device::write_c0nx(uint8_t offset, uint8_t data)
{
	m_wozfdc->write(offset, data);
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t diskiing_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset];
}
