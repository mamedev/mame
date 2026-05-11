// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

Saitek Mephisto Display Module for modular chesscomputers, the 3rd version with
the Saitek GK 2000 LCD.

TODO:
- add output callback? I'm 100% sure there are no chesscomputers with dual LCD
  of this type

*******************************************************************************/

#include "emu.h"
#include "mdisplay3.h"

#include "screen.h"


DEFINE_DEVICE_TYPE(MEPHISTO_DISPLAY_MODULE3, mephisto_display3_device, "mdisplay3", "Mephisto 6. Generation LCD Modul")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

mephisto_display3_device::mephisto_display3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MEPHISTO_DISPLAY_MODULE3, tag, owner, clock),
	m_lcd_pwm(*this, "lcd_pwm"),
	m_out_lcd(*this, "s%u.%u", 0U, 0U)
{
}


//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

ROM_START( mdisplay3 )
	ROM_REGION( 68501, "screen", 0 )
	ROM_LOAD("gk2000.svg", 0, 68501, CRC(80554c49) SHA1(88f06ec8f403eaaf7cbce4cc84807b5742ce7108) )
ROM_END

const tiny_rom_entry *mephisto_display3_device::device_rom_region() const
{
	return ROM_NAME( mdisplay3 );
}


//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void mephisto_display3_device::device_add_mconfig(machine_config &config)
{
	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(2, 24);
	m_lcd_pwm->output_x().set(FUNC(mephisto_display3_device::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/5, 804/5);
	screen.set_visarea_full();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mephisto_display3_device::device_start()
{
	m_out_lcd.resolve();

	// zerofill
	m_clk = 0;
	m_data = 0;
	m_lcd_segs = 0;
	m_lcd_com = 0;

	// register for savestates
	save_item(NAME(m_clk));
	save_item(NAME(m_data));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));
}


//-------------------------------------------------
//  I/O handlers
//-------------------------------------------------

void mephisto_display3_device::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void mephisto_display3_device::update_lcd()
{
	const u32 lcd_segs = bitswap<24>(m_lcd_segs,13,0,18,21,3,14,9,8,15,2,20,22,4,17,10,11,16,5,23,19,1,12,7,6);

	for (int i = 0; i < 2; i++)
	{
		// LCD common is analog (voltage level)
		const u8 com = population_count_32(m_lcd_com >> (i * 2) & 3);
		const u32 data = (com == 0) ? lcd_segs : (com == 2) ? ~lcd_segs : 0;
		m_lcd_pwm->write_row(i, data);
	}
}

void mephisto_display3_device::clk_w(int state)
{
	state = state ? 1 : 0;

	// shift LCD segment data through several CD4015
	if (state && !m_clk)
		m_lcd_segs = m_lcd_segs << 6 | (m_data & 0x3f);

	m_clk = state;
	update_lcd();
}

void mephisto_display3_device::common_w(u8 data)
{
	m_lcd_com = data & 0xf;
	update_lcd();
}
