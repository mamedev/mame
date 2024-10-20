// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2zipdrive.cpp

    ZIP Technologies ZipDrive IDE card
    Parsons Engineering Focus Drive IDE card

    These cards are basically identical, with
    almost byte-identical firmware (written by "Burger" Becky).

    Focus / ZIP partition sector (sector 0):
    +000: "Parsons Engin" or "Zip Technolog"
    +00e: String terminator 0
    +00f: Number of partitions
    +01c: Drive type (index into table of fixed geometries)
    +020: Partition 0 start block
    +024: Parition 0 block count
    +028: P0 Write protect flag
    +02d/02e/02f: Unknown (written by Focus s/w, not read by ROM or GS/OS driver)
    +030: Partition 1 start block
    +034: Partition 1 block count
    +038: P1 write protect flag
    +03d/03e/03f: Unknown (written by Focus s/w, not read by ROM or GS/OS driver)
    +040: Partition 2
    +050: Partition 3
    +060: Partition 4
    +070: Partition 5
    +080: Partition 6
    +090: Partition 7
    [...]
    +1E0: Partition 15

    Sector 1:
    +020: Partition 0 name
    +040: Partition 1 name
    +060: Partition 2 name
    +080: Partition 3 name
    +0a0: Partition 4 name
    +0c0: Partition 5 name
    +0e0: Partition 6 name
    +100: Partition 7 name
    [...]
    +1E0: Partition 15 name

    Drive types:
    Drive type 0: CHS=782,4,27, 84590 total blocks, capacity 43 MB
    Drive type 1: CHS=782,3,27, 63449 total blocks, capacity 32 MB
    Drive type 2: CHS=782,2,27, 42308 total blocks, capacity 21 MB
    Drive type 3: CHS=1334,2,33, 88142 total blocks, capacity 45 MB
    Drive type 4: CHS=1334,4,33, 176252 total blocks, capacity 90 MB
    Drive type 5: CHS=1334,6,33, 264362 total blocks, capacity 135 MB
    Drive type 6: CHS=1334,8,33, 352472 total blocks, capacity 180 MB
    Drive type 7: CHS=670,4,31, 83234 total blocks, capacity 42 MB
    Drive type 8: CHS=977,2,43, 84150 total blocks, capacity 43 MB
    Drive type 9: CHS=614,4,17, 41836 total blocks, capacity 21 MB
    Drive type 10: CHS=65535,5,17, 5570576 total blocks, capacity -1442 MB (???)
    Drive type 11: CHS=872,4,24, 83831 total blocks, capacity 42 MB
    Drive type 12: CHS=977,5,17, 83146 total blocks, capacity 42 MB
    Drive type 13: CHS=547,4,38, 83333 total blocks, capacity 42 MB
    Drive type 14: CHS=977,4,43, 168258 total blocks, capacity 86 MB
    Drive type 15: CHS=973,4,43, 167570 total blocks, capacity 85 MB
    Drive type 16: CHS=791,3,35, 83194 total blocks, capacity 42 MB
    Drive type 17: CHS=548,8,38, 166933 total blocks, capacity 85 MB

*********************************************************************/

#include "emu.h"
#include "a2zipdrive.h"

#include "bus/ata/ataintf.h"
#include "imagedev/harddriv.h"


