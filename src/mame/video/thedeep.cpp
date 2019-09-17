// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Run Deep / The Deep =-

                    driver by   Luca Elia (l.elia@tin.it)

    [ 1 Horizontally Scrolling Layer ]

        Size :  512 x 512
        Tiles:  16 x 16 x 4.

        In addition to a global x & y scroll register each tile-wide column
        has its own y scroll register.

    [ 1 Fixed Layer ]

        Size :  256 x 256
        Tiles:  8 x 8 x 2.

    [ 128? sprites ]

        Sprites tiles are 16 x 16 x 4. Each sprite has a height and width
        specified (1,2,4, or 8 tiles).

        A sprite of width N uses N consecutive sprites: the first one specifies
        all the data (position,flip), the following ones only the tile code and
        color for that column (tile codes in each column are consecutive).

***************************************************************************/

#include "emu.h"
#include "includes/thedeep.h"


/***************************************************************************

                        Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(thedeep_state::get_tile_info)
{
	uint8_t code  =   m_textram[ tile_index * 2 + 0 ];
	uint8_t color =   m_textram[ tile_index * 2 + 1 ];
	SET_TILE_INFO_MEMBER(2,
			code + (color << 8),
			(color & 0xf0) >> 4,
			0);
}

WRITE8_MEMBER(thedeep_state::textram_w)
{
	m_textram[offset] = data;
	m_text_tilemap->mark_tile_dirty(offset / 2);
}


/***************************************************************************

                                Palette Init

***************************************************************************/

void thedeep_state::thedeep_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 512; i++)
		palette.set_pen_color(i, pal4bit(color_prom[0x400 + i] >> 0), pal4bit(color_prom[0x400 + i] >> 4), pal4bit(color_prom[0x200 + i] >> 0));
}

/***************************************************************************

                                Video Init

***************************************************************************/

void thedeep_state::video_start()
{
	m_text_tilemap  = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(thedeep_state::get_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 0x20, 0x20);

	m_text_tilemap->set_transparent_pen(0);
}

/***************************************************************************

                                Screen Drawing

***************************************************************************/

uint32_t thedeep_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_tilegen->deco_bac06_pf_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00, 0);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(0), reinterpret_cast<uint16_t *>(m_spriteram.target()), 0x400/2);
	m_text_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
