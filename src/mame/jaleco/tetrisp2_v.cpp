// license:BSD-3-Clause
// copyright-holders:Luca Elia
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
#include "tetrisp2.h"


void tetrisp2_state::flipscreen_w(int state)
{
	machine().tilemap().set_flip_all(state ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	// TODO: sprite device(s)
	m_rot_ofsx = state ? 0x053f : 0x400;
	m_rot_ofsy = state ? 0x04df : 0x400;
}

void rocknms_state::sub_flipscreen_w(int state)
{
	// ...
}

/***************************************************************************


                                    Palette


***************************************************************************/

/* BBBBBGGGGGRRRRRx xxxxxxxxxxxxxxxx */
void tetrisp2_state::tetrisp2_palette_w(offs_t offset, u16 data, u16 mem_mask)
{
	data = COMBINE_DATA(&m_paletteram[offset]);
	if ((offset & 1) == 0)
		m_palette->set_pen_color(offset/2,pal5bit(data >> 1),pal5bit(data >> 6),pal5bit(data >> 11));
}

void rocknms_state::rocknms_sub_palette_w(offs_t offset, u16 data, u16 mem_mask)
{
	data = COMBINE_DATA(&m_sub_paletteram[offset]);
	if ((offset & 1) == 0)
		m_sub_palette->set_pen_color(offset/2,pal5bit(data >> 1),pal5bit(data >> 6),pal5bit(data >> 11));
}



/***************************************************************************


                                    Priority


***************************************************************************/

void tetrisp2_state::tetrisp2_priority_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_priority[offset] = data;
	else
		m_priority[offset] = data >> 8;
}

u16 tetrisp2_state::tetrisp2_priority_r(offs_t offset)
{
	return m_priority[offset] | 0xff00;
}

void rocknms_state::rocknms_sub_priority_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_rocknms_sub_priority[offset] = data;
	else
		m_rocknms_sub_priority[offset] = data >> 8;
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

TILE_GET_INFO_MEMBER(tetrisp2_state::get_tile_info_bg)
{
	u16 code_hi = m_vram_bg[2 * tile_index + 0];
	u16 code_lo = m_vram_bg[2 * tile_index + 1];
	tileinfo.set(0,
			code_hi,
			code_lo & 0xf,
			0);
}

void tetrisp2_state::tetrisp2_vram_bg_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram_bg[offset]);
	m_tilemap_bg->mark_tile_dirty(offset/2);
}


#define NX_1  (0x40)
#define NY_1  (0x40)

TILE_GET_INFO_MEMBER(tetrisp2_state::get_tile_info_fg)
{
	u16 code_hi = m_vram_fg[2 * tile_index + 0];
	u16 code_lo = m_vram_fg[2 * tile_index + 1];
	tileinfo.set(2,
			code_hi,
			code_lo & 0xf,
			0);
}

void tetrisp2_state::tetrisp2_vram_fg_w(offs_t offset, u16 data, u16 mem_mask)
{
	// VJ and Stepping Stage write to the upper byte here to display ASCII text,
	// other usages in those games outside of ASCII text write a full 16-bit value.
	if (mem_mask == 0xff00)
		m_vram_fg[offset] = data & 0x00ff;
	else
		m_vram_fg[offset] = data;

	m_tilemap_fg->mark_tile_dirty(offset/2);
}


TILE_GET_INFO_MEMBER(tetrisp2_state::get_tile_info_rot)
{
	u16 code_hi = m_vram_rot[ 2 * tile_index + 0];
	u16 code_lo = m_vram_rot[ 2 * tile_index + 1];
	tileinfo.set(1,
			code_hi,
			code_lo & 0xf,
			0);
}

void tetrisp2_state::tetrisp2_vram_rot_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram_rot[offset]);
	m_tilemap_rot->mark_tile_dirty(offset/2);
}

TILE_GET_INFO_MEMBER(rocknms_state::get_tile_info_rocknms_sub_bg)
{
	u16 code_hi = m_rocknms_sub_vram_bg[ 2 * tile_index + 0];
	u16 code_lo = m_rocknms_sub_vram_bg[ 2 * tile_index + 1];
	tileinfo.set(0,
			code_hi,
			code_lo & 0xf,
			0);
}

