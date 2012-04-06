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

#define TMAP_GFX			(0)
#define TILES_PER_PAGE_X	(0x10)
#define TILES_PER_PAGE_Y	(0x10)
#define PAGES_PER_TMAP_X	(0x4)
#define PAGES_PER_TMAP_Y	(0x4)

static TILEMAP_MAPPER( yunsun16_tilemap_scan_pages )
{
	return	(row / TILES_PER_PAGE_Y) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y * PAGES_PER_TMAP_X +
			(row % TILES_PER_PAGE_Y) +

			(col / TILES_PER_PAGE_X) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y +
			(col % TILES_PER_PAGE_X) * TILES_PER_PAGE_Y;
}

static TILE_GET_INFO( get_tile_info_0 )
{
	yunsun16_state *state = machine.driver_data<yunsun16_state>();
	UINT16 code = state->m_vram_0[2 * tile_index + 0];
	UINT16 attr = state->m_vram_0[2 * tile_index + 1];
	SET_TILE_INFO(
			TMAP_GFX,
			code,
			attr & 0xf,
			(attr & 0x20) ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( get_tile_info_1 )
{
	yunsun16_state *state = machine.driver_data<yunsun16_state>();
	UINT16 code = state->m_vram_1[2 * tile_index + 0];
	UINT16 attr = state->m_vram_1[2 * tile_index + 1];
	SET_TILE_INFO(
			TMAP_GFX,
			code,
			attr & 0xf,
			(attr & 0x20) ? TILE_FLIPX : 0);
}

WRITE16_MEMBER(yunsun16_state::yunsun16_vram_0_w)
{

	COMBINE_DATA(&m_vram_0[offset]);
	m_tilemap_0->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(yunsun16_state::yunsun16_vram_1_w)
{

	COMBINE_DATA(&m_vram_1[offset]);
	m_tilemap_1->mark_tile_dirty(offset / 2);
}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

VIDEO_START( yunsun16 )
{
	yunsun16_state *state = machine.driver_data<yunsun16_state>();

	state->m_tilemap_0 = tilemap_create(machine, get_tile_info_0,yunsun16_tilemap_scan_pages,
								16,16, TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);
	state->m_tilemap_1 = tilemap_create(machine, get_tile_info_1,yunsun16_tilemap_scan_pages,
								16,16, TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	state->m_tilemap_0->set_scrolldx(-0x34, 0);
	state->m_tilemap_1->set_scrolldx(-0x38, 0);

	state->m_tilemap_0->set_scrolldy(-0x10, 0);
	state->m_tilemap_1->set_scrolldy(-0x10, 0);

	state->m_tilemap_0->set_transparent_pen(0xff);
	state->m_tilemap_1->set_transparent_pen(0xff);
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

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	yunsun16_state *state = machine.driver_data<yunsun16_state>();
	int offs;
	const rectangle &visarea = machine.primary_screen->visible_area();

	int max_x = visarea.max_x + 1;
	int max_y = visarea.max_y + 1;

	int pri = *state->m_priorityram & 3;
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

	for (offs = (state->m_spriteram_size - 8) / 2 ; offs >= 0; offs -= 8 / 2)
	{
		int x = state->m_spriteram[offs + 0];
		int y = state->m_spriteram[offs + 1];
		int code = state->m_spriteram[offs + 2];
		int attr = state->m_spriteram[offs + 3];
		int flipx = attr & 0x20;
		int flipy = attr & 0x40;

		x += state->m_sprites_scrolldx;
		y += state->m_sprites_scrolldy;

		if (flip_screen_get(machine))	// not used?
		{
			flipx = !flipx;		x = max_x - x - 16;
			flipy = !flipy;		y = max_y - y - 16;
		}

		pdrawgfx_transpen(bitmap,cliprect,machine.gfx[1],
					code,
					attr & 0x1f,
					flipx, flipy,
					x,y,
					machine.priority_bitmap,
					pri_mask,15);
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/


SCREEN_UPDATE_IND16( yunsun16 )
{
	yunsun16_state *state = screen.machine().driver_data<yunsun16_state>();

	state->m_tilemap_0->set_scrollx(0, state->m_scrollram_0[0]);
	state->m_tilemap_0->set_scrolly(0, state->m_scrollram_0[1]);

	state->m_tilemap_1->set_scrollx(0, state->m_scrollram_1[0]);
	state->m_tilemap_1->set_scrolly(0, state->m_scrollram_1[1]);

	//popmessage("%04X", *state->m_priorityram);

	screen.machine().priority_bitmap.fill(0, cliprect);

	if ((*state->m_priorityram & 0x0c) == 4)
	{
		/* The color of the this layer's transparent pen goes below everything */
		state->m_tilemap_0->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		state->m_tilemap_0->draw(bitmap, cliprect, 0, 1);
		state->m_tilemap_1->draw(bitmap, cliprect, 0, 2);
	}
	else if ((*state->m_priorityram & 0x0c) == 8)
	{
		/* The color of the this layer's transparent pen goes below everything */
		state->m_tilemap_1->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		state->m_tilemap_1->draw(bitmap, cliprect, 0, 1);
		state->m_tilemap_0->draw(bitmap, cliprect, 0, 2);
	}

	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
