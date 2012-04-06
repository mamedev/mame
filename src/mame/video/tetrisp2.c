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

#include "emu.h"
#include "machine/jalcrpt.h"
#include "includes/tetrisp2.h"


/***************************************************************************


                                    Palette


***************************************************************************/

/* BBBBBGGGGGRRRRRx xxxxxxxxxxxxxxxx */
WRITE16_MEMBER(tetrisp2_state::tetrisp2_palette_w)
{
	data = COMBINE_DATA(&m_generic_paletteram_16[offset]);
	if ((offset & 1) == 0)
		palette_set_color_rgb(machine(),offset/2,pal5bit(data >> 1),pal5bit(data >> 6),pal5bit(data >> 11));
}

WRITE16_MEMBER(tetrisp2_state::rocknms_sub_palette_w)
{
	data = COMBINE_DATA(&m_generic_paletteram2_16[offset]);
	if ((offset & 1) == 0)
		palette_set_color_rgb(machine(),(0x8000 + (offset/2)),pal5bit(data >> 1),pal5bit(data >> 6),pal5bit(data >> 11));
}



/***************************************************************************


                                    Priority


***************************************************************************/

WRITE8_MEMBER(tetrisp2_state::tetrisp2_priority_w)
{
	//if (ACCESSING_BITS_8_15)
	{
		data |= ((data & 0xff00) >> 8);
		m_priority[offset] = data;
	}
}


WRITE8_MEMBER(tetrisp2_state::rockn_priority_w)
{
	//if (ACCESSING_BITS_8_15)
	{
		m_priority[offset] = data;
	}
}

WRITE16_MEMBER(tetrisp2_state::rocknms_sub_priority_w)
{
	if (ACCESSING_BITS_8_15)
	{
		m_rocknms_sub_priority[offset] = data;
	}
}

READ16_MEMBER(tetrisp2_state::nndmseal_priority_r)
{
	return m_priority[offset] | 0xff00;
}

READ8_MEMBER(tetrisp2_state::tetrisp2_priority_r)
{
	return m_priority[offset];
}

/***************************************************************************


                                    Tilemaps


    Offset:     Bits:                   Value:

        0.w                             Code
        2.w     fedc ba98 7654 ----
                ---- ---- ---- 3210     Color


***************************************************************************/


#define NX_0  (0x40)
#define NY_0  (0x40)

static TILE_GET_INFO( get_tile_info_bg )
{
	tetrisp2_state *state = machine.driver_data<tetrisp2_state>();
	UINT16 code_hi = state->m_vram_bg[ 2 * tile_index + 0];
	UINT16 code_lo = state->m_vram_bg[ 2 * tile_index + 1];
	SET_TILE_INFO(
			1,
			code_hi,
			code_lo & 0xf,
			0);
}

WRITE16_MEMBER(tetrisp2_state::tetrisp2_vram_bg_w)
{
	COMBINE_DATA(&m_vram_bg[offset]);
	m_tilemap_bg->mark_tile_dirty(offset/2);
}



#define NX_1  (0x40)
#define NY_1  (0x40)

static TILE_GET_INFO( get_tile_info_fg )
{
	tetrisp2_state *state = machine.driver_data<tetrisp2_state>();
	UINT16 code_hi = state->m_vram_fg[ 2 * tile_index + 0];
	UINT16 code_lo = state->m_vram_fg[ 2 * tile_index + 1];
	SET_TILE_INFO(
			3,
			code_hi,
			code_lo & 0xf,
			0);
}

WRITE16_MEMBER(tetrisp2_state::tetrisp2_vram_fg_w)
{
	COMBINE_DATA(&m_vram_fg[offset]);
	m_tilemap_fg->mark_tile_dirty(offset/2);
}


static TILE_GET_INFO( get_tile_info_rot )
{
	tetrisp2_state *state = machine.driver_data<tetrisp2_state>();
	UINT16 code_hi = state->m_vram_rot[ 2 * tile_index + 0];
	UINT16 code_lo = state->m_vram_rot[ 2 * tile_index + 1];
	SET_TILE_INFO(
			2,
			code_hi,
			code_lo & 0xf,
			0);
}

