/**************************************************************************

                            Ginga NinkyouDen
                            (C) 1987 Jaleco

                    driver by Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q       shows background
        W       shows foreground
        E       shows frontmost (text) layer
        A       shows sprites

        Keys can be used togheter!


[Screen]
    Visible Size:       256H x 240V
    Dynamic Colors:     256 x 4
    Color Space:        16R x 16G x 16B

[Scrolling layers]
    Format (all layers):    Offset:     0x400    0x000
                            Bit:        fedc---- --------   Color
                                        ----ba98 76543210   Code

    [Background]
        Size:               8192 x 512  (static: stored in ROM)
        Scrolling:          X,Y         (registers: $60006.w, $60004.w)
        Tiles Size:         16 x 16
        Tiles Number:       $400
        Colors:             $300-$3ff

    [Foreground]
        Size:               4096 x 512
        Scrolling:          X,Y         (registers: $60002.w, $60000.w)
        Tiles Size:         16 x 16
        Tiles Number:       $400
        Colors:             $200-$2ff

    [Frontmost]
        Size:               256 x 256
        Scrolling:          -
        Tiles Size:         8 x 8
        Tiles Number:       $200
        Colors:             $000-$0ff


[Sprites]
    On Screen:          256
    In ROM:             $a00
    Colors:             $100-$1ff
    Format:             See Below


**************************************************************************/

#include "driver.h"

/* Variables only used here */
static tilemap *bg_tilemap, *fg_tilemap, *tx_tilemap;
static int layers_ctrl, flipscreen;

/* Variables that driver has access to */
UINT16 *ginganin_fgram16, *ginganin_txtram16, *ginganin_vregs16;

/* Variables defined in drivers */


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/


/* Background - Resides in ROM */

#define BG_GFX (0)
#define BG_NX  (16*32)
#define BG_NY  (16*2)

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = memory_region(REGION_GFX5)[2*tile_index + 0] * 256 + memory_region(REGION_GFX5)[2*tile_index + 1];
	SET_TILE_INFO(
			BG_GFX,
			code,
			code >> 12,
			0);
}


/* Foreground - Resides in RAM */

#define FG_GFX (1)
#define FG_NX  (16*16)
#define FG_NY  (16*2)

static TILE_GET_INFO( get_fg_tile_info )
{
	UINT16 code = ginganin_fgram16[tile_index];
	SET_TILE_INFO(
			FG_GFX,
			code,
			code >> 12,
			0);
}

WRITE16_HANDLER( ginganin_fgram16_w )
{
	COMBINE_DATA(&ginganin_fgram16[offset]);
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}


/* Frontmost (text) Layer - Resides in RAM */

#define TXT_GFX (2)
#define TXT_NX	(32)
#define TXT_NY	(32)

static TILE_GET_INFO( get_txt_tile_info )
{
	UINT16 code = ginganin_txtram16[tile_index];
	SET_TILE_INFO(
			TXT_GFX,
			code,
			code >> 12,
			0);
}

WRITE16_HANDLER( ginganin_txtram16_w )
{
	COMBINE_DATA(&ginganin_txtram16[offset]);
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}


VIDEO_START( ginganin )
{
	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,16,16,BG_NX,BG_NY);
	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,16,16,FG_NX,FG_NY);
	tx_tilemap = tilemap_create(get_txt_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,TXT_NX,TXT_NY);

	tilemap_set_transparent_pen(fg_tilemap,15);
	tilemap_set_transparent_pen(tx_tilemap,15);
}


