// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                              -= Unico Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q / W / E       Shows Layer 0 / 1 / 2
        A               Shows Sprites

        Keys can be used together!


    [ 3 Scrolling Layers ]

        Tile Size:              16 x 16 x 8
        Layer Size (tiles):     64 x 64

    [ 512 Sprites ]

        Sprites are made of 16 x 16 x 8 tiles. Size can vary from 1 to
        16 tiles horizontally, while their height is always 1 tile.
        There seems to be 4 levels of priority (wrt layers) for each
        sprite, following this simple scheme:

        [if we denote the three layers with 0-3 (0 being the backmost)
         and the sprite with S]

        Sprite Priority         Order (back -> front)
                0                   S 0 1 2
                1                   0 S 1 2
                2                   0 1 S 2
                3                   0 1 2 S

***************************************************************************/

#include "emu.h"
#include "includes/unico.h"


/***************************************************************************

                                    Palette

    Byte:   0   1   2   3
    Gun:    R   G   B   0

    6 Bits x Gun

***************************************************************************/

WRITE16_MEMBER(unico_state::unico_palette_w)
{
	UINT16 data1, data2;
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	data1 = m_generic_paletteram_16[offset & ~1];
	data2 = m_generic_paletteram_16[offset |  1];
	m_palette->set_pen_color( offset/2,
			(data1 >> 8) & 0xFC,
			(data1 >> 0) & 0xFC,
			(data2 >> 8) & 0xFC );
}

WRITE32_MEMBER(unico_state::unico_palette32_w)
{
	UINT32 rgb0 = COMBINE_DATA(&m_generic_paletteram_32[offset]);
	m_palette->set_pen_color( offset,
			(rgb0 >> 24) & 0xFC,
			(rgb0 >> 16) & 0xFC,
			(rgb0 >>  8) & 0xFC );
}


/***************************************************************************

                                Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code
        2.w     fedc ba98 7--- ----
                ---- ---- -6-- ----     Flip Y
                ---- ---- --5- ----     Flip X
                ---- ---- ---4 3210     Color

***************************************************************************/


TILE_GET_INFO_MEMBER(unico_state::get_tile_info)
{
	UINT16 *vram = (UINT16 *)tilemap.user_data();
	UINT16 code = vram[2 * tile_index + 0 ];
	UINT16 attr = vram[2 * tile_index + 1 ];
	SET_TILE_INFO_MEMBER(1, code, attr & 0x1f, TILE_FLIPYX( attr >> 5 ));
}

READ16_MEMBER(unico_state::unico_vram_r) { return m_vram[offset]; }

WRITE16_MEMBER(unico_state::unico_vram_w)
{
	UINT16 *vram = m_vram.get();
	int tile = ((offset / 0x2000) + 1) % 3;
	COMBINE_DATA(&vram[offset]);
	m_tilemap[tile]->mark_tile_dirty((offset & 0x1fff)/2);
}


READ16_MEMBER(unico_state::unico_scroll_r) { return m_scroll[offset]; }
WRITE16_MEMBER(unico_state::unico_scroll_w) { COMBINE_DATA(&m_scroll[offset]); }
READ16_MEMBER(unico_state::unico_spriteram_r) { return m_spriteram[offset]; }
WRITE16_MEMBER(unico_state::unico_spriteram_w)  { COMBINE_DATA(&m_spriteram[offset]); }



/***************************************************************************


                            Video Hardware Init


***************************************************************************/


VIDEO_START_MEMBER(unico_state,unico)
{
	m_vram   = make_unique_clear<UINT16[]>(0xc000 / 2);
	m_scroll = make_unique_clear<UINT16[]>(0x18 / 2);
	m_spriteram = make_unique_clear<UINT16[]>(0x800 / 2);

	save_pointer(NAME(m_vram.get()), 0xc000/2);
	save_pointer(NAME(m_scroll.get()), 0x18/2);
	save_pointer(NAME(m_spriteram.get()), 0x800/2);

	m_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(unico_state::get_tile_info),this),TILEMAP_SCAN_ROWS,
									16,16,  0x40, 0x40);

	m_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(unico_state::get_tile_info),this),TILEMAP_SCAN_ROWS,
									16,16,  0x40, 0x40);

	m_tilemap[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(unico_state::get_tile_info),this),TILEMAP_SCAN_ROWS,
									16,16,  0x40, 0x40);

	m_tilemap[0]->set_user_data(&m_vram[0x8000/2]);
	m_tilemap[1]->set_user_data(&m_vram[0x0000/2]);
	m_tilemap[2]->set_user_data(&m_vram[0x4000/2]);

	m_sprites_scrolldx = -0x3f;
	m_sprites_scrolldy = -0x0e;

	m_tilemap[0]->set_scrolldx(-0x32,0);
	m_tilemap[1]->set_scrolldx(-0x30,0);
	m_tilemap[2]->set_scrolldx(-0x2e,0);

	m_tilemap[0]->set_scrolldy(-0x0f,0);
	m_tilemap[1]->set_scrolldy(-0x0f,0);
	m_tilemap[2]->set_scrolldy(-0x0f,0);

	m_tilemap[0]->set_transparent_pen(0x00);
	m_tilemap[1]->set_transparent_pen(0x00);
	m_tilemap[2]->set_transparent_pen(0x00);
}




