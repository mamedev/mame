// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*****************************************************************************
 *
 * video/abc800.c
 *
 ****************************************************************************/

#include "includes/abc80x.h"



// these are needed because the MC6845 emulation does
// not position the active display area correctly
#define HORIZONTAL_PORCH_HACK   115
#define VERTICAL_PORCH_HACK     29



//**************************************************************************
//  HIGH RESOLUTION GRAPHICS
//**************************************************************************

//-------------------------------------------------
//  hrs_w - high resolution scanline write
//-------------------------------------------------

WRITE8_MEMBER( abc800_state::hrs_w )
{
	m_hrs = data;
}


//-------------------------------------------------
//  hrc_w - high resolution color write
//-------------------------------------------------

WRITE8_MEMBER( abc800_state::hrc_w )
{
	m_fgctl = data;
}



//**************************************************************************
//  ABC 800 COLOR
//**************************************************************************

//-------------------------------------------------
//  translate_trom_offset -
//-------------------------------------------------

offs_t abc800c_state::translate_trom_offset(offs_t offset)
{
	int row = offset / 40;
	int col = offset % 40;

	offset = ((row & 0x07) * 0x80) + col;

	if (row & 0x08) offset += 0x28;
	if (row & 0x10) offset += 0x50;

	return offset;
}


//-------------------------------------------------
//  hr_update - high resolution screen update
//-------------------------------------------------

void abc800c_state::hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();

	UINT16 addr = 0;

	for (int y = m_hrs; y < MIN(cliprect.max_y + 1, m_hrs + 480); y += 2)
	{
		int x = 0;

		for (int sx = 0; sx < 64; sx++)
		{
			UINT8 data = m_video_ram[addr++];

			for (int dot = 0; dot < 4; dot++)
			{
				UINT16 fgctl_addr = ((m_fgctl & 0x7f) << 2) | ((data >> 6) & 0x03);
				UINT8 fgctl = m_fgctl_prom->base()[fgctl_addr];
				int color = fgctl & 0x07;

				if (color)
				{
					bool black = bitmap.pix32(y, x) == rgb_t::black;
					bool opaque = !BIT(fgctl, 3);

					if (black || opaque)
					{
						bitmap.pix32(y, x) = pen[color];
						bitmap.pix32(y, x + 1) = pen[color];

						bitmap.pix32(y + 1, x) = pen[color];
						bitmap.pix32(y + 1, x + 1) = pen[color];
					}
				}

				data <<= 2;
				x += 2;
			}
		}
	}
}


void abc800_state::video_start()
{
	// register for state saving
	save_item(NAME(m_hrs));
	save_item(NAME(m_fgctl));
}


//-------------------------------------------------
//  SCREEN_UPDATE( abc800c )
//-------------------------------------------------

UINT32 abc800c_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// clear screen
	bitmap.fill(rgb_t::black, cliprect);

	// draw text
	if (!BIT(m_fgctl, 7))
	{
		m_trom->screen_update(screen, bitmap, cliprect);
	}

	// draw HR graphics
	hr_update(bitmap, cliprect);

	return 0;
}


//-------------------------------------------------
//  SAA5050_INTERFACE( trom_intf )
//-------------------------------------------------

READ8_MEMBER( abc800c_state::char_ram_r )
{
	return m_char_ram[translate_trom_offset(offset)];
}


//-------------------------------------------------
//  PALETTE_INIT( abc800c )
//-------------------------------------------------

PALETTE_INIT_MEMBER( abc800c_state, abc800c )
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
//  MACHINE_CONFIG_FRAGMENT( abc800c_video )
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( abc800c_video )
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(abc800c_state, screen_update)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(480, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 480-1, 0, 480-1)

	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INIT_OWNER(abc800c_state, abc800c)

	MCFG_DEVICE_ADD(SAA5052_TAG, SAA5052, XTAL_12MHz/2)
	MCFG_SAA5050_D_CALLBACK(READ8(abc800c_state, char_ram_r))
	MCFG_SAA5050_SCREEN_SIZE(40, 24, 40)
MACHINE_CONFIG_END



//**************************************************************************
//  ABC 800 MONOCHROME
//**************************************************************************

//-------------------------------------------------
//  hr_update - high resolution screen update
//-------------------------------------------------

void abc800m_state::hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT16 addr = 0;

	const pen_t *pen = m_palette->pens();

	for (int y = m_hrs + VERTICAL_PORCH_HACK; y < MIN(cliprect.max_y + 1, m_hrs + VERTICAL_PORCH_HACK + 240); y++)
	{
		int x = HORIZONTAL_PORCH_HACK;

		for (int sx = 0; sx < 64; sx++)
		{
			UINT8 data = m_video_ram[addr++];

			for (int dot = 0; dot < 4; dot++)
			{
				UINT16 fgctl_addr = ((m_fgctl & 0x7f) << 2) | ((data >> 6) & 0x03);
				int color = (m_fgctl_prom->base()[fgctl_addr] & 0x07) ? 1 : 0;

				bitmap.pix32(y, x++) = pen[color];
				bitmap.pix32(y, x++) = pen[color];

				data <<= 2;
			}
		}
	}
}


//-------------------------------------------------
//  MC6845_UPDATE_ROW( abc800m_update_row )
//-------------------------------------------------

MC6845_UPDATE_ROW( abc800m_state::abc800m_update_row )
{
	int column;
	rgb_t fgpen = m_palette->pen(1);

	y += vbp;

	for (column = 0; column < x_count; column++)
	{
		int bit;

		UINT16 address = (m_char_ram[(ma + column) & 0x7ff] << 4) | (ra & 0x0f);
		UINT8 data = (m_char_rom->base()[address & 0x7ff] & 0x3f);

		if (column == cursor_x)
		{
			data = 0x3f;
		}

		data <<= 2;

		for (bit = 0; bit < ABC800_CHAR_WIDTH; bit++)
		{
			int x = hbp + (column * ABC800_CHAR_WIDTH) + bit;

			if (BIT(data, 7) && de)
			{
				bitmap.pix32(y, x) = fgpen;
			}

			data <<= 1;
		}
	}
}


//-------------------------------------------------
//  SCREEN_UPDATE( abc800m )
//-------------------------------------------------

UINT32 abc800m_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// clear screen
	bitmap.fill(rgb_t::black, cliprect);

	// draw HR graphics
	hr_update(bitmap, cliprect);

	// draw text
	if (!BIT(m_fgctl, 7))
	{
		m_crtc->screen_update(screen, bitmap, cliprect);
	}

	return 0;
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( abc800m_video )
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( abc800m_video )
	MCFG_MC6845_ADD(MC6845_TAG, MC6845, SCREEN_TAG, ABC800_CCLK)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(ABC800_CHAR_WIDTH)
	MCFG_MC6845_UPDATE_ROW_CB(abc800m_state, abc800m_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(DEVWRITELINE(Z80DART_TAG, z80dart_device, rib_w))

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(abc800m_state, screen_update)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(768, 312)
	MCFG_SCREEN_VISIBLE_AREA(0,768-1, 0, 312-1)

	MCFG_PALETTE_ADD_MONOCHROME_YELLOW("palette")
MACHINE_CONFIG_END
