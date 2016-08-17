// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "includes/pc1512.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define LOG 0


static const rgb_t PALETTE_1512[] =
{
	rgb_t::black,
	rgb_t(0x00, 0x00, 0xaa),
	rgb_t(0x00, 0xaa, 0x00),
	rgb_t(0x00, 0xaa, 0xaa),
	rgb_t(0xaa, 0x00, 0x00),
	rgb_t(0xaa, 0x00, 0xaa),
	rgb_t(0xaa, 0x55, 0x00),
	rgb_t(0xaa, 0xaa, 0xaa),
	rgb_t(0x55, 0x55, 0x55),
	rgb_t(0x55, 0x55, 0xff),
	rgb_t(0x55, 0xff, 0x55),
	rgb_t(0x55, 0xff, 0xff),
	rgb_t(0xff, 0x55, 0x55),
	rgb_t(0xff, 0x55, 0xff),
	rgb_t(0xff, 0xff, 0x55),
	rgb_t::white
};

static const int PALETTE_0[] = { 0, 3, 5, 7 };
static const int PALETTE_1[] = { 0, 2, 4, 6 };
static const int PALETTE_2[] = { 0, 3, 4, 7 };


enum
{
	ALPHA_40 = 0,
	ALPHA_80,
	GRAPHICS_1,
	GRAPHICS_2
};


#define MODE_ALPHA_80       0x01
#define MODE_GRAPHICS       0x02
#define MODE_PALETTE_2      0x04
#define MODE_ENABLE_VIDEO   0x08
#define MODE_GRAPHICS_2     0x10
#define MODE_BLINK          0x20


#define COLOR_INTENSITY     0x10
#define COLOR_PALETTE_1     0x20


#define VFP_LORES           22
#define HFP_LORES           16



//**************************************************************************
//  VIDEO RAM ACCESS
//**************************************************************************

//-------------------------------------------------
//  video_ram_r -
//-------------------------------------------------

READ8_MEMBER( pc1512_state::video_ram_r )
{
	UINT8 data = 0;

	switch (get_display_mode(m_vdu_mode))
	{
	case ALPHA_40:
	case ALPHA_80:
		data = m_video_ram[offset];
		break;

	case GRAPHICS_1:
		data = m_video_ram[(offset << 2) | 3];
		break;

	case GRAPHICS_2:
		data = m_video_ram[(offset << 2) | (m_vdu_rdsel ^ 0x03)];
		break;
	}

	return data;
}


//-------------------------------------------------
//  video_ram_w -
//-------------------------------------------------

WRITE8_MEMBER( pc1512_state::video_ram_w )
{
	switch (get_display_mode(m_vdu_mode))
	{
	case ALPHA_40:
	case ALPHA_80:
		m_video_ram[offset] = data;
		break;

	case GRAPHICS_1:
		m_video_ram[(offset << 2) | 3] = data;
		m_video_ram[(offset << 2) | 2] = data;
		m_video_ram[(offset << 2) | 1] = data;
		m_video_ram[(offset << 2) | 0] = data;
		break;

	case GRAPHICS_2:
		if (BIT(m_vdu_plane, 0)) m_video_ram[(offset << 2) | 3] = data;
		if (BIT(m_vdu_plane, 1)) m_video_ram[(offset << 2) | 2] = data;
		if (BIT(m_vdu_plane, 2)) m_video_ram[(offset << 2) | 1] = data;
		if (BIT(m_vdu_plane, 3)) m_video_ram[(offset << 2) | 0] = data;
		break;
	}
}


//-------------------------------------------------
//  vdu_r -
//-------------------------------------------------

READ8_MEMBER( pc1512_state::vdu_r )
{
	UINT8 data = 0;

	switch (offset)
	{
	case 1: case 3: case 5: case 7:
		data = m_vdu->register_r(space, 0);
		break;

	case 0xa: // VDU Status
		/*

		    bit     description

		    0       Toggle Bit
		    1       Light-pen latch select
		    2       Light-pen switch off
		    3       Frame Flyback Time
		    4
		    5
		    6
		    7

		*/

		// toggle bit
		data |= m_toggle;
		m_toggle = !m_toggle;

		// light pen latch
		data |= m_lpen << 1;

		// light pen switch
		data |= 0x04;

		// vertical sync
		//data |= m_vdu->vsync_r();
		int flyback = 0;

		if (machine().first_screen()->vpos() < VFP_LORES - 16) flyback = 1;
		if (machine().first_screen()->vpos() > VFP_LORES + 200) flyback = 1;

		data |= flyback << 3;
		break;
	}

	return data;
}


//-------------------------------------------------
//  vdu_w -
//-------------------------------------------------

