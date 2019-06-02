// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2hsscsi.c

    Implementation of the Apple II High Speed SCSI Card

    This uses an ASIC called "Sandwich II"; the card itself is
    sometimes known as "Cocoon".

    Notes:
    C0n0-C0n7 = NCR5380 registers in normal order
    C0n8 = DMA address low
    C0n9 = DMA address high
    C0nA = DMA count low
    C0nB = DMA count high
    C0nC = DMA control
    C0nD = Enable DMA / reset 5380
    C0nE = Priority (read bits 5-7) / Fire watchdog (write bit 7) / RAM bank (write bits 0-3)
    C0nF = DMA speed (bit 7 = 0 for fast, 1 for slow) / ROM bank (write bits 0-4)

    DMA control register (C0nC):
    0x01 = pseudo-DMA enable
    0x02 = DMA enable
    0x04 = test mode
    0x08 = disable stop-DMA-on-IRQ
    0x10 = DMA direction (read only)
    0x20 = 5380 IRQ enable
    0x40 = system DMA status (read only)
    0x80 = DMA stopped due to IRQ

    Enable DMA / reset 5380 register (C0nD):
    0x01 = Resume DMA after rollover or IRQ
    0x02 = Reset the 5380
    0x40 = Clear test mode
    0x80 = Set test mode

*********************************************************************/

#include "emu.h"
#include "a2hsscsi.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_HSSCSI, a2bus_hsscsi_device, "a2hsscsi", "Apple II High-Speed SCSI Card")

#define SCSI_ROM_REGION  "scsi_rom"
#define SCSI_BUS_TAG     "scsibus"
#define SCSI_5380_TAG    "scsibus:7:ncr5380"

static void hsscsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add_internal("ncr5380", NCR5380N);
}

ROM_START( hsscsi )
	ROM_REGION(0x8000, SCSI_ROM_REGION, 0)
	ROM_LOAD( "341-0803.bin", 0x0000, 0x8000, CRC(2c15618b) SHA1(7d32227299933bfc1b7f8bc2062906fdfe530674) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_hsscsi_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsibus:0", hsscsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:1", hsscsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:2", hsscsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:3", hsscsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:4", hsscsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:5", hsscsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:6", hsscsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsibus:7", hsscsi_devices, "ncr5380", true).set_option_machine_config("ncr5380", [this](device_t *device) {
		device->set_clock(10000000);
		downcast<ncr5380n_device &>(*device).drq_handler().set(*this, FUNC(a2bus_hsscsi_device::drq_w));
	});
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_hsscsi_device::device_rom_region() const
{
	return ROM_NAME( hsscsi );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_hsscsi_device::a2bus_hsscsi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_ncr5380(*this, SCSI_5380_TAG),
	m_scsibus(*this, SCSI_BUS_TAG), m_rom(nullptr), m_rambank(0), m_rombank(0), m_drq(0), m_bank(0), m_816block(false), m_c0ne(0), m_c0nf(0)
{
}

a2bus_hsscsi_device::a2bus_hsscsi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_hsscsi_device(mconfig, A2BUS_HSSCSI, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_hsscsi_device::device_start()
{
	m_rom = machine().root_device().memregion(this->subtag(SCSI_ROM_REGION).c_str())->base();

	memset(m_ram, 0, 8192);

	save_item(NAME(m_ram));
	save_item(NAME(m_rambank));
	save_item(NAME(m_rombank));
	save_item(NAME(m_bank));
	save_item(NAME(m_drq));
	save_item(NAME(m_816block));
}

void a2bus_hsscsi_device::device_reset()
{
	m_rambank = 0;
	m_rombank = 0;
	m_c0ne = m_c0nf = 0;
	m_816block = false;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_hsscsi_device::read_c0nx(uint8_t offset)
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

		case 0xc:
			return 0x00;    // indicate watchdog?

		case 0xe:   // code at cf32 wants to RMW this without killing the ROM bank
			return m_c0ne;

		case 0xf:
			return m_c0nf;

		default:
			logerror("Read c0n%x (%s)\n", offset, machine().describe_context());
			break;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_hsscsi_device::write_c0nx(uint8_t offset, uint8_t data)
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
#if 0
		case 8: // DMA address low
			break;

		case 9: // DMA address high
			break;

		case 0xa: // DMA count low
			break;

		case 0xb: // DMA count high
			break;

		case 0xc:   // DMA control
			break;
#endif

		case 0xd:   // DMA enable / reset
			logerror("%02x to DMA enable/reset\n", data);
			if (data & 0x2)
			{
	//          logerror("Resetting SCSI: %02x at %s\n", data, machine().describe_context());
				m_ncr5380->reset();
			}
			break;

		case 0xe:
			m_c0ne = data;
			m_rombank = (data & 0x1f) * 0x400;
			logerror("c0ne to %x (ROM bank %x)\n", data & 0x1f, m_rombank);
			break;

		case 0xf:
			m_c0nf = data;
			m_rambank = (data & 0x7) * 0x400;
			logerror("c0nf to %x (RAM bank %x)\n", data & 0x7, m_rambank);
			break;

		default:
			logerror("Write %02x to c0n%x (%s)\n", data, offset, machine().describe_context());
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_hsscsi_device::read_cnxx(uint8_t offset)
{
	// one slot image at the start of the ROM, it appears
	return m_rom[offset];
}

void a2bus_hsscsi_device::write_cnxx(uint8_t offset, uint8_t data)
{
//  logerror("Write %02x to cn%02x (PC=%x)\n", data, offset, machine().describe_context());
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_hsscsi_device::read_c800(uint16_t offset)
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
void a2bus_hsscsi_device::write_c800(uint16_t offset, uint8_t data)
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

WRITE_LINE_MEMBER( a2bus_hsscsi_device::drq_w )
{
	m_drq = (state ? 0x80 : 0x00);
}
