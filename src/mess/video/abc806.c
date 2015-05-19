// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*****************************************************************************
 *
 * video/abc806.c
 *
 ****************************************************************************/

#include "includes/abc80x.h"



#define HORIZONTAL_PORCH_HACK   109
#define VERTICAL_PORCH_HACK     27



//-------------------------------------------------
//  hrs_w - high resolution memory banking
//-------------------------------------------------

WRITE8_MEMBER( abc806_state::hrs_w )
{
	/*

	    bit     signal  description

	    0       VM14    visible screen memory area bit 0
	    1       VM15    visible screen memory area bit 1
	    2       VM16    visible screen memory area bit 2
	    3       VM17    visible screen memory area bit 3
	    4       F14     cpu accessible screen memory area bit 0
	    5       F15     cpu accessible screen memory area bit 1
	    6       F16     cpu accessible screen memory area bit 2
	    7       F17     cpu accessible screen memory area bit 3

	*/

	m_hrs = data;
}


//-------------------------------------------------
//  hrc_w - high resolution color write
//-------------------------------------------------

WRITE8_MEMBER( abc806_state::hrc_w )
{
	int reg = (offset >> 8) & 0x0f;

	m_hrc[reg] = data;
}


//-------------------------------------------------
//  charram_r - character RAM read
//-------------------------------------------------

READ8_MEMBER( abc806_state::charram_r )
{
	m_attr_data = m_attr_ram[offset];

	return m_char_ram[offset];
}


//-------------------------------------------------
//  charram_w - character RAM write
//-------------------------------------------------

WRITE8_MEMBER( abc806_state::charram_w )
{
	m_attr_ram[offset] = m_attr_data;

	m_char_ram[offset] = data;
}


//-------------------------------------------------
//  ami_r - attribute memory read
//-------------------------------------------------

READ8_MEMBER( abc806_state::ami_r )
{
	return m_attr_data;
}


//-------------------------------------------------
//  amo_w - attribute memory write
//-------------------------------------------------

WRITE8_MEMBER( abc806_state::amo_w )
{
	m_attr_data = data;
}


//-------------------------------------------------
//  cli_r - palette PROM read
//-------------------------------------------------

READ8_MEMBER( abc806_state::cli_r )
{
	/*

	    bit     description

	    0       HRU II data bit 0
	    1       HRU II data bit 1
	    2       HRU II data bit 2
	    3       HRU II data bit 3
	    4
	    5
	    6
	    7       RTC data output

	*/

	UINT16 hru2_addr = (m_hru2_a8 << 8) | (offset >> 8);
	UINT8 data = m_hru2_prom->base()[hru2_addr] & 0x0f;

	logerror("HRU II %03x : %01x\n", hru2_addr, data);

	data |= m_rtc->dio_r() << 7;

	return data;
}


//-------------------------------------------------
//  sti_r - protection device read
//-------------------------------------------------

READ8_MEMBER( abc806_state::sti_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7       PROT DOUT

	*/

	return 0x7f;
}


//-------------------------------------------------
//  sto_w -
//-------------------------------------------------

WRITE8_MEMBER( abc806_state::sto_w )
{
	int level = BIT(data, 7);

	switch (data & 0x07)
	{
	case 0:
		// external memory enable
		m_eme = level;
		break;
	case 1:
		// 40/80 column display
		m_40 = level;
		break;
	case 2:
		// HRU II address line 8, PROT A0
		m_hru2_a8 = level;
		break;
	case 3:
		// PROT INI
		break;
	case 4:
		// text display enable
		m_txoff = level;
		break;
	case 5:
		// RTC chip select
		m_rtc->cs_w(!level);
		break;
	case 6:
		// RTC clock
		m_rtc->clk_w(level);
		break;
	case 7:
		// RTC data in, PROT DIN
		m_rtc->dio_w(level);
		break;
	}
}


//-------------------------------------------------
//  sso_w - sync offset write
//-------------------------------------------------

WRITE8_MEMBER( abc806_state::sso_w )
{
	m_sync = data & 0x3f;
}


