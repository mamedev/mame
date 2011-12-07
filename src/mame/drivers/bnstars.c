/*
Vs. Janshi Brandnew Stars
(c)1997 Jaleco

Single board version with Dual Screen output
(MS32 version also exists)

for the time being most of this driver is copied
from ms32.c, with some adjustments for dual screen.


Main PCB
--------

PCB ID  : MB-93141 EB91022-20081
CPU     : NEC D70632GD-20 (V70)
OSC     : 48.000MHz, 40.000MHz
RAM     : Cypress CY7C199-25 (x20)
          OKI M511664-80 (x8)
DIPs    : 8 position (x3)
OTHER   : Some PALs

          Custom chips:
                       JALECO SS91022-01 (208 PIN PQFP)
                       JALECO SS91022-02 (100 PIN PQFP) (x2)
                       JALECO SS91022-03 (176 PIN PQFP) (x2)
                       JALECO SS91022-05 (120 PIN PQFP) (x2)
                       JALECO SS91022-07 (208 PIN PQFP) (x2)
                       JALECO GS91022-01 (120 PIN PQFP)
                       JALECO GS91022-02 (160 PIN PQFP)
                       JALECO GS91022-03 (100 PIN PQFP)
                       JALECO GS91022-04 (100 PIN PQFP)
                       JALECO GS90015-03 ( 80 PIN PQFP) (x3) (not present on MS32)


ROMs:     None


ROM PCB (equivalent to MS32 cartridge)
--------------------------------------
PCB ID  : MB-93142 EB93007-20082
DIPs    : determine the size of ROMs?
          8 position (x1, 6 and 8 is on, others are off)
          4 position (x2, both are off on off off)
OTHER   : Some PALs
          Custom chip: JALECO SS92046-01 (144 pin PQFP) (x2)
          (located on small plug-in board with) (ID: SE93139 EB91022-30056)
ROMs    : MB-93142.36   [2eb6a503] (IC42, also printed "?u?????j???[Ver1.2")
          MB-93142.37   [49f60882] (IC57, also printed "?u?????j???[Ver1.2")
          MB-93142.38   [6e1312cd] (IC58, also printed "?u?????j???[Ver1.2")
          MB-93142.39   [56b98539] (IC59, also printed "?u?????j???[Ver1.2")

          VSJANSHI5.6   [fdbbac21] (IC9, actual label is "VS?W?????V 5 Ver1.0")
          VSJANSHI6.5   [fdbbac21] (IC8, actual label is "VS?W?????V 6 Ver1.0")

?@?@?@?@?@MR96004-01.20 [3366d104] (IC29)
?@?@?@?@?@MR96004-02.28 [ad556664] (IC49)
?@?@?@?@?@MR96004-03.21 [b399e2b1] (IC30)
?@?@?@?@?@MR96004-04.29 [f4f4cf4a] (IC50)
?@?@?@?@?@MR96004-05.22 [cd6c357e] (IC31)
?@?@?@?@?@MR96004-06.30 [fc6daad7] (IC51)
?@?@?@?@?@MR96004-07.23 [177e32fa] (IC32)
?@?@?@?@?@MR96004-08.31 [f6df27b2] (IC52)

?@?@?@?@?@MR96004-09.1  [603143e8] (IC4)
?@?@?@?@?@MR96004-09.7  [603143e8] (IC10)

?@?@?@?@?@MR96004-11.11 [e6da552c] (IC17)
?@?@?@?@?@MR96004-11.13 [e6da552c] (IC19)


Sound PCB
---------
PCB ID  : SB-93143 EB91022-20083
SOUND   : Z80, YMF271-F, YAC513(x2)
OSC     : 8.000MHz, 16.9344MHz
DIPs    : 4 position (all off)
ROMs    : SB93145.5     [0424e899] (IC34, also printed "Ver1.0") - Sound program
          MR96004-10.1  [125661cd] (IC19 - Samples)

Sound sub PCB
-------------
PCB ID  : SB-93145 EB93007-20086
SOUND   : YMF271-F
ROMs    : MR96004-10.1  [125661cd] (IC5 - Samples)

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/v60/v60.h"
#include "sound/ymf271.h"
#include "rendlay.h"
#include "machine/jalcrpt.h"


class bnstars_state : public driver_device
{
public:
	bnstars_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	tilemap_t *m_ms32_tx_tilemap[2];
	tilemap_t *m_ms32_bg_tilemap[2];
	tilemap_t *m_ms32_roz_tilemap[2];
	UINT32 *m_ms32_tx0_ram;
	UINT32 *m_ms32_tx1_ram;
	UINT32 *m_ms32_bg0_ram;
	UINT32 *m_ms32_bg1_ram;
	UINT32 *m_ms32_roz0_ram;
	UINT32 *m_ms32_roz1_ram;
	UINT32 *m_ms32_pal_ram[2];
	UINT32 *m_ms32_roz_ctrl[2];
	UINT32 *m_ms32_spram;
	UINT32 *m_ms32_tx0_scroll;
	UINT32 *m_ms32_bg0_scroll;
	UINT32 *m_ms32_tx1_scroll;
	UINT32 *m_ms32_bg1_scroll;
	UINT32 m_bnstars1_mahjong_select;
	int m_ms32_reverse_sprite_order;
	int m_flipscreen;
	UINT16 m_irqreq;
};



static TILE_GET_INFO( get_ms32_tx0_tile_info )
{
	bnstars_state *state = machine.driver_data<bnstars_state>();
	int tileno, colour;

	tileno = state->m_ms32_tx0_ram[tile_index *2+0] & 0x0000ffff;
	colour = state->m_ms32_tx0_ram[tile_index *2+1] & 0x0000000f;

	SET_TILE_INFO(3,tileno,colour,0);
}

static TILE_GET_INFO( get_ms32_tx1_tile_info )
{
	bnstars_state *state = machine.driver_data<bnstars_state>();
	int tileno, colour;

	tileno = state->m_ms32_tx1_ram[tile_index *2+0] & 0x0000ffff;
	colour = state->m_ms32_tx1_ram[tile_index *2+1] & 0x0000000f;

	SET_TILE_INFO(7,tileno,colour,0);
}

static WRITE32_HANDLER( ms32_tx0_ram_w )
{
	bnstars_state *state = space->machine().driver_data<bnstars_state>();
	COMBINE_DATA(&state->m_ms32_tx0_ram[offset]);
	tilemap_mark_tile_dirty(state->m_ms32_tx_tilemap[0],offset/2);
}

static WRITE32_HANDLER( ms32_tx1_ram_w )
{
	bnstars_state *state = space->machine().driver_data<bnstars_state>();
	COMBINE_DATA(&state->m_ms32_tx1_ram[offset]);
	tilemap_mark_tile_dirty(state->m_ms32_tx_tilemap[1],offset/2);
}

/* BG Layers */