void rocknms_state::rocknms_sub_vram_bg_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_rocknms_sub_vram_bg[offset]);
	m_tilemap_sub_bg->mark_tile_dirty(offset/2);
}


TILE_GET_INFO_MEMBER(rocknms_state::get_tile_info_rocknms_sub_fg)
{
	u16 code_hi = m_rocknms_sub_vram_fg[ 2 * tile_index + 0];
	u16 code_lo = m_rocknms_sub_vram_fg[ 2 * tile_index + 1];
	tileinfo.set(2,
			code_hi,
			code_lo & 0xf,
			0);
}

void rocknms_state::rocknms_sub_vram_fg_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_rocknms_sub_vram_fg[offset]);
	m_tilemap_sub_fg->mark_tile_dirty(offset/2);
}


TILE_GET_INFO_MEMBER(rocknms_state::get_tile_info_rocknms_sub_rot)
{
	u16 code_hi = m_rocknms_sub_vram_rot[ 2 * tile_index + 0];
	u16 code_lo = m_rocknms_sub_vram_rot[ 2 * tile_index + 1];
	tileinfo.set(1,
			code_hi,
			code_lo & 0xf,
			0);
}

void rocknms_state::rocknms_sub_vram_rot_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_rocknms_sub_vram_rot[offset]);
	m_tilemap_sub_rot->mark_tile_dirty(offset/2);
}



VIDEO_START_MEMBER(tetrisp2_state,tetrisp2)
{
	m_tilemap_bg = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tetrisp2_state::get_tile_info_bg)), TILEMAP_SCAN_ROWS, 16,16, NX_0,NY_0);
	m_tilemap_fg = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tetrisp2_state::get_tile_info_fg)), TILEMAP_SCAN_ROWS, 8,8, NX_1,NY_1);
	m_tilemap_rot = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tetrisp2_state::get_tile_info_rot)), TILEMAP_SCAN_ROWS, 16,16, NX_0*2,NY_0*2);
	m_tilemap_bg->set_transparent_pen(0);
	m_tilemap_fg->set_transparent_pen(0);
	m_tilemap_rot->set_transparent_pen(0);

	// should be smaller and mirrored like m32 I guess
	m_priority = std::make_unique<u8[]>(0x40000);

	save_pointer(NAME(m_priority), 0x40000);
}

VIDEO_START_MEMBER(tetrisp2_state,nndmseal)
{
	VIDEO_START_CALL_MEMBER( tetrisp2 );
	m_tilemap_bg->set_scrolldx(-4,-4);

	m_bank_hi = 0;
	m_bank_lo = 0;
}

VIDEO_START_MEMBER(tetrisp2_state,rockntread)
{
	m_tilemap_bg = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tetrisp2_state::get_tile_info_bg)), TILEMAP_SCAN_ROWS, 16,16, 256,16);   // rockn ms(main),1,2,3,4
	m_tilemap_fg = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tetrisp2_state::get_tile_info_fg)), TILEMAP_SCAN_ROWS, 8,8, 64,64);
	m_tilemap_rot = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tetrisp2_state::get_tile_info_rot)), TILEMAP_SCAN_ROWS, 16,16, 128,128);

	m_tilemap_bg->set_transparent_pen(0);
	m_tilemap_fg->set_transparent_pen(0);
	m_tilemap_rot->set_transparent_pen(0);

	// should be smaller and mirrored like m32 I guess
	m_priority = std::make_unique<u8[]>(0x40000);

	save_item(NAME(m_rot_ofsx));
	save_item(NAME(m_rot_ofsy));
	save_pointer(NAME(m_priority), 0x40000);
}


