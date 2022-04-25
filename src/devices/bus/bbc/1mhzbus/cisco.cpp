// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cisco Terminal

    This appears to be an information booth found in exhibitions and tourist
    information sites. No information or photos of the actual hardware are
    known. This romset was provided by the family of ex-Cisco developer
    F.Verrelli.

    This hardware is completely unknown, this device is derived from what the
    software requires to run.

    Notes:
    - ROM containing frame data, 1K per page. This set has 64K but the system
      could handle much more.
    - RAM used for storing messages, maybe 32K but could be less, and is likely
      battery-backed.
    - RTC is unknown, and doesn't seem to match any of those currently emulated.
    - Configuration data is unknown so has been derived, maybe stored in the RTC
      device? It determines the mapping of ROM/RAM with their lo/hi addresses.
    - A keyboard would be attached to the userport to limit the keys available
      to the user. This is not emulated as we don't know which keys to include.

    Special keys:
         @ - word search
    CTRL-C - customized frame, requires password ...


**********************************************************************/


#include "emu.h"
#include "cisco.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_CISCO, bbc_cisco_device, "bbc_cisco", "Cisco Terminal Data Board");


//-------------------------------------------------
//  ROM( cisco )
//-------------------------------------------------

ROM_START(cisco)
	ROM_REGION(0x10000, "rom", 0)
	ROM_SYSTEM_BIOS(0, "confex", "Confex '88")
	ROMX_LOAD("101.bin", 0x0000, 0x4000, CRC(36b42ed2) SHA1(bbbcef59e12da4cddc7a5cebf4c8597b0e76fbac), ROM_BIOS(0))
	ROMX_LOAD("102.bin", 0x4000, 0x4000, CRC(e323e6b1) SHA1(d3344a964c0a37b4dfbdd0d01c8f16a1b796197b), ROM_BIOS(0))
	ROMX_LOAD("103.bin", 0x8000, 0x4000, CRC(f8faaf74) SHA1(b11edeaa9eefd75bcfcb5e39bab01b74b7dbc45f), ROM_BIOS(0))
	ROMX_LOAD("104.bin", 0xc000, 0x4000, CRC(c66ae87c) SHA1(500339258bd5cad363709c774c0cee36a17986cb), ROM_BIOS(0))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_cisco_device::device_rom_region() const
{
	return ROM_NAME(cisco);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_cisco_device::device_add_mconfig(machine_config &config)
{
	// unknown RTC device
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_cisco_device - constructor
//-------------------------------------------------

bbc_cisco_device::bbc_cisco_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_CISCO, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, device_rtc_interface(mconfig , *this)
	, m_rom(*this, "rom")
	, m_rom_page(0)
	, m_rtc_minute(0)
	, m_rtc_hour(0)
	, m_rtc_day(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_cisco_device::device_start()
{
	m_ram = make_unique_clear<uint8_t[]>(0x8000); // unknown RAM, likely non-volatile, 32K is a guess
	m_cfg = make_unique_clear<uint8_t[]>(0x80);   // unknown RAM, likely non-volatile, maybe with RTC

	// default configuration data, unknown so has been derived from the software
	static const uint8_t config[0x80] =
	{
		0x00, 0x00, 0x01, // LOFRM frame ROM low address
		0xff, 0xff, 0x01, // HIFRM frame ROM high address
		0x00, 0x00, 0x00, // LOANSW
		0x00, 0x00, 0x00, // HIANSW
		0x00, 0x00, 0x00, // LODTBSE
		0x00, 0x00, 0x00, // HIDTBSE
		0x00, 0x00, 0x02, // message RAM low address
		0xff, 0x7f, 0x02, // message RAM high address
		0x00, 0x00, 0x00, // not used
		0x00, 0x00, 0x00, // not used
		0x00, 0x00, 0x00, // not used

		0x00, 0x08, // NORM page rotation delay
		0x00, 0x10, // SLOW page rotation delay

		0x1c, 0x00, // NFREE pages to rotate when free (32 max)
		0x01, 0x00, // FREE main index
		0x21, 0x00, // FREE cisco
		0x01, 0x00, // FREE
		0x26, 0x00, // FREE insurance services
		0x01, 0x00, // FREE
		0x27, 0x00, // FREE investment opportunities
		0x01, 0x00, // FREE
		0x28, 0x00, // FREE sport and leisure
		0x01, 0x00, // FREE
		0x29, 0x00, // FREE food and drink
		0x01, 0x00, // FREE
		0x2a, 0x00, // FREE marketing services
		0x01, 0x00, // FREE
		0x2b, 0x00, // FREE management services
		0x01, 0x00, // FREE
		0x2c, 0x00, // FREE office services
		0x01, 0x00, // FREE
		0x2d, 0x00, // FREE places to visit
		0x01, 0x00, // FREE
		0x2e, 0x00, // FREE publications
		0x01, 0x00, // FREE
		0x2f, 0x00, // FREE professional services
		0x01, 0x00, // FREE
		0x30, 0x00, // FREE professional bodies
		0x01, 0x00, // FREE
		0x31, 0x00, // FREE security
		0x01, 0x00, // FREE
		0x32, 0x00, // FREE travel
		0x00, 0x00, // FREE
		0x00, 0x00, // FREE
		0x00, 0x00, // FREE
		0x00, 0x00  // FREE
	};
	memcpy(m_cfg.get(), config, 0x80);

	save_pointer(NAME(m_ram), 0x8000);
	save_pointer(NAME(m_cfg), 0x80);
	save_item(NAME(m_rom_page));
}


void bbc_cisco_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_rtc_minute = convert_to_bcd(minute);
	m_rtc_hour   = convert_to_bcd(hour);
	m_rtc_day    = day;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_cisco_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0xe8: // unknown RTC
		data = BIT(m_rtc_minute, 0, 4);
		break;
	case 0xe9:
		data = BIT(m_rtc_minute, 4, 4);
		break;
	case 0xea:
		data = BIT(m_rtc_hour, 0, 4);
		break;
	case 0xeb:
		data = BIT(m_rtc_hour, 4, 4);
		break;
	case 0xfa:
		data = m_rtc_day;
		break;
	case 0xfc: // expects 0x55
		data = 0x55;
		break;
	default:
		logerror("fred_r: unknown %02x = %02x\n", offset, data);
		break;
	}

	return data;
}

void bbc_cisco_device::fred_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0xfc: // always writes 0x55
		break;
	case 0xfe: // JIM page MSB
		m_rom_page = (m_rom_page & 0x00ff) | (data << 8);
		break;
	case 0xff: // JIM page LSB
		m_rom_page = (m_rom_page & 0xff00) | (data << 0);
		break;
	default:
		logerror("fred_w: unknown %02x = %02x\n", offset, data);
		break;
	}
}


uint8_t bbc_cisco_device::jim_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_rom_page >> 8)
	{
	case 0:
		data = m_cfg[offset & 0x7f];
		break;
	case 1:
		data = m_rom[((m_rom_page << 8) & 0xff00) | offset];
		break;
	case 2:
		data = m_ram[((m_rom_page << 8) & 0xff00) | offset];
		break;
	default:
		logerror("jim_r: %d %06x = %02x\n", m_rom_page >> 8, (m_rom_page << 8) | offset, data);
		break;
	}

	return data;
}

void bbc_cisco_device::jim_w(offs_t offset, uint8_t data)
{
	switch (m_rom_page >> 8)
	{
	case 0:
		m_cfg[offset & 0x7f] = data;
		break;
	case 2:
		m_ram[((m_rom_page << 8) & 0xff00) | offset] = data;
		break;
	default:
		logerror("jim_w: %d %06x = %02x\n", m_rom_page >> 8, (m_rom_page << 8) | offset, data);
		break;
	}
}
