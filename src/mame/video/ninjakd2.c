#include "driver.h"
#include "ninjakd2.h"

UINT8* ninjakd2_bg_videoram;
UINT8* ninjakd2_fg_videoram;

static int next_sprite_overdraw_enabled;
static int (*stencil_compare_function) (UINT16 pal);
static int sprites_updated;
static bitmap_t *sp_bitmap;
// in robokid and omegaf big sprites are laid out differently in ROM
static int robokid_sprites;

static tilemap* fg_tilemap;
static tilemap* bg_tilemap;
static tilemap* bg0_tilemap;
static tilemap* bg1_tilemap;
static tilemap* bg2_tilemap;

static int bank_mask;
static int robokid_bg0_bank = 0;
static int robokid_bg1_bank = 0;
static int robokid_bg2_bank = 0;
static UINT8* robokid_bg0_videoram;
static UINT8* robokid_bg1_videoram;
static UINT8* robokid_bg2_videoram;

#define TRANSPARENTCODE (15)

/*************************************
 *
 *  Callbacks for the TileMap code
 *
 *************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	int const lo = ninjakd2_fg_videoram[(tile_index << 1)];
	int const hi = ninjakd2_fg_videoram[(tile_index << 1) | 1];
	int const tile = ((hi & 0xc0) << 2) | lo;
	int const flipyx = (hi & 0x30) >> 4;
	int const color = hi & 0x0f;

	SET_TILE_INFO(
			0,
			tile,
			color,
			TILE_FLIPYX(flipyx));
}

static TILE_GET_INFO( ninjakd2_get_bg_tile_info )
{
	int const lo = ninjakd2_bg_videoram[(tile_index << 1)];
	int const hi = ninjakd2_bg_videoram[(tile_index << 1) | 1];
	int const tile = ((hi & 0xc0) << 2) | lo;
	int const flipyx = (hi & 0x30) >> 4;
	int const color = hi & 0x0f;

	SET_TILE_INFO(
			2,
			tile,
			color,
			TILE_FLIPYX(flipyx));
}

static TILE_GET_INFO( mnight_get_bg_tile_info )
{
	int const lo = ninjakd2_bg_videoram[(tile_index << 1)];
	int const hi = ninjakd2_bg_videoram[(tile_index << 1) | 1];
	int const tile = ((hi & 0x10) << 6) | ((hi & 0xc0) << 2) | lo;
	int const flipy = (hi & 0x20) >> 5;
	int const color = hi & 0x0f;

	SET_TILE_INFO(
			2,
			tile,
			color,
			flipy ? TILE_FLIPY : 0);
}

static TILEMAP_MAPPER( robokid_bg_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) | ((row & 0x1f) << 4) | ((col & 0x10) << 5);
}

static TILEMAP_MAPPER( omegaf_bg_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) | ((row & 0x1f) << 4) | ((col & 0x70) << 5);
}

static void robokid_get_bg_tile_info(running_machine* machine, tile_data* const tileinfo, tilemap_memory_index const tile_index, int const gfxnum, const UINT8* const videoram)
{
	int const lo = machine->generic.videoram.u8[(tile_index << 1)];
	int const hi = machine->generic.videoram.u8[(tile_index << 1) | 1];
	int const tile = ((hi & 0x10) << 7) | ((hi & 0x20) << 5) | ((hi & 0xc0) << 2) | lo;
	int const color = hi & 0x0f;

	SET_TILE_INFO(
			gfxnum,
			tile,
			color,
			0);
}

static TILE_GET_INFO( robokid_get_bg0_tile_info )
{
	robokid_get_bg_tile_info(machine, tileinfo, tile_index, 2, robokid_bg0_videoram);
}

static TILE_GET_INFO( robokid_get_bg1_tile_info )
{
	robokid_get_bg_tile_info(machine, tileinfo, tile_index, 3, robokid_bg1_videoram);
}

static TILE_GET_INFO( robokid_get_bg2_tile_info )
{
	robokid_get_bg_tile_info(machine, tileinfo, tile_index, 4, robokid_bg2_videoram);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

static void videoram_alloc(running_machine* machine, int const size)
{
	if (size)
	{
		/* create video ram */
		robokid_bg0_videoram = auto_alloc_array_clear(machine, UINT8, size);
		robokid_bg1_videoram = auto_alloc_array_clear(machine, UINT8, size);
		robokid_bg2_videoram = auto_alloc_array_clear(machine, UINT8, size);
	}

	sp_bitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);
}

