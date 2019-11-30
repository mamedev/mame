// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                          -= Yun Sung 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


    [ 2 Scrolling Layers ]

    Tiles are 16 x 16 x 8. The layout of the tilemap is a bit weird:
    16 consecutive tile codes define a vertical column.
    16 columns form a page (256 x 256).
    The tilemap is made of 4 x 4 pages (1024 x 1024)

    [ 512? Sprites ]

    Sprites are 16 x 16 x 4 in size. There's RAM for 512, but
    the game just copies 384 entries.


***************************************************************************/

#include "emu.h"
#include "includes/yunsun16.h"


/***************************************************************************


                                    Tilemaps


***************************************************************************/

/*
#define TILES_PER_PAGE_X    (0x10)
#define TILES_PER_PAGE_Y    (0x10)
#define PAGES_PER_TMAP_X    (0x4)
#define PAGES_PER_TMAP_Y    (0x4)
*/

TILEMAP_MAPPER_MEMBER(yunsun16_state::tilemap_scan_pages)
{
	return  ((row & 0x30) << 6) | ((col & 0x3f) << 4) | (row & 0xf);
}

template<int Layer>
TILE_GET_INFO_MEMBER(yunsun16_state::get_tile_info)
{
	uint16_t code = m_vram[Layer][2 * tile_index + 0];
	uint16_t attr = m_vram[Layer][2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(0,
			code,
			attr & 0xf,
			(attr & 0x20) ? TILE_FLIPX : 0);
}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

void yunsun16_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(yunsun16_state::get_tile_info<0>)), tilemap_mapper_delegate(*this, FUNC(yunsun16_state::tilemap_scan_pages)),
			16, 16, 0x40, 0x40);
	m_tilemap[1] = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(yunsun16_state::get_tile_info<1>)), tilemap_mapper_delegate(*this, FUNC(yunsun16_state::tilemap_scan_pages)),
			16, 16, 0x40, 0x40);

	m_tilemap[0]->set_scrolldx(-0x34, 0);
	m_tilemap[1]->set_scrolldx(-0x38, 0);

	m_tilemap[0]->set_scrolldy(-0x10, 0);
	m_tilemap[1]->set_scrolldy(-0x10, 0);

	m_tilemap[0]->set_transparent_pen(0xff);
	m_tilemap[1]->set_transparent_pen(0xff);
}


/***************************************************************************


                                Sprites Drawing


        0.w                             X

        2.w                             Y

        4.w                             Code

        6.w     fedc ba98 7--- ----
                ---- ---- -6-- ----     Flip Y
                ---- ---- --5- ----     Flip X
                ---- ---- ---4 3210     Color


***************************************************************************/

void yunsun16_state::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;
	const rectangle &visarea = m_screen->visible_area();

	int max_x = visarea.max_x + 1;
	int max_y = visarea.max_y + 1;

	int pri = *m_priorityram & 3;
	int pri_mask;

	switch (pri)
	{
		case 1:
			pri_mask = (1 << 1) | (1 << 2) | (1 << 3);
			break;
		case 2:
			pri_mask = (1 << 2) | (1 << 3);
			break;
		case 3:
		default:
			pri_mask = 0;
			break;
	}

	for (offs = (m_spriteram.bytes() - 8) / 2 ; offs >= 0; offs -= 8 / 2)
	{
		int x = m_spriteram[offs + 0];
		int y = m_spriteram[offs + 1];
		int code = m_spriteram[offs + 2];
		int attr = m_spriteram[offs + 3];
		int flipx = attr & 0x20;
		int flipy = attr & 0x40;

		x += m_sprites_scrolldx;
		y += m_sprites_scrolldy;

		if (flip_screen())   // not used?
		{
			flipx = !flipx;     x = max_x - x - 16;
			flipy = !flipy;     y = max_y - y - 16;
		}

		m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
					code,
					attr & 0x1f,
					flipx, flipy,
					x,y,
					screen.priority(),
					pri_mask,15);
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/


uint32_t yunsun16_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->set_scrollx(0, m_scrollram[0][0]);
	m_tilemap[0]->set_scrolly(0, m_scrollram[0][1]);

	m_tilemap[1]->set_scrollx(0, m_scrollram[1][0]);
	m_tilemap[1]->set_scrolly(0, m_scrollram[1][1]);

	//popmessage("%04X", *m_priorityram);

	screen.priority().fill(0, cliprect);

	if ((*m_priorityram & 0x0c) == 4)
	{
		/* The color of the this layer's transparent pen goes below everything */
		m_tilemap[0]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 1);
		m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 2);
	}
	else if ((*m_priorityram & 0x0c) == 8)
	{
		/* The color of the this layer's transparent pen goes below everything */
		m_tilemap[1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 1);
		m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 2);
	}

	draw_sprites(screen, bitmap, cliprect);
	return 0;
}