VIDEO_START_MEMBER(rocknms_state,rocknms)
{
	VIDEO_START_CALL_MEMBER( rockntread );

	m_tilemap_sub_bg = &machine().tilemap().create(*m_sub_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rocknms_state::get_tile_info_rocknms_sub_bg)), TILEMAP_SCAN_ROWS, 16,16, 32,256);
	m_tilemap_sub_fg = &machine().tilemap().create(*m_sub_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rocknms_state::get_tile_info_rocknms_sub_fg)), TILEMAP_SCAN_ROWS, 8,8, 64,64);
	m_tilemap_sub_rot = &machine().tilemap().create(*m_sub_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rocknms_state::get_tile_info_rocknms_sub_rot)), TILEMAP_SCAN_ROWS, 16,16, 128,128);

	m_tilemap_sub_bg->set_transparent_pen(0);
	m_tilemap_sub_fg->set_transparent_pen(0);
	m_tilemap_sub_rot->set_transparent_pen(0);
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
                ---- ba98 7654 3210     Tile Page (32x32 tiles = 256x256 pixels each)

    6.w         fedc ba98 ---- ----     Y Size - 1 (*)
                ---- ---- 7654 3210     X Size - 1 (*)

    8.w         fedc ba-- ---- ----
                ---- --98 7654 3210     Y (Signed)

    A.w         fedc b--- ---- ----
                ---- -a98 7654 3210     X (Signed)

    C.w         fedc ba98 7654 3210     ZOOM X (unused)

    E.w         fedc ba98 7654 3210     ZOOM Y (unused)

(*) 1 pixel granularity

***************************************************************************/

/* this is also used by ms32.cpp */
/* sprites should be able to create shadows too, how?
  -- it appears that sprites which should be shadows are often rendered *UNDER* the tilemaps, maybe related?
*/
template<class BitmapClass>
static void tetrisp2_draw_sprites(BitmapClass &bitmap, bitmap_ind8 &bitmap_pri, const rectangle &cliprect, u8* priority_ram,
									u16 *sprram_top, size_t sprram_size, ms32_sprite_device *chip)
{
	u16  *source =   sprram_top;
	u16  *finish =   sprram_top + (sprram_size - 0x10) / 2;

	for (; source<finish; source+=8)
	{
		bool disable;
		u8 pri;
		bool flipx, flipy;
		u32 code, color;
		u8 tx, ty;
		u16 xsize, ysize;
		s32 sx, sy;
		u16 xzoom, yzoom;

		chip->extract_parameters(source, disable, pri, flipx, flipy, code, color, tx, ty, xsize, ysize, sx, sy, xzoom, yzoom);

		if (disable || !xzoom || !yzoom)
			continue;

		u32 primask = 0;
		if (priority_ram)
		{
			if (priority_ram[(pri | 0x0a00 | 0x1500) / 2] & 0x38) primask |= 1 << 0;
			if (priority_ram[(pri | 0x0a00 | 0x1400) / 2] & 0x38) primask |= 1 << 1;
			if (priority_ram[(pri | 0x0a00 | 0x1100) / 2] & 0x38) primask |= 1 << 2;
			if (priority_ram[(pri | 0x0a00 | 0x1000) / 2] & 0x38) primask |= 1 << 3;
			if (priority_ram[(pri | 0x0a00 | 0x0500) / 2] & 0x38) primask |= 1 << 4;
			if (priority_ram[(pri | 0x0a00 | 0x0400) / 2] & 0x38) primask |= 1 << 5;
			if (priority_ram[(pri | 0x0a00 | 0x0100) / 2] & 0x38) primask |= 1 << 6;
			if (priority_ram[(pri | 0x0a00 | 0x0000) / 2] & 0x38) primask |= 1 << 7;
		}

		chip->prio_zoom_transpen(bitmap,cliprect,
				code,
				color,
				flipx, flipy,
				sx,sy,
				tx, ty, xsize, ysize,
				xzoom, yzoom, bitmap_pri,primask, 0);

	}   /* end sprite loop */
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

u32 tetrisp2_state::screen_update_tetrisp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int asc_pri;
	int scr_pri;
	int rot_pri;

	/* Black background color */
	bitmap.fill(0, cliprect);
	screen.priority().fill(0);

	m_tilemap_bg->set_scrollx(0, (((m_scroll_bg[ 0 ] + 0x0014) + m_scroll_bg[ 2 ] ) & 0xffff));
	m_tilemap_bg->set_scrolly(0, (((m_scroll_bg[ 3 ] + 0x0000) + m_scroll_bg[ 5 ] ) & 0xffff));

	m_tilemap_fg->set_scrollx(0, m_scroll_fg[ 2 ]);
	m_tilemap_fg->set_scrolly(0, m_scroll_fg[ 5 ]);

	m_tilemap_rot->set_scrollx(0, (m_rotregs[ 0 ] - m_rot_ofsx));
	m_tilemap_rot->set_scrolly(0, (m_rotregs[ 2 ] - m_rot_ofsy));

	asc_pri = scr_pri = rot_pri = 0;

	if((m_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if((m_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if((m_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 0)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 0)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 1)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 1)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 1)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 2)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 2)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 2)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	tetrisp2_draw_sprites(bitmap, screen.priority(), cliprect, m_priority.get(),
							m_spriteram, m_spriteram.bytes(), m_sprite);
	return 0;
}