static int stencil_ninjakd2( UINT16 pal );
static int stencil_mnight(   UINT16 pal );
static int stencil_arkarea(  UINT16 pal );
static int stencil_robokid(  UINT16 pal );
static int stencil_omegaf(   UINT16 pal );

VIDEO_START( ninjakd2 )
{
	videoram_alloc(machine, 0);

	fg_tilemap = tilemap_create(         machine, get_fg_tile_info, tilemap_scan_rows,   8,  8, 32, 32);
	bg_tilemap = tilemap_create(machine, ninjakd2_get_bg_tile_info, tilemap_scan_rows,  16, 16, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, TRANSPARENTCODE);

	robokid_sprites = 0;
	stencil_compare_function = stencil_ninjakd2 ;
}

VIDEO_START( mnight )
{
	videoram_alloc(machine, 0);

	fg_tilemap = tilemap_create(       machine, get_fg_tile_info, tilemap_scan_rows,   8,  8, 32, 32);
	bg_tilemap = tilemap_create(machine, mnight_get_bg_tile_info, tilemap_scan_rows,  16, 16, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, TRANSPARENTCODE);

	robokid_sprites = 0;
	stencil_compare_function = stencil_mnight ;
}

VIDEO_START( arkarea )
{
	videoram_alloc(machine, 0);

	fg_tilemap = tilemap_create(       machine, get_fg_tile_info, tilemap_scan_rows,   8,  8, 32, 32);
	bg_tilemap = tilemap_create(machine, mnight_get_bg_tile_info, tilemap_scan_rows,  16, 16, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap, TRANSPARENTCODE);

	robokid_sprites = 0;
	stencil_compare_function = stencil_arkarea ;
}

VIDEO_START( robokid )
{
	bank_mask = 1;

	videoram_alloc(machine, 0x0800);

	fg_tilemap  = tilemap_create(        machine, get_fg_tile_info,  tilemap_scan_rows,   8,  8, 32, 32);
	bg0_tilemap = tilemap_create(machine, robokid_get_bg0_tile_info, robokid_bg_scan,    16, 16, 32, 32);
	bg1_tilemap = tilemap_create(machine, robokid_get_bg1_tile_info, robokid_bg_scan,    16, 16, 32, 32);
	bg2_tilemap = tilemap_create(machine, robokid_get_bg2_tile_info, robokid_bg_scan,    16, 16, 32, 32);

	tilemap_set_transparent_pen(fg_tilemap,  TRANSPARENTCODE);
	tilemap_set_transparent_pen(bg1_tilemap, TRANSPARENTCODE);
	tilemap_set_transparent_pen(bg2_tilemap, TRANSPARENTCODE);

	robokid_sprites = 1;
	stencil_compare_function = stencil_robokid ;
}

VIDEO_START( omegaf )
{
	bank_mask = 7;

	videoram_alloc(machine, 0x2000);

	fg_tilemap  = tilemap_create(        machine, get_fg_tile_info,  tilemap_scan_rows,   8,  8,  32, 32);
	bg0_tilemap = tilemap_create(machine, robokid_get_bg0_tile_info, omegaf_bg_scan,     16, 16, 128, 32);
	bg1_tilemap = tilemap_create(machine, robokid_get_bg1_tile_info, omegaf_bg_scan,     16, 16, 128, 32);
	bg2_tilemap = tilemap_create(machine, robokid_get_bg2_tile_info, omegaf_bg_scan,     16, 16, 128, 32);

	tilemap_set_transparent_pen(fg_tilemap,  TRANSPARENTCODE);
	tilemap_set_transparent_pen(bg0_tilemap, TRANSPARENTCODE);
	tilemap_set_transparent_pen(bg1_tilemap, TRANSPARENTCODE);
	tilemap_set_transparent_pen(bg2_tilemap, TRANSPARENTCODE);

	robokid_sprites = 1;
	stencil_compare_function = stencil_omegaf ;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_HANDLER( ninjakd2_bgvideoram_w )
{
	ninjakd2_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset >> 1);
}

WRITE8_HANDLER( ninjakd2_fgvideoram_w )
{
	ninjakd2_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset >> 1);
}



WRITE8_HANDLER( robokid_bg0_bank_w )
{
	robokid_bg0_bank = data & bank_mask;
}

WRITE8_HANDLER( robokid_bg1_bank_w )
{
	robokid_bg1_bank = data & bank_mask;
}

WRITE8_HANDLER( robokid_bg2_bank_w )
{
	robokid_bg2_bank = data & bank_mask;
}