static TILE_GET_INFO( get_ms32_bg0_tile_info )
{
	bnstars_state *state = machine.driver_data<bnstars_state>();
	int tileno,colour;

	tileno = state->m_ms32_bg0_ram[tile_index *2+0] & 0x0000ffff;
	colour = state->m_ms32_bg0_ram[tile_index *2+1] & 0x0000000f;

	SET_TILE_INFO(2,tileno,colour,0);
}

static TILE_GET_INFO( get_ms32_bg1_tile_info )
{
	bnstars_state *state = machine.driver_data<bnstars_state>();
	int tileno,colour;

	tileno = state->m_ms32_bg1_ram[tile_index *2+0] & 0x0000ffff;
	colour = state->m_ms32_bg1_ram[tile_index *2+1] & 0x0000000f;

	SET_TILE_INFO(6,tileno,colour,0);
}

static WRITE32_HANDLER( ms32_bg0_ram_w )
{
	bnstars_state *state = space->machine().driver_data<bnstars_state>();
	COMBINE_DATA(&state->m_ms32_bg0_ram[offset]);
	tilemap_mark_tile_dirty(state->m_ms32_bg_tilemap[0],offset/2);
}

static WRITE32_HANDLER( ms32_bg1_ram_w )
{
	bnstars_state *state = space->machine().driver_data<bnstars_state>();
	COMBINE_DATA(&state->m_ms32_bg1_ram[offset]);
	tilemap_mark_tile_dirty(state->m_ms32_bg_tilemap[1],offset/2);
}

/* ROZ Layers */

static void draw_roz(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int priority, int chip)
{
	bnstars_state *state = machine.driver_data<bnstars_state>();
	/* TODO: registers 0x40/4 / 0x44/4 and 0x50/4 / 0x54/4 are used, meaning unknown */

	if (state->m_ms32_roz_ctrl[chip][0x5c/4] & 1)	/* "super" mode */
	{
		printf("no lineram!\n");
		return;
		/*
        rectangle my_clip;
        int y,maxy;

        my_clip.min_x = cliprect->min_x;
        my_clip.max_x = cliprect->max_x;

        y = cliprect->min_y;
        maxy = cliprect->max_y;

        while (y <= maxy)
        {
            UINT32 *lineaddr = ms32_lineram + 8 * (y & 0xff);

            int start2x = (lineaddr[0x00/4] & 0xffff) | ((lineaddr[0x04/4] & 3) << 16);
            int start2y = (lineaddr[0x08/4] & 0xffff) | ((lineaddr[0x0c/4] & 3) << 16);
            int incxx  = (lineaddr[0x10/4] & 0xffff) | ((lineaddr[0x14/4] & 1) << 16);
            int incxy  = (lineaddr[0x18/4] & 0xffff) | ((lineaddr[0x1c/4] & 1) << 16);
            int startx = (state->m_ms32_roz_ctrl[0x00/4] & 0xffff) | ((state->m_ms32_roz_ctrl[0x04/4] & 3) << 16);
            int starty = (state->m_ms32_roz_ctrl[0x08/4] & 0xffff) | ((state->m_ms32_roz_ctrl[0x0c/4] & 3) << 16);
            int offsx  = state->m_ms32_roz_ctrl[0x30/4];
            int offsy  = state->m_ms32_roz_ctrl[0x34/4];

            my_clip.min_y = my_clip.max_y = y;

            offsx += (state->m_ms32_roz_ctrl[0x38/4] & 1) * 0x400;   // ??? gratia, hayaosi1...
            offsy += (state->m_ms32_roz_ctrl[0x3c/4] & 1) * 0x400;   // ??? gratia, hayaosi1...

            // extend sign
            if (start2x & 0x20000) start2x |= ~0x3ffff;
            if (start2y & 0x20000) start2y |= ~0x3ffff;
            if (startx & 0x20000) startx |= ~0x3ffff;
            if (starty & 0x20000) starty |= ~0x3ffff;
            if (incxx & 0x10000) incxx |= ~0x1ffff;
            if (incxy & 0x10000) incxy |= ~0x1ffff;

            tilemap_draw_roz(bitmap, &my_clip, state->m_ms32_roz_tilemap,
                    (start2x+startx+offsx)<<16, (start2y+starty+offsy)<<16,
                    incxx<<8, incxy<<8, 0, 0,
                    1, // Wrap
                    0, priority);

            y++;
        }
        */
	}
	else	/* "simple" mode */
	{
		int startx = (state->m_ms32_roz_ctrl[chip][0x00/4] & 0xffff) | ((state->m_ms32_roz_ctrl[chip][0x04/4] & 3) << 16);
		int starty = (state->m_ms32_roz_ctrl[chip][0x08/4] & 0xffff) | ((state->m_ms32_roz_ctrl[chip][0x0c/4] & 3) << 16);
		int incxx  = (state->m_ms32_roz_ctrl[chip][0x10/4] & 0xffff) | ((state->m_ms32_roz_ctrl[chip][0x14/4] & 1) << 16);
		int incxy  = (state->m_ms32_roz_ctrl[chip][0x18/4] & 0xffff) | ((state->m_ms32_roz_ctrl[chip][0x1c/4] & 1) << 16);
		int incyy  = (state->m_ms32_roz_ctrl[chip][0x20/4] & 0xffff) | ((state->m_ms32_roz_ctrl[chip][0x24/4] & 1) << 16);
		int incyx  = (state->m_ms32_roz_ctrl[chip][0x28/4] & 0xffff) | ((state->m_ms32_roz_ctrl[chip][0x2c/4] & 1) << 16);
		int offsx  = state->m_ms32_roz_ctrl[chip][0x30/4];
		int offsy  = state->m_ms32_roz_ctrl[chip][0x34/4];

		offsx += (state->m_ms32_roz_ctrl[chip][0x38/4] & 1) * 0x400;	// ??? gratia, hayaosi1...
		offsy += (state->m_ms32_roz_ctrl[chip][0x3c/4] & 1) * 0x400;	// ??? gratia, hayaosi1...

		/* extend sign */
		if (startx & 0x20000) startx |= ~0x3ffff;
		if (starty & 0x20000) starty |= ~0x3ffff;
		if (incxx & 0x10000) incxx |= ~0x1ffff;
		if (incxy & 0x10000) incxy |= ~0x1ffff;
		if (incyy & 0x10000) incyy |= ~0x1ffff;
		if (incyx & 0x10000) incyx |= ~0x1ffff;

		tilemap_draw_roz(bitmap, cliprect, state->m_ms32_roz_tilemap[chip],
				(startx+offsx)<<16, (starty+offsy)<<16,
				incxx<<8, incxy<<8, incyx<<8, incyy<<8,
				1, // Wrap
				0, priority);
	}
}


