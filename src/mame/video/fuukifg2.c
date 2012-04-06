/***************************************************************************

                          -= Fuuki 16 Bit Games (FG-2) =-

                    driver by   Luca Elia (l.elia@tin.it)
                    c.f. Fuuki FG-3


    [ 4 Scrolling Layers ]

                            [ Layer 0 ]     [ Layer 1 ]     [ Layers 2&3 (double-buffered) ]

    Tile Size:              16 x 16 x 4     16 x 16 x 8     8 x 8 x 4
    Layer Size (tiles):     64 x 32         64 x 32         64 x 32

    [ 1024? Zooming Sprites ]

    Sprites are made of 16 x 16 x 4 tiles. Size can vary from 1 to 16
    tiles both horizontally and vertically.
    There is zooming (from full size to half size) and 4 levels of
    priority (wrt layers)

    * Note: the game does hardware assisted raster effects *

***************************************************************************/

#include "emu.h"
#include "includes/fuukifg2.h"

/***************************************************************************


                                    Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code

        2.w     fedc ba98 ---- ----
                ---- ---- 7--- ----     Flip Y
                ---- ---- -6-- ----     Flip X
                ---- ---- --54 3210     Color


***************************************************************************/

INLINE void get_tile_info(running_machine &machine, tile_data &tileinfo, tilemap_memory_index tile_index, int _N_)
{
	fuuki16_state *state = machine.driver_data<fuuki16_state>();
	UINT16 code = state->m_vram[_N_][2 * tile_index + 0];
	UINT16 attr = state->m_vram[_N_][2 * tile_index + 1];
	SET_TILE_INFO(1 + _N_, code, attr & 0x3f, TILE_FLIPYX((attr >> 6) & 3));
}

static TILE_GET_INFO( get_tile_info_0 ) { get_tile_info(machine, tileinfo, tile_index, 0); }
static TILE_GET_INFO( get_tile_info_1 ) { get_tile_info(machine, tileinfo, tile_index, 1); }
static TILE_GET_INFO( get_tile_info_2 ) { get_tile_info(machine, tileinfo, tile_index, 2); }
static TILE_GET_INFO( get_tile_info_3 ) { get_tile_info(machine, tileinfo, tile_index, 3); }

INLINE void fuuki16_vram_w(address_space *space, offs_t offset, UINT16 data, UINT16 mem_mask, int _N_)
{
	fuuki16_state *state = space->machine().driver_data<fuuki16_state>();
	COMBINE_DATA(&state->m_vram[_N_][offset]);
	state->m_tilemap[_N_]->mark_tile_dirty(offset / 2);
}

WRITE16_MEMBER(fuuki16_state::fuuki16_vram_0_w){ fuuki16_vram_w(&space, offset, data, mem_mask, 0); }
WRITE16_MEMBER(fuuki16_state::fuuki16_vram_1_w){ fuuki16_vram_w(&space, offset, data, mem_mask, 1); }
WRITE16_MEMBER(fuuki16_state::fuuki16_vram_2_w){ fuuki16_vram_w(&space, offset, data, mem_mask, 2); }
WRITE16_MEMBER(fuuki16_state::fuuki16_vram_3_w){ fuuki16_vram_w(&space, offset, data, mem_mask, 3); }


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

/* Not used atm, seems to be fine without clearing pens? */
#if 0
PALETTE_INIT( fuuki16 )
{
	int pen;

	/* The game does not initialise the palette at startup. It should
       be totally black */
	for (pen = 0; pen < machine.total_colors(); pen++)
		palette_set_color(machine,pen,MAKE_RGB(0,0,0));
}
#endif

VIDEO_START( fuuki16 )
{
	fuuki16_state *state = machine.driver_data<fuuki16_state>();
	state->m_tilemap[0] = tilemap_create(machine, get_tile_info_0, tilemap_scan_rows, 16, 16, 64, 32);
	state->m_tilemap[1] = tilemap_create(machine, get_tile_info_1, tilemap_scan_rows, 16, 16, 64, 32);
	state->m_tilemap[2] = tilemap_create(machine, get_tile_info_2, tilemap_scan_rows, 8, 8, 64, 32);
	state->m_tilemap[3] = tilemap_create(machine, get_tile_info_3, tilemap_scan_rows, 8, 8, 64, 32);

	state->m_tilemap[0]->set_transparent_pen(0x0f);	// 4 bits
	state->m_tilemap[1]->set_transparent_pen(0xff);	// 8 bits
	state->m_tilemap[2]->set_transparent_pen(0x0f);	// 4 bits
	state->m_tilemap[3]->set_transparent_pen(0x0f);	// 4 bits

	machine.gfx[2]->color_granularity = 16; /* 256 colour tiles with palette selectable on 16 colour boundaries */
}


