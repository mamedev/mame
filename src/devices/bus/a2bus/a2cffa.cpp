// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2cffa.cpp

    Implementation of Rich Dreher's IDE/CompactFlash card
    for the Apple II.

    http://www.dreher.net/

*********************************************************************/

#include "emu.h"
#include "a2cffa.h"

#include "imagedev/harddriv.h"
#include "bus/ata/ataintf.h"

#include "softlist_dev.h"


namespace {

#define CFFA2_ROM_REGION  "cffa2_rom"
#define CFFA2_ATA_TAG     "cffa2_ata"

ROM_START( cffa2 )
	ROM_REGION(0x1000, CFFA2_ROM_REGION, 0)
	ROM_LOAD( "cffa20eec02.bin", 0x000000, 0x001000, CRC(fb3726f8) SHA1(080ff88f19de22328e162954ee2b51ee65f9d5cd) )
ROM_END

ROM_START( cffa2_6502 )
	ROM_REGION(0x1000, CFFA2_ROM_REGION, 0)
	ROM_LOAD( "cffa20ee02.bin", 0x000000, 0x001000, CRC(3ecafce5) SHA1(d600692ed9626668233a22a48236af639410cb7b) )
ROM_END

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_cffa2000_device:
	public device_t,
	public device_a2bus_card_interface
{
protected:
	// construction/destruction
	a2bus_cffa2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;

	required_device<ata_interface_device> m_ata;
	required_region_ptr<uint8_t> m_rom;

	uint8_t m_eeprom[0x1000];

private:
	uint16_t m_lastdata, m_lastreaddata;
	bool m_writeprotect;
	bool m_inwritecycle;
};

class a2bus_cffa2_device : public a2bus_cffa2000_device, public device_nvram_interface
{
public:
	a2bus_cffa2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_config_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
};

class a2bus_cffa2_6502_device : public a2bus_cffa2000_device, public device_nvram_interface
{
public:
	a2bus_cffa2_6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// device_config_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_cffa2000_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", "hdd", false);

// not yet, the core explodes
//  SOFTWARE_LIST(config, "hdd_list").set_original("apple2gs_hdd");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_cffa2000_device::device_rom_region() const
{
	return ROM_NAME( cffa2 );
}

const tiny_rom_entry *a2bus_cffa2_6502_device::device_rom_region() const
{
	return ROM_NAME( cffa2_6502 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_cffa2000_device::a2bus_cffa2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_ata(*this, CFFA2_ATA_TAG),
	m_rom(*this, CFFA2_ROM_REGION),
	m_lastdata(0), m_lastreaddata(0), m_writeprotect(false), m_inwritecycle(false)
{
}

a2bus_cffa2_device::a2bus_cffa2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_cffa2000_device(mconfig, A2BUS_CFFA2, tag, owner, clock),
	device_nvram_interface(mconfig, *this)
{
}

a2bus_cffa2_6502_device::a2bus_cffa2_6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_cffa2000_device(mconfig, A2BUS_CFFA2_6502, tag, owner, clock),
	device_nvram_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_cffa2000_device::device_start()
{
	// patch default setting so slave device is enabled and up to 13 devices on both connectors
	m_rom[0x800] = 13;
	m_rom[0x801] = 13;

	save_item(NAME(m_lastdata));
	save_item(NAME(m_lastreaddata));
	save_item(NAME(m_writeprotect));
	save_item(NAME(m_eeprom));
	save_item(NAME(m_inwritecycle));
}

void a2bus_cffa2000_device::device_reset()
{
	m_writeprotect = true;
	m_inwritecycle = false;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_cffa2000_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
			return m_lastreaddata >> 8;

		case 3:
			m_writeprotect = false;
			break;

		case 4:
			m_writeprotect = true;
			break;

		case 8:
			// Apple /// driver uses sta $c080,x when writing, which causes spurious reads of c088
			if (!m_inwritecycle)
			{
				m_lastreaddata = m_ata->cs0_r(offset - 8);
			}
			return m_lastreaddata & 0xff;

		case 9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			return m_ata->cs0_r(offset - 8, 0xff);
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_cffa2000_device::write_c0nx(uint8_t offset, uint8_t data)
{
	m_inwritecycle = false;

	switch (offset)
	{
		case 0:
			m_lastdata &= 0x00ff;
			m_lastdata |= data<<8;
//          printf("%02x to 0, m_lastdata = %x\n", data, m_lastdata);
			m_inwritecycle = true;
			break;

		case 3:
			m_writeprotect = false;
			break;

		case 4:
			m_writeprotect = true;
			break;

		case 8:
			m_lastdata &= 0xff00;
			m_lastdata |= data;
//          printf("%02x to 8, m_lastdata = %x\n", data, m_lastdata);
			m_ata->cs0_w(offset-8, m_lastdata);
			break;

		case 9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			m_ata->cs0_w(offset-8, data, 0xff);
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_cffa2000_device::read_cnxx(uint8_t offset)
{
	int const slotimg = slotno() * 0x100;

	// ROM contains a CnXX image for each of slots 1-7
	return m_eeprom[offset+slotimg];
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_cffa2000_device::read_c800(uint16_t offset)
{
	return m_eeprom[offset+0x800];
}

void a2bus_cffa2000_device::write_c800(uint16_t offset, uint8_t data)
{
	if (!m_writeprotect)
	{
		m_eeprom[offset + 0x800] = data;
	}
}

// NVRAM device virtual overrides to provide saving/loading of settings changes
void a2bus_cffa2_device::nvram_default()
{
	memcpy(m_eeprom, m_rom, 0x1000);
}

bool a2bus_cffa2_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_eeprom, 0x1000);
	return !err && (actual == 0x1000);
}

bool a2bus_cffa2_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_eeprom, 0x1000);
	return !err;
}

void a2bus_cffa2_6502_device::nvram_default()
{
	memcpy(m_eeprom, m_rom, 0x1000);
}

bool a2bus_cffa2_6502_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_eeprom, 0x1000);
	return !err && (actual == 0x1000);
}

bool a2bus_cffa2_6502_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_eeprom, 0x1000);
	return !err;
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_CFFA2,      device_a2bus_card_interface, a2bus_cffa2_device,      "a2cffa2",  "CFFA 2.0 Compact Flash (65C02 firmware, www.dreher.net)")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_CFFA2_6502, device_a2bus_card_interface, a2bus_cffa2_6502_device, "a2cffa02", "CFFA 2.0 Compact Flash (6502 firmware, www.dreher.net)")
