// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= American Speedway =-

                    driver by   Luca Elia (l.elia@tin.it)


- 8x8 4 Color Tiles (with 8 palettes) used for both:

    - 1 256x256 non scrolling layer
    - 64 (32?) Sprites

***************************************************************************/

#include "emu.h"
#include "includes/amspdwy.h"


WRITE8_MEMBER(amspdwy_state::amspdwy_flipscreen_w)
{
	m_flipscreen ^= 1;
	flip_screen_set(m_flipscreen);
}

/***************************************************************************

                        Callbacks for the TileMap code

                              [ Tiles Format ]

    Videoram:   76543210    Code Low Bits
    Colorram:   765-----
                ---43---    Code High Bits
                -----210    Color

***************************************************************************/

TILE_GET_INFO_MEMBER(amspdwy_state::get_tile_info)
{
	UINT8 code = m_videoram[tile_index];
	UINT8 color = m_colorram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			code + ((color & 0x18)<<5),
			color & 0x07,
			0);
}

WRITE8_MEMBER(amspdwy_state::amspdwy_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(amspdwy_state::amspdwy_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


/* logical (col,row) -> memory offset */
TILEMAP_MAPPER_MEMBER(amspdwy_state::tilemap_scan_cols_back)
{
	return col * num_rows + (num_rows - row - 1);
}


void amspdwy_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(amspdwy_state::get_tile_info),this), tilemap_mapper_delegate(FUNC(amspdwy_state::tilemap_scan_cols_back),this), 8, 8, 0x20, 0x20);
}



/***************************************************************************

                                Sprites Drawing

Offset:     Format:     Value:

0                       Y
1                       X
2                       Code Low Bits
3           7-------    Flip X
            -6------    Flip Y
            --5-----
            ---4----    ?
            ----3---    Code High Bit?
            -----210    Color

***************************************************************************/

void amspdwy_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int i;
	int max_x = m_screen->width()  - 1;
	int max_y = m_screen->height() - 1;

	for (i = 0; i < m_spriteram.bytes() ; i += 4)
	{
		int y = spriteram[i + 0];
		int x = spriteram[i + 1];
		int code = spriteram[i + 2];
		int attr = spriteram[i + 3];
		int flipx = attr & 0x80;
		int flipy = attr & 0x40;

		if (flip_screen())
		{
			x = max_x - x - 8;
			y = max_y - y - 8;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
//              code + ((attr & 0x18)<<5),
				code + ((attr & 0x08)<<5),
				attr,
				flipx, flipy,
				x,y,0 );
	}
}




/***************************************************************************

                                Screen Drawing

***************************************************************************/

UINT32 amspdwy_state::screen_update_amspdwy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
