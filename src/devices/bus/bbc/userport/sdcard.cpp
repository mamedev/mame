// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Micro SD Card

    Original Interface Hardware
    ===========================

    This is the original interface design, as conceived by Martin Mather
    in 2006.

    User Port     SD Card
     (Master)     (Slave)
    =========     =======
      CB1/PB1 ==> S_CLK  (Clock)
          CB2 <== S_MISO (Dout)
          PB0 ==> S_MOSI (Din)
           0V ==> S_SEL  (Select)

    TurboMMC Interface Hardware
    ===========================

    The TurboMMC hardware is a more complicated, as it uses PB[4:2] to
    control three buffers, providing two different operating modes.

    When PB[4:2] are 010, then the hardware is configured as above, and
    the interface is compatible with the original hardware.

    When PB[4:2] are 101, then the hardware is re-configured as:

    User Port     SD Card
     (Master)     (Slave)
    =========     =======
      CB1/PB1 ==> S_CLK  (Clock)
          n/c <== S_MISO (Dout)
          CB2 ==> S_MOSI (Din)
           0V ==> S_SEL  (Select)

**********************************************************************/


#include "emu.h"
#include "sdcard.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_SDCARD, bbc_sdcard_device, "bbc_sdcard", "BBC Micro SD Card")
DEFINE_DEVICE_TYPE(BBC_SDCARDT, bbc_sdcardt_device, "bbc_sdcardt", "BBC Micro Turbo SD Card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_sdcard_device::device_add_mconfig(machine_config &config)
{
	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->spi_miso_callback().set([this](int state) { m_slot->cb2_w(state); });
}

void bbc_sdcardt_device::device_add_mconfig(machine_config &config)
{
	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->spi_miso_callback().set([this](int state) { if (!m_turbo) m_slot->cb2_w(state); });
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_sdcard_device - constructor
//-------------------------------------------------

bbc_sdcard_device::bbc_sdcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_userport_interface(mconfig, *this)
	, m_sdcard(*this, "sdcard")
{
}

bbc_sdcard_device::bbc_sdcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_sdcard_device(mconfig, BBC_SDCARD, tag, owner, clock)
{
}

bbc_sdcardt_device::bbc_sdcardt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_sdcard_device(mconfig, BBC_SDCARDT, tag, owner, clock)
	, m_turbo(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_sdcard_device::device_start()
{
}

void bbc_sdcardt_device::device_start()
{
	save_item(NAME(m_turbo));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_sdcard_device::write_cb1(int state)
{
	m_sdcard->spi_ss_w(1);
	m_sdcard->spi_clock_w(state);
}

void bbc_sdcard_device::pb_w(uint8_t data)
{
	m_sdcard->spi_ss_w(1);
	m_sdcard->spi_mosi_w(BIT(data, 0));
	m_sdcard->spi_clock_w(BIT(data, 1));

	m_slot->cb1_w(BIT(data, 1));
}


void bbc_sdcardt_device::write_cb1(int state)
{
	m_sdcard->spi_ss_w(1);
	m_sdcard->spi_clock_w(state);
}

void bbc_sdcardt_device::write_cb2(int state)
{
	if (m_turbo)
	{
		m_sdcard->spi_ss_w(1);
		m_sdcard->spi_mosi_w(state);
	}
}

void bbc_sdcardt_device::pb_w(uint8_t data)
{
	m_turbo = (data & 0x1c) == 0x14;

	m_sdcard->spi_ss_w(1);
	m_sdcard->spi_mosi_w(BIT(data, 0));
	m_sdcard->spi_clock_w(BIT(data, 1));

	m_slot->cb1_w(BIT(data, 1));
}