/***************************************************************************

                                Sprites Drawing


        0.w                             X

        2.w                             Y

        4.w                             Code

        6.w     fe-- ---- ---- ----
                --dc ---- ---- ----     Priority
                ---- ba98 ---- ----     Number of tiles along X, minus 1
                ---- ---- 7--- ----
                ---- ---- -6-- ----     Flip Y?
                ---- ---- --5- ----     Flip X
                ---- ---- ---4 3210     Color


***************************************************************************/

void unico_state::unico_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	UINT16 *spriteram16 = m_spriteram.get();
	int offs;

	/* Draw them backwards, for pdrawgfx */
	for ( offs = (0x800-8)/2; offs >= 0 ; offs -= 8/2 )
	{
		int x, startx, endx, incx;

		int sx          =   spriteram16[ offs + 0 ];
		int sy          =   spriteram16[ offs + 1 ];
		int code        =   spriteram16[ offs + 2 ];
		int attr        =   spriteram16[ offs + 3 ];

		int flipx       =   attr & 0x020;
		int flipy       =   attr & 0x040;   // not sure

		int dimx        =   ((attr >> 8) & 0xf) + 1;

		int priority    =   ((attr >> 12) & 0x3);
		int pri_mask;

		switch( priority )
		{
			case 0:     pri_mask = 0xfe;    break;  // below all
			case 1:     pri_mask = 0xf0;    break;  // above layer 0
			case 2:     pri_mask = 0xfc;    break;  // above layer 1
			default:
			case 3:     pri_mask = 0x00;            // above all
		}

		sx  +=  m_sprites_scrolldx;
		sy  +=  m_sprites_scrolldy;

		sx  =   (sx & 0x1ff) - (sx & 0x200);
		sy  =   (sy & 0x1ff) - (sy & 0x200);

		if (flipx)  {   startx = sx+(dimx-1)*16;    endx = sx-16;       incx = -16; }
		else        {   startx = sx;                endx = sx+dimx*16;  incx = +16; }

		for (x = startx ; x != endx ; x += incx)
		{
			m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
						code++,
						attr & 0x1f,
						flipx, flipy,
						x, sy,
						screen.priority(),
						pri_mask,0x00   );
		}
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

UINT32 unico_state::screen_update_unico(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

	m_tilemap[0]->set_scrollx(0, m_scroll[0x00]);
	m_tilemap[0]->set_scrolly(0, m_scroll[0x01]);

	m_tilemap[1]->set_scrollx(0, m_scroll[0x05]);
	m_tilemap[1]->set_scrolly(0, m_scroll[0x0a]);

	m_tilemap[2]->set_scrollx(0, m_scroll[0x04]);
	m_tilemap[2]->set_scrolly(0, m_scroll[0x02]);

#ifdef MAME_DEBUG
if ( machine().input().code_pressed(KEYCODE_Z) || machine().input().code_pressed(KEYCODE_X) )
{
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
	if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
	if (machine().input().code_pressed(KEYCODE_E))  msk |= 4;
	if (machine().input().code_pressed(KEYCODE_A))  msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	/* The background color is the first of the last palette */
	bitmap.fill(0x1f00, cliprect);
	screen.priority().fill(0, cliprect);

	if (layers_ctrl & 1)    m_tilemap[0]->draw(screen, bitmap, cliprect, 0,1);
	if (layers_ctrl & 2)    m_tilemap[1]->draw(screen, bitmap, cliprect, 0,2);
	if (layers_ctrl & 4)    m_tilemap[2]->draw(screen, bitmap, cliprect, 0,4);

	/* Sprites are drawn last, using pdrawgfx */
	if (layers_ctrl & 8)    unico_draw_sprites(screen,bitmap,cliprect);

	return 0;
}