/***************************************************************************


                                Sprites Drawing

    Offset:     Bits:                   Value:

        0.w     fedc ---- ---- ----     Number Of Tiles Along X - 1
                ---- b--- ---- ----     Flip X
                ---- -a-- ---- ----     1 = Don't Draw This Sprite
                ---- --98 7654 3210     X (Signed)

        2.w     fedc ---- ---- ----     Number Of Tiles Along Y - 1
                ---- b--- ---- ----     Flip Y
                ---- -a-- ---- ----
                ---- --98 7654 3210     Y (Signed)

        4.w     fedc ---- ---- ----     Zoom X ($0 = Full Size, $F = Half Size)
                ---- ba98 ---- ----     Zoom Y ""
                ---- ---- 76-- ----     Priority
                ---- ---- --54 3210     Color

        6.w                             Code


***************************************************************************/

static void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	fuuki16_state *state = screen.machine().driver_data<fuuki16_state>();
	int offs;
	const gfx_element *gfx = screen.machine().gfx[0];
	bitmap_ind8 &priority_bitmap = screen.machine().priority_bitmap;
	const rectangle &visarea = screen.visible_area();
	UINT16 *spriteram16 = state->m_spriteram;
	int max_x =	visarea.max_x + 1;
	int max_y =	visarea.max_y + 1;

	/* Draw them backwards, for pdrawgfx */
	for ( offs = (state->m_spriteram_size - 8) / 2; offs >=0; offs -= 8 / 2 )
	{
		int x, y, xstart, ystart, xend, yend, xinc, yinc;
		int xnum, ynum, xzoom, yzoom, flipx, flipy;
		int pri_mask;

		int sx = spriteram16[offs + 0];
		int sy = spriteram16[offs + 1];
		int attr = spriteram16[offs + 2];
		int code = spriteram16[offs + 3];

		if (sx & 0x400)
			continue;

		flipx = sx & 0x0800;
		flipy = sy & 0x0800;

		xnum = ((sx >> 12) & 0xf) + 1;
		ynum = ((sy >> 12) & 0xf) + 1;

		xzoom = 16 * 8 - (8 * ((attr >> 12) & 0xf)) / 2;
		yzoom = 16 * 8 - (8 * ((attr >>  8) & 0xf)) / 2;

		switch ((attr >> 6) & 3)
		{
			case 3:	pri_mask = 0xf0 | 0xcc | 0xaa;	break;	// behind all layers
			case 2:	pri_mask = 0xf0 | 0xcc;			break;	// behind fg + middle layer
			case 1:	pri_mask = 0xf0;				break;	// behind fg layer
			case 0:
			default:	pri_mask = 0;						// above all
		}

		sx = (sx & 0x1ff) - (sx & 0x200);
		sy = (sy & 0x1ff) - (sy & 0x200);

		if (flip_screen_get(screen.machine()))
		{
			flipx = !flipx;		sx = max_x - sx - xnum * 16;
			flipy = !flipy;		sy = max_y - sy - ynum * 16;
		}

		if (flipx)	{ xstart = xnum-1;  xend = -1;    xinc = -1; }
		else		{ xstart = 0;       xend = xnum;  xinc = +1; }

		if (flipy)	{ ystart = ynum-1;  yend = -1;    yinc = -1; }
		else		{ ystart = 0;       yend = ynum;  yinc = +1; }

		for (y = ystart; y != yend; y += yinc)
		{
			for (x = xstart; x != xend; x += xinc)
			{
				if (xzoom == (16*8) && yzoom == (16*8))
					pdrawgfx_transpen(bitmap,cliprect,gfx,
									code++,
									attr & 0x3f,
									flipx, flipy,
									sx + x * 16, sy + y * 16,
									priority_bitmap,
									pri_mask,15	);
				else
					pdrawgfxzoom_transpen(bitmap,cliprect,gfx,
									code++,
									attr & 0x3f,
									flipx, flipy,
									sx + (x * xzoom) / 8, sy + (y * yzoom) / 8,
									(0x10000/0x10/8) * (xzoom + 8),(0x10000/0x10/8) * (yzoom + 8),	priority_bitmap,// nearest greater integer value to avoid holes
									pri_mask,15	);
			}
		}

#ifdef MAME_DEBUG
#if 0
if (screen.machine().input().code_pressed(KEYCODE_X))
{	/* Display some info on each sprite */
	char buf[40];
	sprintf(buf, "%Xx%X %X",xnum,ynum,(attr>>6)&3);
	ui_draw_text(buf, sx, sy);
}
#endif
#endif
	}
}