u32 tetrisp2_state::screen_update_rockntread(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int asc_pri;
	int scr_pri;
	int rot_pri;

	/* Black background color */
	bitmap.fill(0, cliprect);
	screen.priority().fill(0);

	m_tilemap_bg->set_scrollx(0, (((m_scroll_bg[ 0 ] + 0x0014) + m_scroll_bg[ 2 ] ) & 0xffff));
	m_tilemap_bg->set_scrolly(0, (((m_scroll_bg[ 3 ] + 0x0000) + m_scroll_bg[ 5 ] ) & 0xffff));

	m_tilemap_fg->set_scrollx(0, m_scroll_fg[ 2 ]);
	m_tilemap_fg->set_scrolly(0, m_scroll_fg[ 5 ]);

	m_tilemap_rot->set_scrollx(0, (m_rotregs[ 0 ] - m_rot_ofsx));
	m_tilemap_rot->set_scrolly(0, (m_rotregs[ 2 ] - m_rot_ofsy));

	asc_pri = scr_pri = rot_pri = 0;

	if((m_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if((m_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if((m_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 0)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 0)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 1)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 1)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 1)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 2)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 2)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 2)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	tetrisp2_draw_sprites(bitmap, screen.priority(), cliprect, m_priority.get(),
							m_spriteram, m_spriteram.bytes(), m_sprite);
	return 0;
}




u32 rocknms_state::screen_update_rocknms_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int asc_pri;
	int scr_pri;
	int rot_pri;

	m_tilemap_sub_bg->set_scrollx(0, m_rocknms_sub_scroll_bg[ 2 ] + 0x000);
	m_tilemap_sub_bg->set_scrolly(0, m_rocknms_sub_scroll_bg[ 5 ] + 0x000);
	m_tilemap_sub_fg->set_scrollx(0, m_rocknms_sub_scroll_fg[ 2 ] + 0x000);
	m_tilemap_sub_fg->set_scrolly(0, m_rocknms_sub_scroll_fg[ 5 ] + 0x000);
	m_tilemap_sub_rot->set_scrollx(0, m_rocknms_sub_rotregs[ 0 ] + 0x400);
	m_tilemap_sub_rot->set_scrolly(0, m_rocknms_sub_rotregs[ 2 ] + 0x400);

	bitmap.fill(m_palette->pen(0x0000), cliprect);
	screen.priority().fill(0, cliprect);

	asc_pri = scr_pri = rot_pri = 0;

	if((m_rocknms_sub_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if((m_rocknms_sub_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if((m_rocknms_sub_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		m_tilemap_sub_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 0)
		m_tilemap_sub_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 0)
		m_tilemap_sub_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 1)
		m_tilemap_sub_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 1)
		m_tilemap_sub_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 1)
		m_tilemap_sub_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 2)
		m_tilemap_sub_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 2)
		m_tilemap_sub_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 2)
		m_tilemap_sub_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	tetrisp2_draw_sprites(bitmap, screen.priority(), cliprect, m_priority.get(),
							m_spriteram2, m_spriteram2.bytes(), m_rocknms_sub_sprite);

	return 0;
}

