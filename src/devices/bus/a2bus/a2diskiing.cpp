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

const device_type A2BUS_DISKIING = &device_creator<a2bus_diskiing_device>;

#define WOZFDC_TAG         "wozfdc"
#define DISKII_ROM_REGION  "diskii_rom"

static SLOT_INTERFACE_START( a2_floppies )
	SLOT_INTERFACE( "525", FLOPPY_525_SD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( diskiing )
	MCFG_DEVICE_ADD(WOZFDC_TAG, DISKII_FDC, 1021800*2)
	MCFG_FLOPPY_DRIVE_ADD("0", a2_floppies, "525", a2bus_diskiing_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("1", a2_floppies, "525", a2bus_diskiing_device::floppy_formats)
MACHINE_CONFIG_END

ROM_START( diskiing )
	ROM_REGION(0x100, DISKII_ROM_REGION, 0)
	ROM_LOAD( "341-0027-a.p5",  0x0000, 0x0100, CRC(ce7144f6) SHA1(d4181c9f046aafc3fb326b381baac809d9e38d16) ) /* 341-0027-a: 16-sector disk drive (older version), PROM P5 */
ROM_END

FLOPPY_FORMATS_MEMBER( a2bus_diskiing_device::floppy_formats )
	FLOPPY_A216S_FORMAT, FLOPPY_RWTS18_FORMAT, FLOPPY_EDD_FORMAT
FLOPPY_FORMATS_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_diskiing_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( diskiing );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_diskiing_device::device_rom_region() const
{
	return ROM_NAME( diskiing );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_diskiing_device::a2bus_diskiing_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_DISKIING, "Apple Disk II NG controller", tag, owner, clock, "a2diskiing", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_wozfdc(*this, WOZFDC_TAG),
	floppy0(*this, "0"),
	floppy1(*this, "1"), m_rom(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_diskiing_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(DISKII_ROM_REGION).c_str())->base();
}

void a2bus_diskiing_device::device_reset()
{
	m_wozfdc->set_floppies(floppy0, floppy1);
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_diskiing_device::read_c0nx(address_space &space, UINT8 offset)
{
	return m_wozfdc->read(space, offset);
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_diskiing_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	m_wozfdc->write(space, offset, data);
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_diskiing_device::read_cnxx(address_space &space, UINT8 offset)
{
	return m_rom[offset];
}
