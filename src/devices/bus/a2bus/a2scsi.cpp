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

#include "emu.h"
#include "a2scsi.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "speaker.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_SCSI, a2bus_scsi_device, "a2scsi", "Apple II SCSI Card")

#define SCSI_ROM_REGION  "scsi_rom"
#define SCSI_BUS_TAG     "scsibus"
#define SCSI_5380_TAG    "scsibus:7:ncr5380"

ROM_START( scsi )
	ROM_REGION(0x4000, SCSI_ROM_REGION, 0)  // this is the Rev. C ROM
	ROM_LOAD( "341-0437-a.bin", 0x0000, 0x4000, CRC(5aff85d3) SHA1(451c85c46b92e6ad2ad930f055ccf0fe3049936d) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_scsi_device::device_add_mconfig(machine_config &config)
{
	// These machines were strictly external CD-ROMs so sound didn't route back into them; the AppleCD SC had
	// RCA jacks for connection to speakers/a stereo.
	SPEAKER(config, "speaker", 2).front();

	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsibus:0", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:1").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config(
		[](device_t *device)
		{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^speaker", 1.0, 0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^speaker", 1.0, 1);
		});
	NSCSI_CONNECTOR(config, "scsibus:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:6", default_scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsibus:7").option_set("ncr5380", NCR5380).machine_config([this](device_t *device) {
		downcast<ncr5380_device &>(*device).drq_handler().set(*this, FUNC(a2bus_scsi_device::drq_w));
	});
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_scsi_device::device_rom_region() const
{
	return ROM_NAME( scsi );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_scsi_device::a2bus_scsi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_ncr5380(*this, SCSI_5380_TAG),
	m_scsibus(*this, SCSI_BUS_TAG), m_rom(*this, SCSI_ROM_REGION), m_rambank(0), m_rombank(0), m_drq(0), m_bank(0), m_816block(false)
{
}

a2bus_scsi_device::a2bus_scsi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_scsi_device(mconfig, A2BUS_SCSI, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_scsi_device::device_start()
{
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

uint8_t a2bus_scsi_device::read_c0nx(uint8_t offset)
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
//          logerror("Read 5380 @ %x\n", offset);
			return m_ncr5380->read(offset);

		case 8:     // read and DACK
			return m_ncr5380->dma_r();

		case 9:     // our SCSI ID (normally 0x80 = 7)
			return (1<<7);

		case 0xe:   // DRQ status in bit 7
			return m_drq;

		default:
			logerror("Read c0n%x (%s)\n", offset, machine().describe_context());
			break;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_scsi_device::write_c0nx(uint8_t offset, uint8_t data)
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
//          logerror("%02x to 5380 reg %x\n", data, offset);
			m_ncr5380->write(offset, data);
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
//          logerror("RAM bank to %x, ROM bank to %x\n", m_rambank, m_rombank);
			m_816block = false; // does this reset block mode?
			break;

		case 0xb:   // reset 5380
//          logerror("Resetting SCSI: %02x at %s\n", data, machine().describe_context());
			m_ncr5380->reset();
			m_816block = false;
			break;

		case 0xc:   // set IIgs block mode DMA
			logerror("%02x to block-mode DMA mode\n", data);
			m_816block = true;
			break;

		case 0xd:   // set Mac-style pseudo-DMA
//          logerror("%02x to pseudo-DMA mode\n", data);
			m_816block = false;
			break;

		default:
			logerror("Write %02x to c0n%x (%s)\n", data, offset, machine().describe_context());
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_scsi_device::read_cnxx(uint8_t offset)
{
	// one slot image at the start of the ROM, it appears
	return m_rom[offset];
}

void a2bus_scsi_device::write_cnxx(uint8_t offset, uint8_t data)
{
	// there are writes to cn0A, possibly misguided C0nA (bank select?) writes?
//  logerror("Write %02x to cn%02x (%s)\n", data, offset, machine().describe_context());
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_scsi_device::read_c800(uint16_t offset)
{
	// bankswitched RAM at c800-cbff
	// bankswitched ROM at cc00-cfff
	if (offset < 0x400)
	{
//      logerror("Read RAM at %x = %02x\n", offset+m_rambank, m_ram[offset + m_rambank]);
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
void a2bus_scsi_device::write_c800(uint16_t offset, uint8_t data)
{
	if (offset < 0x400)
	{
//      logerror("%02x to RAM at %x\n", data, offset+m_rambank);
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

void a2bus_scsi_device::drq_w(int state)
{
	m_drq = (state ? 0x80 : 0x00);
}
