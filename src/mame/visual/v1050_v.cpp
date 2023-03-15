// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "v1050.h"

#include "screen.h"

/*

    TODO:

    - bright in reverse video

*/

#define V1050_ATTR_BRIGHT   0x01
#define V1050_ATTR_BLINKING 0x02
#define V1050_ATTR_ATTEN    0x04
#define V1050_ATTR_REVERSE  0x10
#define V1050_ATTR_BLANK    0x20
#define V1050_ATTR_BOLD     0x40
#define V1050_ATTR_BLINK    0x80

/* Video RAM Access */

uint8_t v1050_state::attr_r()
{
	return m_attr;
}

void v1050_state::attr_w(uint8_t data)
{
	m_attr = data;
}

uint8_t v1050_state::videoram_r(offs_t offset)
{
	if (offset >= 0x2000)
	{
		m_attr = (m_attr & 0xfc) | (m_attr_ram[offset] & 0x03);
	}

	return m_video_ram[offset];
}

void v1050_state::videoram_w(offs_t offset, uint8_t data)
{
	m_video_ram[offset] = data;

	if (offset >= 0x2000 && BIT(m_attr, 2))
	{
		m_attr_ram[offset] = m_attr & 0x03;
	}
}

/* MC6845 Interface */

MC6845_UPDATE_ROW( v1050_state::crtc_update_row )
{
	for (int column = 0; column < x_count; column++)
	{
		uint16_t address = (((ra & 0x03) + 1) << 13) | ((ma & 0x1fff) + column);
		uint8_t data = m_video_ram[address & V1050_VIDEORAM_MASK];
		uint8_t attr = (m_attr & 0xfc) | (m_attr_ram[address] & 0x03);

		for (int bit = 0; bit < 8; bit++)
		{
			int x = (column * 8) + bit;
			int color = BIT(data, 7);

			/* blinking */
			if ((attr & V1050_ATTR_BLINKING) && !(attr & V1050_ATTR_BLINK)) color = 0;

			/* reverse video */
			color ^= BIT(attr, 4);

			/* bright */
			if (color && (!(attr & V1050_ATTR_BOLD) ^ (attr & V1050_ATTR_BRIGHT))) color = 2;

			/* display blank */
			if (attr & V1050_ATTR_BLANK) color = 0;

			bitmap.pix(vbp + y, hbp + x) = m_palette->pen(de ? color : 0);

			data <<= 1;
		}
	}
}

WRITE_LINE_MEMBER( v1050_state::crtc_vs_w )
{
	m_subcpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);

	set_interrupt(INT_VSYNC, state);
}

/* Video Start */

void v1050_state::video_start()
{
	/* register for state saving */
	save_item(NAME(m_attr));
}

/* Machine Drivers */

void v1050_state::v1050_video(machine_config &config)
{
	HD6845S(config, m_crtc, 15.36_MHz_XTAL/8); // HD6845SP according to Programmer's Technical Document
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(v1050_state::crtc_update_row));
	m_crtc->out_vsync_callback().set(FUNC(v1050_state::crtc_vs_w));

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_screen_update(H46505_TAG, FUNC(hd6845s_device::screen_update));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(640, 400);
	screen.set_visarea(0,640-1, 0, 400-1);

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);
}
