// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    MasterSD BBC Master SD Cartridge

    http://ramtop-retro.uk/mastersd.html

**********************************************************************/


#include "emu.h"
#include "mastersd.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_MASTERSD, bbc_mastersd_device, "bbc_mastersd", "MasterSD BBC Master SD Cartridge")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_mastersd_device::device_add_mconfig(machine_config &config)
{
	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->spi_miso_callback().set([this](int state) { m_in_bit = state; });
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_mastersd_device - constructor
//-------------------------------------------------

bbc_mastersd_device::bbc_mastersd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_MASTERSD, tag, owner, clock)
	, device_bbc_cart_interface(mconfig, *this)
	, m_sdcard(*this, "sdcard")
	, m_spi_clock_state(false)
	, m_spi_clock_sysclk(false)
	, m_spi_clock_cycles(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_mastersd_device::device_start()
{
	m_ram = make_unique_clear<uint8_t[]>(0x8000);

	m_spi_clock = timer_alloc(FUNC(bbc_mastersd_device::spi_clock), this);

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

void bbc_mastersd_device::device_reset()
{
	m_spi_clock->adjust(attotime::never);
	m_spi_clock_cycles = 0;
	m_spi_clock_state = false;
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t bbc_mastersd_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (oe)
	{
		if (offset >= 0x3600 || romqa == 1)
		{
			data = m_ram[(offset & 0x3fff) | (romqa << 14)];
		}
		else if (offset == 0x35fe) // SPI controller data port
		{
			data = m_in_latch;
		}
		else if (offset == 0x35ff) // SPI controller status register
		{
			data = m_spi_clock_cycles > 0 ? 0x01 : 0x00;
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

void bbc_mastersd_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (oe)
	{
		if (offset >= 0x3600 || romqa == 1)
		{
			m_ram[(offset & 0x3fff) | (romqa << 14)] = data;
		}
		else if (offset == 0x35fe) // SPI controller data port
		{
			m_out_latch = data;
			m_spi_clock_cycles = 8;

			if (m_spi_clock_sysclk) // TODO: confirm fast/slow clock and dividers
				m_spi_clock->adjust(attotime::from_hz(16_MHz_XTAL / 2), 0, attotime::from_hz(16_MHz_XTAL / 2));
			else
				m_spi_clock->adjust(attotime::from_hz(16_MHz_XTAL / 32), 0, attotime::from_hz(16_MHz_XTAL / 32));
		}
		else if (offset == 0x35ff) // SPI controller clock register
		{
			m_spi_clock_sysclk = bool(BIT(data, 0));
		}
	}
}


TIMER_CALLBACK_MEMBER(bbc_mastersd_device::spi_clock)
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