u32 rocknms_state::screen_update_rocknms_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int asc_pri;
	int scr_pri;
	int rot_pri;

	m_tilemap_bg->set_scrollx(0, m_scroll_bg[ 2 ] + 0x000);
	m_tilemap_bg->set_scrolly(0, m_scroll_bg[ 5 ] + 0x000);
	m_tilemap_fg->set_scrollx(0, m_scroll_fg[ 2 ] + 0x000);
	m_tilemap_fg->set_scrolly(0, m_scroll_fg[ 5 ] + 0x000);
	m_tilemap_rot->set_scrollx(0, m_rotregs[ 0 ] + 0x400);
	m_tilemap_rot->set_scrolly(0, m_rotregs[ 2 ] + 0x400);

	/* Black background color */
	bitmap.fill(m_palette->pen(0x0000), cliprect);
	screen.priority().fill(0, cliprect);

	asc_pri = scr_pri = rot_pri = 0;

	if((m_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if((m_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if((m_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 0)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 0)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 1)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 1)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 1)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 2)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 2)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 2)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	tetrisp2_draw_sprites(bitmap, screen.priority(), cliprect, m_priority.get(),
							m_spriteram, m_spriteram.bytes(), m_sprite);

	return 0;
}

/***************************************************************************

                              Stepping Stage

***************************************************************************/

VIDEO_START_MEMBER(stepstag_state,stepstag)
{
	m_tilemap_bg = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(stepstag_state::get_tile_info_bg)), TILEMAP_SCAN_ROWS, 16,16, NX_0,NY_0);
	m_tilemap_fg = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(stepstag_state::get_tile_info_fg)), TILEMAP_SCAN_ROWS, 8,8, NX_1,NY_1);
	m_tilemap_rot = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(stepstag_state::get_tile_info_rot)), TILEMAP_SCAN_ROWS, 16,16, NX_0*2,NY_0*2);
	m_tilemap_bg->set_transparent_pen(0);
	m_tilemap_fg->set_transparent_pen(0);
	m_tilemap_rot->set_transparent_pen(0);

	// should be smaller and mirrored like m32 I guess
	m_priority = std::make_unique<u8[]>(0x40000);
}

u32 stepstag_state::screen_update_stepstag_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0);

	tetrisp2_draw_sprites(
			bitmap, screen.priority(), cliprect, nullptr,
			m_spriteram1_data.get(), 0x400, m_vj_sprite_l);

	m_jaleco_vj_pc->render_video_frame<0>(bitmap);

	return 0;
}

u32 stepstag_state::screen_update_stepstag_mid(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// Text sprites on the middle screen might only be displayed when the service switch is toggled.
	// There's a relay with the RGBS wires main PCB going into it that seems to click based on the service switch.

	bitmap.fill(0, cliprect);
	screen.priority().fill(0);

	tetrisp2_draw_sprites(
			bitmap, screen.priority(), cliprect, nullptr,
			m_spriteram2_data.get(), 0x400, m_vj_sprite_m);

	m_jaleco_vj_pc->render_video_frame<1>(bitmap);

	m_tilemap_bg->set_scrollx(0, (((m_scroll_bg[0] + 0x0014) + m_scroll_bg[2]) & 0xffff));
	m_tilemap_bg->set_scrolly(0, (((m_scroll_bg[3] + 0x0000) + m_scroll_bg[5]) & 0xffff));

	m_tilemap_fg->set_scrollx(0, m_scroll_fg[2]);
	m_tilemap_fg->set_scrolly(0, m_scroll_fg[5]);

	m_tilemap_rot->set_scrollx(0, (m_rotregs[0] - m_rot_ofsx));
	m_tilemap_rot->set_scrolly(0, (m_rotregs[2] - m_rot_ofsy));

	int asc_pri = 0, scr_pri = 0, rot_pri = 0;

	if ((m_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if ((m_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if ((m_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 0)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 0)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 1)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 1)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 1)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 2)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 2)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 2)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	tetrisp2_draw_sprites(
			bitmap, screen.priority(), cliprect, m_priority.get(),
			m_spriteram, m_spriteram.bytes(), m_sprite);

	return 0;
}

u32 stepstag_state::screen_update_stepstag_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0);

	tetrisp2_draw_sprites(
			bitmap, screen.priority(), cliprect, nullptr,
			m_spriteram3_data.get(), 0x400, m_vj_sprite_r);

	m_jaleco_vj_pc->render_video_frame<2>(bitmap);

	return 0;
}