//-------------------------------------------------
//  MC6845_UPDATE_ROW( abc806_update_row )
//-------------------------------------------------

MC6845_UPDATE_ROW( abc806_state::abc806_update_row )
{
	const pen_t *pen = m_palette->pens();

	int fg_color = 7;
	int bg_color = 0;
	int underline = 0;
	int flash = 0;
	int e5 = m_40;
	int e6 = m_40;
	int th = 0;

	y += m_sync + vbp;

	for (int column = 0; column < x_count; column++)
	{
		UINT8 data = m_char_ram[(ma + column) & 0x7ff];
		UINT8 attr = m_attr_ram[(ma + column) & 0x7ff];
		UINT8 rad_data;

		if ((attr & 0x07) == ((attr >> 3) & 0x07))
		{
			// special case
			switch (attr >> 6)
			{
			case 0:
				// use previously selected attributes
				break;

			case 1:
				// reserved for future use
				break;

			case 2:
				// blank
				fg_color = 0;
				bg_color = 0;
				underline = 0;
				flash = 0;
				break;

			case 3:
				// double width
				e5 = BIT(attr, 0);
				e6 = BIT(attr, 1);

				// read attributes from next byte
				attr = m_attr_ram[(ma + column + 1) & 0x7ff];

				if (attr != 0x00)
				{
					fg_color = attr & 0x07;
					bg_color = (attr >> 3) & 0x07;
					underline = BIT(attr, 6);
					flash = BIT(attr, 7);
				}
				break;
			}
		}
		else
		{
			// normal case
			fg_color = attr & 0x07;
			bg_color = (attr >> 3) & 0x07;
			underline = BIT(attr, 6);
			flash = BIT(attr, 7);
			e5 = m_40;
			e6 = m_40;
		}

		if (column == cursor_x)
		{
			rad_data = 0x0f;
		}
		else
		{
			UINT16 rad_addr = (e6 << 8) | (e5 << 7) | (flash << 6) | (underline << 4) | (m_flshclk << 5) | (ra & 0x0f);
			rad_data = m_rad_prom->base()[rad_addr] & 0x0f;
		}

		UINT16 chargen_addr = (th << 12) | (data << 4) | rad_data;
		UINT8 chargen_data = m_char_rom->base()[chargen_addr & 0xfff] << 2;
		int x = hbp + (column + 4) * ABC800_CHAR_WIDTH;

		for (int bit = 0; bit < ABC800_CHAR_WIDTH; bit++)
		{
			int color = BIT(chargen_data, 7) ? fg_color : bg_color;
			if (!de) color = 0;

			bitmap.pix32(y, x++) = pen[color];

			if (e5 || e6)
			{
				bitmap.pix32(y, x++) = pen[color];
			}

			chargen_data <<= 1;
		}

		if (e5 || e6)
		{
			column++;
		}
	}
}


//-------------------------------------------------
//  hs_w - horizontal sync write
//-------------------------------------------------

WRITE_LINE_MEMBER( abc806_state::hs_w )
{
	int vsync;

	if (!state)
	{
		m_v50_addr++;

		// clock current vsync value into the shift register
		m_vsync_shift <<= 1;
		m_vsync_shift |= m_vsync;

		vsync = BIT(m_vsync_shift, m_sync);

		if (!m_d_vsync && vsync)
		{
			// clear V50 address
			m_v50_addr = 0;
		}
		else if (m_d_vsync && !vsync)
		{
			// flash clock
			if (m_flshclk_ctr & 0x20)
			{
				m_flshclk = !m_flshclk;
				m_flshclk_ctr = 0;
			}
			else
			{
				m_flshclk_ctr++;
			}
		}

		if (m_d_vsync != vsync)
		{
			// signal _DEW to DART
			m_dart->rib_w(!vsync);
		}

		m_d_vsync = vsync;
	}
}


//-------------------------------------------------
//  vs_w - vertical sync write
//-------------------------------------------------

WRITE_LINE_MEMBER( abc806_state::vs_w )
{
	m_vsync = state;
}


