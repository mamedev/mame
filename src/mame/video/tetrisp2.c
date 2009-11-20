/***************************************************************************

                              -= Tetris Plus 2 =-

                    driver by   Luca Elia (l.elia@tin.it)


    [ 2 Scrolling Layers ]

    The Background is a 64 x 64 Tilemap with 16 x 16 x 8 tiles (1024 x 1024).
    The Foreground is a 64 x 64 Tilemap with 8 x 8 x 8 tiles (512 x 512).
    Each tile needs 4 bytes.

    [ 1024? Sprites ]

    Sprites are "nearly" tile based: if the data in the ROMs is decoded
    as 8x8 tiles, and each 32 consecutive tiles are drawn like a column
    of 256 pixels, it turns out that every 32 x 32 tiles form a picture
    of 256 x 256 pixels (a "page").

    Sprites are portions of any size from those page's graphics rendered
    to screen.

To Do:

-   There is a 3rd unimplemented layer capable of rotation (not used by
    the game, can be tested in service mode).
-   Priority RAM is not properly taken into account.
-   Can the Tetris Plus 2 sprites zoom, or is this an MS32 only feature?

***************************************************************************/

#include "driver.h"
#include "includes/tetrisp2.h"


/* Variables needed by driver: */

UINT16 *tetrisp2_vram_bg, *tetrisp2_scroll_bg;
UINT16 *tetrisp2_vram_fg, *tetrisp2_scroll_fg;
UINT16 *tetrisp2_vram_rot, *tetrisp2_rotregs;

UINT8 *tetrisp2_priority;

UINT16 *rocknms_sub_vram_bg, *rocknms_sub_scroll_bg;
UINT16 *rocknms_sub_vram_fg, *rocknms_sub_scroll_fg;
UINT16 *rocknms_sub_vram_rot, *rocknms_sub_rotregs;

UINT16 *rocknms_sub_priority;

static int flipscreen_old;

/***************************************************************************


                                    Palette


***************************************************************************/

/* BBBBBGGGGGRRRRRx xxxxxxxxxxxxxxxx */
WRITE16_HANDLER( tetrisp2_palette_w )
{
	data = COMBINE_DATA(&paletteram16[offset]);
	if ((offset & 1) == 0)
		palette_set_color_rgb(space->machine,offset/2,pal5bit(data >> 1),pal5bit(data >> 6),pal5bit(data >> 11));
}

WRITE16_HANDLER( rocknms_sub_palette_w )
{
	data = COMBINE_DATA(&paletteram16_2[offset]);
	if ((offset & 1) == 0)
		palette_set_color_rgb(space->machine,(0x8000 + (offset/2)),pal5bit(data >> 1),pal5bit(data >> 6),pal5bit(data >> 11));
}



/***************************************************************************


                                    Priority


***************************************************************************/

WRITE8_HANDLER( tetrisp2_priority_w )
{
	//if (ACCESSING_BITS_8_15)
	{
		data |= ((data & 0xff00) >> 8);
		tetrisp2_priority[offset] = data;
	}
}


WRITE8_HANDLER( rockn_priority_w )
{
	//if (ACCESSING_BITS_8_15)
	{
		tetrisp2_priority[offset] = data;
	}
}

WRITE16_HANDLER( rocknms_sub_priority_w )
{
	if (ACCESSING_BITS_8_15)
	{
		rocknms_sub_priority[offset] = data;
	}
}

READ16_HANDLER( nndmseal_priority_r )
{
	return tetrisp2_priority[offset] | 0xff00;
}

READ8_HANDLER( tetrisp2_priority_r )
{
	return tetrisp2_priority[offset];
}

/***************************************************************************


                                    Tilemaps


    Offset:     Bits:                   Value:

        0.w                             Code
        2.w     fedc ba98 7654 ----
                ---- ---- ---- 3210     Color


***************************************************************************/

static tilemap *tilemap_bg, *tilemap_fg, *tilemap_rot;
static tilemap *tilemap_sub_bg, *tilemap_sub_fg, *tilemap_sub_rot;