WRITE16_HANDLER( ginganin_vregs16_w )
{
	COMBINE_DATA(&ginganin_vregs16[offset]);
	data = ginganin_vregs16[offset];

	switch (offset)
	{
	case 0:
		tilemap_set_scrolly(fg_tilemap, 0, data);
		break;
	case 1:
		tilemap_set_scrollx(fg_tilemap, 0, data);
		break;
	case 2:
		tilemap_set_scrolly(bg_tilemap, 0, data);
		break;
	case 3:
		tilemap_set_scrollx(bg_tilemap, 0, data);
		break;
	case 4:
		layers_ctrl = data;
		break;
/*  case 5:
 *      break;
 */
	case 6:
		flipscreen = !(data & 1);
		tilemap_set_flip(ALL_TILEMAPS,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		break;
	case 7:
		soundlatch_w(0,data);
		cpunum_set_input_line(1, INPUT_LINE_NMI, PULSE_LINE);
		break;
	default:
		logerror("CPU #0 PC %06X : Warning, videoreg %04X <- %04X\n",activecpu_get_pc(),offset,data);
	}
}



/* --------------------------[ Sprites Format ]----------------------------

Offset:         Values:         Format:

0000.w          y position      fedc ba9- ---- ----     unused
                                ---- ---8 ---- ----     subtract 256
                                ---- ---- 7654 3210     position

0002.w          x position      See above

0004.w          code            f--- ---- ---- ----     y flip
                                -e-- ---- ---- ----     x flip
                                --dc ---- ---- ----     unused?
                                ---- ba98 7654 3210     code

0006.w          colour          fedc ---- ---- ----     colour code
                                ---- ba98 7654 3210     unused?

------------------------------------------------------------------------ */

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
int offs;

	for ( offs = 0 ; offs < (spriteram_size >> 1); offs += 4 )
	{
		int y		=	spriteram16[offs + 0];
		int x		=	spriteram16[offs + 1];
		int code	=	spriteram16[offs + 2];
		int attr	=	spriteram16[offs + 3];
		int flipx	=	code & 0x4000;
		int flipy	=	code & 0x8000;

		x = (x & 0xFF) - (x & 0x100);
		y = (y & 0xFF) - (y & 0x100);

		if (flipscreen)
		{
			x = 240 - x;		y = 240 - y;
			flipx = !flipx; 	flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[3],
				code & 0x3fff,
				attr >> 12,
				flipx, flipy,
				x,y,
				cliprect,TRANSPARENCY_PEN,15);

	}
}


VIDEO_UPDATE( ginganin )
{
	int layers_ctrl1;

	layers_ctrl1 = layers_ctrl;

#ifdef MAME_DEBUG
if (input_code_pressed(KEYCODE_Z))
{
	int msk = 0;
	static int posx,posy;

	if (input_code_pressed(KEYCODE_Q)) { msk |= 0xfff1;}
	if (input_code_pressed(KEYCODE_W)) { msk |= 0xfff2;}
	if (input_code_pressed(KEYCODE_E)) { msk |= 0xfff4;}
	if (input_code_pressed(KEYCODE_A))	{ msk |= 0xfff8;}
	if (msk != 0) layers_ctrl1 &= msk;

#define SETSCROLL \
	tilemap_set_scrollx(bg_tilemap, 0, posx); \
	tilemap_set_scrolly(bg_tilemap, 0, posy); \
	tilemap_set_scrollx(fg_tilemap, 0, posx); \
	tilemap_set_scrolly(fg_tilemap, 0, posy); \
	popmessage("B>%04X:%04X F>%04X:%04X",posx%(BG_NX*16),posy%(BG_NY*16),posx%(FG_NX*16),posy%(FG_NY*16));

	if (input_code_pressed(KEYCODE_L))	{ posx +=8; SETSCROLL }
	if (input_code_pressed(KEYCODE_J))	{ posx -=8; SETSCROLL }
	if (input_code_pressed(KEYCODE_K))	{ posy +=8; SETSCROLL }
	if (input_code_pressed(KEYCODE_I))	{ posy -=8; SETSCROLL }
	if (input_code_pressed(KEYCODE_H))	{ posx = posy = 0;	SETSCROLL }

}
#endif


	if (layers_ctrl1 & 1)	tilemap_draw(bitmap,cliprect, bg_tilemap,  0,0);
	else					fillbitmap(bitmap,machine->pens[0],cliprect);

	if (layers_ctrl1 & 2)	tilemap_draw(bitmap,cliprect, fg_tilemap,  0,0);
	if (layers_ctrl1 & 8)	draw_sprites(machine, bitmap,cliprect);
	if (layers_ctrl1 & 4)	tilemap_draw(bitmap,cliprect, tx_tilemap, 0,0);

	return 0;
}

