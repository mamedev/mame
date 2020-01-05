// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

Hegener + Glaser Mephisto Display Module for modular chesscomputers,
the 2nd version with 2 LCD lines. The 16/32bit module also includes 8KB NVRAM,
but that part is emulated in the driver.

TODO:
- add mmdisplay1.cpp, the one with shift registers and 4-digit lcd

*********************************************************************/

#include "emu.h"

#include "mmdisplay2.h"
#include "sound/volt_reg.h"


DEFINE_DEVICE_TYPE(MEPHISTO_DISPLAY_MODULE2, mephisto_display_module2_device, "mdisplay2", "Mephisto Display Module 2")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

mephisto_display_module2_device::mephisto_display_module2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MEPHISTO_DISPLAY_MODULE2, tag, owner, clock)
	, m_lcdc(*this, "hd44780")
	, m_dac(*this, "dac")
{
}


//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void mephisto_display_module2_device::device_add_mconfig(machine_config &config)
{
	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_size(16*6, 9*2);
	screen.set_visarea(0, 16*6-1, 0, 9*2-3);
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");
	PALETTE(config, "palette", FUNC(mephisto_display_module2_device::lcd_palette), 2);

	HD44780(config, m_lcdc, 0);
	m_lcdc->set_lcd_size(2, 16);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_BINARY_WEIGHTED_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}

void mephisto_display_module2_device::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mephisto_display_module2_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_ctrl));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mephisto_display_module2_device::device_reset()
{
	m_latch = 0;
	m_ctrl = 0;
}


//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

WRITE8_MEMBER(mephisto_display_module2_device::latch_w)
{
	m_latch = data;
}

WRITE8_MEMBER(mephisto_display_module2_device::io_w)
{
	if (BIT(data, 1) && !BIT(m_ctrl, 1))
		m_lcdc->write(BIT(data, 0), m_latch);

	m_dac->write(data >> 2 & 3);

	m_ctrl = data;
}
