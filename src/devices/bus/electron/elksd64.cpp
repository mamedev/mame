// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ElkSD64 Electron SD Interface

    SD Card Interface & 64K RAM Expansion

    http://ramtop-retro.uk/elksd64.html

**********************************************************************/


#include "emu.h"
#include "elksd64.h"

#include "machine/spi_sdcard.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_elksd64_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_elksd64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_electron_expansion_interface implementation
	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_flash;
	required_device<spi_sdcard_device> m_sdcard;

	uint8_t m_romsel;
	uint8_t m_status;

	std::unique_ptr<uint8_t[]> m_ram;
};


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
	m_sdcard->set_prefer_sdhc();
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
			if ((offset == 0xfe05) && !(data & 0xf0))
			{
				m_romsel = data & 0x0f;
			}
			break;
		}
	}
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ELECTRON_ELKSD64, device_electron_expansion_interface, electron_elksd64_device, "electron_elksd64", "ElkSD64 Electron SD Interface")
