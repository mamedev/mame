// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                              -= Blomby Car =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q       shows the background
        W       shows the foreground
        A       shows the sprites

        Keys can be used together!


    [ 2 Scrolling Layers ]

    The Tilemaps are 64 x 32 tiles in size (1024 x 512).
    Tiles are 16 x 16 x 4, with 32 color codes and 2 priority
    leves (wrt sprites). Each tile needs 4 bytes.

    [ 1024? Sprites ]

    They use the same graphics the tilemaps use (16 x 16 x 4 tiles)
    with 16 color codes and 2 levels of priority


***************************************************************************/

#include "emu.h"
#include "includes/blmbycar.h"
#include "screen.h"


/***************************************************************************


                                Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code
        2.w     fedc ba98 ---- ----
                ---- ---- 7--- ----     Flip Y
                ---- ---- -6-- ----     Flip X
                ---- ---- --5- ----     Priority (0 = Low)
                ---- ---- ---4 3210     Color

***************************************************************************/

#define DIM_NX      (0x40)
#define DIM_NY      (0x20)

template<int Layer>
TILE_GET_INFO_MEMBER(blmbycar_state::get_tile_info)
{
	uint16_t code = m_vram[Layer][tile_index * 2 + 0];
	uint16_t attr = m_vram[Layer][tile_index * 2 + 1];
	SET_TILE_INFO_MEMBER(0,
			code,
			attr & 0x1f,
			TILE_FLIPYX((attr >> 6) & 3));

	tileinfo.category = (attr >> 5) & 1;
}

/***************************************************************************


                                Video Init


***************************************************************************/

void blmbycar_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(blmbycar_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, DIM_NX, DIM_NY );
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(blmbycar_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, DIM_NX, DIM_NY );
	m_tilemap[1]->set_transparent_pen(0);
}

/***************************************************************************


                                Screen Drawing


***************************************************************************/

uint32_t blmbycar_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_sprites->draw_sprites(cliprect,m_spriteram,flip_screen());

	m_tilemap[0]->set_scrolly(0, m_scroll[0][0]);
	m_tilemap[0]->set_scrollx(0, m_scroll[0][1]);

	m_tilemap[1]->set_scrolly(0, m_scroll[1][0] + 1);
	m_tilemap[1]->set_scrollx(0, m_scroll[1][1] + 5);

	screen.priority().fill(0, cliprect);

	bitmap.fill(0, cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	m_sprites->mix_sprites(bitmap, cliprect, 0);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 1, 1);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 1, 1);

	m_sprites->mix_sprites(bitmap, cliprect, 1);

	return 0;
}
