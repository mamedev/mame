// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ElkSD-Plus 1 Electron SD Cartridge

    http://ramtop-retro.uk/elksdp1.html

**********************************************************************/


#include "emu.h"
#include "elksdp1.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_ELKSDP1, electron_elksdp1_device, "electron_elksdp1", "ElkSD-Plus 1 Electron SD Cartridge")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_elksdp1_device::device_add_mconfig(machine_config &config)
{
	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->spi_miso_callback().set([this](int state) { m_in_bit = state; });
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_elksdp1_device - constructor
//-------------------------------------------------

electron_elksdp1_device::electron_elksdp1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_ELKSDP1, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
	, m_sdcard(*this, "sdcard")
	, m_spi_clock_state(false)
	, m_spi_clock_sysclk(false)
	, m_spi_clock_cycles(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_elksdp1_device::device_start()
{
	m_ram = make_unique_clear<uint8_t[]>(0x8000);

	m_spi_clock = timer_alloc(FUNC(electron_elksdp1_device::spi_clock), this);

	save_pointer(NAME(m_ram), 0x8000);
	save_item(NAME(m_spi_clock_state));
	save_item(NAME(m_spi_clock_sysclk));
	save_item(NAME(m_spi_clock_cycles));
	save_item(NAME(m_in_bit));
	save_item(NAME(m_in_latch));
	save_item(NAME(m_out_latch));
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------

void electron_elksdp1_device::device_reset()
{
	m_spi_clock->adjust(attotime::never);
	m_spi_clock_cycles = 0;
	m_spi_clock_state = false;
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_elksdp1_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (infc)
	{
		switch (offset)
			{
			case 0x80: // SPI controller data port
				data = m_in_latch;
				break;

			case 0x81: // SPI controller status register
				data = m_spi_clock_cycles > 0 ? 0x01 : 0x00;
				break;
			}
	}
	if (oe)
	{
		if (offset >= 0x3600 || romqa == 1)
		{
			data = m_ram[(offset & 0x3fff) | (romqa << 14)];
		}
		else
		{
			data = m_rom[offset & 0x3fff];
		}
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void electron_elksdp1_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (infc)
	{
		switch (offset)
			{
			case 0x80: // SPI controller data port
				m_out_latch = data;
				m_spi_clock_cycles = 8;

				if (m_spi_clock_sysclk) // TODO: confirm fast/slow clock dividers
					m_spi_clock->adjust(attotime::from_hz(16_MHz_XTAL / 2), 0, attotime::from_hz(16_MHz_XTAL / 2));
				else
					m_spi_clock->adjust(attotime::from_hz(16_MHz_XTAL / 32), 0, attotime::from_hz(16_MHz_XTAL / 32));
				break;

			case 0x81: // SPI controller clock register
				m_spi_clock_sysclk = bool(BIT(data, 0));
				break;
			}
	}
	if (oe)
	{
		if (offset >= 0x3600 || romqa == 1)
		{
			m_ram[(offset & 0x3fff) | (romqa << 14)] = data;
		}
	}
}


TIMER_CALLBACK_MEMBER(electron_elksdp1_device::spi_clock)
{
	if (m_spi_clock_cycles > 0)
	{
		m_sdcard->spi_ss_w(1);

		if (m_spi_clock_state)
		{
			m_in_latch <<= 1;
			m_in_latch &= ~0x01;
			m_in_latch |= m_in_bit;

			m_sdcard->spi_clock_w(1);

			m_spi_clock_cycles--;
		}
		else
		{
			m_sdcard->spi_mosi_w(BIT(m_out_latch, 7));
			m_sdcard->spi_clock_w(0);

			m_out_latch <<= 1;
		}

		m_spi_clock_state = !m_spi_clock_state;
	}
	else
	{
		m_spi_clock_state = false;
		m_spi_clock->adjust(attotime::never);
	}
}