#define NX_0  (0x40)
#define NY_0  (0x40)

static TILE_GET_INFO( get_tile_info_bg )
{
	UINT16 code_hi = tetrisp2_vram_bg[ 2 * tile_index + 0];
	UINT16 code_lo = tetrisp2_vram_bg[ 2 * tile_index + 1];
	SET_TILE_INFO(
			1,
			code_hi,
			code_lo & 0xf,
			0);
}

WRITE16_HANDLER( tetrisp2_vram_bg_w )
{
	COMBINE_DATA(&tetrisp2_vram_bg[offset]);
	tilemap_mark_tile_dirty(tilemap_bg,offset/2);
}



#define NX_1  (0x40)
#define NY_1  (0x40)

static TILE_GET_INFO( get_tile_info_fg )
{
	UINT16 code_hi = tetrisp2_vram_fg[ 2 * tile_index + 0];
	UINT16 code_lo = tetrisp2_vram_fg[ 2 * tile_index + 1];
	SET_TILE_INFO(
			3,
			code_hi,
			code_lo & 0xf,
			0);
}

WRITE16_HANDLER( tetrisp2_vram_fg_w )
{
	COMBINE_DATA(&tetrisp2_vram_fg[offset]);
	tilemap_mark_tile_dirty(tilemap_fg,offset/2);
}


static TILE_GET_INFO( get_tile_info_rot )
{
	UINT16 code_hi = tetrisp2_vram_rot[ 2 * tile_index + 0];
	UINT16 code_lo = tetrisp2_vram_rot[ 2 * tile_index + 1];
	SET_TILE_INFO(
			2,
			code_hi,
			code_lo & 0xf,
			0);
}

WRITE16_HANDLER( tetrisp2_vram_rot_w )
{
	COMBINE_DATA(&tetrisp2_vram_rot[offset]);
	tilemap_mark_tile_dirty(tilemap_rot,offset/2);
}

static TILE_GET_INFO( get_tile_info_rocknms_sub_bg )
{
	UINT16 code_hi = rocknms_sub_vram_bg[ 2 * tile_index + 0];
	UINT16 code_lo = rocknms_sub_vram_bg[ 2 * tile_index + 1];
	SET_TILE_INFO(
			5,
			code_hi,
			code_lo & 0xf,
			0);
}

WRITE16_HANDLER( rocknms_sub_vram_bg_w )
{
	COMBINE_DATA(&rocknms_sub_vram_bg[offset]);
	tilemap_mark_tile_dirty(tilemap_sub_bg,offset/2);
}


static TILE_GET_INFO( get_tile_info_rocknms_sub_fg )
{
	UINT16 code_hi = rocknms_sub_vram_fg[ 2 * tile_index + 0];
	UINT16 code_lo = rocknms_sub_vram_fg[ 2 * tile_index + 1];
	SET_TILE_INFO(
			7,
			code_hi,
			code_lo & 0xf,
			0);
}

WRITE16_HANDLER( rocknms_sub_vram_fg_w )
{
	COMBINE_DATA(&rocknms_sub_vram_fg[offset]);
	tilemap_mark_tile_dirty(tilemap_sub_fg,offset/2);
}


static TILE_GET_INFO( get_tile_info_rocknms_sub_rot )
{
	UINT16 code_hi = rocknms_sub_vram_rot[ 2 * tile_index + 0];
	UINT16 code_lo = rocknms_sub_vram_rot[ 2 * tile_index + 1];
	SET_TILE_INFO(
			6,
			code_hi,
			code_lo & 0xf,
			0);
}

WRITE16_HANDLER( rocknms_sub_vram_rot_w )
{
	COMBINE_DATA(&rocknms_sub_vram_rot[offset]);
	tilemap_mark_tile_dirty(tilemap_sub_rot,offset/2);
}



extern void ms32_rearrange_sprites(running_machine *machine, const char *region);