WRITE8_MEMBER( pc1512_state::vdu_w )
{
	switch (offset)
	{
	case 0: case 2: case 4: case 6:
		m_vdu->address_w(space, 0, data);
		break;

	case 1: case 3: case 5: case 7:
		m_vdu->register_w(space, 0, data);
		break;

	case 8: // VDU Mode Control
		/*

		    bit     description

		    0       Select Alpha 80 Char mode (de-select 40 Char mode)
		    1       Select Graphics modes (de-select Alpha modes)
		    2       Select Palette 2 (de-select palettes 0,1)
		    3       Enable Video Display
		    4       Select Graphics Mode 2 (de-select graphics mode 1)
		    5       Enable Blinking Chars (disable intensified backgrounds)
		    6
		    7

		*/

		if (LOG) logerror("VDU Mode Control %02x\n", data);

		if ((get_display_mode(m_vdu_mode) != GRAPHICS_2) && (get_display_mode(data) == GRAPHICS_2))
		{
			m_vdu_plane = 0x0f;
			m_vdu_border = 0;
		}

		if (get_display_mode(data) != GRAPHICS_2)
		{
			m_vdu_rdsel = 0;
		}

		if (get_display_mode(m_vdu_mode) != get_display_mode(data))
		{
			switch (get_display_mode(data))
			{
			case ALPHA_40:
			case GRAPHICS_1:
				m_vdu->set_hpixels_per_column(8);
				m_vdu->set_clock(XTAL_28_63636MHz/32);
				break;

			case ALPHA_80:
				m_vdu->set_hpixels_per_column(8);
				m_vdu->set_clock(XTAL_28_63636MHz/16);
				break;

			case GRAPHICS_2:
				m_vdu->set_hpixels_per_column(16);
				m_vdu->set_clock(XTAL_28_63636MHz/32);
				break;
			}
		}

		m_vdu_mode = data;
		break;

	case 9: // VDU Colour Select
		/*

		    bit     description

		    0
		    1
		    2
		    3
		    4
		    5
		    6
		    7

		*/

		if (LOG) logerror("VDU Colour Select %02x\n", data);

		m_vdu_color = data;
		break;

	case 0xb: // Clear Light Pen Latch
		if (LOG) logerror("VDU Clear Light Pen Latch\n");

		m_lpen = 0;
		break;

	case 0xc: // Set Light Pen Latch
		if (LOG) logerror("VDU Set Light Pen Latch\n");

		if (!m_lpen)
		{
			m_vdu->assert_light_pen_input();
		}

		m_lpen = 1;
		break;

	case 0xd: // VDU Colour Plane Write
		/*

		    bit     description

		    0       Allow CPU write to Blue Plane
		    1       Allow CPU write to Green Plane
		    2       Allow CPU write to Red Plane
		    3       Allow CPU write to Intensity Plane
		    4
		    5
		    6
		    7

		*/

		if (LOG) logerror("VDU Colour Plane Write %01x\n", data & 0x0f);

		if (get_display_mode(m_vdu_mode) == GRAPHICS_2)
		{
			m_vdu_plane = data;
		}
		break;

	case 0xe: // VDU Colour Plane Read
		/*

		    bit     description

		    0       Read Select bit 0 (RDSEL0)
		    1       Read Select bit 1 (RDSEL1)
		    2
		    3
		    4
		    5
		    6
		    7

		*/

		if (LOG) logerror("VDU Colour Plane Read %u\n", data & 0x03);

		if (get_display_mode(m_vdu_mode) == GRAPHICS_2)
		{
			m_vdu_rdsel = data & 0x03;
		}
		break;

	case 0xf: // VDU Graphics Mode 2 Border
		/*

		    bit     description

		    0       Border Blue
		    1       Border Green
		    2       Border Red
		    3       Border Intensity
		    4
		    5
		    6
		    7

		*/

		if (LOG) logerror("VDU Graphics Mode 2 Border %u\n", data & 0x0f);

		m_vdu_border = data;
		break;
	}
}


//-------------------------------------------------
//  mc6845
//-------------------------------------------------

int pc1512_state::get_display_mode(UINT8 mode)
{
	if (mode & MODE_GRAPHICS)
	{
		if (mode & MODE_GRAPHICS_2)
		{
			return GRAPHICS_2;
		}
		else
		{
			return GRAPHICS_1;
		}
	}
	else
	{
		if (mode & MODE_ALPHA_80)
		{
			return ALPHA_80;
		}
		else
		{
			return ALPHA_40;
		}
	}
}

offs_t pc1512_state::get_char_rom_offset()
{
	return ((m_lk->read() >> 5) & 0x03) << 11;
}

MC6845_UPDATE_ROW( pc1512_state::draw_alpha )
{
	offs_t char_rom_offset = get_char_rom_offset();
	UINT32 *p = &bitmap.pix32(y + vbp, hbp);

	if (get_display_mode(m_vdu_mode) == ALPHA_40)
		p = &bitmap.pix32(y + vbp, hbp);

	if (y > 199) return;

	for (int column = 0; column < x_count; column++)
	{
		UINT8 code = m_video_ram[(ma + column) << 1];
		UINT8 attr = m_video_ram[((ma + column) << 1) + 1];
		int fg = attr & 0x0f;
		int bg = attr >> 4;

		if (m_vdu_mode & MODE_BLINK)
		{
			bg &= 0x07;

			if (BIT(attr, 7) && !m_blink)
			{
				fg = bg;
			}
		}

		offs_t addr = char_rom_offset | (code << 3) | (ra & 0x07);
		UINT8 data = m_char_rom->base()[addr & 0x1fff];

		if ((column == cursor_x) && m_cursor)
		{
			data = 0xff;
		}

		for (int bit = 0; bit < 8; bit++)
		{
			int color = BIT(data, 7) ? fg : bg;

			*p = PALETTE_1512[de ? color : 0]; p++;

			data <<= 1;
		}
	}
}

