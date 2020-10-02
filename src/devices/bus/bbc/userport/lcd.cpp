// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Sprow LCD Display

    http://www.sprow.co.uk/bbc/buildit.htm#Lcddisplay

**********************************************************************/


#include "emu.h"
#include "lcd.h"
#include "screen.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_LCD, bbc_lcd_device, "bbc_lcd", "Sprow LCD Display")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_lcd_device::device_add_mconfig(machine_config &config)
{
	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_LCD);
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(120, 36);
	screen.set_visarea_full();
	screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(bbc_lcd_device::lcd_palette), 3);

	HD44780(config, m_lcdc);
	m_lcdc->set_lcd_size(4, 20);
	m_lcdc->set_pixel_update_cb(FUNC(bbc_lcd_device::lcd_pixel_update));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_lcd_device - constructor
//-------------------------------------------------

bbc_lcd_device::bbc_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_LCD, tag, owner, clock)
	, device_bbc_userport_interface(mconfig, *this)
	, m_lcdc(*this, "lcdc")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_lcd_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

HD44780_PIXEL_UPDATE(bbc_lcd_device::lcd_pixel_update)
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (pos < 40)
	{
		if (pos >= 20)
		{
			line += 2;
			pos -= 20;
		}

		if (line < 4)
			bitmap.pix(line * (8 + 1) + y, pos * 6 + x) = state ? 1 : 2;
	}
}

void bbc_lcd_device::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t( 92,  83,  88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

uint8_t bbc_lcd_device::pb_r()
{
	return m_lcdc->db_r() >> 1;
}

void bbc_lcd_device::pb_w(uint8_t data)
{
	m_lcdc->rs_w(BIT(data, 0));
	m_lcdc->rw_w(BIT(data, 1));
	m_lcdc->e_w(BIT(data, 2));
	m_lcdc->db_w(data << 1);
}
