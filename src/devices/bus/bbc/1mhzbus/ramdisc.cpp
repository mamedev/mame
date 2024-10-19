// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Morley Electronics RAM Disc

    Notes:
    - 1MB model has 4096 sectors &0000 to &0fff
    - 2MB model has 8192 sectors &0000 to &1fff

**********************************************************************/


#include "emu.h"
#include "ramdisc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_RAMDISC, bbc_ramdisc_device, "bbc_ramdisc", "Morley Electronics RAM Disc");


//-------------------------------------------------
//  INPUT_PORTS( ramdisc )
//-------------------------------------------------

static INPUT_PORTS_START(ramdisc)
	PORT_START("POWER")
	PORT_CONFNAME(0x01, 0x01, "Power") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bbc_ramdisc_device::power_changed), 0)
	PORT_CONFSETTING(0x00, DEF_STR(Off))
	PORT_CONFSETTING(0x01, DEF_STR(On))
	PORT_START("SIZE")
	PORT_CONFNAME(0x03, 0x02, "RAM Disk Capacity")
	PORT_CONFSETTING(0x01, "1MB")
	PORT_CONFSETTING(0x02, "2MB")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_ramdisc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ramdisc);
}

//-------------------------------------------------
//  ROM( ramdisc )
//-------------------------------------------------

ROM_START(ramdisc)
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_LOAD("ramdisc101.rom", 0x0000, 0x4000, CRC(627568c2) SHA1(17e727998756fe35ff451fd2ce1d4b5977be24fc))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_ramdisc_device::device_add_mconfig(machine_config &config)
{
	/* ram disk */
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	BBC_1MHZBUS_SLOT(config, m_1mhzbus, DERIVED_CLOCK(1, 1), bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_1mhzbus->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_ramdisc_device::device_rom_region() const
{
	return ROM_NAME(ramdisc);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_ramdisc_device - constructor
//-------------------------------------------------

bbc_ramdisc_device::bbc_ramdisc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_RAMDISC, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_1mhzbus(*this, "1mhzbus")
	, m_nvram(*this, "nvram")
	, m_ram_size(*this, "SIZE")
	, m_power(*this, "POWER")
	, m_sector(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_ramdisc_device::device_start()
{
	/* define 2mb ram */
	m_ram = std::make_unique<uint8_t[]>(0x200000);
	m_nvram->set_base(m_ram.get(), 0x200000);

	/* register for save states */
	save_pointer(NAME(m_ram), 0x200000);
	save_item(NAME(m_sector));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

INPUT_CHANGED_MEMBER(bbc_ramdisc_device::power_changed)
{
	/* clear RAM on power off */
	if (!newval)
	{
		memset(m_ram.get(), 0xff, 0x200000);
	}
}

uint8_t bbc_ramdisc_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_power->read())
	{
		switch (offset)
		{
		case 0xc0:
			/* sector LSB */
			data = m_sector & 0x00ff;
			break;
		case 0xc2:
			/* sector MSB */
			data = (m_sector & 0xff00) >> 8;
			break;
		case 0xc1:
		case 0xc3:
			/* TODO: unknown purpose, must return 0x3f or 0x5f */
			data = 0x3f;
			logerror("Read %04x -> %02x\n", offset | 0xfcc0, data);
			break;
		}
	}

	data &= m_1mhzbus->fred_r(offset);

	return data;
}

void bbc_ramdisc_device::fred_w(offs_t offset, uint8_t data)
{
	if (m_power->read())
	{
		switch (offset)
		{
		case 0xc0:
			/* sector LSB */
			m_sector = (m_sector & 0xff00) | data;
			break;
		case 0xc2:
			/* sector MSB */
			m_sector = (m_sector & 0x00ff) | (data << 8);
			break;
		case 0xc1:
		case 0xc3:
			/* TODO: unknown purpose, always writes 0x00 or 0xff */
			logerror("Write %04x <- %02x\n", offset | 0xfcc0, data);
			break;
		}
	}

	m_1mhzbus->fred_w(offset, data);
}

uint8_t bbc_ramdisc_device::jim_r(offs_t offset)
{
	uint8_t data = 0xff;

	/* power on and sector < 2mb */
	if (m_power->read() && m_sector < (m_ram_size->read() << 8))
	{
		data &= m_ram[(m_sector << 8) | offset];
	}

	data &= m_1mhzbus->jim_r(offset);

	return data;
}

void bbc_ramdisc_device::jim_w(offs_t offset, uint8_t data)
{
	/* power on and sector < 2mb */
	if (m_power->read() && m_sector < (m_ram_size->read() << 8))
	{
		m_ram[(m_sector << 8) | offset] = data;
	}

	m_1mhzbus->jim_w(offset, data);
}
