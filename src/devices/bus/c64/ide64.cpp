// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IDE64 v4.1 cartridge emulation

**********************************************************************/

/*

    TODO:

    - FT245 USB
    - ShortBus (ETH64, DUART, DigiMAX, ETFE)
    - clock port (ETH64 II, RR-Net, SilverSurfer, MP3@64)

*/

#include "emu.h"
#include "ide64.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define AT29C010A_TAG "u3"
#define DS1302_TAG    "u4"
#define FT245R_TAG    "u21"
#define ATA0_TAG      "ata0"
#define ATA1_TAG      "ata1"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_IDE64, c64_ide64_cartridge_device, "c64_ide64", "C64 IDE64 cartridge")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_ide64_cartridge_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	auto cdrom_config = [](device_t *device)
	{
		device->subdevice<cdda_device>("cdda")->add_route(0, "^^speaker", 1.0, 0);
		device->subdevice<cdda_device>("cdda")->add_route(1, "^^speaker", 1.0, 1);
	};

	ATMEL_29C010(config, m_flash_rom);
	DS1302(config, m_rtc, 32.768_kHz_XTAL);

	ATA_INTERFACE(config, m_ata0).set_slot_options(0, ata_devices, "cf", true);
	m_ata0->set_slot_options(1, ata_devices, "cdrom", false);
	m_ata0->default_data(0xff7f);
	m_ata0->slot(1).set_option_machine_config("cdrom", cdrom_config);

	ATA_INTERFACE(config, m_ata1).options(ata_devices, nullptr, nullptr, false);
	m_ata1->default_data(0xff7f);
	m_ata1->slot(0).set_option_machine_config("cdrom", cdrom_config);
	m_ata1->slot(1).set_option_machine_config("cdrom", cdrom_config);
}


//-------------------------------------------------
//  ROM( c64_ide64 )
//-------------------------------------------------

ROM_START( c64_ide64 )
	ROM_REGION( 0x1f, DS1302_TAG, 0 )
	ROM_LOAD( "ds1302.u4", 0x00, 0x1f, CRC(3399de09) SHA1(abfcee87b934b78c0d0b37e3655c8197fd9b4549) )
ROM_END


//-------------------------------------------------
//  device_rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c64_ide64_cartridge_device::device_rom_region() const
{
	return ROM_NAME( c64_ide64 );
}


//-------------------------------------------------
//  INPUT_PORTS( c64_ide64 )
//-------------------------------------------------