static TILE_GET_INFO( get_ms32_roz0_tile_info )
{
	bnstars_state *state = machine.driver_data<bnstars_state>();
	int tileno,colour;

	tileno = state->m_ms32_roz0_ram[tile_index *2+0] & 0x0000ffff;
	colour = state->m_ms32_roz0_ram[tile_index *2+1] & 0x0000000f;

	SET_TILE_INFO(1,tileno,colour,0);
}

static TILE_GET_INFO( get_ms32_roz1_tile_info )
{
	bnstars_state *state = machine.driver_data<bnstars_state>();
	int tileno,colour;

	tileno = state->m_ms32_roz1_ram[tile_index *2+0] & 0x0000ffff;
	colour = state->m_ms32_roz1_ram[tile_index *2+1] & 0x0000000f;

	SET_TILE_INFO(5,tileno,colour,0);
}

static WRITE32_HANDLER( ms32_roz0_ram_w )
{
	bnstars_state *state = space->machine().driver_data<bnstars_state>();
	COMBINE_DATA(&state->m_ms32_roz0_ram[offset]);
	tilemap_mark_tile_dirty(state->m_ms32_roz_tilemap[0],offset/2);
}

static WRITE32_HANDLER( ms32_roz1_ram_w )
{
	bnstars_state *state = space->machine().driver_data<bnstars_state>();
	COMBINE_DATA(&state->m_ms32_roz1_ram[offset]);
	tilemap_mark_tile_dirty(state->m_ms32_roz_tilemap[1],offset/2);
}


static void update_color(running_machine &machine, int color, int screen)
{
	bnstars_state *state = machine.driver_data<bnstars_state>();
	int r,g,b;

	r = ((state->m_ms32_pal_ram[screen][color*2] & 0xff00) >>8 );
	g = ((state->m_ms32_pal_ram[screen][color*2] & 0x00ff) >>0 );
	b = ((state->m_ms32_pal_ram[screen][color*2+1] & 0x00ff) >>0 );

	palette_set_color(machine,color+screen*0x8000,MAKE_RGB(r,g,b));
}

static WRITE32_HANDLER( ms32_pal0_ram_w )
{
	bnstars_state *state = space->machine().driver_data<bnstars_state>();
	COMBINE_DATA(&state->m_ms32_pal_ram[0][offset]);
	update_color(space->machine(), offset/2, 0);
}

static WRITE32_HANDLER( ms32_pal1_ram_w )
{
	bnstars_state *state = space->machine().driver_data<bnstars_state>();
	COMBINE_DATA(&state->m_ms32_pal_ram[1][offset]);
	update_color(space->machine(), offset/2, 1);
}