VIDEO_START( tetrisp2 )
{
	flipscreen_old = -1;

	tilemap_bg = tilemap_create(	machine, get_tile_info_bg,tilemap_scan_rows,

								16,16,NX_0,NY_0);

	tilemap_fg = tilemap_create(	machine, get_tile_info_fg,tilemap_scan_rows,

								8,8,NX_1,NY_1);

	tilemap_rot = tilemap_create(	machine, get_tile_info_rot,tilemap_scan_rows,

								16,16,NX_0*2,NY_0*2);

	tilemap_set_transparent_pen(tilemap_bg,0);
	tilemap_set_transparent_pen(tilemap_fg,0);
	tilemap_set_transparent_pen(tilemap_rot,0);

	// should be smaller and mirrored like m32 I guess
	tetrisp2_priority = auto_alloc_array(machine, UINT8, 0x40000);
	ms32_rearrange_sprites(machine, "gfx1");
}

VIDEO_START( nndmseal )
{
	VIDEO_START_CALL( tetrisp2 );
	tilemap_set_scrolldx(tilemap_bg, -4,-4);
}

VIDEO_START( rockntread )
{
	flipscreen_old = -1;

	tilemap_bg = tilemap_create(	machine, get_tile_info_bg,tilemap_scan_rows,

								16, 16, 256, 16);	// rockn ms(main),1,2,3,4

	tilemap_fg = tilemap_create(	machine, get_tile_info_fg,tilemap_scan_rows,

								8, 8, 64, 64);

	tilemap_rot = tilemap_create(	machine, get_tile_info_rot,tilemap_scan_rows,

								16, 16, 128, 128);

	tilemap_set_transparent_pen(tilemap_bg, 0);
	tilemap_set_transparent_pen(tilemap_fg, 0);
	tilemap_set_transparent_pen(tilemap_rot, 0);

	// should be smaller and mirrored like m32 I guess
	tetrisp2_priority = auto_alloc_array(machine, UINT8, 0x40000);
	ms32_rearrange_sprites(machine, "gfx1");
}


VIDEO_START( rocknms )
{
	VIDEO_START_CALL( rockntread );

	tilemap_sub_bg = tilemap_create(machine, get_tile_info_rocknms_sub_bg,tilemap_scan_rows,

					16, 16, 32, 256);	// rockn ms(sub)

	tilemap_sub_fg = tilemap_create(machine, get_tile_info_rocknms_sub_fg,tilemap_scan_rows,

					8, 8, 64, 64);

	tilemap_sub_rot = tilemap_create( machine, get_tile_info_rocknms_sub_rot,tilemap_scan_rows,

					16, 16, 128, 128);

	tilemap_set_transparent_pen(tilemap_sub_bg, 0);
	tilemap_set_transparent_pen(tilemap_sub_fg, 0);
	tilemap_set_transparent_pen(tilemap_sub_rot, 0);

	ms32_rearrange_sprites(machine, "gfx5");
}


/***************************************************************************


                                Sprites Drawing

    Offset:     Bits:                   Meaning:

    0.w         fedc ba98 ---- ----
                ---- ---- 7654 ----     Priority
                ---- ---- ---- 3---
                ---- ---- ---- -2--     Draw this sprite
                ---- ---- ---- --1-     Flip Y
                ---- ---- ---- ---0     Flip X

    2.w         fedc ba98 ---- ----     Tile's Y position in the tile page (*)
                ---- ---- 7654 3210     Tile's X position in the tile page (*)

    4.w         fedc ---- ---- ----     Color
                ---- ba98 7--- ----
                ---- ---- -654 3210     Tile Page (32x32 tiles = 256x256 pixels each)

    6.w         fedc ba98 ---- ----
                ---- ---- 7654 ----     Y Size - 1 (*)
                ---- ---- ---- 3210     X Size - 1 (*)

    8.w         fedc ba-- ---- ----
                ---- --98 7654 3210     Y (Signed)

    A.w         fedc b--- ---- ----
                ---- -a98 7654 3210     X (Signed)

    C.w         fedc ba98 7654 3210     ZOOM X (unused)

    E.w         fedc ba98 7654 3210     ZOOM Y (unused)

(*) 1 pixel granularity

***************************************************************************/