static INPUT_PORTS_START( c64_ide64 )
	PORT_START("JP1")
	PORT_DIPNAME( 0x01, 0x01, "Flash ROM Write Protect" )
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPSETTING(    0x01, "Enabled" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_ide64_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_ide64 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_ide64_cartridge_device - constructor
//-------------------------------------------------

c64_ide64_cartridge_device::c64_ide64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_IDE64, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_flash_rom(*this, AT29C010A_TAG),
	m_rtc(*this, DS1302_TAG),
	m_ata0(*this, ATA0_TAG),
	m_ata1(*this, ATA1_TAG),
	m_jp1(*this, "JP1"),
	m_ram(*this, "ram", 0x8000, ENDIANNESS_LITTLE),
	m_bank(0), m_ata_data(0), m_ata_selected(0), m_wp(0), m_enable(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_ide64_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
	save_item(NAME(m_ata_data));
	save_item(NAME(m_ata_selected));
	save_item(NAME(m_enable));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_ide64_cartridge_device::device_reset()
{
	m_bank = 0;
	m_ata_selected = 0;

	m_enable = 1;

	m_wp = m_jp1->read();
	m_game = 1;
	m_exrom = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_ide64_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_enable) return data;

	int rom_oe = 1, ram_oe = 1;

	if (!m_game && m_exrom && sphi2 && ba)
	{
		if (offset >= 0x1000 && offset < 0x8000)
		{
			ram_oe = 0;
		}
		else if (offset >= 0x8000 && offset < 0xc000)
		{
			rom_oe = 0;
		}
		else if (offset >= 0xc000 && offset < 0xd000)
		{
			ram_oe = 0;
		}
	}

	if (!roml || !romh)
	{
		rom_oe = 0;
	}

	if (!io1 && sphi2 && ba)
	{
		// 0x20-0x2f    IDE
		// 0x30-0x37    I/O
		// 0x5d-0x5e    FT245
		// 0x5f-0x5f    DS1302
		// 0x60-0xff    ROM

		uint8_t io1_offset = offset & 0xff;

		if (io1_offset >= 0x20 && io1_offset < 0x28)
		{
			m_ata_data = (m_ata_selected ? m_ata1 : m_ata0)->cs0_r(offset & 0x07);

			data = m_ata_data & 0xff;
		}
		else if (io1_offset >= 0x2e && io1_offset < 0x30)
		{
			m_ata_data = (m_ata_selected ? m_ata1 : m_ata0)->cs1_r(offset & 0x07);

			data = m_ata_data & 0xff;
		}
		else if (io1_offset == 0x31)
		{
			data = m_ata_data >> 8;
		}
		else if (io1_offset == 0x32)
		{
			/*

			    bit     description

			    0       EXROM
			    1       GAME
			    2       A14
			    3       A15
			    4       A16
			    5       v4.x
			    6
			    7

			*/

			data = 0x20 | (m_bank << 2) | (m_game << 1) | m_exrom;
		}
		else if (io1_offset == 0x5f)
		{
			m_rtc->sclk_w(0);

			data &= ~0x01;
			data |= m_rtc->io_r();

			m_rtc->sclk_w(1);
		}
		else if (io1_offset >= 0x60)
		{
			rom_oe = 0;
		}
	}

	if (!rom_oe)
	{
		offs_t addr = (m_bank << 14) | (offset & 0x3fff);

		data = m_flash_rom->read(addr);
	}
	else if (!ram_oe)
	{
		data = m_ram[offset & 0x7fff];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_ide64_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_enable) return;

	if (!m_game && m_exrom)
	{
		if (offset >= 0x1000 && offset < 0x8000)
		{
			m_ram[offset & 0x7fff] = data;
		}
		else if (offset >= 0xc000 && offset < 0xd000)
		{
			m_ram[offset & 0x7fff] = data;
		}
	}

	if ((offset >= 0x8000 && offset < 0xc000) && !m_wp)
	{
		offs_t addr = (m_bank << 14) | (offset & 0x3fff);
		m_flash_rom->write(addr, data);
	}

	if (!io1)
	{
		// 0x20-0x2f    IDE
		// 0x30-0x37    I/O
		// 0x5d-0x5e    FT245
		// 0x5f-0x5f    DS1302
		// 0x60-0xff    ROM

		uint8_t io1_offset = offset & 0xff;

		if (io1_offset >= 0x20 && io1_offset < 0x28)
		{
			m_ata_data = (m_ata_data & 0xff00) | data;

			(m_ata_selected ? m_ata1 : m_ata0)->cs0_w(offset & 0x07, m_ata_data);
		}
		else if (io1_offset >= 0x28 && io1_offset < 0x2a)
		{
			m_ata_selected = io1_offset & 0x01;
		}
		else if (io1_offset >= 0x2e && io1_offset < 0x30)
		{
			m_ata_data = (m_ata_data & 0xff00) | data;

			(m_ata_selected ? m_ata1 : m_ata0)->cs1_w(offset & 0x07, m_ata_data);
		}
		else if (io1_offset == 0x31)
		{
			m_ata_data = (data << 8) | (m_ata_data & 0xff);
		}
		else if (io1_offset == 0x5f)
		{
			m_rtc->sclk_w(0);

			m_rtc->io_w(BIT(data, 0));

			m_rtc->sclk_w(1);
		}
		else if (io1_offset >= 0x60 && io1_offset < 0x68)
		{
			m_bank = offset & 0x07;
		}
		else if (io1_offset == 0xfb)
		{
			/*

			    bit     description

			    0       disable cartridge
			    1       RTC CE
			    2
			    3
			    4
			    5
			    6
			    7

			*/

			m_enable = !BIT(data, 0);
			m_rtc->ce_w(BIT(data, 1));
		}
		else if (io1_offset >= 0xfc)
		{
			m_game = BIT(offset, 0);
			m_exrom = BIT(offset, 1);
		}
	}
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_ide64_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return (m_enable && sphi2 && ba) ? m_game : 1;
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_ide64_cartridge_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	return (m_enable && sphi2 && ba) ? m_exrom : 1;
}
