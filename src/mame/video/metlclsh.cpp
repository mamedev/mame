// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                              -= Metal Clash =-

    Sprites:
        16x16 tiles with 8 pens and 2 color codes,
        each sprite can be 1 or 2 tiles tall

    Background:
        size is 512x256 (scrolling, peculiar memory layout),
        16x16 tiles with 8 pens and no color code,
        1 byte per tile (tiles are banked)

    Foreground:
        size is 256x256 (no scrolling),
        8x8 tiles with 4 pens and 4 color codes,
        2 bytes per tile, low and high per tile priority

***************************************************************************/

#include "emu.h"
#include "includes/metlclsh.h"


WRITE8_MEMBER(metlclsh_state::metlclsh_rambank_w)
{
	if (data & 1)
	{
		m_write_mask = 0;
		membank("bank1")->set_base(m_bgram);
	}
	else
	{
		m_write_mask = 1 << (data >> 1);
		membank("bank1")->set_base(m_otherram.get());
	}
}

WRITE8_MEMBER(metlclsh_state::metlclsh_gfxbank_w)
{
	if (!(data & 4) && (m_gfxbank != data))
	{
		m_bg_tilemap->mark_all_dirty();
		m_gfxbank = data & 3;
	}
}

/***************************************************************************

                            Background tilemap

                memory offset of each tile of the tilemap:

                    00 08  ..  78 100 108  ..  178
                     .  .       .   .   .        .
                     .  .       .   .   .        .
                    07 0f  ..  7f 107 10f  ..  17f
                    80 88  ..  f8 180 188  ..  1f8
                     .  .       .   .   .        .
                     .  .       .   .   .        .
                    87 8f  ..  ff 187 18f  ..  1ff

***************************************************************************/

TILEMAP_MAPPER_MEMBER(metlclsh_state::metlclsh_bgtilemap_scan)
{
	return  (row & 7) + ((row & ~7) << 4) + ((col & 0xf) << 3) + ((col & ~0xf) << 4);
}

TILE_GET_INFO_MEMBER(metlclsh_state::get_bg_tile_info)
{
	SET_TILE_INFO_MEMBER(1, m_bgram[tile_index] + (m_gfxbank << 7), 0, 0);
}

WRITE8_MEMBER(metlclsh_state::metlclsh_bgram_w)
{
	/*  This ram is banked: it's either the tilemap (e401 = 1)
	    or bit n of another area (e401 = n << 1)? (that I don't understand) */
	if (m_write_mask)
	{
		/* unknown area - the following is almost surely wrong */
// 405b (e401 = e c a 8 6 4 2 0) writes d400++
// 4085 (e401 = e c a 8 6 4 2 0) writes d400++
// 4085 (e401 = e a 6 2) writes d000++
// 405b (e401 = e a 6 2) writes d000++

//      m_otherram[offset] |= (data & m_write_mask);
		m_otherram[offset] = (m_otherram[offset] & ~m_write_mask) | (data & m_write_mask);
	}
	else
	{
		/* tilemap */
		m_bgram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset & 0x1ff);
	}
}

/***************************************************************************

                            Foreground tilemap

    Offset:     Bits:           Value:

        0x000                   Code
        0x400   7--- ----       Priority (0/1 -> over/below sprites and background)
                -65- ----       Color
                ---- --10       Code

***************************************************************************/

TILE_GET_INFO_MEMBER(metlclsh_state::get_fg_tile_info)
{
	UINT8 code = m_fgram[tile_index + 0x000];
	UINT8 attr = m_fgram[tile_index + 0x400];
	SET_TILE_INFO_MEMBER(2, code + ((attr & 0x03) << 8), (attr >> 5) & 3, 0);
	tileinfo.category = ((attr & 0x80) ? 1 : 2);
}

WRITE8_MEMBER(metlclsh_state::metlclsh_fgram_w)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}


/***************************************************************************

                            Video Hardware Init

***************************************************************************/

void metlclsh_state::video_start()
{
	m_otherram = std::make_unique<UINT8[]>(0x800); // banked ram

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(metlclsh_state::get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(metlclsh_state::metlclsh_bgtilemap_scan),this), 16, 16, 32, 16);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(metlclsh_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);

	save_pointer(NAME(m_otherram.get()), 0x800);
}


/***************************************************************************

                                Sprites Drawing

    Offset:     Bits:           Value:

        0       7--- ----       0
                -65- ----       Code (high bits)
                ---4 ----       Double height (2 tiles)
                ---- 3---       Color
                ---- -2--       Flip X
                ---- --1-       Flip Y
                ---- ---0       Enable

        1                       Code (low bits)
        2                       Y (bottom -> top)
        3                       X (right -> left)

***************************************************************************/

void metlclsh_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int offs;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int attr, code, color, sx, sy, flipx, flipy, wrapy, sizey;

		attr = spriteram[offs];
		if (!(attr & 0x01))
			continue;   // enable

		flipy = (attr & 0x02);
		flipx = (attr & 0x04);
		color = (attr & 0x08) >> 3;
		sizey = (attr & 0x10);  // double height
		code = ((attr & 0x60) << 3) + spriteram[offs + 1];

		sx = 240 - spriteram[offs + 3];
		if (sx < -7)
			sx += 256;

		sy = 240 - spriteram[offs + 2];

		if (flip_screen())
		{
			sx = 240 - sx;  flipx = !flipx;
			sy = 240 - sy;  flipy = !flipy;     if (sizey)  sy += 16;
			if (sy > 240)   sy -= 256;
		}

		/* Draw twice, at sy and sy + 256 (wrap around) */
		for (wrapy = 0; wrapy <= 256; wrapy += 256)
		{
			if (sizey)
			{
				gfx->transpen(bitmap,cliprect, code & ~1, color, flipx,flipy,
						sx, sy + (flipy ? 0 : -16) + wrapy,0);

				gfx->transpen(bitmap,cliprect, code |  1, color, flipx,flipy,
						sx,sy + (flipy ? -16 : 0) + wrapy,0);
			}
			else
			{
				gfx->transpen(bitmap,cliprect, code, color, flipx,flipy,
						sx,sy + wrapy,0);
			}
		}
	}
}

/***************************************************************************

                                Screen Drawing

    Video register e402 (metlclsh seems to only use the values 0,8,9,b):

        7654 ----       0
        ---- 3---       Background enable
        ---- -2--       0
        ---- --1-       Background scroll x high bit
        ---- ---0       ? (not bg flipx!)

***************************************************************************/

UINT32 metlclsh_state::screen_update_metlclsh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x10, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 1, 0); // low priority tiles of foreground

	if (m_scrollx[0] & 0x08)                    // background (if enabled)
	{
		/* The background seems to be always flipped along x */
		m_bg_tilemap->set_flip((flip_screen() ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0) ^ TILEMAP_FLIPX);
		m_bg_tilemap->set_scrollx(0, m_scrollx[1] + ((m_scrollx[0] & 0x02) << 7) );
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	draw_sprites(bitmap, cliprect);          // sprites
	m_fg_tilemap->draw(screen, bitmap, cliprect, 2, 0); // high priority tiles of foreground

//  popmessage("%02X", m_scrollx[0]);
	return 0;
}