/* this is also used by ms32.c */
/* sprites should be able to create shadows too, how?
  -- it appears that sprites which should be shadows are often rendered *UNDER* the tilemaps, maybe related?
*/
void tetrisp2_draw_sprites(running_machine *machine, bitmap_t *bitmap, bitmap_t *bitmap_pri, const rectangle *cliprect, UINT8* priram, UINT16 *sprram_top, size_t sprram_size, int gfxnum, int reverseorder, int flip, int allowzoom)
{
	int tx, ty, sx, sy, flipx, flipy;
	int xsize, ysize;
	int code, attr, color, size;
	int flipscreen;
	int pri;
	int xzoom, yzoom;
	UINT32 primask;
	UINT8 *priority_ram;
	gfx_element *gfx = machine->gfx[gfxnum];

	UINT16		*source	=	sprram_top;
	UINT16	*finish	=	sprram_top + (sprram_size - 0x10) / 2;

	flipscreen = flip;

	if (reverseorder == 1)
	{
		source	= sprram_top + (sprram_size - 0x10) / 2;
		finish	= sprram_top;
	}

	priority_ram = priram;


	for (;reverseorder ? (source>=finish) : (source<finish); reverseorder ? (source-=8) : (source+=8))
	{
		attr	=	source[ 0 ];

		pri = (attr & 0x00f0);

		if ((attr & 0x0004) == 0)			continue;

		flipx	=	attr & 1;
		flipy	=	attr & 2;

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

		// tetrisp2 hardware doesn't work with zoom?
		if (allowzoom)
		{
			if (!yzoom || !xzoom)
				continue;

			yzoom = 0x1000000/yzoom;
			xzoom = 0x1000000/xzoom;
		}
		else
		{
			xzoom = 0x10000;
			yzoom = 0x10000;
		}


		gfx_element_set_source_clip(gfx, tx, xsize, ty, ysize);

		if (priority_ram == NULL)
		{
			// passes the priority as the upper bits of the colour
			// for post-processing in mixer instead
			pdrawgfxzoom_transpen_raw(bitmap, cliprect, gfx,
					code,
					color<<8 | pri<<8,
					flipx, flipy,
					sx,sy,
					xzoom, yzoom, bitmap_pri,0, 0);
		}
		else
		{
			primask = 0;
			if (priority_ram[(pri | 0x0a00 | 0x1500) / 2] & 0x38) primask |= 1 << 0;
			if (priority_ram[(pri | 0x0a00 | 0x1400) / 2] & 0x38) primask |= 1 << 1;
			if (priority_ram[(pri | 0x0a00 | 0x1100) / 2] & 0x38) primask |= 1 << 2;
			if (priority_ram[(pri | 0x0a00 | 0x1000) / 2] & 0x38) primask |= 1 << 3;
			if (priority_ram[(pri | 0x0a00 | 0x0500) / 2] & 0x38) primask |= 1 << 4;
			if (priority_ram[(pri | 0x0a00 | 0x0400) / 2] & 0x38) primask |= 1 << 5;
			if (priority_ram[(pri | 0x0a00 | 0x0100) / 2] & 0x38) primask |= 1 << 6;
			if (priority_ram[(pri | 0x0a00 | 0x0000) / 2] & 0x38) primask |= 1 << 7;


			pdrawgfxzoom_transpen(bitmap, cliprect, gfx,
					code,
					color,
					flipx, flipy,
					sx,sy,
					xzoom, yzoom, bitmap_pri,primask, 0);

		}


	}	/* end sprite loop */
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

VIDEO_UPDATE( tetrisp2 )
{
	int flipscreen;
	int asc_pri;
	int scr_pri;
	int rot_pri;
	int rot_ofsx, rot_ofsy;

	flipscreen = (tetrisp2_systemregs[0x00] & 0x02);

	/* Black background color */
	bitmap_fill(bitmap, cliprect, 0);
	bitmap_fill(screen->machine->priority_bitmap, NULL, 0);

	/* Flip Screen */
	if (flipscreen != flipscreen_old)
	{
		flipscreen_old = flipscreen;
		tilemap_set_flip_all(screen->machine, flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	}

	/* Flip Screen */
	if (flipscreen)
	{
		rot_ofsx = 0x053f;
		rot_ofsy = 0x04df;
	}
	else
	{
		rot_ofsx = 0x400;
		rot_ofsy = 0x400;
	}

	tilemap_set_scrollx(tilemap_bg, 0, (((tetrisp2_scroll_bg[ 0 ] + 0x0014) + tetrisp2_scroll_bg[ 2 ] ) & 0xffff));
	tilemap_set_scrolly(tilemap_bg, 0, (((tetrisp2_scroll_bg[ 3 ] + 0x0000) + tetrisp2_scroll_bg[ 5 ] ) & 0xffff));

	tilemap_set_scrollx(tilemap_fg, 0, tetrisp2_scroll_fg[ 2 ]);
	tilemap_set_scrolly(tilemap_fg, 0, tetrisp2_scroll_fg[ 5 ]);

	tilemap_set_scrollx(tilemap_rot, 0, (tetrisp2_rotregs[ 0 ] - rot_ofsx));
	tilemap_set_scrolly(tilemap_rot, 0, (tetrisp2_rotregs[ 2 ] - rot_ofsy));

	asc_pri = scr_pri = rot_pri = 0;

	if((tetrisp2_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if((tetrisp2_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if((tetrisp2_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		tilemap_draw(bitmap,cliprect, tilemap_rot, 0, 1 << 1);
	else if (scr_pri == 0)
		tilemap_draw(bitmap,cliprect, tilemap_bg,  0, 1 << 0);
	else if (asc_pri == 0)
		tilemap_draw(bitmap,cliprect, tilemap_fg,  0, 1 << 2);

	if (rot_pri == 1)
		tilemap_draw(bitmap,cliprect, tilemap_rot, 0, 1 << 1);
	else if (scr_pri == 1)
		tilemap_draw(bitmap,cliprect, tilemap_bg,  0, 1 << 0);
	else if (asc_pri == 1)
		tilemap_draw(bitmap,cliprect, tilemap_fg,  0, 1 << 2);

	if (rot_pri == 2)
		tilemap_draw(bitmap,cliprect, tilemap_rot, 0, 1 << 1);
	else if (scr_pri == 2)
		tilemap_draw(bitmap,cliprect, tilemap_bg,  0, 1 << 0);
	else if (asc_pri == 2)
		tilemap_draw(bitmap,cliprect, tilemap_fg,  0, 1 << 2);

	tetrisp2_draw_sprites(screen->machine, bitmap,screen->machine->priority_bitmap, cliprect, tetrisp2_priority, spriteram16, spriteram_size, 0, 0, (tetrisp2_systemregs[0x00] & 0x02), 0);
	return 0;
}

VIDEO_UPDATE( rockntread )
{
	int flipscreen;
	int asc_pri;
	int scr_pri;
	int rot_pri;
	int rot_ofsx, rot_ofsy;

	flipscreen = (tetrisp2_systemregs[0x00] & 0x02);

	/* Black background color */
	bitmap_fill(bitmap, cliprect, 0);
	bitmap_fill(screen->machine->priority_bitmap, NULL, 0);

	/* Flip Screen */
	if (flipscreen != flipscreen_old)
	{
		flipscreen_old = flipscreen;
		tilemap_set_flip_all(screen->machine, flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	}

	/* Flip Screen */
	if (flipscreen)
	{
		rot_ofsx = 0x053f;
		rot_ofsy = 0x04df;
	}
	else
	{
		rot_ofsx = 0x400;
		rot_ofsy = 0x400;
	}

	tilemap_set_scrollx(tilemap_bg, 0, (((tetrisp2_scroll_bg[ 0 ] + 0x0014) + tetrisp2_scroll_bg[ 2 ] ) & 0xffff));
	tilemap_set_scrolly(tilemap_bg, 0, (((tetrisp2_scroll_bg[ 3 ] + 0x0000) + tetrisp2_scroll_bg[ 5 ] ) & 0xffff));

	tilemap_set_scrollx(tilemap_fg, 0, tetrisp2_scroll_fg[ 2 ]);
	tilemap_set_scrolly(tilemap_fg, 0, tetrisp2_scroll_fg[ 5 ]);

	tilemap_set_scrollx(tilemap_rot, 0, (tetrisp2_rotregs[ 0 ] - rot_ofsx));
	tilemap_set_scrolly(tilemap_rot, 0, (tetrisp2_rotregs[ 2 ] - rot_ofsy));

	asc_pri = scr_pri = rot_pri = 0;

	if((tetrisp2_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if((tetrisp2_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if((tetrisp2_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		tilemap_draw(bitmap,cliprect, tilemap_rot, 0, 1 << 1);
	else if (scr_pri == 0)
		tilemap_draw(bitmap,cliprect, tilemap_bg,  0, 1 << 0);
	else if (asc_pri == 0)
		tilemap_draw(bitmap,cliprect, tilemap_fg,  0, 1 << 2);

	if (rot_pri == 1)
		tilemap_draw(bitmap,cliprect, tilemap_rot, 0, 1 << 1);
	else if (scr_pri == 1)
		tilemap_draw(bitmap,cliprect, tilemap_bg,  0, 1 << 0);
	else if (asc_pri == 1)
		tilemap_draw(bitmap,cliprect, tilemap_fg,  0, 1 << 2);

	if (rot_pri == 2)
		tilemap_draw(bitmap,cliprect, tilemap_rot, 0, 1 << 1);
	else if (scr_pri == 2)
		tilemap_draw(bitmap,cliprect, tilemap_bg,  0, 1 << 0);
	else if (asc_pri == 2)
		tilemap_draw(bitmap,cliprect, tilemap_fg,  0, 1 << 2);

	tetrisp2_draw_sprites(screen->machine, bitmap,screen->machine->priority_bitmap,cliprect, tetrisp2_priority, spriteram16, spriteram_size, 0, 0, (tetrisp2_systemregs[0x00] & 0x02), 0);
	return 0;
}




VIDEO_UPDATE( rocknms )
{
	int asc_pri;
	int scr_pri;
	int rot_pri;

	const device_config *left_screen  = devtag_get_device(screen->machine, "lscreen");
	const device_config *right_screen = devtag_get_device(screen->machine, "rscreen");

	/* Black background color */
	if (screen == left_screen)
	{
		tilemap_set_scrollx(tilemap_sub_bg, 0, rocknms_sub_scroll_bg[ 2 ] + 0x000);
		tilemap_set_scrolly(tilemap_sub_bg, 0, rocknms_sub_scroll_bg[ 5 ] + 0x000);
		tilemap_set_scrollx(tilemap_sub_fg, 0, rocknms_sub_scroll_fg[ 2 ] + 0x000);
		tilemap_set_scrolly(tilemap_sub_fg, 0, rocknms_sub_scroll_fg[ 5 ] + 0x000);
		tilemap_set_scrollx(tilemap_sub_rot, 0, rocknms_sub_rotregs[ 0 ] + 0x400);
		tilemap_set_scrolly(tilemap_sub_rot, 0, rocknms_sub_rotregs[ 2 ] + 0x400);

		bitmap_fill(bitmap, cliprect, screen->machine->pens[0x0000]);
		bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

		asc_pri = scr_pri = rot_pri = 0;

		if((rocknms_sub_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
			asc_pri++;
		else
			rot_pri++;

		if((rocknms_sub_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
			asc_pri++;
		else
			scr_pri++;

		if((rocknms_sub_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
			scr_pri++;
		else
			rot_pri++;

		if (rot_pri == 0)
			tilemap_draw(bitmap,cliprect, tilemap_sub_rot, 0, 1 << 1);
		else if (scr_pri == 0)
			tilemap_draw(bitmap,cliprect, tilemap_sub_bg,  0, 1 << 0);
		else if (asc_pri == 0)
			tilemap_draw(bitmap,cliprect, tilemap_sub_fg,  0, 1 << 2);

		if (rot_pri == 1)
			tilemap_draw(bitmap,cliprect, tilemap_sub_rot, 0, 1 << 1);
		else if (scr_pri == 1)
			tilemap_draw(bitmap,cliprect, tilemap_sub_bg,  0, 1 << 0);
		else if (asc_pri == 1)
			tilemap_draw(bitmap,cliprect, tilemap_sub_fg,  0, 1 << 2);

		if (rot_pri == 2)
			tilemap_draw(bitmap,cliprect, tilemap_sub_rot, 0, 1 << 1);
		else if (scr_pri == 2)
			tilemap_draw(bitmap,cliprect, tilemap_sub_bg,  0, 1 << 0);
		else if (asc_pri == 2)
			tilemap_draw(bitmap,cliprect, tilemap_sub_fg,  0, 1 << 2);

		tetrisp2_draw_sprites(screen->machine, bitmap,screen->machine->priority_bitmap,cliprect, tetrisp2_priority, spriteram16_2, spriteram_2_size, 4, 0, (tetrisp2_systemregs[0x00] & 0x02), 0);
	}
	else if (screen == right_screen) /* game screen */
	{
		tilemap_set_scrollx(tilemap_bg, 0, tetrisp2_scroll_bg[ 2 ] + 0x000);
		tilemap_set_scrolly(tilemap_bg, 0, tetrisp2_scroll_bg[ 5 ] + 0x000);
		tilemap_set_scrollx(tilemap_fg, 0, tetrisp2_scroll_fg[ 2 ] + 0x000);
		tilemap_set_scrolly(tilemap_fg, 0, tetrisp2_scroll_fg[ 5 ] + 0x000);
		tilemap_set_scrollx(tilemap_rot, 0, tetrisp2_rotregs[ 0 ] + 0x400);
		tilemap_set_scrolly(tilemap_rot, 0, tetrisp2_rotregs[ 2 ] + 0x400);

		/* Black background color */
		bitmap_fill(bitmap, cliprect, screen->machine->pens[0x0000]);
		bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

		asc_pri = scr_pri = rot_pri = 0;

		if((tetrisp2_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
			asc_pri++;
		else
			rot_pri++;

		if((tetrisp2_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
			asc_pri++;
		else
			scr_pri++;

		if((tetrisp2_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
			scr_pri++;
		else
			rot_pri++;

		if (rot_pri == 0)
			tilemap_draw(bitmap,cliprect, tilemap_rot, 0, 1 << 1);
		else if (scr_pri == 0)
			tilemap_draw(bitmap,cliprect, tilemap_bg,  0, 1 << 0);
		else if (asc_pri == 0)
			tilemap_draw(bitmap,cliprect, tilemap_fg,  0, 1 << 2);

		if (rot_pri == 1)
			tilemap_draw(bitmap,cliprect, tilemap_rot, 0, 1 << 1);
		else if (scr_pri == 1)
			tilemap_draw(bitmap,cliprect, tilemap_bg,  0, 1 << 0);
		else if (asc_pri == 1)
			tilemap_draw(bitmap,cliprect, tilemap_fg,  0, 1 << 2);

		if (rot_pri == 2)
			tilemap_draw(bitmap,cliprect, tilemap_rot, 0, 1 << 1);
		else if (scr_pri == 2)
			tilemap_draw(bitmap,cliprect, tilemap_bg,  0, 1 << 0);
		else if (asc_pri == 2)
			tilemap_draw(bitmap,cliprect, tilemap_fg,  0, 1 << 2);

		tetrisp2_draw_sprites(screen->machine, bitmap,screen->machine->priority_bitmap,cliprect, tetrisp2_priority, spriteram16, spriteram_size, 0, 0, (tetrisp2_systemregs[0x00] & 0x02), 0);
	}

	return 0;
}