WRITE16_MEMBER(tetrisp2_state::tetrisp2_vram_rot_w)
{
	COMBINE_DATA(&m_vram_rot[offset]);
	m_tilemap_rot->mark_tile_dirty(offset/2);
}

static TILE_GET_INFO( get_tile_info_rocknms_sub_bg )
{
	tetrisp2_state *state = machine.driver_data<tetrisp2_state>();
	UINT16 code_hi = state->m_rocknms_sub_vram_bg[ 2 * tile_index + 0];
	UINT16 code_lo = state->m_rocknms_sub_vram_bg[ 2 * tile_index + 1];
	SET_TILE_INFO(
			5,
			code_hi,
			code_lo & 0xf,
			0);
}

WRITE16_MEMBER(tetrisp2_state::rocknms_sub_vram_bg_w)
{
	COMBINE_DATA(&m_rocknms_sub_vram_bg[offset]);
	m_tilemap_sub_bg->mark_tile_dirty(offset/2);
}


static TILE_GET_INFO( get_tile_info_rocknms_sub_fg )
{
	tetrisp2_state *state = machine.driver_data<tetrisp2_state>();
	UINT16 code_hi = state->m_rocknms_sub_vram_fg[ 2 * tile_index + 0];
	UINT16 code_lo = state->m_rocknms_sub_vram_fg[ 2 * tile_index + 1];
	SET_TILE_INFO(
			7,
			code_hi,
			code_lo & 0xf,
			0);
}

WRITE16_MEMBER(tetrisp2_state::rocknms_sub_vram_fg_w)
{
	COMBINE_DATA(&m_rocknms_sub_vram_fg[offset]);
	m_tilemap_sub_fg->mark_tile_dirty(offset/2);
}


static TILE_GET_INFO( get_tile_info_rocknms_sub_rot )
{
	tetrisp2_state *state = machine.driver_data<tetrisp2_state>();
	UINT16 code_hi = state->m_rocknms_sub_vram_rot[ 2 * tile_index + 0];
	UINT16 code_lo = state->m_rocknms_sub_vram_rot[ 2 * tile_index + 1];
	SET_TILE_INFO(
			6,
			code_hi,
			code_lo & 0xf,
			0);
}

WRITE16_MEMBER(tetrisp2_state::rocknms_sub_vram_rot_w)
{
	COMBINE_DATA(&m_rocknms_sub_vram_rot[offset]);
	m_tilemap_sub_rot->mark_tile_dirty(offset/2);
}



VIDEO_START( tetrisp2 )
{
	tetrisp2_state *state = machine.driver_data<tetrisp2_state>();
	state->m_flipscreen_old = -1;

	state->m_tilemap_bg = tilemap_create(	machine, get_tile_info_bg,tilemap_scan_rows,

								16,16,NX_0,NY_0);

	state->m_tilemap_fg = tilemap_create(	machine, get_tile_info_fg,tilemap_scan_rows,

								8,8,NX_1,NY_1);

	state->m_tilemap_rot = tilemap_create(	machine, get_tile_info_rot,tilemap_scan_rows,

								16,16,NX_0*2,NY_0*2);

	state->m_tilemap_bg->set_transparent_pen(0);
	state->m_tilemap_fg->set_transparent_pen(0);
	state->m_tilemap_rot->set_transparent_pen(0);

	// should be smaller and mirrored like m32 I guess
	state->m_priority = auto_alloc_array(machine, UINT8, 0x40000);
	ms32_rearrange_sprites(machine, "gfx1");
}

VIDEO_START( nndmseal )
{
	tetrisp2_state *state = machine.driver_data<tetrisp2_state>();
	VIDEO_START_CALL( tetrisp2 );
	state->m_tilemap_bg->set_scrolldx(-4,-4);
}

