// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "includes/v1050.h"

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

READ8_MEMBER( v1050_state::attr_r )
{
	return m_attr;
}

WRITE8_MEMBER( v1050_state::attr_w )
{
	m_attr = data;
}

READ8_MEMBER( v1050_state::videoram_r )
{
	if (offset >= 0x2000)
	{
		m_attr = (m_attr & 0xfc) | (m_attr_ram[offset] & 0x03);
	}

	return m_video_ram[offset];
}

WRITE8_MEMBER( v1050_state::videoram_w )
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
	int column, bit;

	for (column = 0; column < x_count; column++)
	{
		UINT16 address = (((ra & 0x03) + 1) << 13) | ((ma & 0x1fff) + column);
		UINT8 data = m_video_ram[address & V1050_VIDEORAM_MASK];
		UINT8 attr = (m_attr & 0xfc) | (m_attr_ram[address] & 0x03);

		for (bit = 0; bit < 8; bit++)
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

			bitmap.pix32(vbp + y, hbp + x) = m_palette->pen(de ? color : 0);

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
	/* allocate memory */
	m_attr_ram.allocate(V1050_VIDEORAM_SIZE);

	/* register for state saving */
	save_item(NAME(m_attr));
}

/* Machine Drivers */

MACHINE_CONFIG_FRAGMENT( v1050_video )
	MCFG_MC6845_ADD(H46505_TAG, H46505, SCREEN_TAG, XTAL_15_36MHz/8)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(v1050_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(v1050_state, crtc_vs_w))

	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, RASTER, rgb_t::green)
	MCFG_SCREEN_UPDATE_DEVICE(H46505_TAG, h46505_device, screen_update)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0,640-1, 0, 400-1)

	MCFG_PALETTE_ADD_MONOCHROME_HIGHLIGHT("palette")
MACHINE_CONFIG_END
