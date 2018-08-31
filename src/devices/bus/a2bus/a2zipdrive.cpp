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

#include "emu.h"
#include "a2zipdrive.h"
#include "machine/ataintf.h"
#include "imagedev/harddriv.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_ZIPDRIVE, a2bus_zipdrive_device, "a2zipdrv", "Zip Technologies ZipDrive")

#define ZIPDRIVE_ROM_REGION  "zipdrive_rom"
#define ZIPDRIVE_ATA_TAG     "zipdrive_ata"

ROM_START( zipdrive )
	ROM_REGION(0x2000, ZIPDRIVE_ROM_REGION, 0)
	ROM_LOAD( "zip drive - rom.bin", 0x000000, 0x002000, CRC(fd800a40) SHA1(46636bfed88c864139e3d2826661908a8c07c459) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_zipdrivebase_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_zipdrivebase_device::device_rom_region() const
{
	return ROM_NAME( zipdrive );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_zipdrivebase_device::a2bus_zipdrivebase_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_ata(*this, ZIPDRIVE_ATA_TAG), m_rom(nullptr), m_lastdata(0)
{
}

a2bus_zipdrive_device::a2bus_zipdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_zipdrivebase_device(mconfig, A2BUS_ZIPDRIVE, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_zipdrivebase_device::device_start()
{
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

uint8_t a2bus_zipdrivebase_device::read_c0nx(uint8_t offset)
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
			return m_ata->read_cs0(offset, 0xff);

		case 8: // data port
			m_lastdata = m_ata->read_cs0(offset);
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

void a2bus_zipdrivebase_device::write_c0nx(uint8_t offset, uint8_t data)
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
			m_ata->write_cs0(offset, data, 0xff);
			break;

		case 8:
//          printf("%02x to IDE data lo\n", data);
			m_lastdata = data;
			break;

		case 9:
//          printf("%02x to IDE data hi\n", data);
			m_lastdata &= 0x00ff;
			m_lastdata |= (data << 8);
			m_ata->write_cs0(0, m_lastdata);
			break;

		default:
			logerror("a2zipdrive: write %02x @ unhandled C0n%x\n", data, offset);
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_zipdrivebase_device::read_cnxx(uint8_t offset)
{
	int const slotimg = slotno() * 0x100;

	// ROM contains CnXX images for each of slots 1-7 at 0x0 and 0x1000
	return m_rom[offset+slotimg+0x1000];
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_zipdrivebase_device::read_c800(uint16_t offset)
{
	offset &= 0x7ff;

	return m_rom[offset+0x1800];
}
