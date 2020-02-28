// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*****************************************************************************
 *
 * video/abc800.c
 *
 ****************************************************************************/

#include "emu.h"
#include "includes/abc80x.h"
#include "screen.h"



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
//  hr_update - high resolution screen update
//-------------------------------------------------

void abc800c_state::hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();

	uint16_t addr = 0;

	for (int y = m_hrs; y < std::min(cliprect.max_y + 1, m_hrs + 480); y += 2)
	{
		int x = 0;

		for (int sx = 0; sx < 64; sx++)
		{
			uint8_t data = m_video_ram[addr++];

			for (int dot = 0; dot < 4; dot++)
			{
				uint16_t fgctl_addr = ((m_fgctl & 0x7f) << 2) | ((data >> 6) & 0x03);
				uint8_t fgctl = m_fgctl_prom->base()[fgctl_addr];
				int color = fgctl & 0x07;

				if (color)
				{
					bool black = bitmap.pix32(y, x) == rgb_t::black();
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


//-------------------------------------------------
//  SCREEN_UPDATE( abc800c )
//-------------------------------------------------

uint32_t abc800c_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// clear screen
	bitmap.fill(rgb_t::black(), cliprect);

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
	int row = offset / 40;
	int col = offset % 40;

	offset = ((row & 0x07) * 0x80) + col;

	if (row & 0x08) offset += 0x28;
	if (row & 0x10) offset += 0x50;

	return m_char_ram[offset];
}


//-------------------------------------------------
//  PALETTE_INIT( abc800c )
//-------------------------------------------------

void abc800c_state::abc800c_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t(0xff, 0x00, 0x00)); // red
	palette.set_pen_color(2, rgb_t::green());
	palette.set_pen_color(3, rgb_t(0xff, 0xff, 0x00)); // yellow
	palette.set_pen_color(4, rgb_t(0x00, 0x00, 0xff)); // blue
	palette.set_pen_color(5, rgb_t(0xff, 0x00, 0xff)); // magenta
	palette.set_pen_color(6, rgb_t(0x00, 0xff, 0xff)); // cyan
	palette.set_pen_color(7, rgb_t::white());
}


//-------------------------------------------------
//  machine_config( abc800c_video )
//-------------------------------------------------

void abc800c_state::abc800c_video(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(abc800c_state::screen_update));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(480, 480);
	screen.set_visarea(0, 480-1, 0, 480-1);

	PALETTE(config, m_palette, FUNC(abc800c_state::abc800c_palette), 8);

	SAA5052(config, m_trom, XTAL(12'000'000)/2);
	m_trom->d_cb().set(FUNC(abc800c_state::char_ram_r));
	m_trom->set_screen_size(40, 24, 40);
}



//**************************************************************************
//  ABC 800 MONOCHROME
//**************************************************************************

//-------------------------------------------------
//  hr_update - high resolution screen update
//-------------------------------------------------

void abc800m_state::hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t addr = 0;

	const pen_t *pen = m_palette->pens();

	for (int y = m_hrs + VERTICAL_PORCH_HACK; y < std::min(cliprect.max_y + 1, m_hrs + VERTICAL_PORCH_HACK + 240); y++)
	{
		int x = HORIZONTAL_PORCH_HACK;

		for (int sx = 0; sx < 64; sx++)
		{
			uint8_t data = m_video_ram[addr++];

			for (int dot = 0; dot < 4; dot++)
			{
				uint16_t fgctl_addr = ((m_fgctl & 0x7f) << 2) | ((data >> 6) & 0x03);
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

		uint16_t address = (m_char_ram[(ma + column) & 0x7ff] << 4) | (ra & 0x0f);
		uint8_t data = (m_char_rom->base()[address & 0x7ff] & 0x3f);

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

uint32_t abc800m_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// clear screen
	bitmap.fill(rgb_t::black(), cliprect);

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
//  machine_config( abc800m_video )
//-------------------------------------------------

void abc800m_state::abc800m_video(machine_config &config)
{
	mc6845_device &mc6845(MC6845(config, MC6845_TAG, ABC800_CCLK));
	mc6845.set_screen(SCREEN_TAG);
	mc6845.set_show_border_area(true);
	mc6845.set_char_width(ABC800_CHAR_WIDTH);
	mc6845.set_update_row_callback(FUNC(abc800m_state::abc800m_update_row));
	mc6845.out_vsync_callback().set(m_dart, FUNC(z80dart_device::rib_w)).invert();

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t(0xff, 0xff, 0x00)));
	screen.set_screen_update(FUNC(abc800m_state::screen_update));
	screen.set_raw(XTAL(12'000'000), 0x300, 0, 0x1e0, 0x13a, 0, 0xf0);

	PALETTE(config, m_palette, palette_device::MONOCHROME);
}
