// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    cmsscsi.cpp

    Implementation of the CMS SCSI II Card

    Details:
    ROM is $2000 bytes.  The last $800 bytes are 7 slot images at
    offset (slot*$100).  There is no slot 0 image, naturally, so $1800-$18FF
    is blank.

    The other $1800 bytes of the ROM are banked at $C800.  The first $80
    bytes at $C800 are the first $80 bytes of the 2K SRAM chip regardless
    of the ROM bank setting.  This also means the first $80 bytes of each
    ROM bank are inaccessible.  (Most of the SRAM is also inaccessible)

    $C0(n+8)X space:
        $0-$7: NCR5380 registers
        $8: Appears to control SCSI delay time.  1 is shortest, 0 is longest ($FF is second longest, etc)
        $9: ROM bank at $C800 in the low 2 bits.
        $A: read/write to the DMA data register on the 5380
        $B, $C, $D, $E: the jumpers on the card.  Not sure yet which is which.
        $F: Card will not boot if it doesn't return 0.

*********************************************************************/

#include "emu.h"
#include "cmsscsi.h"
#include "bus/nscsi/devices.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_CMSSCSI, a2bus_cmsscsi_device, "cmsscsi", "CMS SCSI II Card")

#define SCSI_ROM_REGION  "scsi_rom"
#define SCSI_BUS_TAG     "scsibus"
#define SCSI_5380_TAG    "scsibus:7:ncr5380"

ROM_START( scsi )
	ROM_REGION(0x4000, SCSI_ROM_REGION, 0)
		ROM_DEFAULT_BIOS("cms3190")
		ROM_SYSTEM_BIOS( 0, "cms3190", "CMS SCSI BIOS dated 3-1-90" )
		ROMX_LOAD( "cms scsi ii interface card rom 3-1-90.bin", 0x000000, 0x002000, CRC(9452cc76) SHA1(b981ce644dde479c22d2ba270d472fe3b5d7f3e9), ROM_BIOS(0) )

		ROM_SYSTEM_BIOS( 1, "cms871109", "CMS SCSI BIOS dated 1987-11-09" )
		ROMX_LOAD( "cms 1987-11-09 scsi.bin", 0x000000, 0x002000, CRC(89d1d917) SHA1(f6028f6f2b16250950297e2ff5317ce463006796), ROM_BIOS(1) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_cmsscsi_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsibus:0", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:6", default_scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsibus:7").option_set("ncr5380", NCR5380);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_cmsscsi_device::device_rom_region() const
{
	return ROM_NAME( scsi );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_cmsscsi_device::a2bus_cmsscsi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_ncr5380(*this, SCSI_5380_TAG),
	m_scsibus(*this, SCSI_BUS_TAG), m_rom(*this, SCSI_ROM_REGION), m_rombank(0)
{
}

a2bus_cmsscsi_device::a2bus_cmsscsi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_cmsscsi_device(mconfig, A2BUS_CMSSCSI, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_cmsscsi_device::device_start()
{
	memset(m_ram, 0, 2048);

	save_item(NAME(m_ram));
	save_item(NAME(m_rombank));
}

void a2bus_cmsscsi_device::device_reset()
{
	m_rombank = 0;
}

void a2bus_cmsscsi_device::reset_from_bus()
{
	m_ncr5380->reset();
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_cmsscsi_device::read_c0nx(uint8_t offset)
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

		case 8:     // appears to be delay time
			return 1;

		case 0xa:     // read and DACK
			return m_ncr5380->dma_r();

		case 0xb:   //
			return 0x1;

		case 0xc:   //
			return 0x1;

		case 0xd:   //
			return 0x1;

		case 0xe:   //
			return 0x1;

		case 0xf:   // must be 0 to boot
			return 0;

		default:
			logerror("Read c0n%x (%s)\n", offset, machine().describe_context().c_str());
			break;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_cmsscsi_device::write_c0nx(uint8_t offset, uint8_t data)
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

		case 9:
			m_rombank = (data & 3) * 0x800;
			//printf("ROM bank to %x\n", m_rombank);
			break;

		case 0xa: // write and DACK
			m_ncr5380->dma_w(data);
			break;

		default:
			logerror("Write %02x to c0n%x (%s)\n", data, offset, machine().describe_context().c_str());
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_cmsscsi_device::read_cnxx(uint8_t offset)
{
	// slot images at $1800 + (slot * 100)
	return m_rom[offset + (slotno() * 0x100) + 0x1800];
}

void a2bus_cmsscsi_device::write_cnxx(uint8_t offset, uint8_t data)
{
	// there are writes to cn0A, possibly misguided C0nA (bank select?) writes?
//  logerror("Write %02x to cn%02x (%s)\n", data, offset, machine().describe_context());
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_cmsscsi_device::read_c800(uint16_t offset)
{
	if (offset < 0x80)
	{
			return m_ram[offset];
	}

	return m_rom[offset + m_rombank];
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_cmsscsi_device::write_c800(uint16_t offset, uint8_t data)
{
	if (offset < 0x80)
	{
		m_ram[offset] = data;
	}
}
