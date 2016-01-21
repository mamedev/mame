// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2scsi.c

    Implementation of the Apple II SCSI Card

    Schematic at:
    http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/SCSI%20Controllers/Apple%20II%20SCSI%20Card/Schematics/Rev.%20C%20SCSI%20Schematic%20-%20Updated%202-23-6.jpg


    Notes:
    C0n0-C0n7 = NCR5380 registers in normal order
    C0n8 = pseudo-DMA read/write and DACK
    C0n9 = DIP switches
    C0na = RAM and ROM bank switching
    C0nb = reset 5380
    C0nc = set IIgs block mode
    C0nd = set pseudo-DMA
    C0ne = read DRQ status in bit 7

    In IIgs block mode, any read from C800-CBFF window fetches
    the next byte from the 5380's DMA port.  This lets you use the
    65816 MVN/MVP operations to burst-transfer up to 1K at a time.
    (Requires a cycle-by-cycle haltable 65816 core; don't install the
    GS/OS driver right now to avoid this)

    Pseudo-DMA works similarly to the Mac implementation; use C0n8
    to read/write "DMA" bytes in that mode.

*********************************************************************/

#include "a2scsi.h"
#include "includes/apple2.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_SCSI = &device_creator<a2bus_scsi_device>;

#define SCSI_ROM_REGION  "scsi_rom"
#define SCSI_BUS_TAG     "scsibus"
#define SCSI_5380_TAG    "scsibus:7:ncr5380"

static MACHINE_CONFIG_FRAGMENT( ncr5380 )
	MCFG_DEVICE_CLOCK(10000000)
	MCFG_NCR5380N_DRQ_HANDLER(DEVWRITELINE("^^", a2bus_scsi_device, drq_w))
MACHINE_CONFIG_END

static SLOT_INTERFACE_START( scsi_devices )
	SLOT_INTERFACE("cdrom", NSCSI_CDROM)
	SLOT_INTERFACE("harddisk", NSCSI_HARDDISK)
	SLOT_INTERFACE_INTERNAL("ncr5380", NCR5380N)
SLOT_INTERFACE_END

MACHINE_CONFIG_FRAGMENT( scsi )
	MCFG_NSCSI_BUS_ADD(SCSI_BUS_TAG)
	MCFG_NSCSI_ADD("scsibus:0", scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:1", scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:2", scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:3", scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:4", scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:5", scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:6", scsi_devices, "harddisk", false)
	MCFG_NSCSI_ADD("scsibus:7", scsi_devices, "ncr5380", true)
	MCFG_DEVICE_CARD_MACHINE_CONFIG("ncr5380", ncr5380)
MACHINE_CONFIG_END