VIDEO_START( rockntread )
{
	tetrisp2_state *state = machine.driver_data<tetrisp2_state>();
	state->m_flipscreen_old = -1;

	state->m_tilemap_bg = tilemap_create(	machine, get_tile_info_bg,tilemap_scan_rows,

								16, 16, 256, 16);	// rockn ms(main),1,2,3,4

	state->m_tilemap_fg = tilemap_create(	machine, get_tile_info_fg,tilemap_scan_rows,

								8, 8, 64, 64);

	state->m_tilemap_rot = tilemap_create(	machine, get_tile_info_rot,tilemap_scan_rows,

								16, 16, 128, 128);

	state->m_tilemap_bg->set_transparent_pen(0);
	state->m_tilemap_fg->set_transparent_pen(0);
	state->m_tilemap_rot->set_transparent_pen(0);

	// should be smaller and mirrored like m32 I guess
	state->m_priority = auto_alloc_array(machine, UINT8, 0x40000);
	ms32_rearrange_sprites(machine, "gfx1");
}


VIDEO_START( rocknms )
{
	tetrisp2_state *state = machine.driver_data<tetrisp2_state>();
	VIDEO_START_CALL( rockntread );

	state->m_tilemap_sub_bg = tilemap_create(machine, get_tile_info_rocknms_sub_bg,tilemap_scan_rows,

					16, 16, 32, 256);	// rockn ms(sub)

	state->m_tilemap_sub_fg = tilemap_create(machine, get_tile_info_rocknms_sub_fg,tilemap_scan_rows,

					8, 8, 64, 64);

	state->m_tilemap_sub_rot = tilemap_create( machine, get_tile_info_rocknms_sub_rot,tilemap_scan_rows,

					16, 16, 128, 128);

	state->m_tilemap_sub_bg->set_transparent_pen(0);
	state->m_tilemap_sub_fg->set_transparent_pen(0);
	state->m_tilemap_sub_rot->set_transparent_pen(0);

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
template<class _BitmapClass>
static void tetrisp2_draw_sprites(running_machine &machine, _BitmapClass &bitmap, bitmap_ind8 &bitmap_pri, const rectangle &cliprect, UINT8* priority_ram, UINT16 *sprram_top, size_t sprram_size, int gfxnum, int flip)
{
	int tx, ty, sx, sy, flipx, flipy;
	int xsize, ysize;
	int code, attr, color, size;
	int pri;
	int xzoom, yzoom;
	UINT32 primask;
	gfx_element *gfx = machine.gfx[gfxnum];

	UINT16		*source	=	sprram_top;
	UINT16	*finish	=	sprram_top + (sprram_size - 0x10) / 2;

	for (; source<finish; source+=8)
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

		sx		=	(source[5] & 0x3ff) - (source[5] & 0x400);
		sy		=	(source[4] & 0x1ff) - (source[4] & 0x200);

		xzoom = 0x10000;
		yzoom = 0x10000;

		gfx_element_set_source_clip(gfx, tx, xsize, ty, ysize);

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

SCREEN_UPDATE_IND16( tetrisp2 )
{
	tetrisp2_state *state = screen.machine().driver_data<tetrisp2_state>();
	int flipscreen;
	int asc_pri;
	int scr_pri;
	int rot_pri;
	int rot_ofsx, rot_ofsy;

	flipscreen = (state->m_systemregs[0x00] & 0x02);

	/* Black background color */
	bitmap.fill(0, cliprect);
	screen.machine().priority_bitmap.fill(0);

	/* Flip Screen */
	if (flipscreen != state->m_flipscreen_old)
	{
		state->m_flipscreen_old = flipscreen;
		screen.machine().tilemap().set_flip_all(flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
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

	state->m_tilemap_bg->set_scrollx(0, (((state->m_scroll_bg[ 0 ] + 0x0014) + state->m_scroll_bg[ 2 ] ) & 0xffff));
	state->m_tilemap_bg->set_scrolly(0, (((state->m_scroll_bg[ 3 ] + 0x0000) + state->m_scroll_bg[ 5 ] ) & 0xffff));

	state->m_tilemap_fg->set_scrollx(0, state->m_scroll_fg[ 2 ]);
	state->m_tilemap_fg->set_scrolly(0, state->m_scroll_fg[ 5 ]);

	state->m_tilemap_rot->set_scrollx(0, (state->m_rotregs[ 0 ] - rot_ofsx));
	state->m_tilemap_rot->set_scrolly(0, (state->m_rotregs[ 2 ] - rot_ofsy));

	asc_pri = scr_pri = rot_pri = 0;

	if((state->m_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if((state->m_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if((state->m_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		state->m_tilemap_rot->draw(bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 0)
		state->m_tilemap_bg->draw(bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 0)
		state->m_tilemap_fg->draw(bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 1)
		state->m_tilemap_rot->draw(bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 1)
		state->m_tilemap_bg->draw(bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 1)
		state->m_tilemap_fg->draw(bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 2)
		state->m_tilemap_rot->draw(bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 2)
		state->m_tilemap_bg->draw(bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 2)
		state->m_tilemap_fg->draw(bitmap, cliprect, 0, 1 << 2);

	tetrisp2_draw_sprites(screen.machine(), bitmap, screen.machine().priority_bitmap, cliprect, state->m_priority, state->m_spriteram, state->m_spriteram.bytes(), 0, (state->m_systemregs[0x00] & 0x02));
	return 0;
}

SCREEN_UPDATE_IND16( rockntread )
{
	tetrisp2_state *state = screen.machine().driver_data<tetrisp2_state>();
	int flipscreen;
	int asc_pri;
	int scr_pri;
	int rot_pri;
	int rot_ofsx, rot_ofsy;

	flipscreen = (state->m_systemregs[0x00] & 0x02);

	/* Black background color */
	bitmap.fill(0, cliprect);
	screen.machine().priority_bitmap.fill(0);

	/* Flip Screen */
	if (flipscreen != state->m_flipscreen_old)
	{
		state->m_flipscreen_old = flipscreen;
		screen.machine().tilemap().set_flip_all(flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
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

	state->m_tilemap_bg->set_scrollx(0, (((state->m_scroll_bg[ 0 ] + 0x0014) + state->m_scroll_bg[ 2 ] ) & 0xffff));
	state->m_tilemap_bg->set_scrolly(0, (((state->m_scroll_bg[ 3 ] + 0x0000) + state->m_scroll_bg[ 5 ] ) & 0xffff));

	state->m_tilemap_fg->set_scrollx(0, state->m_scroll_fg[ 2 ]);
	state->m_tilemap_fg->set_scrolly(0, state->m_scroll_fg[ 5 ]);

	state->m_tilemap_rot->set_scrollx(0, (state->m_rotregs[ 0 ] - rot_ofsx));
	state->m_tilemap_rot->set_scrolly(0, (state->m_rotregs[ 2 ] - rot_ofsy));

	asc_pri = scr_pri = rot_pri = 0;

	if((state->m_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if((state->m_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if((state->m_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		state->m_tilemap_rot->draw(bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 0)
		state->m_tilemap_bg->draw(bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 0)
		state->m_tilemap_fg->draw(bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 1)
		state->m_tilemap_rot->draw(bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 1)
		state->m_tilemap_bg->draw(bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 1)
		state->m_tilemap_fg->draw(bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 2)
		state->m_tilemap_rot->draw(bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 2)
		state->m_tilemap_bg->draw(bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 2)
		state->m_tilemap_fg->draw(bitmap, cliprect, 0, 1 << 2);

	tetrisp2_draw_sprites(screen.machine(), bitmap, screen.machine().priority_bitmap,cliprect, state->m_priority, state->m_spriteram, state->m_spriteram.bytes(), 0, (state->m_systemregs[0x00] & 0x02));
	return 0;
}




SCREEN_UPDATE_RGB32( rocknms_left )
{
	tetrisp2_state *state = screen.machine().driver_data<tetrisp2_state>();
	int asc_pri;
	int scr_pri;
	int rot_pri;

	state->m_tilemap_sub_bg->set_scrollx(0, state->m_rocknms_sub_scroll_bg[ 2 ] + 0x000);
	state->m_tilemap_sub_bg->set_scrolly(0, state->m_rocknms_sub_scroll_bg[ 5 ] + 0x000);
	state->m_tilemap_sub_fg->set_scrollx(0, state->m_rocknms_sub_scroll_fg[ 2 ] + 0x000);
	state->m_tilemap_sub_fg->set_scrolly(0, state->m_rocknms_sub_scroll_fg[ 5 ] + 0x000);
	state->m_tilemap_sub_rot->set_scrollx(0, state->m_rocknms_sub_rotregs[ 0 ] + 0x400);
	state->m_tilemap_sub_rot->set_scrolly(0, state->m_rocknms_sub_rotregs[ 2 ] + 0x400);

	bitmap.fill(screen.machine().pens[0x0000], cliprect);
	screen.machine().priority_bitmap.fill(0, cliprect);

	asc_pri = scr_pri = rot_pri = 0;

	if((state->m_rocknms_sub_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if((state->m_rocknms_sub_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if((state->m_rocknms_sub_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		state->m_tilemap_sub_rot->draw(bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 0)
		state->m_tilemap_sub_bg->draw(bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 0)
		state->m_tilemap_sub_fg->draw(bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 1)
		state->m_tilemap_sub_rot->draw(bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 1)
		state->m_tilemap_sub_bg->draw(bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 1)
		state->m_tilemap_sub_fg->draw(bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 2)
		state->m_tilemap_sub_rot->draw(bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 2)
		state->m_tilemap_sub_bg->draw(bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 2)
		state->m_tilemap_sub_fg->draw(bitmap, cliprect, 0, 1 << 2);

	tetrisp2_draw_sprites(screen.machine(), bitmap, screen.machine().priority_bitmap,cliprect, state->m_priority, state->m_spriteram2, state->m_spriteram2.bytes(), 4, (state->m_systemregs[0x00] & 0x02));

	return 0;
}

SCREEN_UPDATE_RGB32( rocknms_right )
{
	tetrisp2_state *state = screen.machine().driver_data<tetrisp2_state>();
	int asc_pri;
	int scr_pri;
	int rot_pri;

	state->m_tilemap_bg->set_scrollx(0, state->m_scroll_bg[ 2 ] + 0x000);
	state->m_tilemap_bg->set_scrolly(0, state->m_scroll_bg[ 5 ] + 0x000);
	state->m_tilemap_fg->set_scrollx(0, state->m_scroll_fg[ 2 ] + 0x000);
	state->m_tilemap_fg->set_scrolly(0, state->m_scroll_fg[ 5 ] + 0x000);
	state->m_tilemap_rot->set_scrollx(0, state->m_rotregs[ 0 ] + 0x400);
	state->m_tilemap_rot->set_scrolly(0, state->m_rotregs[ 2 ] + 0x400);

	/* Black background color */
	bitmap.fill(screen.machine().pens[0x0000], cliprect);
	screen.machine().priority_bitmap.fill(0, cliprect);

	asc_pri = scr_pri = rot_pri = 0;

	if((state->m_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if((state->m_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if((state->m_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		state->m_tilemap_rot->draw(bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 0)
		state->m_tilemap_bg->draw(bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 0)
		state->m_tilemap_fg->draw(bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 1)
		state->m_tilemap_rot->draw(bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 1)
		state->m_tilemap_bg->draw(bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 1)
		state->m_tilemap_fg->draw(bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 2)
		state->m_tilemap_rot->draw(bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 2)
		state->m_tilemap_bg->draw(bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 2)
		state->m_tilemap_fg->draw(bitmap, cliprect, 0, 1 << 2);

	tetrisp2_draw_sprites(screen.machine(), bitmap, screen.machine().priority_bitmap,cliprect, state->m_priority, state->m_spriteram, state->m_spriteram.bytes(), 0, (state->m_systemregs[0x00] & 0x02));

	return 0;
}
