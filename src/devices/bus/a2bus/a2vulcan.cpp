// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2vulcan.c

    Applied Engineering Vulcan IDE controller

    Our copy of ROM version 1.4 will refuse any drive > 40 megs (top 2 bytes of # blocks >= 0x15b).
    Protection against field upgrades?

    Vulcan Gold ROMs omit this protection but don't work with the version of the partitioner program
    we have.

    Recognized drives by IDE features parameters:
    (# of cylinders is never checked, just heads, sectors, and the vendor specific at 0x0A)

    H  S    Vendor specific #5
    8, 33 + 0x69  0x31d blocks   (400K?!)
    2, 33 + 0x69  0xa208 blocks  (20 megs,  21237760  bytes)
    4, 26 + 0x69  0x14500 blocks (40 megs,  42598400  bytes)
    5, 29 + (any) 0x25c5b blocks (80 megs,  79214080  bytes) (chs = 1067,5,29)
    7, 29 + 0x44  0x34e19 blocks (100 megs, 110899712 bytes)
    9, 29 + (any) 0x44068 blocks (140 megs, 142659584 bytes) (chs = 1068,9,29)
    9, 36 + 0x44  0x54888 blocks (180 megs, 177278976 bytes)
    9, 36 + 0xff  0x645a8 blocks (200 megs, 210456576 bytes)
    7, 34 + (any) 0x32252 blocks (100 megs, 105161728 bytes) (chs = 863,7,34)
    4, 17 + 0x55  0xa218 blocks  (20 megs,  21245952  bytes)
    4, 26 + 0x55  0xa218 blocks  (20 megs,  21245952  bytes)
    5, 17 + 0x55  0x15234 blocks (40 megs,  44328960  bytes)
    6, 26 + 0x55  0x15234 blocks (40 megs,  44328960  bytes)
    2, 28 + 0x36  0xa250 blocks  (20 megs,  21274624  bytes)
    4, 28 + 0x36  0x143c0 blocks (40 megs,  42434450  bytes)
    4, 28 + 0x67  0x143c0 blocks (40 megs,  42434450  bytes)
    4, 27 + 0x43  0x147cc blocks (40 megs,  42964992  bytes)
    5, 17 + 0x26  0x13ec0 blocks (40 megs,  41779200  bytes) (chs = 960,5,17)
    15, 32 + 0x43  0x5f6e0 blocks (200 megs, 200130560 bytes)
    16, 38 + 0x94  0x6540c blocks (200 megs, 212342784 bytes)
    10, 17 + (any) 0x2792f blocks (80 megs,  82992640  bytes) (chs = 954,10,17)

    Partition block:
    +0000: 0xAE 0xAE  signature
    +0002: bytesum of remaining 508 bytes of partition block
    +0005: total # of blocks (3 bytes)
    +000E: boot partition # (0 based)
    +0100: partition records

    Partition record:
    +02: partition number (seems to be only valud for non-CLEAR partitions)
    +03: little-endian unsigned word: # of 512 byte blocks
    +06: bit 6 set for ON, bits 0-2 = 0 CLEAR, 1 PRODOS, 2 DOS 3.3, 3 PASCAL, 4 CP/M
    +07: Partition name (Apple high-ASCII, zero terminated unless full 10 chars)

*********************************************************************/

#include "a2vulcan.h"
#include "includes/apple2.h"
#include "machine/ataintf.h"
#include "imagedev/harddriv.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_VULCAN = &device_creator<a2bus_vulcan_device>;
const device_type A2BUS_VULCANGOLD = &device_creator<a2bus_vulcangold_device>;

#define VULCAN_ROM_REGION  "vulcan_rom"
#define VULCAN_ATA_TAG     "vulcan_ata"

static MACHINE_CONFIG_FRAGMENT( vulcan )
	MCFG_ATA_INTERFACE_ADD(VULCAN_ATA_TAG, ata_devices, "hdd", nullptr, false)
MACHINE_CONFIG_END

ROM_START( vulcan )
	ROM_REGION(0x4000, VULCAN_ROM_REGION, 0)
	ROM_LOAD( "ae vulcan rom v1.4.bin", 0x000000, 0x004000, CRC(798d5825) SHA1(1d668e856e33c6eeb10fe26975341afa8acb81f5) )
ROM_END

ROM_START( vulcangold )
	ROM_REGION(0x4000, VULCAN_ROM_REGION, 0)
	ROM_LOAD( "ae vulcan gold rom v2.0.bin", 0x000000, 0x004000, CRC(19bc3958) SHA1(96a22c2540fa603648a4e638e176eee76523b4e1) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_vulcanbase_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vulcan );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_vulcan_device::device_rom_region() const
{
	return ROM_NAME( vulcan );
}

const rom_entry *a2bus_vulcangold_device::device_rom_region() const
{
	return ROM_NAME( vulcangold );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_vulcanbase_device::a2bus_vulcanbase_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_ata(*this, VULCAN_ATA_TAG), m_rom(nullptr), m_lastdata(0), m_rombank(0), m_rambank(0), m_last_read_was_0(false)
{
}

a2bus_vulcan_device::a2bus_vulcan_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	a2bus_vulcanbase_device(mconfig, A2BUS_VULCAN, "Applied Engineering Vulcan IDE controller", tag, owner, clock, "a2vulcan", __FILE__)
{
}

a2bus_vulcangold_device::a2bus_vulcangold_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	a2bus_vulcanbase_device(mconfig, A2BUS_VULCAN, "Applied Engineering Vulcan Gold IDE controller", tag, owner, clock, "a2vulgld", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_vulcanbase_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(VULCAN_ROM_REGION).c_str())->base();

	save_item(NAME(m_lastdata));
	save_item(NAME(m_ram));
	save_item(NAME(m_rombank));
	save_item(NAME(m_rambank));
}

void a2bus_vulcan_device::device_start()
{
	// call base class
	a2bus_vulcanbase_device::device_start();

	// disable 40 meg partition size limit / protection in v1.4 ROMs
	m_rom[0x59e] = 0xea;
	m_rom[0x59f] = 0xea;
}

void a2bus_vulcanbase_device::device_reset()
{
	m_rombank = m_rambank = 0;
	m_last_read_was_0 = false;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_vulcanbase_device::read_c0nx(address_space &space, UINT8 offset)
{
	switch (offset)
	{
		case 0:
			m_lastdata = m_ata->read_cs0(space, offset, 0xffff);
//          printf("IDE: read %04x\n", m_lastdata);
			m_last_read_was_0 = true;
			return m_lastdata&0xff;

		case 1:
			if (m_last_read_was_0)
			{
				m_last_read_was_0 = false;
				return (m_lastdata>>8) & 0xff;
			}
			else
			{
				return m_ata->read_cs0(space, offset, 0xff);
			}

		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			return m_ata->read_cs0(space, offset, 0xff);

		default:
			logerror("a2vulcan: unknown read @ C0n%x\n", offset);
			break;

	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_vulcanbase_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		case 0:
			m_lastdata = data;
			m_last_read_was_0 = true;
			break;

		case 1:
			if (m_last_read_was_0)
			{
				m_last_read_was_0 = false;
				m_lastdata &= 0x00ff;
				m_lastdata |= (data << 8);
//              printf("IDE: write %04x\n", m_lastdata);
				m_ata->write_cs0(space, 0, m_lastdata, 0xffff);
			}
			else
			{
				m_ata->write_cs0(space, offset, data, 0xff);
			}
			break;

		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
//          printf("%02x to IDE controller @ %x\n", data, offset);
			m_ata->write_cs0(space, offset, data, 0xff);
			break;

		case 9: // ROM bank
//          printf("%x (%x) to ROM bank\n", data, (data & 0xf) * 0x400);
			m_rombank = (data & 0xf) * 0x400;
			break;

		case 0xa: // RAM bank
//          printf("%x to RAM bank\n", data);
			m_rambank = (data & 7) * 0x400;
			break;

		default:
			logerror("a2vulcan: write %02x @ unhandled C0n%x\n", data, offset);
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_vulcanbase_device::read_cnxx(address_space &space, UINT8 offset)
{
	int slotimg = m_slot * 0x100;

	// ROM contains a CnXX image for each of slots 1-7 at 0x3400
	return m_rom[offset+slotimg+0x3400];
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

UINT8 a2bus_vulcanbase_device::read_c800(address_space &space, UINT16 offset)
{
	offset &= 0x7ff;
	if (offset < 0x400) // c800-cbff is banked RAM window, cc00-cfff is banked ROM window
	{
//      printf("read RAM @ %x (bank %x)\n", offset, m_rambank);
		return m_ram[offset + m_rambank];
	}

	offset -= 0x400;
	return m_rom[offset+m_rombank];
}

void a2bus_vulcanbase_device::write_c800(address_space &space, UINT16 offset, UINT8 data)
{
	offset &= 0x7ff;
	if (offset < 0x400)
	{
//      printf("%02x to RAM @ %x (bank %x)\n", data, offset, m_rambank);
		m_ram[offset + m_rambank] = data;
	}
}