ROM_START( scsi )
	ROM_REGION(0x4000, SCSI_ROM_REGION, 0)  // this is the Rev. C ROM
	ROM_LOAD( "341-0437-a.bin", 0x0000, 0x4000, CRC(5aff85d3) SHA1(451c85c46b92e6ad2ad930f055ccf0fe3049936d) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_scsi_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( scsi );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_scsi_device::device_rom_region() const
{
	return ROM_NAME( scsi );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_scsi_device::a2bus_scsi_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_ncr5380(*this, SCSI_5380_TAG),
	m_scsibus(*this, SCSI_BUS_TAG), m_rom(nullptr), m_rambank(0), m_rombank(0), m_drq(0), m_bank(0), m_816block(false)
{
}

a2bus_scsi_device::a2bus_scsi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_SCSI, "Apple II SCSI Card", tag, owner, clock, "a2scsi", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_ncr5380(*this, SCSI_5380_TAG),
	m_scsibus(*this, SCSI_BUS_TAG), m_rom(nullptr), m_rambank(0), m_rombank(0), m_drq(0), m_bank(0), m_816block(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_scsi_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(SCSI_ROM_REGION).c_str())->base();

	memset(m_ram, 0, 8192);

	save_item(NAME(m_ram));
	save_item(NAME(m_rambank));
	save_item(NAME(m_rombank));
	save_item(NAME(m_bank));
	save_item(NAME(m_drq));
	save_item(NAME(m_816block));
}

void a2bus_scsi_device::device_reset()
{
	m_rambank = m_rombank = 0;      // CLR on 74LS273 at U3E is connected to RES, so these clear on reset
	m_816block = false;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_scsi_device::read_c0nx(address_space &space, UINT8 offset)
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
//          printf("Read 5380 @ %x\n", offset);
			return m_ncr5380->read(space, offset);

		case 8:     // read and DACK
			return m_ncr5380->dma_r();

		case 9:     // our SCSI ID (normally 0x80 = 7)
			return (1<<7);

		case 0xa:   // RAM/ROM bank
			return m_bank;

		case 0xe:   // DRQ status in bit 7
			return m_drq;

		default:
			printf("Read c0n%x (PC=%x)\n", offset, space.device().safe_pc());
			break;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_scsi_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
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
//          printf("%02x to 5380 reg %x\n", data, offset);
			m_ncr5380->write(space, offset, data);
			break;

		case 8: // write and DACK
			m_ncr5380->dma_w(data);
			break;

		case 0xa:  // ROM and RAM banking (74LS273 at U3E)
			/*
			    ROM banking:
			    (bits EA8-EA13 are all zeroed when /IOSEL is asserted, so CnXX always gets the first page of the ROM)
			    EA10 = bit 0
			    EA11 = bit 1
			    EA12 = bit 2
			    EA13 = bit 3 (N/C)

			    RAM banking:
			    RA10 = bit 4
			    RA11 = bit 5
			    RA12 = bit 6
			*/

			m_rambank = ((data>>4) & 0x7) * 0x400;
			m_rombank = (data & 0xf) * 0x400;
			m_bank = data;
//          printf("RAM bank to %x, ROM bank to %x\n", m_rambank, m_rombank);
			m_816block = false; // does this reset block mode?
			break;

		case 0xb:   // reset 5380
//          printf("Resetting SCSI: %02x at %x\n", data, space.device().safe_pc());
			m_ncr5380->reset();
			m_816block = false;
			break;

		case 0xc:   // set IIgs block mode DMA
			printf("%02x to block-mode DMA mode\n", data);
			m_816block = true;
			break;

		case 0xd:   // set Mac-style pseudo-DMA
//          printf("%02x to pseudo-DMA mode\n", data);
			m_816block = false;
			break;

		default:
			printf("Write %02x to c0n%x (PC=%x)\n", data, offset, space.device().safe_pc());
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_scsi_device::read_cnxx(address_space &space, UINT8 offset)
{
	// one slot image at the start of the ROM, it appears
	return m_rom[offset];
}

void a2bus_scsi_device::write_cnxx(address_space &space, UINT8 offset, UINT8 data)
{
	// there are writes to cn0A, possibly misguided C0nA (bank select?) writes?
//  printf("Write %02x to cn%02x (PC=%x)\n", data, offset, space.device().safe_pc());
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

UINT8 a2bus_scsi_device::read_c800(address_space &space, UINT16 offset)
{
	// bankswitched RAM at c800-cbff
	// bankswitched ROM at cc00-cfff
	if (offset < 0x400)
	{
//      printf("Read RAM at %x = %02x\n", offset+m_rambank, m_ram[offset + m_rambank]);
		if (m_816block)
		{
			return m_ncr5380->dma_r();
		}

		return m_ram[offset + m_rambank];
	}
	else
	{
		return m_rom[(offset-0x400) + m_rombank];
	}
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_scsi_device::write_c800(address_space &space, UINT16 offset, UINT8 data)
{
	if (offset < 0x400)
	{
//      printf("%02x to RAM at %x\n", data, offset+m_rambank);
		if (m_816block)
		{
			m_ncr5380->dma_w(data);
		}
		else
		{
			m_ram[offset + m_rambank] = data;
		}
	}
}

WRITE_LINE_MEMBER( a2bus_scsi_device::drq_w )
{
	m_drq = (state ? 0x80 : 0x00);
}