READ8_HANDLER( robokid_bg0_videoram_r )
{
	return robokid_bg0_videoram[(robokid_bg0_bank << 10) | offset];
}

READ8_HANDLER( robokid_bg1_videoram_r )
{
	return robokid_bg1_videoram[(robokid_bg1_bank << 10) | offset];
}

READ8_HANDLER( robokid_bg2_videoram_r )
{
	return robokid_bg2_videoram[(robokid_bg2_bank << 10) | offset];
}

WRITE8_HANDLER( robokid_bg0_videoram_w )
{
	int const address = (robokid_bg0_bank << 10 ) | offset;

	robokid_bg0_videoram[address] = data;
	tilemap_mark_tile_dirty(bg0_tilemap, address >> 1);
}

WRITE8_HANDLER( robokid_bg1_videoram_w )
{
	int const address = (robokid_bg1_bank << 10 ) | offset;

	robokid_bg1_videoram[address] = data;
	tilemap_mark_tile_dirty(bg1_tilemap, address >> 1);
}

WRITE8_HANDLER( robokid_bg2_videoram_w )
{
	int const address = (robokid_bg2_bank << 10 ) | offset;

	robokid_bg2_videoram[address] = data;
	tilemap_mark_tile_dirty(bg2_tilemap, address >> 1);
}



static void bg_ctrl(int offset, int data, tilemap* tilemap)
{
	int scrollx = tilemap_get_scrollx(tilemap, 0);
	int scrolly = tilemap_get_scrolly(tilemap, 0);

	switch (offset)
	{
		case 0:	scrollx = ((scrollx & 0x100) | data);        break;
		case 1:	scrollx = ((scrollx & 0x0ff) | (data << 8)); break;
		case 2:	scrolly = ((scrolly & 0x100) | data);        break;
		case 3:	scrolly = ((scrolly & 0x0ff) | (data << 8)); break;
		case 4:	tilemap_set_enable(tilemap, data & 1);       break;
	}

	tilemap_set_scrollx(tilemap, 0, scrollx);
	tilemap_set_scrolly(tilemap, 0, scrolly);
}

WRITE8_HANDLER( ninjakd2_bg_ctrl_w )
{
	bg_ctrl(offset, data, bg_tilemap);
}

WRITE8_HANDLER( robokid_bg0_ctrl_w )
{
	bg_ctrl(offset, data, bg0_tilemap);
}

WRITE8_HANDLER( robokid_bg1_ctrl_w )
{
	bg_ctrl(offset, data, bg1_tilemap);
}

WRITE8_HANDLER( robokid_bg2_ctrl_w )
{
	bg_ctrl(offset, data, bg2_tilemap);
}



