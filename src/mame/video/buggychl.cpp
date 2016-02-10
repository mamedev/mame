// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Nicola Salmoria
#include "emu.h"
#include "includes/buggychl.h"


PALETTE_INIT_MEMBER(buggychl_state, buggychl)
{
	/* arbitrary blue shading for the sky, estimation */
	for (int i = 0; i < 128; i++)
		palette.set_pen_color(i + 128, rgb_t(0, 240-i, 255));
}

void buggychl_state::video_start()
{
	m_screen->register_screen_bitmap(m_tmp_bitmap1);
	m_screen->register_screen_bitmap(m_tmp_bitmap2);

	save_item(NAME(m_tmp_bitmap1));
	save_item(NAME(m_tmp_bitmap2));

	m_gfxdecode->gfx(0)->set_source(m_charram);
}



WRITE8_MEMBER(buggychl_state::buggychl_chargen_w)
{
	if (m_charram[offset] != data)
	{
		m_charram[offset] = data;
		m_gfxdecode->gfx(0)->mark_dirty((offset / 8) & 0xff);
	}
}

WRITE8_MEMBER(buggychl_state::buggychl_sprite_lookup_bank_w)
{
	m_sl_bank = (data & 0x10) << 8;
}

WRITE8_MEMBER(buggychl_state::buggychl_sprite_lookup_w)
{
	m_sprite_lookup[offset + m_sl_bank] = data;
}

WRITE8_MEMBER(buggychl_state::buggychl_ctrl_w)
{
/*
    bit7 = lamp
    bit6 = lockout
    bit4 = OJMODE
    bit3 = SKY OFF
    bit2 = /SN3OFF
    bit1 = HINV
    bit0 = VINV
*/

	flip_screen_y_set(data & 0x01);
	flip_screen_x_set(data & 0x02);

	m_bg_on = data & 0x04;
	m_sky_on = data & 0x08;

	m_sprite_color_base = (data & 0x10) ? 1 * 16 : 3 * 16;

	machine().bookkeeping().coin_lockout_global_w((~data & 0x40) >> 6);
	output().set_led_value(0, ~data & 0x80);
}

WRITE8_MEMBER(buggychl_state::buggychl_bg_scrollx_w)
{
	m_bg_scrollx = -(data - 0x12);
}


void buggychl_state::draw_sky( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	for (int y = 0; y < 256; y++)
		for (int x = 0; x < 256; x++)
			bitmap.pix16(y, x) = 128 + x / 2;
}


void buggychl_state::draw_bg( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;
	int scroll[256];

	/* prevent wraparound */
	rectangle clip = cliprect;
	if (flip_screen_x()) clip.min_x += 8*8;
	else clip.max_x -= 8*8;

	for (offs = 0; offs < 0x400; offs++)
	{
		int code = m_videoram[0x400 + offs];

		int sx = offs % 32;
		int sy = offs / 32;

		if (flip_screen_x())
			sx = 31 - sx;
		if (flip_screen_y())
			sy = 31 - sy;

		m_gfxdecode->gfx(0)->opaque(m_tmp_bitmap1,m_tmp_bitmap1.cliprect(),
				code,
				2,
				flip_screen_x(),flip_screen_y(),
				8*sx,8*sy);
	}

	/* first copy to a temp bitmap doing column scroll */
	for (offs = 0; offs < 256; offs++)
		scroll[offs] = -m_scrollv[offs / 8];

	copyscrollbitmap(m_tmp_bitmap2, m_tmp_bitmap1, 1, &m_bg_scrollx, 256, scroll, m_tmp_bitmap2.cliprect());

	/* then copy to the screen doing row scroll */
	for (offs = 0; offs < 256; offs++)
		scroll[offs] = -m_scrollh[offs];

	copyscrollbitmap_trans(bitmap, m_tmp_bitmap2, 256, scroll, 0, nullptr, clip, 32);
}


void buggychl_state::draw_fg( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0; offs < 0x400; offs++)
	{
		int sx = offs % 32;
		int sy = offs / 32;
		int flipx = flip_screen_x();
		int flipy = flip_screen_y();

		int code = m_videoram[offs];

		if (flipx)
			sx = 31 - sx;
		if (flipy)
			sy = 31 - sy;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				0,
				flipx,flipy,
				8*sx,8*sy,
				0);
	}
}


void buggychl_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;
	const UINT8 *gfx;

	g_profiler.start(PROFILER_USER1);

	gfx = memregion("gfx2")->base();
	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int sx, sy, flipy, zoom, ch, x, px, y;
		const UINT8 *lookup;
		const UINT8 *zoomx_rom, *zoomy_rom;

		sx = m_spriteram[offs + 3] - ((m_spriteram[offs + 2] & 0x80) << 1);
		sy = 256 - 64 - m_spriteram[offs] + ((m_spriteram[offs + 1] & 0x80) << 1);
		flipy = m_spriteram[offs + 1] & 0x40;
		zoom = m_spriteram[offs + 1] & 0x3f;
		zoomy_rom = gfx + (zoom << 6);
		zoomx_rom = gfx + 0x2000 + (zoom << 3);

		lookup = m_sprite_lookup + ((m_spriteram[offs + 2] & 0x7f) << 6);

		for (y = 0; y < 64; y++)
		{
			int dy = flip_screen_y() ? (255 - sy - y) : (sy + y);

			if ((dy & ~0xff) == 0)
			{
				int charline, base_pos;

				charline = zoomy_rom[y] & 0x07;
				base_pos = zoomy_rom[y] & 0x38;
				if (flipy)
					base_pos ^= 0x38;

				px = 0;
				for (ch = 0; ch < 4; ch++)
				{
					int pos, code, realflipy;
					const UINT8 *pendata;

					pos = base_pos + 2 * ch;
					code = 8 * (lookup[pos] | ((lookup[pos + 1] & 0x07) << 8));
					realflipy = (lookup[pos + 1] & 0x80) ? !flipy : flipy;
					code += (realflipy ? (charline ^ 7) : charline);
					pendata = m_gfxdecode->gfx(1)->get_data(code);

					for (x = 0; x < 16; x++)
					{
						int col = pendata[x];
						if (col)
						{
							int dx = flip_screen_x() ? (255 - sx - px) : (sx + px);
							if ((dx & ~0xff) == 0)
								bitmap.pix16(dy, dx) = m_sprite_color_base + col;
						}

						/* the following line is almost certainly wrong */
						if (zoomx_rom[7 - (2 * ch + x / 8)] & (1 << (x & 7)))
							px++;
					}
				}
			}
		}
	}

	g_profiler.stop();
}


UINT32 buggychl_state::screen_update_buggychl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_sky_on)
		draw_sky(bitmap, cliprect);
	else
		bitmap.fill(0, cliprect);

	if (m_bg_on)
		draw_bg(bitmap, cliprect);

	draw_sprites(bitmap, cliprect);

	draw_fg(bitmap, cliprect);

	return 0;
}
