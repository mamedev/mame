// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*******************************************************************************

Hegener + Glaser Mephisto Display Module for modular chesscomputers, the 2nd
version with 2 LCD lines (HD44780A00 + HD44100H or Sanyo LC7930). The 16/32bit
module also includes 8KB NVRAM, but that part is emulated in the driver.

*******************************************************************************/

#include "emu.h"
#include "mmdisplay2.h"

#include "screen.h"
#include "speaker.h"


DEFINE_DEVICE_TYPE(MEPHISTO_DISPLAY_MODULE2, mephisto_display2_device, "mdisplay2", "Mephisto Display Module 2")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

mephisto_display2_device::mephisto_display2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MEPHISTO_DISPLAY_MODULE2, tag, owner, clock)
	, m_lcd(*this, "hd44780")
	, m_dac(*this, "dac")
{
}


//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void mephisto_display2_device::device_add_mconfig(machine_config &config)
{
	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60); // arbitrary
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(6*16+1, 9*2+1);
	screen.set_visarea_full();
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(mephisto_display2_device::lcd_palette), 3);

	HD44780(config, m_lcd, 270'000); // OSC = 91K resistor
	m_lcd->set_lcd_size(2, 16);
	m_lcd->set_pixel_update_cb(FUNC(mephisto_display2_device::lcd_pixel_update));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.125);
}

void mephisto_display2_device::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0xff, 0xff, 0xff)); // background
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0x00)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(0xe8, 0xe8, 0xe8)); // lcd pixel off
}

HD44780_PIXEL_UPDATE(mephisto_display2_device::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line*9 + 1 + y, 1 + pos*6 + x) = state ? 1 : 2;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mephisto_display2_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_ctrl));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mephisto_display2_device::device_reset()
{
	m_latch = 0;
	m_ctrl = 0;
}


//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

void mephisto_display2_device::latch_w(u8 data)
{
	m_latch = data;
}

void mephisto_display2_device::io_w(u8 data)
{
	if (BIT(data, 1) && !BIT(m_ctrl, 1))
		m_lcd->write(BIT(data, 0), m_latch);

	m_dac->write(data >> 2 & 3);

	m_ctrl = data;
}