WRITE8_HANDLER( ninjakd2_sprite_overdraw_w )
{
	next_sprite_overdraw_enabled = data & 1;
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites(running_machine* machine, bitmap_t* bitmap)
{
	const gfx_element* const gfx = machine->gfx[1];
	int const big_xshift = robokid_sprites ? 1 : 0;
	int const big_yshift = robokid_sprites ? 0 : 1;

	UINT8* sprptr = &machine->generic.spriteram.u8[11];
	int sprites_drawn = 0;

	// the sprite generator draws exactly 96 16x16 sprites per frame. When big
	// (32x32) sprites are drawn, this counts for 4 sprites drawn, so the sprite
	// list is reduced accordingly (i.e. three slots at the end of the list will
	// be ignored). Note that a disabled sprite, even if it is not drawn, still
	// counts as one sprite drawn.
	// This is proven by Mutant Night, which doesn't work correctly (leaves shots
	// on screen) if we don't take big sprites into account.

	for (;;)
	{
		if (sprptr[2] & 0x02)
		{
			int sx = sprptr[1] - ((sprptr[2] & 0x01) << 8);
			int sy = sprptr[0];
			// Ninja Kid II doesn't use the topmost bit (it has smaller ROMs) so it might not be connected on the board
			int code = sprptr[3] + ((sprptr[2] & 0xc0) << 2) + ((sprptr[2] & 0x08) << 7);
			int flipx = (sprptr[2] & 0x10) >> 4;
			int flipy = (sprptr[2] & 0x20) >> 5;
			int const color = sprptr[4] & 0x0f;
			// Ninja Kid II doesn't use the 'big' feature so it might not be available on the board
			int const big = (sprptr[2] & 0x04) >> 2;

			if (flip_screen_get(machine))
			{
				sx = 240 - 16*big - sx;
				sy = 240 - 16*big - sy;
				flipx ^= 1;
				flipy ^= 1;
			}

			if (big)
			{
				code &= ~3;
				code ^= flipx << big_xshift;
				code ^= flipy << big_yshift;
			}

			{
				int y;
				for (y = 0; y <= big; ++y)
				{
					int x;
					for (x = 0; x <= big; ++x)
					{
						int const tile = code ^ (x << big_xshift) ^ (y << big_yshift);

						drawgfx_transpen(bitmap, 0, gfx,
								tile,
								color,
								flipx,flipy,
								sx + 16*x, sy + 16*y, 15);

						++sprites_drawn;
						if (sprites_drawn >= 96)
							return;
					}
				}
			}
		}
		else
		{
			++sprites_drawn;
			if (sprites_drawn >= 96)
				return;
		}

		sprptr += 16;
	}
}

static int stencil_ninjakd2( UINT16 pal ) { return( (pal & 0xf0) == 0xf0 ); }
static int stencil_mnight(   UINT16 pal ) { return( (pal & 0xf0) == 0xf0 ); }
static int stencil_arkarea(  UINT16 pal ) { return( (pal & 0xf0) == 0xf0 ); }
static int stencil_robokid(  UINT16 pal ) { return( (pal & 0xf0) <  0xe0 ); }
static int stencil_omegaf(   UINT16 pal ) { return( TRUE ); }
//////            OVERDRAW     STENCIL     UNKNOWN
//////  NINJAKD2  023459ABCDE  F           1678
//////    MNIGHT  0134568ABCDE F           279
//////   ARKAREA  012345679BDE             8ACF
//////   ROBOKID  EF           01236       45789ABCD
//////    OMEGAF  -            -           -         (unused)
// I could not find a port to select overdraw or stencil.
// Temporarily, I compare with constant number.
// This is very hackish.
// (Is there a possibility that software can't select it but hardware can?)

static void erase_sprites(running_machine* machine, bitmap_t* bitmap, const rectangle* cliprect)
{
	// if sprite overdraw is disabled, clear the sprite framebuffer
	if (!next_sprite_overdraw_enabled)
		bitmap_fill(sp_bitmap, cliprect, TRANSPARENTCODE);
	else
	{
		int y;
		for (y = 0; y < sp_bitmap->height; ++y)
		{
			int x;
			for (x = 0; x < sp_bitmap->width; ++x)
			{
				UINT16* const ptr = BITMAP_ADDR16(sp_bitmap, y, x);

				if ( (*stencil_compare_function)(*ptr) ) *ptr = TRANSPARENTCODE ;
			}
		}
	}
}


static void update_sprites(running_machine* machine)
{
	erase_sprites(machine, sp_bitmap, 0);
	draw_sprites(machine, sp_bitmap);
}
	////// Before modified, this was written.
		// we want to erase the sprites with the old setting and draw them with the
		// new one. Not doing this causes a glitch in Ninja Kid II when taking the top
		// exit from stage 3.
	////// The glitch is correct behavior.


VIDEO_UPDATE( ninjakd2 )
{
	// updating sprites here instead than in video_eof avoids a palette glitch
	// at the end of the "rainbow sky" screens.
	update_sprites(screen->machine);
	sprites_updated = 1;

	bitmap_fill(bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	copybitmap_trans(bitmap, sp_bitmap, 0, 0, 0, 0, cliprect, TRANSPARENTCODE);

	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	return 0;
}

VIDEO_UPDATE( robokid )
{
	update_sprites(screen->machine);
	sprites_updated = 1;

	bitmap_fill(bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, bg0_tilemap, 0, 0);

	tilemap_draw(bitmap, cliprect, bg1_tilemap, 0, 0);

	copybitmap_trans(bitmap, sp_bitmap, 0, 0, 0, 0, cliprect, TRANSPARENTCODE);

	tilemap_draw(bitmap, cliprect, bg2_tilemap, 0, 0);

	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	return 0;
}

VIDEO_UPDATE( omegaf )
{
	update_sprites(screen->machine);
	sprites_updated = 1;

	bitmap_fill(bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, bg0_tilemap, 0, 0);

	tilemap_draw(bitmap, cliprect, bg1_tilemap, 0, 0);

	tilemap_draw(bitmap, cliprect, bg2_tilemap, 0, 0);

	copybitmap_trans(bitmap, sp_bitmap, 0, 0, 0, 0, cliprect, TRANSPARENTCODE);

	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	return 0;
}


VIDEO_EOF( ninjakd2 )
{
	if (!sprites_updated)
		update_sprites(machine);

	sprites_updated = 0;
}