/* SPRITES based on tetrisp2 for now, readd priority bits later */
static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT32 *sprram_top, size_t sprram_size, int region)
{
	bnstars_state *state = machine.driver_data<bnstars_state>();
/***************************************************************************


                                Sprites Drawing

    Offset:     Bits:                   Meaning:

    0.w         fedc ba98 ---- ----
                ---- ---- 7654 ----     Priority
                ---- ---- ---- 3---
                ---- ---- ---- -2--     Draw this sprite
                ---- ---- ---- --1-     Flip Y
                ---- ---- ---- ---0     Flip X

    1.w         fedc ba98 ---- ----     Tile's Y position in the tile page (*)
                ---- ---- 7654 3210     Tile's X position in the tile page (*)

    2.w         fedc ---- ---- ----     Color
                ---- ba98 7654 3210     Tile Page (32x32 tiles = 256x256 pixels each)

    3.w         fedc ba98 ---- ----     Y Size - 1 (*)
                ---- ---- 7654 3210     X Size - 1 (*)

    4.w         fedc ba-- ---- ----
                ---- --98 7654 3210     Y (Signed)

    5.w         fedc b--- ---- ----
                ---- -a98 7654 3210     X (Signed)

    6.w         fedc ba98 7654 3210     Zoom Y

    7.w         fedc ba98 7654 3210     Zoom X

(*) 1 pixel granularity

***************************************************************************/

	int tx, ty, sx, sy, flipx, flipy;
	int xsize, ysize, xzoom, yzoom;
	int code, attr, color, size, pri, pri_mask;
	gfx_element *gfx = machine.gfx[region];

	UINT32		*source	= sprram_top;
	const UINT32	*finish	= sprram_top + (sprram_size - 0x10) / 4;


	if (state->m_ms32_reverse_sprite_order == 1)
	{
		source	= sprram_top + (sprram_size - 0x10) / 4;
		finish	= sprram_top;
	}


	for (;state->m_ms32_reverse_sprite_order ? (source>=finish) : (source<finish); state->m_ms32_reverse_sprite_order ? (source-=4) : (source+=4))
	{
		attr	=	source[ 0 ];

		if ((attr & 0x0004) == 0)			continue;

		flipx	=	attr & 1;
		flipy	=	attr & 2;

		pri = (attr >> 4)&0xf;

		code	=	source[ 1 ];
		color	=	source[ 2 ];

		tx		=	(code >> 0) & 0xff;
		ty		=	(code >> 8) & 0xff;

		code	=	(color & 0x0fff);

		color	=	(color >> 12) & 0xf;

		size	=	source[ 3 ];

		xsize	=	((size >> 0) & 0xff) + 1;
		ysize	=	((size >> 8) & 0xff) + 1;

		sy		=	source[ 4 ];
		sx		=	source[ 5 ];

		sx		=	(sx & 0x3ff) - (sx & 0x400);
		sy		=	(sy & 0x1ff) - (sy & 0x200);

		xzoom	=	(source[ 6 ]&0xffff);
		yzoom	=	(source[ 7 ]&0xffff);

		if (!yzoom || !xzoom)				continue;

		yzoom = 0x1000000/yzoom;
		xzoom = 0x1000000/xzoom;

		// there are surely also shadows (see gametngk) but how they're enabled we don't know

		if (state->m_flipscreen)
		{
			sx = 320 - ((xsize*xzoom)>>16) - sx;
			sy = 224 - ((ysize*yzoom)>>16) - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		/* TODO: priority handling is completely wrong, but better than nothing */
		if (pri == 0x0)
			pri_mask = 0x00;
		else if (pri <= 0xd)
			pri_mask = 0xf0;
		else if (pri <= 0xe)
			pri_mask = 0xfc;
		else
			pri_mask = 0xfe;

		gfx_element_set_source_clip(gfx, tx, xsize, ty, ysize);
		pdrawgfxzoom_transpen(bitmap, cliprect, gfx,
				code,
				color,
				flipx, flipy,
				sx,sy,
				xzoom, yzoom, machine.priority_bitmap,pri_mask, 0);
	}	/* end sprite loop */
}



static WRITE32_HANDLER( ms32_spramx_w )
{
	bnstars_state *state = space->machine().driver_data<bnstars_state>();
	COMBINE_DATA(&state->m_ms32_spram[offset]);
}


static VIDEO_START(bnstars)
{
	bnstars_state *state = machine.driver_data<bnstars_state>();
	state->m_ms32_tx_tilemap[0] = tilemap_create(machine, get_ms32_tx0_tile_info,tilemap_scan_rows, 8, 8,64,64);
	state->m_ms32_tx_tilemap[1] = tilemap_create(machine, get_ms32_tx1_tile_info,tilemap_scan_rows, 8, 8,64,64);
	tilemap_set_transparent_pen(state->m_ms32_tx_tilemap[0],0);
	tilemap_set_transparent_pen(state->m_ms32_tx_tilemap[1],0);

	state->m_ms32_bg_tilemap[0] = tilemap_create(machine, get_ms32_bg0_tile_info,tilemap_scan_rows,16,16,64,64);
	state->m_ms32_bg_tilemap[1] = tilemap_create(machine, get_ms32_bg1_tile_info,tilemap_scan_rows,16,16,64,64);
	tilemap_set_transparent_pen(state->m_ms32_bg_tilemap[0],0);
	tilemap_set_transparent_pen(state->m_ms32_bg_tilemap[1],0);

	state->m_ms32_roz_tilemap[0] = tilemap_create(machine, get_ms32_roz0_tile_info,tilemap_scan_rows,16,16,128,128);
	state->m_ms32_roz_tilemap[1] = tilemap_create(machine, get_ms32_roz1_tile_info,tilemap_scan_rows,16,16,128,128);
	tilemap_set_transparent_pen(state->m_ms32_roz_tilemap[0],0);
	tilemap_set_transparent_pen(state->m_ms32_roz_tilemap[1],0);


}





static SCREEN_UPDATE(bnstars)
{
	bnstars_state *state = screen->machine().driver_data<bnstars_state>();
	device_t *left_screen  = screen->machine().device("lscreen");
	device_t *right_screen = screen->machine().device("rscreen");

	bitmap_fill(screen->machine().priority_bitmap,cliprect,0);

	if (screen==left_screen)
	{
		bitmap_fill(bitmap,cliprect,0);	/* bg color */


		tilemap_set_scrollx(state->m_ms32_bg_tilemap[0], 0, state->m_ms32_bg0_scroll[0x00/4] + state->m_ms32_bg0_scroll[0x08/4] + 0x10 );
		tilemap_set_scrolly(state->m_ms32_bg_tilemap[0], 0, state->m_ms32_bg0_scroll[0x0c/4] + state->m_ms32_bg0_scroll[0x14/4] );
		tilemap_draw(bitmap,cliprect,state->m_ms32_bg_tilemap[0],0,1);

		draw_roz(screen->machine(),bitmap,cliprect,2,0);

		tilemap_set_scrollx(state->m_ms32_tx_tilemap[0], 0, state->m_ms32_tx0_scroll[0x00/4] + state->m_ms32_tx0_scroll[0x08/4] + 0x18);
		tilemap_set_scrolly(state->m_ms32_tx_tilemap[0], 0, state->m_ms32_tx0_scroll[0x0c/4] + state->m_ms32_tx0_scroll[0x14/4]);
		tilemap_draw(bitmap,cliprect,state->m_ms32_tx_tilemap[0],0,4);


		draw_sprites(screen->machine(),bitmap,cliprect, state->m_ms32_spram, 0x20000, 0);
	}
	else if (screen == right_screen)
	{
		bitmap_fill(bitmap,cliprect,0x8000+0);	/* bg color */


		tilemap_set_scrollx(state->m_ms32_bg_tilemap[1], 0, state->m_ms32_bg1_scroll[0x00/4] + state->m_ms32_bg1_scroll[0x08/4] + 0x10 );
		tilemap_set_scrolly(state->m_ms32_bg_tilemap[1], 0, state->m_ms32_bg1_scroll[0x0c/4] + state->m_ms32_bg1_scroll[0x14/4] );
		tilemap_draw(bitmap,cliprect,state->m_ms32_bg_tilemap[1],0,1);

		draw_roz(screen->machine(),bitmap,cliprect,2,1);

		tilemap_set_scrollx(state->m_ms32_tx_tilemap[1], 0, state->m_ms32_tx1_scroll[0x00/4] + state->m_ms32_tx1_scroll[0x08/4] + 0x18);
		tilemap_set_scrolly(state->m_ms32_tx_tilemap[1], 0, state->m_ms32_tx1_scroll[0x0c/4] + state->m_ms32_tx1_scroll[0x14/4]);
		tilemap_draw(bitmap,cliprect,state->m_ms32_tx_tilemap[1],0,4);

		draw_sprites(screen->machine(),bitmap,cliprect, state->m_ms32_spram+(0x20000/4), 0x20000, 4);
	}

	return 0;
}

static INPUT_PORTS_START( bnstars )
	PORT_START("IN0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_DIPNAME(     0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000100, 0x00000100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000100, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000200, 0x00000200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000200, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000400, 0x00000400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000800, 0x00000800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000800, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00001000, 0x00001000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00001000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00002000, 0x00002000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00002000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00004000, 0x00004000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00004000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00008000, 0x00008000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00008000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME(     0x00020000, 0x00020000, "MAH1" )
	PORT_DIPSETTING(  0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME(     0x00080000, 0x00080000, "Service Mode ? 1" )
	PORT_DIPSETTING(  0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x01000000, 0x01000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x02000000, 0x02000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x04000000, 0x04000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x08000000, 0x08000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x10000000, 0x10000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x20000000, 0x20000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x40000000, 0x40000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x80000000, 0x80000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME(     0x00000001, 0x00000001, "MAH2" )
	PORT_DIPSETTING(  0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_DIPNAME(     0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000100, 0x00000100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000100, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000200, 0x00000200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000200, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000400, 0x00000400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000800, 0x00000800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000800, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00001000, 0x00001000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00001000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00002000, 0x00002000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00002000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00004000, 0x00004000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00004000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00008000, 0x00008000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00008000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00010000, 0x00010000, "MAH3" )
	PORT_DIPSETTING(  0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x01000000, 0x01000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x02000000, 0x02000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x04000000, 0x04000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x08000000, 0x08000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x10000000, 0x10000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x20000000, 0x20000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x40000000, 0x40000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x80000000, 0x80000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME(     0x00000001, 0x00000001, "MAH4" )
	PORT_DIPSETTING(  0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_DIPNAME(     0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000100, 0x00000100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000100, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000200, 0x00000200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000200, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000400, 0x00000400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000800, 0x00000800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000800, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00001000, 0x00001000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00001000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00002000, 0x00002000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00002000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00004000, 0x00004000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00004000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00008000, 0x00008000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00008000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00010000, 0x00010000, "MAH5" )
	PORT_DIPSETTING(  0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x01000000, 0x01000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x02000000, 0x02000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x04000000, 0x04000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x08000000, 0x08000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x10000000, 0x10000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x20000000, 0x20000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x40000000, 0x40000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x80000000, 0x80000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME(     0x00000001, 0x00000001, "MAH6" )
	PORT_DIPSETTING(  0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_DIPNAME(     0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000100, 0x00000100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000100, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000200, 0x00000200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000200, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000400, 0x00000400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000800, 0x00000800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000800, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00001000, 0x00001000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00001000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00002000, 0x00002000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00002000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00004000, 0x00004000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00004000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00008000, 0x00008000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00008000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00010000, 0x00010000, "MAH7" )
	PORT_DIPSETTING(  0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x01000000, 0x01000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x02000000, 0x02000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x04000000, 0x04000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x08000000, 0x08000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x10000000, 0x10000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x20000000, 0x20000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x40000000, 0x40000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x80000000, 0x80000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_START2 )
	/* The follow 4 bits active 4 button each one for the second player */
	PORT_DIPNAME(     0x00000002, 0x00000002, "P2: A,B,C,D" )
	PORT_DIPSETTING(  0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000004, 0x00000004, "P2: E,F,G,H" )
	PORT_DIPSETTING(  0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000008, 0x00000008, "P2: M,N,Pon,Chie" )
	PORT_DIPSETTING(  0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000010, 0x00000010, "P2: I,J,K,L" )
	PORT_DIPSETTING(  0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000020, 0x00000020, "P2: Kan,Reach,Ron" )
	PORT_DIPSETTING(  0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000100, 0x00000100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000100, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000200, 0x00000200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000200, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000400, 0x00000400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000800, 0x00000800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000800, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00001000, 0x00001000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00001000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00002000, 0x00002000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00002000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00004000, 0x00004000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00004000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00008000, 0x00008000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00008000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME(     0x00020000, 0x00020000, "MAH9" )
	PORT_DIPSETTING(  0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_DIPNAME(     0x00080000, 0x00080000, "Service Mode ? 2" )
	PORT_DIPSETTING(  0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x01000000, 0x01000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x02000000, 0x02000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x04000000, 0x04000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x08000000, 0x08000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x10000000, 0x10000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x20000000, 0x20000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x40000000, 0x40000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x80000000, 0x80000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )

	PORT_START("IN5")
	PORT_DIPNAME(     0x00000001, 0x00000001, "Test Mode" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(  0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000002, 0x00000002, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(  0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x0000001c, 0x0000001c, "First Point" ) PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(  0x0000000c, "5000"  )
	PORT_DIPSETTING(  0x00000014, "10000" )
	PORT_DIPSETTING(  0x00000004, "15000" )
	PORT_DIPSETTING(  0x0000001c, "20000" )
	PORT_DIPSETTING(  0x00000018, "23000" )
	PORT_DIPSETTING(  0x00000008, "25000" )
	PORT_DIPSETTING(  0x00000010, "26000" )
	PORT_DIPSETTING(  0x00000000, "30000" )
	PORT_DIPNAME(     0x000000e0, 0x000000e0, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(  0x00000000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(  0x00000080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(  0x00000040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(  0x000000c0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(  0x000000e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(  0x00000060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(  0x000000a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(  0x00000020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME(     0x00000100, 0x00000100, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(  0x00000100, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000200, 0x00000200, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(  0x00000200, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000400, 0x00000400, "Taisen Only" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(  0x00000400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000800, 0x00000800, "Tumo Pinfu" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(  0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000800, DEF_STR( On ) )
	PORT_DIPNAME(     0x00001000, 0x00001000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(  0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00001000, DEF_STR( On ) )
	PORT_DIPNAME(     0x0000e000, 0x0000e000, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(  0x00000000, DEF_STR( Easiest ) )
	PORT_DIPSETTING(  0x00008000, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(  0x00004000, DEF_STR( Easier ) )
	PORT_DIPSETTING(  0x0000c000, DEF_STR( Easy ) )
	PORT_DIPSETTING(  0x0000e000, DEF_STR( Normal ) )
	PORT_DIPSETTING(  0x00006000, DEF_STR( Hard ) )
	PORT_DIPSETTING(  0x0000a000, DEF_STR( Harder ) )
	PORT_DIPSETTING(  0x00002000, DEF_STR( Hardest ) )

	PORT_DIPNAME(     0x00010000, 0x00010000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(  0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00020000, 0x00020000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(  0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00040000, 0x00040000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(  0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00080000, 0x00080000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(  0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00100000, 0x00100000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(  0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00200000, 0x00200000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(  0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00400000, 0x00400000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(  0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00800000, 0x00800000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(  0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x01000000, 0x01000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x02000000, 0x02000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x04000000, 0x04000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x08000000, 0x08000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x10000000, 0x10000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x20000000, 0x20000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x40000000, 0x40000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x80000000, 0x80000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )

	PORT_START("IN6")
	PORT_DIPNAME(     0x00000001, 0x00000001, "4" )
	PORT_DIPSETTING(  0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000002, 0x00000002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000040, 0x00000040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000040, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000080, 0x00000080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000080, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000100, 0x00000100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000100, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000200, 0x00000200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000200, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000400, 0x00000400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000800, 0x00000800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00000800, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00001000, 0x00001000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00001000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00002000, 0x00002000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00002000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00004000, 0x00004000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00004000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00008000, 0x00008000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00008000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00010000, 0x00010000, "5" )
	PORT_DIPSETTING(  0x00010000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00020000, 0x00020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00020000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00040000, 0x00040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00040000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00080000, 0x00080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00080000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00100000, 0x00100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00100000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00200000, 0x00200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00200000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00400000, 0x00400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00800000, 0x00800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x00800000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x01000000, 0x01000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x02000000, 0x02000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x04000000, 0x04000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x08000000, 0x08000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x10000000, 0x10000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x10000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x20000000, 0x20000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x20000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x40000000, 0x40000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x40000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x80000000, 0x80000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(  0x80000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
INPUT_PORTS_END



/* sprites are contained in 256x256 "tiles" */
static GFXLAYOUT_RAW( spritelayout, 8, 256, 256, 256*8, 256*256*8 )
static GFXLAYOUT_RAW( bglayout, 8, 16, 16, 16*8, 16*16*8 )
static GFXLAYOUT_RAW( txlayout, 8, 8, 8, 8*8, 8*8*8 )

static GFXDECODE_START( bnstars )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0x0000, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0, bglayout,     0x5000, 0x10 ) /* Roz scr1 */
	GFXDECODE_ENTRY( "gfx4", 0, bglayout,     0x1000, 0x10 ) /* Bg scr1 */
	GFXDECODE_ENTRY( "gfx5", 0, txlayout,     0x6000, 0x10 ) /* Tx scr1 */

	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0x8000+0x0000, 0x10 )
	GFXDECODE_ENTRY( "gfx3", 0, bglayout,     0x8000+0x5000, 0x10 ) /* Roz scr2 */
	GFXDECODE_ENTRY( "gfx6", 0, bglayout,     0x8000+0x1000, 0x10 ) /* Bg scr2 */
	GFXDECODE_ENTRY( "gfx7", 0, txlayout,     0x8000+0x6000, 0x10 ) /* Tx scr2 */

GFXDECODE_END

static READ32_HANDLER( bnstars1_r )
{
	bnstars_state *state = space->machine().driver_data<bnstars_state>();
	switch (state->m_bnstars1_mahjong_select & 0x2080)
	{
		default:
			printf("unk bnstars1_r %08x\n",state->m_bnstars1_mahjong_select);
			return 0xffffffff;

		case 0x0000:
			return input_port_read(space->machine(), "IN0");

		case 0x0080:
			return input_port_read(space->machine(), "IN1");

		case 0x2000:
			return input_port_read(space->machine(), "IN2");

		case 0x2080:
			return input_port_read(space->machine(), "IN3");

	}
}

static READ32_HANDLER( bnstars2_r )
{
	return input_port_read(space->machine(), "IN4");
}

static READ32_HANDLER( bnstars3_r )
{
	return input_port_read(space->machine(), "IN5");
}

static WRITE32_HANDLER( bnstars1_mahjong_select_w )
{
	bnstars_state *state = space->machine().driver_data<bnstars_state>();
	state->m_bnstars1_mahjong_select = data;
//  printf("%08x\n",state->m_bnstars1_mahjong_select);
}

static ADDRESS_MAP_START( bnstars_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM

	AM_RANGE(0xfcc00004, 0xfcc00007) AM_READ( bnstars1_r )
	AM_RANGE(0xfcc00008, 0xfcc0000b) AM_READ( bnstars2_r )
	AM_RANGE(0xfcc00010, 0xfcc00013) AM_READ( bnstars3_r )

	AM_RANGE(0xfce00034, 0xfce00037) AM_WRITENOP

	AM_RANGE(0xfce00050, 0xfce00053) AM_WRITENOP
	AM_RANGE(0xfce00058, 0xfce0005b) AM_WRITENOP
	AM_RANGE(0xfce0005c, 0xfce0005f) AM_WRITENOP

	AM_RANGE(0xfce00400, 0xfce0045f) AM_WRITEONLY AM_BASE_MEMBER(bnstars_state, m_ms32_roz_ctrl[0])
	AM_RANGE(0xfce00700, 0xfce0075f) AM_WRITEONLY AM_BASE_MEMBER(bnstars_state, m_ms32_roz_ctrl[1]) // guess
	AM_RANGE(0xfce00a00, 0xfce00a17) AM_WRITEONLY AM_BASE_MEMBER(bnstars_state, m_ms32_tx0_scroll)
	AM_RANGE(0xfce00a20, 0xfce00a37) AM_WRITEONLY AM_BASE_MEMBER(bnstars_state, m_ms32_bg0_scroll)
	AM_RANGE(0xfce00c00, 0xfce00c17) AM_WRITEONLY AM_BASE_MEMBER(bnstars_state, m_ms32_tx1_scroll)
	AM_RANGE(0xfce00c20, 0xfce00c37) AM_WRITEONLY AM_BASE_MEMBER(bnstars_state, m_ms32_bg1_scroll)

	AM_RANGE(0xfce00e00, 0xfce00e03) AM_WRITE(bnstars1_mahjong_select_w) // ?

	/* wrote together */
	AM_RANGE(0xfd040000, 0xfd047fff) AM_RAM // priority ram
	AM_RANGE(0xfd080000, 0xfd087fff) AM_RAM
	AM_RANGE(0xfd200000, 0xfd237fff) AM_RAM_WRITE(ms32_pal1_ram_w) AM_BASE_MEMBER(bnstars_state, m_ms32_pal_ram[1])
	AM_RANGE(0xfd400000, 0xfd437fff) AM_RAM_WRITE(ms32_pal0_ram_w) AM_BASE_MEMBER(bnstars_state, m_ms32_pal_ram[0])
	AM_RANGE(0xfe000000, 0xfe01ffff) AM_RAM_WRITE(ms32_roz1_ram_w) AM_BASE_MEMBER(bnstars_state, m_ms32_roz1_ram)
	AM_RANGE(0xfe400000, 0xfe41ffff) AM_RAM_WRITE(ms32_roz0_ram_w) AM_BASE_MEMBER(bnstars_state, m_ms32_roz0_ram)
	AM_RANGE(0xfe800000, 0xfe83ffff) AM_RAM_WRITE(ms32_spramx_w) AM_BASE_MEMBER(bnstars_state, m_ms32_spram)
	AM_RANGE(0xfea00000, 0xfea07fff) AM_RAM_WRITE(ms32_tx1_ram_w) AM_BASE_MEMBER(bnstars_state, m_ms32_tx1_ram)
	AM_RANGE(0xfea08000, 0xfea0ffff) AM_RAM_WRITE(ms32_bg1_ram_w) AM_BASE_MEMBER(bnstars_state, m_ms32_bg1_ram)
	AM_RANGE(0xfec00000, 0xfec07fff) AM_RAM_WRITE(ms32_tx0_ram_w) AM_BASE_MEMBER(bnstars_state, m_ms32_tx0_ram)
	AM_RANGE(0xfec08000, 0xfec0ffff) AM_RAM_WRITE(ms32_bg0_ram_w) AM_BASE_MEMBER(bnstars_state, m_ms32_bg0_ram)

	AM_RANGE(0xfee00000, 0xfee1ffff) AM_RAM
	AM_RANGE(0xffe00000, 0xffffffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

#if 0
static ADDRESS_MAP_START( bnstars_z80_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END
#endif


static IRQ_CALLBACK(irq_callback)
{
	bnstars_state *state = device->machine().driver_data<bnstars_state>();
	int i;
	for(i=15; i>=0 && !(state->m_irqreq & (1<<i)); i--);
	state->m_irqreq &= ~(1<<i);
	if(!state->m_irqreq)
		device_set_input_line(device, 0, CLEAR_LINE);
	return i;
}

static void irq_init(running_machine &machine)
{
	bnstars_state *state = machine.driver_data<bnstars_state>();
	state->m_irqreq = 0;
	cputag_set_input_line(machine, "maincpu", 0, CLEAR_LINE);
	device_set_irq_callback(machine.device("maincpu"), irq_callback);
}

static void irq_raise(running_machine &machine, int level)
{
	bnstars_state *state = machine.driver_data<bnstars_state>();
	state->m_irqreq |= (1<<level);
	cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);
}

/* TODO: fix this arrangement (derived from old deprecat lib) */
static TIMER_DEVICE_CALLBACK(ms32_interrupt)
{
	int scanline = param;
	if( scanline == 0 ) irq_raise(timer.machine(), 10);
	if( scanline == 8)  irq_raise(timer.machine(), 9);
	/* hayaosi1 needs at least 12 IRQ 0 per frame to work (see code at FFE02289)
       kirarast needs it too, at least 8 per frame, but waits for a variable amount
       47pi2 needs ?? per frame (otherwise it hangs when you lose)
       in different points. Could this be a raster interrupt?
       Other games using it but not needing it to work:
       desertwr
       p47aces
       */
	if( (scanline % 8) == 0 && scanline <= 224 ) irq_raise(timer.machine(), 0);
}

static MACHINE_RESET( ms32 )
{
	irq_init(machine);
}


static MACHINE_CONFIG_START( bnstars, bnstars_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V70, 20000000) // 20MHz
	MCFG_CPU_PROGRAM_MAP(bnstars_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", ms32_interrupt, "lscreen", 0, 1)

//  MCFG_CPU_ADD("audiocpu", Z80, 4000000)
//  MCFG_CPU_PROGRAM_MAP(bnstars_z80_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60000))

	MCFG_MACHINE_RESET(ms32)

	MCFG_GFXDECODE(bnstars)
	MCFG_PALETTE_LENGTH(0x8000*2)

	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE(bnstars)

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE(bnstars)

	MCFG_VIDEO_START(bnstars)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymf1", YMF271, 16934400)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("ymf2", YMF271, 16934400)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

MACHINE_CONFIG_END


ROM_START( bnstars1 )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mb-93142.36", 0x000003, 0x80000, CRC(2eb6a503) SHA1(27c02ab1b4321924fd4499844467ea4dc97de25d) )
	ROM_LOAD32_BYTE( "mb-93142.37", 0x000002, 0x80000, CRC(49f60882) SHA1(2ff5b0989aaf970103304a453773e0b9517ebb8d) )
	ROM_LOAD32_BYTE( "mb-93142.38", 0x000001, 0x80000, CRC(6e1312cd) SHA1(4c22f8f9f1574eefd96147453cf240f50c17f5dc) )
	ROM_LOAD32_BYTE( "mb-93142.39", 0x000000, 0x80000, CRC(56b98539) SHA1(5eb0e77729b31e6a100c1b43813a39fea57bedee) )

	/* Sprites - shared by both screens? */
	ROM_REGION( 0x1000000, "gfx1", 0 ) /* sprites, don't dispose since we use GFX_RAW */
	ROM_LOAD32_WORD( "mr96004-01.20",   0x0000000, 0x200000, CRC(3366d104) SHA1(2de0cabe2ead777b5b02cade7f2003ef7f90b75b) )
	ROM_LOAD32_WORD( "mr96004-02.28",   0x0000002, 0x200000, CRC(ad556664) SHA1(4b36f8d8d9efa37cf515af41d14433e7eafa27a2) )
	ROM_LOAD32_WORD( "mr96004-03.21",   0x0400000, 0x200000, CRC(b399e2b1) SHA1(9b6a00a219db8d66dcf592160b7b5f7a86b8f0c9) )
	ROM_LOAD32_WORD( "mr96004-04.29",   0x0400002, 0x200000, CRC(f4f4cf4a) SHA1(fe497989cf96c68602f68f14920aed44fd934573) )
	ROM_LOAD32_WORD( "mr96004-05.22",   0x0800000, 0x200000, CRC(cd6c357e) SHA1(44cd2d0607c7ccd80f701cf1675fd283acb07252) )
	ROM_LOAD32_WORD( "mr96004-06.30",   0x0800002, 0x200000, CRC(fc6daad7) SHA1(99f14ac6b06ad9a8a3d2e9f69b693c7ce420a47d) )
	ROM_LOAD32_WORD( "mr96004-07.23",   0x0c00000, 0x200000, CRC(177e32fa) SHA1(3ca1f397dc28f1fa3a4136705b92c63e4e438f05) )
	ROM_LOAD32_WORD( "mr96004-08.31",   0x0c00002, 0x200000, CRC(f6df27b2) SHA1(60590976020d86bdccd4eaf57b349ea31bec6830) )

	/* Roz Tiles #1 (Screen 1) */
	ROM_REGION( 0x400000, "gfx2", 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr96004-09.1", 0x000000, 0x400000, CRC(7f8ea9f0) SHA1(f1fe682dcb884f1aa4a5536e17ab94157a99f519) )

	/* Roz Tiles #2 (Screen 2) */
	ROM_REGION( 0x400000, "gfx3", 0 ) /* roz tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr96004-09.7", 0x000000, 0x400000, CRC(7f8ea9f0) SHA1(f1fe682dcb884f1aa4a5536e17ab94157a99f519) )

	/* BG Tiles #1 (Screen 1?) */
	ROM_REGION( 0x200000, "gfx4", 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr96004-11.11", 0x000000, 0x200000,  CRC(e6da552c) SHA1(69a5af3015883793c7d1343243ccae23db9ef77c) )

	/* TX Tiles #1 (Screen 1?) */
	ROM_REGION( 0x080000, "gfx5", 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "vsjanshi6.5", 0x000000, 0x080000, CRC(fdbbac21) SHA1(c77d852e53126cc8ebfe1e79d1134e42b54d1aab) )

	/* BG Tiles #2 (Screen 2?) */
	ROM_REGION( 0x200000, "gfx6", 0 ) /* bg tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "mr96004-11.13", 0x000000, 0x200000, CRC(e6da552c) SHA1(69a5af3015883793c7d1343243ccae23db9ef77c) )

	/* TX Tiles #2 (Screen 2?) */
	ROM_REGION( 0x080000, "gfx7", 0 ) /* tx tiles, don't dispose since we use GFX_RAW */
	ROM_LOAD( "vsjanshi5.6", 0x000000, 0x080000, CRC(fdbbac21) SHA1(c77d852e53126cc8ebfe1e79d1134e42b54d1aab) )

	/* Sound Program (one, driving both screen sound) */
	ROM_REGION( 0x50000, "cpu1", 0 ) /* z80 program */
	ROM_LOAD( "sb93145.5",  0x000000, 0x040000, CRC(0424e899) SHA1(fbcdebfa3d5f52b10cf30f7e416f5f53994e4d55) )
	ROM_RELOAD(              0x010000, 0x40000 )

	/* Samples #1 (Screen 1?) */
	ROM_REGION( 0x400000, "ymf1", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr96004-10.1",  0x000000, 0x400000, CRC(83f4303a) SHA1(90ee010591afe1d35744925ef0e8d9a7e2ef3378) )

	/* Samples #2 (Screen 2?) */
	ROM_REGION( 0x400000, "ymf2", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr96004-10.1",  0x000000, 0x400000, CRC(83f4303a) SHA1(90ee010591afe1d35744925ef0e8d9a7e2ef3378) )
ROM_END


/* SS92046_01: bbbxing, f1superb, tetrisp, hayaosi1 */
static DRIVER_INIT (bnstars)
{
	ms32_rearrange_sprites(machine, "gfx1");

	decrypt_ms32_tx(machine, 0x00020,0x7e, "gfx5");
	decrypt_ms32_bg(machine, 0x00001,0x9b, "gfx4");
	decrypt_ms32_tx(machine, 0x00020,0x7e, "gfx7");
	decrypt_ms32_bg(machine, 0x00001,0x9b, "gfx6");

	memory_set_bankptr(machine, "bank1", machine.region("maincpu")->base());
}

GAME( 1997, bnstars1, 0,        bnstars, bnstars, bnstars, ROT0,   "Jaleco", "Vs. Janshi Brandnew Stars", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND )
