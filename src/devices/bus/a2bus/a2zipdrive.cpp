// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2zipdrive.c

    ZIP Technologies ZipDrive IDE card

    NOTE: No known dump exists of the formatter utility and the
    format of the custom partition record (block 0) that the card
    expects has not yet been determined, so this is largely untested
    and will work only with a drive dump from real h/w.

    PLEASE use it only on a backup copy of said dump and contact MESSdev
    if you have one!

    Partition block format:
    +0000: ASCII "Zip Technologies"

*********************************************************************/

#include "a2zipdrive.h"
#include "includes/apple2.h"
#include "machine/ataintf.h"
#include "imagedev/harddriv.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_ZIPDRIVE = &device_creator<a2bus_zipdrive_device>;

#define ZIPDRIVE_ROM_REGION  "zipdrive_rom"
#define ZIPDRIVE_ATA_TAG     "zipdrive_ata"

static MACHINE_CONFIG_FRAGMENT( zipdrive )
	MCFG_ATA_INTERFACE_ADD(ZIPDRIVE_ATA_TAG, ata_devices, "hdd", nullptr, false)
MACHINE_CONFIG_END

ROM_START( zipdrive )
	ROM_REGION(0x2000, ZIPDRIVE_ROM_REGION, 0)
	ROM_LOAD( "zip drive - rom.bin", 0x000000, 0x002000, CRC(fd800a40) SHA1(46636bfed88c864139e3d2826661908a8c07c459) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_zipdrivebase_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( zipdrive );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_zipdrivebase_device::device_rom_region() const
{
	return ROM_NAME( zipdrive );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_zipdrivebase_device::a2bus_zipdrivebase_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_ata(*this, ZIPDRIVE_ATA_TAG), m_rom(nullptr), m_lastdata(0)
{
}

a2bus_zipdrive_device::a2bus_zipdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	a2bus_zipdrivebase_device(mconfig, A2BUS_ZIPDRIVE, "Zip Technologies ZipDrive", tag, owner, clock, "a2zipdrv", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_zipdrivebase_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(ZIPDRIVE_ROM_REGION).c_str())->base();

	save_item(NAME(m_lastdata));
}

void a2bus_zipdrivebase_device::device_reset()
{
	popmessage("Zip Drive partition format unknown, contact MESSdev if you have the software or a drive dump!");
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_zipdrivebase_device::read_c0nx(address_space &space, UINT8 offset)
{
	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			return m_ata->read_cs0(space, offset, 0xff);

		case 8: // data port
			m_lastdata = m_ata->read_cs0(space, offset, 0xffff);
//          printf("%04x @ IDE data\n", m_lastdata);
			return m_lastdata&0xff;

		case 9:
			return (m_lastdata>>8) & 0xff;

		default:
			logerror("a2zipdrive: unhandled read @ C0n%x\n", offset);
			break;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_zipdrivebase_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
//          printf("%02x to IDE controller @ %x\n", data, offset);
			m_ata->write_cs0(space, offset, data, 0xff);
			break;

		case 8:
//          printf("%02x to IDE data lo\n", data);
			m_lastdata = data;
			break;

		case 9:
//          printf("%02x to IDE data hi\n", data);
			m_lastdata &= 0x00ff;
			m_lastdata |= (data << 8);
			m_ata->write_cs0(space, 0, m_lastdata, 0xffff);
			break;

		default:
			logerror("a2zipdrive: write %02x @ unhandled C0n%x\n", data, offset);
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_zipdrivebase_device::read_cnxx(address_space &space, UINT8 offset)
{
	int slotimg = m_slot * 0x100;

	// ROM contains CnXX images for each of slots 1-7 at 0x0 and 0x1000
	return m_rom[offset+slotimg+0x1000];
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

UINT8 a2bus_zipdrivebase_device::read_c800(address_space &space, UINT16 offset)
{
	offset &= 0x7ff;

	return m_rom[offset+0x1800];
}