// Stepping Stage encodes palette as YUV422.
// Convert them on the fly
void stepstag_state::convert_yuv422_to_rgb888(palette_device *paldev, u16 *palram, u32 offset)
{
	u8 const u =  palram[offset/4*4 + 0] & 0xff;
	u8 const y1 = palram[offset/4*4 + 1] & 0xff;
	u8 const v =  palram[offset/4*4 + 2] & 0xff;
	//u8 const y2 = palram[offset/4*4 + 3] & 0xff;

	double const bf = y1 + (1.772 * (u - 128));
	double const gf = y1 - (0.334 * (u - 128)) - (0.714 * (v - 128));
	double const rf = y1 + (1.402 * (v - 128));

	// clamp to 0-255 range
	u8 const r = u8(std::clamp(rf, 0.0, 255.0));
	u8 const g = u8(std::clamp(gf, 0.0, 255.0));
	u8 const b = u8(std::clamp(bf, 0.0, 255.0));

	paldev->set_pen_color(offset/4, r, g, b);
}

void stepstag_state::stepstag_palette_left_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vj_paletteram_l[offset]);
	convert_yuv422_to_rgb888(m_vj_palette_l,m_vj_paletteram_l,offset);
}

void stepstag_state::stepstag_palette_mid_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vj_paletteram_m[offset]);
	convert_yuv422_to_rgb888(m_vj_palette_m,m_vj_paletteram_m,offset);
}

void stepstag_state::stepstag_palette_right_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vj_paletteram_r[offset]);
	convert_yuv422_to_rgb888(m_vj_palette_r,m_vj_paletteram_r,offset);
}

u32 stepstag_state::screen_update_vjdash_main(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Black background color
	bitmap.fill(0, cliprect);
	screen.priority().fill(0);

	m_tilemap_bg->set_scrollx(0, (((m_scroll_bg[0] + 0x0014) + m_scroll_bg[2]) & 0xffff));
	m_tilemap_bg->set_scrolly(0, (((m_scroll_bg[3] + 0x0000) + m_scroll_bg[5]) & 0xffff));

	m_tilemap_fg->set_scrollx(0, m_scroll_fg[2]);
	m_tilemap_fg->set_scrolly(0, m_scroll_fg[5]);

	m_tilemap_rot->set_scrollx(0, (m_rotregs[0] - m_rot_ofsx));
	m_tilemap_rot->set_scrolly(0, (m_rotregs[2] - m_rot_ofsy));

	int asc_pri = 0, scr_pri = 0, rot_pri = 0;

	if ((m_priority[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if ((m_priority[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if ((m_priority[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 0)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 0)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 1)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 1)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 1)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	if (rot_pri == 2)
		m_tilemap_rot->draw(screen, bitmap, cliprect, 0, 1 << 1);
	else if (scr_pri == 2)
		m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 1 << 0);
	else if (asc_pri == 2)
		m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 1 << 2);

	tetrisp2_draw_sprites(
			bitmap, screen.priority(), cliprect, m_priority.get(),
			m_spriteram, m_spriteram.bytes(), m_sprite);

	return 0;
}

u32 stepstag_state::screen_update_vjdash_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0);

	tetrisp2_draw_sprites(
			bitmap, screen.priority(), cliprect, nullptr,
			m_spriteram1_data.get(), 0x400, m_vj_sprite_l);

	m_jaleco_vj_pc->render_video_frame<0>(bitmap);

	return 0;
}

u32 stepstag_state::screen_update_vjdash_mid(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0);

	tetrisp2_draw_sprites(
			bitmap, screen.priority(), cliprect, nullptr,
			m_spriteram2_data.get(), 0x400, m_vj_sprite_m);

	m_jaleco_vj_pc->render_video_frame<1>(bitmap);

	return 0;
}

u32 stepstag_state::screen_update_vjdash_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0);

	tetrisp2_draw_sprites(
			bitmap, screen.priority(), cliprect, nullptr,
			m_spriteram3_data.get(), 0x400, m_vj_sprite_r);

	m_jaleco_vj_pc->render_video_frame<2>(bitmap);

	return 0;
}

u32 stepstag_state::screen_update_nop(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0);
	return 0;
}
