// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    CMS High Resolution Colour Graphics Card

    Part No. CMS Video-4

**********************************************************************/


#include "emu.h"
#include "hires.h"
#include "screen.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CMS_HIRES, cms_hires_device, "cms_hires", "CMS High Resolution Colour Graphics Card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cms_hires_device::device_add_mconfig(machine_config &config)
{
	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_size(512, 312);
	m_screen->set_visarea(0, 512 - 1, 0, 256 - 1);
	m_screen->set_refresh_hz(50);
	m_screen->set_screen_update("ef9366", FUNC(ef9365_device::screen_update));
	PALETTE(config, "palette").set_entries(16);

	TIMER(config, "flash_rate").configure_periodic(FUNC(cms_hires_device::flash_rate), attotime::from_hz(3)); // from 555 timer (4.7uF, 100K, 470R)

	EF9365(config, m_gdp, 14_MHz_XTAL / 8);
	m_gdp->set_screen("screen");
	m_gdp->set_palette_tag("palette");
	m_gdp->set_nb_bitplanes(4);
	m_gdp->set_display_mode(ef9365_device::DISPLAY_MODE_512x256);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cms_hires_device - constructor
//-------------------------------------------------

cms_hires_device::cms_hires_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CMS_HIRES, tag, owner, clock)
	, device_acorn_bus_interface(mconfig, *this)
	, m_screen(*this, "screen")
	, m_gdp(*this, "ef9366")
	, m_flash_state(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cms_hires_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0xfc10, 0xfc1f, read8_delegate(FUNC(ef9365_device::data_r), m_gdp.target()), write8_delegate(FUNC(ef9365_device::data_w), m_gdp.target()));
	space.install_write_handler(0xfc20, 0xfc2f, write8_delegate(FUNC(cms_hires_device::colour_reg_w), this));

	save_item(NAME(m_flash_state));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cms_hires_device::device_reset()
{
	/* Generate hardware palette */
	for (int i=0; i<8; i++)
	{
		m_gdp->set_color_entry(i + 8, ((i & 1) ^ 1) * 255, (((i & 2) >> 1) ^ 1) * 255, (((i & 4) >> 2) ^ 1) * 255);
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

TIMER_DEVICE_CALLBACK_MEMBER(cms_hires_device::flash_rate)
{
	m_flash_state = !m_flash_state;

	/* Generate hardware flashing palette */
	for (int i=0; i<8; i++)
	{
		if (m_flash_state)
			m_gdp->set_color_entry(i, ((i & 1) ^ 1) * 255, (((i & 2) >> 1) ^ 1) * 255, (((i & 4) >> 2) ^ 1) * 255);
		else
			m_gdp->set_color_entry(i, 0, 0, 0);
	}
}

WRITE8_MEMBER(cms_hires_device::colour_reg_w)
{
	m_gdp->set_color_filler(data & 0x0f);
}