/***************************************************************************


                                Screen Drawing

    Video Registers (fuuki16_vregs):

        00.w        Layer 0 Scroll Y
        02.w        Layer 0 Scroll X
        04.w        Layer 1 Scroll Y
        06.w        Layer 1 Scroll X
        08.w        Layer 2 Scroll Y
        0a.w        Layer 2 Scroll X
        0c.w        Layers Y Offset
        0e.w        Layers X Offset

        10-1a.w     ? 0
        1c.w        Trigger a level 5 irq on this raster line
        1e.w        ? $3390/$3393 (Flip Screen Off/On), $0040 is buffer for tilemap 2 or 3

    Priority Register (fuuki16_priority):

        fedc ba98 7654 3---
        ---- ---- ---- -210     Layer Order


    Unknown Registers (fuuki16_unknown):

        00.w        ? $0200/$0201   (Flip Screen Off/On)
        02.w        ? $f300/$0330

***************************************************************************/

/* Wrapper to handle bg and bg2 ttogether */
static void fuuki16_draw_layer( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int i, int flag, int pri )
{
	fuuki16_state *state = machine.driver_data<fuuki16_state>();
	int buffer = (state->m_vregs[0x1e / 2] & 0x40);

	switch( i )
	{
		case 2:	if (buffer)	state->m_tilemap[3]->draw(bitmap, cliprect, flag, pri);
				else		state->m_tilemap[2]->draw(bitmap, cliprect, flag, pri);
				return;
		case 1:	state->m_tilemap[1]->draw(bitmap, cliprect, flag, pri);
				return;
		case 0:	state->m_tilemap[0]->draw(bitmap, cliprect, flag, pri);
				return;
	}
}

SCREEN_UPDATE_IND16( fuuki16 )
{
	fuuki16_state *state = screen.machine().driver_data<fuuki16_state>();
	UINT16 layer0_scrollx, layer0_scrolly;
	UINT16 layer1_scrollx, layer1_scrolly;
	UINT16 layer2_scrollx, layer2_scrolly;
	UINT16 scrollx_offs, scrolly_offs;

	/*
    It's not independent bits causing layers to switch, that wouldn't make sense with 3 bits.
    See fuukifg3 for more justification
    */
	static const int pri_table[6][3] = {
		{ 0, 1, 2 },
		{ 0, 2, 1 },
		{ 1, 0, 2 },
		{ 1, 2, 0 },
		{ 2, 0, 1 },
		{ 2, 1, 0 }};

	int tm_front  = pri_table[state->m_priority[0] & 0x0f][0];
	int tm_middle = pri_table[state->m_priority[0] & 0x0f][1];
	int tm_back   = pri_table[state->m_priority[0] & 0x0f][2];

	flip_screen_set(screen.machine(), state->m_vregs[0x1e / 2] & 1);

	/* Layers scrolling */

	scrolly_offs = state->m_vregs[0xc / 2] - (flip_screen_get(screen.machine()) ? 0x103 : 0x1f3);
	scrollx_offs = state->m_vregs[0xe / 2] - (flip_screen_get(screen.machine()) ? 0x2a7 : 0x3f6);

	layer0_scrolly = state->m_vregs[0x0 / 2] + scrolly_offs;
	layer0_scrollx = state->m_vregs[0x2 / 2] + scrollx_offs;
	layer1_scrolly = state->m_vregs[0x4 / 2] + scrolly_offs;
	layer1_scrollx = state->m_vregs[0x6 / 2] + scrollx_offs;

	layer2_scrolly = state->m_vregs[0x8 / 2];
	layer2_scrollx = state->m_vregs[0xa / 2];

	state->m_tilemap[0]->set_scrollx(0, layer0_scrollx);
	state->m_tilemap[0]->set_scrolly(0, layer0_scrolly);
	state->m_tilemap[1]->set_scrollx(0, layer1_scrollx);
	state->m_tilemap[1]->set_scrolly(0, layer1_scrolly);

	state->m_tilemap[2]->set_scrollx(0, layer2_scrollx + 0x10);
	state->m_tilemap[2]->set_scrolly(0, layer2_scrolly /*+ 0x02*/);
	state->m_tilemap[3]->set_scrollx(0, layer2_scrollx + 0x10);
	state->m_tilemap[3]->set_scrolly(0, layer2_scrolly /*+ 0x02*/);

	/* The backmost tilemap decides the background color(s) but sprites can
       go below the opaque pixels of that tilemap. We thus need to mark the
       transparent pixels of this layer with a different priority value */
//  fuuki16_draw_layer(screen.machine(), bitmap, cliprect, tm_back, TILEMAP_DRAW_OPAQUE, 0);

	/* Actually, bg colour is simply the last pen i.e. 0x1fff -pjp */
	bitmap.fill((0x800 * 4) - 1, cliprect);
	screen.machine().priority_bitmap.fill(0, cliprect);

	fuuki16_draw_layer(screen.machine(), bitmap, cliprect, tm_back,   0, 1);
	fuuki16_draw_layer(screen.machine(), bitmap, cliprect, tm_middle, 0, 2);
	fuuki16_draw_layer(screen.machine(), bitmap, cliprect, tm_front,  0, 4);

	draw_sprites(screen, bitmap, cliprect);

	return 0;
}