int pc1512_state::get_color(UINT8 data)
{
	if (data == 0) return m_vdu_color & 0x0f;

	int color = PALETTE_0[data & 0x03];

	if (m_vdu_color & COLOR_PALETTE_1)
	{
		color = PALETTE_1[data & 0x03];
	}
	else if (m_vdu_mode & MODE_PALETTE_2)
	{
		color = PALETTE_2[data & 0x03];
	}

	if (m_vdu_color & COLOR_INTENSITY)
	{
		color += 8;
	}

	return color;
}

MC6845_UPDATE_ROW( pc1512_state::draw_graphics_1 )
{
	if (y > 199) return;

	UINT32 *p = &bitmap.pix32(y + vbp, hbp);

	for (int column = 0; column < x_count; column++)
	{
		offs_t offset = ((ra & 0x01) << 15) | ((ma + column) << 3);

		UINT16 b = (m_video_ram[offset | 3] << 8) | m_video_ram[offset | 7];

		for (int x = 0; x < 8; x++)
		{
			*p = PALETTE_1512[de ? get_color((BIT(b, 15) << 1) | BIT(b, 14)) : 0]; p++;
			b <<= 2;
		}
	}
}

MC6845_UPDATE_ROW( pc1512_state::draw_graphics_2 )
{
	if (y > 199) return;

	UINT32 *p = &bitmap.pix32(y + vbp, hbp);

	for (int column = 0; column < x_count; column++)
	{
		offs_t offset = ((ra & 0x01) << 15) | ((ma + column) << 3);

		UINT16 i = BIT(m_vdu_color, 3) ? ((m_video_ram[offset | 0] << 8) | m_video_ram[offset | 4]) : 0;
		UINT16 r = BIT(m_vdu_color, 2) ? ((m_video_ram[offset | 1] << 8) | m_video_ram[offset | 5]) : 0;
		UINT16 g = BIT(m_vdu_color, 1) ? ((m_video_ram[offset | 2] << 8) | m_video_ram[offset | 6]) : 0;
		UINT16 b = BIT(m_vdu_color, 0) ? ((m_video_ram[offset | 3] << 8) | m_video_ram[offset | 7]) : 0;

		for (int x = 0; x < 16; x++)
		{
			*p = PALETTE_1512[de ? (BIT(i, 15) << 3) | (BIT(r, 15) << 2) | (BIT(g, 15) << 1) | BIT(b, 15) : 0]; p++;
			i <<= 1; r <<= 1; g <<= 1; b <<= 1;
		}
	}
}

MC6845_UPDATE_ROW( pc1512_state::crtc_update_row )
{
	switch (get_display_mode(m_vdu_mode))
	{
	case ALPHA_40:
	case ALPHA_80:
		draw_alpha(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
		break;

	case GRAPHICS_1:
		draw_graphics_1(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
		break;

	case GRAPHICS_2:
		draw_graphics_2(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
		break;
	}
}


void pc1512_state::video_start()
{
	// allocate memory
	m_video_ram.allocate(0x10000);
}


UINT32 pc1512_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_vdu_mode & MODE_ENABLE_VIDEO)
	{
		m_blink_ctr++;

		if (m_blink_ctr == 0x08)
		{
			m_cursor = !m_cursor;
		}
		else if (m_blink_ctr == 0x10)
		{
			m_cursor = !m_cursor;
			m_blink = !m_blink;
			m_blink_ctr = 0;
		}

		switch (get_display_mode(m_vdu_mode))
		{
		case ALPHA_40:
		case GRAPHICS_1:
			screen.set_visible_area(0, 359, 0, 245);
			break;

		case ALPHA_80:
		case GRAPHICS_2:
			screen.set_visible_area(0, 831, 0, 245);
			break;
		}

		switch (get_display_mode(m_vdu_mode))
		{
		case ALPHA_40:
		case ALPHA_80:
		case GRAPHICS_1:
			bitmap.fill(PALETTE_1512[m_vdu_color & 0x0f], cliprect);
			break;

		case GRAPHICS_2:
			bitmap.fill(PALETTE_1512[m_vdu_border & 0x0f], cliprect);
			break;
		}

		m_vdu->screen_update(screen, bitmap, cliprect);
	}
	else
	{
		bitmap.fill(rgb_t::black, cliprect);
	}

	return 0;
}


//-------------------------------------------------
//  MACHINE_CONFIG( pc1512 )
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( pc1512_video )
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(pc1512_state, screen_update)
	MCFG_SCREEN_SIZE(80*8, 24*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 24*8-1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_REFRESH_RATE(50)

	MCFG_MC6845_ADD(AMS40041_TAG, AMS40041, SCREEN_TAG, XTAL_28_63636MHz/32)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(pc1512_state, crtc_update_row)
MACHINE_CONFIG_END
