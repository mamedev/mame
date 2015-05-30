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

TILEMAP_MAPPER_MEMBER(thedeep_state::tilemap_scan_rows_back)
{
	return (col & 0x0f) + ((col & 0x10) << 5) + (row << 4);
}

TILE_GET_INFO_MEMBER(thedeep_state::get_tile_info_0)
{
	UINT8 code  =   m_vram_0[ tile_index * 2 + 0 ];
	UINT8 color =   m_vram_0[ tile_index * 2 + 1 ];
	SET_TILE_INFO_MEMBER(1,
			code + (color << 8),
			(color & 0xf0) >> 4,
			TILE_FLIPX  );  // why?
}

TILE_GET_INFO_MEMBER(thedeep_state::get_tile_info_1)
{
	UINT8 code  =   m_vram_1[ tile_index * 2 + 0 ];
	UINT8 color =   m_vram_1[ tile_index * 2 + 1 ];
	SET_TILE_INFO_MEMBER(2,
			code + (color << 8),
			(color & 0xf0) >> 4,
			0);
}

WRITE8_MEMBER(thedeep_state::vram_0_w)
{
	m_vram_0[offset] = data;
	m_tilemap_0->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(thedeep_state::vram_1_w)
{
	m_vram_1[offset] = data;
	m_tilemap_1->mark_tile_dirty(offset / 2);
}


/***************************************************************************

                                Palette Init

***************************************************************************/

PALETTE_INIT_MEMBER(thedeep_state, thedeep)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	for (i = 0;i < 512;i++)
		palette.set_pen_color(i,pal4bit(color_prom[0x400 + i] >> 0),pal4bit(color_prom[0x400 + i] >> 4),pal4bit(color_prom[0x200 + i] >> 0));
}

/***************************************************************************

                                Video Init

***************************************************************************/

void thedeep_state::video_start()
{
	m_tilemap_0  = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(thedeep_state::get_tile_info_0),this),tilemap_mapper_delegate(FUNC(thedeep_state::tilemap_scan_rows_back),this),16,16,0x20,0x20);
	m_tilemap_1  = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(thedeep_state::get_tile_info_1),this),TILEMAP_SCAN_ROWS,8,8,0x20,0x20);

	m_tilemap_0->set_transparent_pen(0 );
	m_tilemap_1->set_transparent_pen(0 );

	m_tilemap_0->set_scroll_cols(0x20); // column scroll for the background
}

/***************************************************************************

                                Screen Drawing

***************************************************************************/

UINT32 thedeep_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int scrollx = m_scroll[0] + (m_scroll[1]<<8);
	int scrolly = m_scroll[2] + (m_scroll[3]<<8);
	int x;

	m_tilemap_0->set_scrollx(0, scrollx);

	for (x = 0; x < 0x20; x++)
	{
		int y = m_scroll2[x*2+0] + (m_scroll2[x*2+1]<<8);
		m_tilemap_0->set_scrolly(x, y + scrolly);
	}

	bitmap.fill(m_palette->black_pen(), cliprect);

	m_tilemap_0->draw(screen, bitmap, cliprect, 0,0);
	m_spritegen->draw_sprites(bitmap, cliprect,  reinterpret_cast<UINT16 *>(m_spriteram.target()), 0x00, 0x00, 0x0f);
	m_tilemap_1->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
