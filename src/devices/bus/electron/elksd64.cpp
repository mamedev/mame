// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ElkSD64 Electron SD Interface

    SD Card Interface & 64K RAM Expansion

    http://ramtop-retro.uk/elksd64.html

**********************************************************************/


#include "emu.h"
#include "elksd64.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_ELKSD64, electron_elksd64_device, "electron_elksd64", "ElkSD64 Electron SD Interface")


//-------------------------------------------------
//  ROM( elksd64 )
//-------------------------------------------------

ROM_START( elksd64 )
	ROM_REGION(0x20000, "flash", ROMREGION_ERASE00) // SST39SF010
	ROM_LOAD("zemmfs.bin", 0x0000, 0x4000, CRC(c3d38702) SHA1(8a1f0615ff93549815f4c8d21e35fb89535941dc))
	ROM_LOAD("ap6rom.rom", 0x4000, 0x4000, CRC(364591eb) SHA1(316a25aeeda0266dae510eea52324b087875740f))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_elksd64_device::device_add_mconfig(machine_config &config)
{
	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->spi_miso_callback().set([this](int state) { m_status = state << 7; });
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *electron_elksd64_device::device_rom_region() const
{
	return ROM_NAME( elksd64 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_elksd64_device - constructor
//-------------------------------------------------

electron_elksd64_device::electron_elksd64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_ELKSD64, tag, owner, clock)
	, device_electron_expansion_interface(mconfig, *this)
	, m_flash(*this, "flash")
	, m_sdcard(*this, "sdcard")
	, m_romsel(0)
	, m_status(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_elksd64_device::device_start()
{
	m_ram = make_unique_clear<uint8_t[]>(0x8000);

	save_item(NAME(m_romsel));
	save_item(NAME(m_status));
	save_pointer(NAME(m_ram), 0x8000);
}

//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_elksd64_device::expbus_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset >> 12)
	{
	case 0x8: case 0x9: case 0xa: case 0xb:
		switch (m_romsel)
		{
		case 12: case 13:
			data = m_ram[(m_romsel - 12) << 14 | (offset & 0x3fff)];
			break;

		case 14: case 15:
			data = m_flash->base()[(m_romsel - 14) << 14 | (offset & 0x3fff)];
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			if (offset == 0xfc72)
			{
				data = m_status;
			}
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_elksd64_device::expbus_w(offs_t offset, uint8_t data)
{
	switch (offset >> 12)
	{
	case 0x8: case 0x9: case 0xa: case 0xb:
		switch (m_romsel)
		{
		case 12: case 13:
			m_ram[(m_romsel - 12) << 14 | (offset & 0x3fff)] = data;
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			if (offset == 0xfc71)
			{
				m_sdcard->spi_ss_w(1);
				m_sdcard->spi_mosi_w(BIT(data, 0));
				m_sdcard->spi_clock_w(BIT(data, 1));
			}
			break;

		case 0xfe:
			if (offset == 0xfe05)
			{
				m_romsel = data & 0x0f;
			}
			break;
		}
	}
}