namespace {

#define ZIPDRIVE_ROM_REGION  "zipdrive_rom"
#define ZIPDRIVE_ATA_TAG     "zipdrive_ata"

ROM_START( zipdrive )
	ROM_REGION(0x2000, ZIPDRIVE_ROM_REGION, 0)
	ROM_LOAD( "zip drive - rom.bin", 0x000000, 0x002000, CRC(fd800a40) SHA1(46636bfed88c864139e3d2826661908a8c07c459) )
ROM_END

ROM_START( focusdrive )
	ROM_REGION(0x2000, ZIPDRIVE_ROM_REGION, 0)
	ROM_LOAD( "focusrom.bin", 0x001000, 0x001000, CRC(0fd0ba25) SHA1(acf414aa145fcfa1c12aca0269f1f7ada82f1c04) )
ROM_END

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_zipdrivebase_device:
	public device_t,
	public device_a2bus_card_interface
{
protected:
	// construction/destruction
	a2bus_zipdrivebase_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual uint8_t read_c800(uint16_t offset) override;

	required_device<ata_interface_device> m_ata;
	required_region_ptr<uint8_t> m_rom;

	uint16_t m_lastdata;
};

class a2bus_zipdrive_device : public a2bus_zipdrivebase_device
{
public:
	a2bus_zipdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class a2bus_focusdrive_device : public a2bus_zipdrivebase_device
{
public:
	a2bus_focusdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
};

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

const tiny_rom_entry *a2bus_focusdrive_device::device_rom_region() const
{
	return ROM_NAME( focusdrive );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_zipdrivebase_device::a2bus_zipdrivebase_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_ata(*this, ZIPDRIVE_ATA_TAG),
	m_rom(*this, ZIPDRIVE_ROM_REGION),
	m_lastdata(0)
{
}

a2bus_zipdrive_device::a2bus_zipdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_zipdrivebase_device(mconfig, A2BUS_ZIPDRIVE, tag, owner, clock)
{
}

a2bus_focusdrive_device::a2bus_focusdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_zipdrivebase_device(mconfig, A2BUS_FOCUSDRIVE, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_zipdrivebase_device::device_start()
{
	save_item(NAME(m_lastdata));
}

void a2bus_zipdrivebase_device::device_reset()
{
	m_rom[0x1c44] = 0x03;
}

void a2bus_focusdrive_device::device_reset()
{
	m_rom[0x1c6c] = 0x03;   // eat 3 IDE words here instead of 1, fixes a bug? in the original ROM
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
			return m_ata->cs0_r(offset, 0xff);

		case 8: // data port
			m_lastdata = m_ata->cs0_r(0, 0xffff);
//          printf("%04x @ IDE data\n", m_lastdata);
			return m_lastdata&0xff;

		case 9:
			return (m_lastdata>>8) & 0xff;

		default:
			logerror("unhandled read @ C0n%x\n", offset);
			break;
	}

	return 0xff;
}

uint8_t a2bus_focusdrive_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 8:
		case 9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			return m_ata->cs0_r(offset&7, 0xff);

		case 0: // data port
			m_lastdata = m_ata->cs0_r(0, 0xffff);
			//printf("%04x @ IDE data\n", m_lastdata);
			return m_lastdata&0xff;

		case 1:
			return (m_lastdata>>8) & 0xff;

		default:
			logerror("unhandled read @ C0n%x\n", offset);
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
			m_ata->cs0_w(offset, data, 0xff);
			break;

		case 8:
//          printf("%02x to IDE data lo\n", data);
			m_lastdata = data;
			break;

		case 9:
//          printf("%02x to IDE data hi\n", data);
			m_lastdata &= 0x00ff;
			m_lastdata |= (data << 8);
			m_ata->cs0_w(0, m_lastdata, 0xffff);
			break;

		default:
			logerror("a2zipdrive: write %02x @ unhandled C0n%x\n", data, offset);
			break;
	}
}

void a2bus_focusdrive_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 8:
		case 9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			// due to a bug in the 6502 firmware, eat data if DRQ is set
			#if 0
			while (m_ata->cs0_r(7, 0xff) & 0x08)
			{
				m_ata->cs0_r(0, 0xffff);
				printf("eating 2 bytes to clear DRQ\n");
			}
			#endif
//          printf("%02x to IDE controller @ %x\n", data, offset);
			m_ata->cs0_w(offset & 7, data, 0xff);
			break;

		case 0:
//          printf("%02x to IDE data lo\n", data);
			m_lastdata = data;
			break;

		case 1:
//          printf("%02x to IDE data hi\n", data);
			m_lastdata &= 0x00ff;
			m_lastdata |= (data << 8);
			m_ata->cs0_w(0, m_lastdata, 0xffff);
			break;

		default:
			printf("focus: write %02x @ unhandled C0n%x\n", data, offset);
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

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_ZIPDRIVE,   device_a2bus_card_interface, a2bus_zipdrive_device,   "a2zipdrv", "Zip Technologies ZipDrive")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_FOCUSDRIVE, device_a2bus_card_interface, a2bus_focusdrive_device, "a2focdrv", "Parsons Engineering Focus Drive")