//-------------------------------------------------
//  hr_update - high resolution screen update
//-------------------------------------------------

void abc806_state::hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();

	UINT32 addr = (m_hrs & 0x0f) << 15;

	for (int y = m_sync + VERTICAL_PORCH_HACK; y < MIN(cliprect.max_y + 1, m_sync + VERTICAL_PORCH_HACK + 240); y++)
	{
		for (int sx = 0; sx < 128; sx++)
		{
			UINT8 data = m_video_ram[addr++];
			UINT16 dot = (m_hrc[data >> 4] << 8) | m_hrc[data & 0x0f];

			for (int pixel = 0; pixel < 4; pixel++)
			{
				int x = HORIZONTAL_PORCH_HACK + (ABC800_CHAR_WIDTH * 4) - 16 + (sx * 4) + pixel;

				if (BIT(dot, 15) || (bitmap.pix32(y, x) == rgb_t::black))
				{
					bitmap.pix32(y, x) = pen[(dot >> 12) & 0x07];
				}

				dot <<= 4;
			}
		}
	}
}


void abc806_state::video_start()
{
	// initialize variables
	for (int i = 0; i < 16; i++)
	{
		m_hrc[i] = 0;
	}

	m_sync = 10;
	m_d_vsync = 1;
	m_vsync = 1;
	m_40 = 1;

	// allocate memory
	m_char_ram.allocate(ABC806_CHAR_RAM_SIZE);
	m_attr_ram.allocate(ABC806_ATTR_RAM_SIZE);

	// register for state saving
	save_item(NAME(m_txoff));
	save_item(NAME(m_40));
	save_item(NAME(m_flshclk_ctr));
	save_item(NAME(m_flshclk));
	save_item(NAME(m_attr_data));
	save_item(NAME(m_hrs));
	save_item(NAME(m_hrc));
	save_item(NAME(m_sync));
	save_item(NAME(m_v50_addr));
	save_item(NAME(m_hru2_a8));
	save_item(NAME(m_vsync_shift));
	save_item(NAME(m_vsync));
	save_item(NAME(m_d_vsync));
}


//-------------------------------------------------
//  SCREEN_UPDATE( abc806 )
//-------------------------------------------------

UINT32 abc806_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// clear screen
	bitmap.fill(rgb_t::black, cliprect);

	if (!m_txoff)
	{
		// draw text
		m_crtc->screen_update(screen, bitmap, cliprect);
	}

	// draw HR graphics
	hr_update(bitmap, cliprect);

	return 0;
}


//-------------------------------------------------
//  PALETTE_INIT( abc806 )
//-------------------------------------------------

PALETTE_INIT_MEMBER( abc806_state, abc806 )
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00)); // black
	palette.set_pen_color(1, rgb_t(0xff, 0x00, 0x00)); // red
	palette.set_pen_color(2, rgb_t(0x00, 0xff, 0x00)); // green
	palette.set_pen_color(3, rgb_t(0xff, 0xff, 0x00)); // yellow
	palette.set_pen_color(4, rgb_t(0x00, 0x00, 0xff)); // blue
	palette.set_pen_color(5, rgb_t(0xff, 0x00, 0xff)); // magenta
	palette.set_pen_color(6, rgb_t(0x00, 0xff, 0xff)); // cyan
	palette.set_pen_color(7, rgb_t(0xff, 0xff, 0xff)); // white
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( abc806_video )
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( abc806_video )
	MCFG_MC6845_ADD(MC6845_TAG, MC6845, SCREEN_TAG, ABC800_CCLK)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(ABC800_CHAR_WIDTH)
	MCFG_MC6845_UPDATE_ROW_CB(abc806_state, abc806_update_row)
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(abc806_state, hs_w))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(abc806_state, vs_w))

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(abc806_state, screen_update)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(768, 312)
	MCFG_SCREEN_VISIBLE_AREA(0, 768-1, 0, 312-1)

	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INIT_OWNER(abc806_state, abc806)
MACHINE_CONFIG_END
