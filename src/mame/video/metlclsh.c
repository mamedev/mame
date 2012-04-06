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
		memory_set_bankptr(machine(), "bank1", m_bgram);
	}
	else
	{
		m_write_mask = 1 << (data >> 1);
		memory_set_bankptr(machine(), "bank1", m_otherram);
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

static TILEMAP_MAPPER( metlclsh_bgtilemap_scan )
{
	return	(row & 7) + ((row & ~7) << 4) + ((col & 0xf) << 3) + ((col & ~0xf) << 4);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	metlclsh_state *state = machine.driver_data<metlclsh_state>();
	SET_TILE_INFO(1, state->m_bgram[tile_index] + (state->m_gfxbank << 7), 0, 0);
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

static TILE_GET_INFO( get_fg_tile_info )
{
	metlclsh_state *state = machine.driver_data<metlclsh_state>();
	UINT8 code = state->m_fgram[tile_index + 0x000];
	UINT8 attr = state->m_fgram[tile_index + 0x400];
	SET_TILE_INFO(2, code + ((attr & 0x03) << 8), (attr >> 5) & 3, 0);
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

VIDEO_START( metlclsh )
{
	metlclsh_state *state = machine.driver_data<metlclsh_state>();

	state->m_otherram = auto_alloc_array(machine, UINT8, 0x800);	// banked ram

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, metlclsh_bgtilemap_scan, 16, 16, 32, 16);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_bg_tilemap->set_transparent_pen(0);
	state->m_fg_tilemap->set_transparent_pen(0);

	state->save_pointer(NAME(state->m_otherram), 0x800);
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

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	metlclsh_state *state = machine.driver_data<metlclsh_state>();
	UINT8 *spriteram = state->m_spriteram;
	gfx_element *gfx = machine.gfx[0];
	int offs;

	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		int attr, code, color, sx, sy, flipx, flipy, wrapy, sizey;

		attr = spriteram[offs];
		if (!(attr & 0x01))
			continue;	// enable

		flipy = (attr & 0x02);
		flipx = (attr & 0x04);
		color = (attr & 0x08) >> 3;
		sizey = (attr & 0x10);	// double height
		code = ((attr & 0x60) << 3) + spriteram[offs + 1];

		sx = 240 - spriteram[offs + 3];
		if (sx < -7)
			sx += 256;

		sy = 240 - spriteram[offs + 2];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;	flipx = !flipx;
			sy = 240 - sy;	flipy = !flipy;		if (sizey)	sy += 16;
			if (sy > 240)	sy -= 256;
		}

		/* Draw twice, at sy and sy + 256 (wrap around) */
		for (wrapy = 0; wrapy <= 256; wrapy += 256)
		{
			if (sizey)
			{
				drawgfx_transpen(bitmap,cliprect,gfx, code & ~1, color, flipx,flipy,
						sx, sy + (flipy ? 0 : -16) + wrapy,0);

				drawgfx_transpen(bitmap,cliprect,gfx, code |  1, color, flipx,flipy,
						sx,sy + (flipy ? -16 : 0) + wrapy,0);
			}
			else
			{
				drawgfx_transpen(bitmap,cliprect,gfx, code, color, flipx,flipy,
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

SCREEN_UPDATE_IND16( metlclsh )
{
	metlclsh_state *state = screen.machine().driver_data<metlclsh_state>();

	bitmap.fill(0x10, cliprect);

	state->m_fg_tilemap->draw(bitmap, cliprect, 1, 0);	// low priority tiles of foreground

	if (state->m_scrollx[0] & 0x08)					// background (if enabled)
	{
		/* The background seems to be always flipped along x */
		state->m_bg_tilemap->set_flip((flip_screen_get(screen.machine()) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0) ^ TILEMAP_FLIPX);
		state->m_bg_tilemap->set_scrollx(0, state->m_scrollx[1] + ((state->m_scrollx[0] & 0x02) << 7) );
		state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	}
	draw_sprites(screen.machine(), bitmap, cliprect);			// sprites
	state->m_fg_tilemap->draw(bitmap, cliprect, 2, 0);	// high priority tiles of foreground

//  popmessage("%02X", state->m_scrollx[0]);
	return 0;
}

