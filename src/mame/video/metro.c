/***************************************************************************

                              -= Metro Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

                Q       Shows Layer 0
                W       Shows Layer 1
                E       Shows Layer 2
                A       Shows Sprites

        Keys can be used together!


                            [ 3 Scrolling Layers ]

        There is memory for a huge layer, but the actual tilemap
        is a smaller window (of fixed size) carved from anywhere
        inside that layer.

        Tile Size:                  8 x 8 x 4
        (later games can switch to  8 x 8 x 8, 16 x 16 x 4/8 at run time)

        Big Layer Size:         2048 x 2048 (8x8 tiles) or 4096 x 4096 (16x16 tiles)

        Tilemap Window Size:    512 x 256 (8x8 tiles) or 1024 x 512 (16x16 tiles)

        The tile codes in memory do not map directly to tiles. They
        are indexes into a table (with 0x200 entries) that defines
        a virtual set of tiles for the 3 layers. Each entry in that
        table adds 16 tiles to the set of available tiles, and decides
        their color code.

        Tile code with their msbit set are different as they mean:
        draw a tile filled with a single color (0-1ff)


                            [ 512 Zooming Sprites ]

        The sprites are NOT tile based: the "tile" size can vary from
        8 to 64 (independently for width and height) with an 8 pixel
        granularity. The "tile" address is a multiple of 8x8 pixels.

        Each sprite can be shrinked to ~1/4 or enlarged to ~32x following
        an exponential curve of sizes (with one zoom value for both width
        and height)


***************************************************************************/

#include "emu.h"
#include "includes/metro.h"
#include "video/konicdev.h"


static TILE_GET_INFO( metro_k053936_get_tile_info )
{
	metro_state *state = (metro_state *)machine->driver_data;
	int code = state->k053936_ram[tile_index];

	SET_TILE_INFO(
			2,
			code & 0x7fff,
			0x1e,
			0);
}

static TILE_GET_INFO( metro_k053936_gstrik2_get_tile_info )
{
	metro_state *state = (metro_state *)machine->driver_data;
	int code = state->k053936_ram[tile_index];

	SET_TILE_INFO(
			2,
			(code & 0x7fff)>>2,
			0x1e,
			0);
}

WRITE16_HANDLER( metro_k053936_w )
{
	metro_state *state = (metro_state *)space->machine->driver_data;
	COMBINE_DATA(&state->k053936_ram[offset]);
	tilemap_mark_tile_dirty(state->k053936_tilemap, offset);
}

static TILEMAP_MAPPER( tilemap_scan_gstrik2 )
{
	/* logical (col,row) -> memory offset */
	int val;

	val = (row & 0x3f) * (256 * 2) + (col * 2);

	if (row & 0x40) val += 1;
	if (row & 0x80) val += 256;

	return val;
}



/***************************************************************************


                        Tilemaps: Tiles Set & Window

    Each entry in the Tiles Set RAM uses 2 words to specify a starting
    tile code and a color code. This adds 16 consecutive tiles with
    that color code to the set of available tiles.

        Offset:     Bits:                   Value:

        0.w         fedc ---- ---- ----
                    ---- ba98 7654 ----     Color Code
                    ---- ---- ---- 3210     Code High Bits

        2.w                                 Code Low Bits


***************************************************************************/


/***************************************************************************


                            Tilemaps: Rendering


***************************************************************************/

/* A 2048 x 2048 virtual tilemap */
#define BIG_NX		(0x100)
#define BIG_NY		(0x100)

/* A smaller 512 x 256 window defines the actual tilemap */

#define WIN_NX		(0x40)
#define WIN_NY		(0x20)

/* 8x8x4 tiles only */
INLINE void get_tile_info( running_machine *machine, tile_data *tileinfo, int tile_index, int layer, UINT16 *vram )
{
	metro_state *state = (metro_state *)machine->driver_data;
	UINT16 code;
	int table_index;
	UINT32 tile;

	/* Fetch the code */
	code = vram[tile_index];

	/* Use it as an index into the tiles set table */
	table_index = ((code & 0x1ff0) >> 4) * 2;
	tile = (state->tiletable[table_index + 0] << 16) + state->tiletable[table_index + 1];

	if (code & 0x8000) /* Special: draw a tile of a single color (i.e. not from the gfx ROMs) */
	{
		int _code = code & 0x000f;
		tileinfo->pen_data = state->empty_tiles + _code * 16 * 16;
		tileinfo->palette_base = ((code & 0x0ff0)) + 0x1000;
		tileinfo->flags = 0;
		tileinfo->category = 0;
	}
	else
	{
		tileinfo->category = 0;
		SET_TILE_INFO(
				0,
				(tile & 0xfffff) + (code & 0xf),
				(((tile & 0x0ff00000) >> 20)) + 0x100,
				TILE_FLIPXY((code & 0x6000) >> 13));
	}
}

/* 8x8x4 or 8x8x8 tiles. It's the tile's color that decides: if its low 4
   bits are high ($f,$1f,$2f etc) the tile is 8bpp, otherwise it's 4bpp */
INLINE void get_tile_info_8bit( running_machine *machine, tile_data *tileinfo, int tile_index, int layer, UINT16 *vram )
{
	metro_state *state = (metro_state *)machine->driver_data;
	UINT16 code;
	int table_index;
	UINT32 tile;

	/* Fetch the code */
	code = vram[tile_index];

	/* Use it as an index into the tiles set table */
	table_index = ((code & 0x1ff0) >> 4) * 2;
	tile = (state->tiletable[table_index + 0] << 16) + state->tiletable[table_index + 1];

	if (code & 0x8000) /* Special: draw a tile of a single color (i.e. not from the gfx ROMs) */
	{
		int _code = code & 0x000f;
		tileinfo->pen_data = state->empty_tiles + _code * 16 * 16;
		tileinfo->palette_base = ((code & 0x0ff0)) + 0x1000;
		tileinfo->flags = 0;
		tileinfo->category = 0;
	}
	else if ((tile & 0x00f00000) == 0x00f00000)	/* draw tile as 8bpp */
	{
		tileinfo->category = 1;
		SET_TILE_INFO(
				1,
				(tile & 0xfffff) + 2*(code & 0xf),
				((tile & 0x0f000000) >> 24) + 0x10,
				TILE_FLIPXY((code & 0x6000) >> 13));
	}
	else
	{
		tileinfo->category = 0;
		SET_TILE_INFO(
				0,
				(tile & 0xfffff) + (code & 0xf),
				(((tile & 0x0ff00000) >> 20)) + 0x100,
				TILE_FLIPXY((code & 0x6000) >> 13));
	}
}

/* 16x16x4 or 16x16x8 tiles. It's the tile's color that decides: if its low 4
   bits are high ($f,$1f,$2f etc) the tile is 8bpp, otherwise it's 4bpp */
INLINE void get_tile_info_16x16_8bit( running_machine *machine, tile_data *tileinfo, int tile_index, int layer, UINT16 *vram )
{
	metro_state *state = (metro_state *)machine->driver_data;
	UINT16 code;
	int table_index;
	UINT32 tile;

	/* Fetch the code */
	code = vram[tile_index];

	/* Use it as an index into the tiles set table */
	table_index = ((code & 0x1ff0) >> 4) * 2;
	tile = (state->tiletable[table_index + 0] << 16) + state->tiletable[table_index + 1];

	if (code & 0x8000) /* Special: draw a tile of a single color (i.e. not from the gfx ROMs) */
	{
		int _code = code & 0x000f;
		tileinfo->pen_data = state->empty_tiles + _code * 16 * 16;
		tileinfo->palette_base = ((code & 0x0ff0)) + 0x1000;
		tileinfo->flags = 0;
		tileinfo->category = 0;
	}
	else if ((tile & 0x00f00000) == 0x00f00000)	/* draw tile as 8bpp */
	{
		tileinfo->category = 1;
		SET_TILE_INFO(
				3,
				(tile & 0xfffff) + 8*(code & 0xf),
				((tile & 0x0f000000) >> 24) + 0x10,
				TILE_FLIPXY((code & 0x6000) >> 13));
	}
	else
	{
		tileinfo->category = 0;
		SET_TILE_INFO(
				2,
				(tile & 0xfffff) + 4*(code & 0xf),
				(((tile & 0x0ff00000) >> 20)) + 0x100,
				TILE_FLIPXY((code & 0x6000) >> 13));

	}
}

INLINE void metro_vram_w( running_machine *machine, offs_t offset, UINT16 data, UINT16 mem_mask, int layer, UINT16 *vram )
{
	metro_state *state = (metro_state *)machine->driver_data;

	COMBINE_DATA(&vram[offset]);

	tilemap_mark_tile_dirty(state->bg_tilemap[layer], offset);
	if (state->bg_tilemap_16x16[layer])
		tilemap_mark_tile_dirty(state->bg_tilemap_16x16[layer], offset);
}


static TILE_GET_INFO( get_tile_info_0 ) { metro_state *state = (metro_state *)machine->driver_data;  get_tile_info(machine, tileinfo, tile_index, 0, state->vram_0); }
static TILE_GET_INFO( get_tile_info_1 ) { metro_state *state = (metro_state *)machine->driver_data;  get_tile_info(machine, tileinfo, tile_index, 1, state->vram_1); }
static TILE_GET_INFO( get_tile_info_2 ) { metro_state *state = (metro_state *)machine->driver_data;  get_tile_info(machine, tileinfo, tile_index, 2, state->vram_2); }

static TILE_GET_INFO( get_tile_info_0_8bit ) { metro_state *state = (metro_state *)machine->driver_data;  get_tile_info_8bit(machine, tileinfo, tile_index, 0, state->vram_0); }
static TILE_GET_INFO( get_tile_info_1_8bit ) { metro_state *state = (metro_state *)machine->driver_data;  get_tile_info_8bit(machine, tileinfo, tile_index, 1, state->vram_1); }
static TILE_GET_INFO( get_tile_info_2_8bit ) { metro_state *state = (metro_state *)machine->driver_data;  get_tile_info_8bit(machine, tileinfo, tile_index, 2, state->vram_2); }

static TILE_GET_INFO( get_tile_info_0_16x16_8bit ) { metro_state *state = (metro_state *)machine->driver_data;  get_tile_info_16x16_8bit(machine, tileinfo, tile_index, 0, state->vram_0); }
static TILE_GET_INFO( get_tile_info_1_16x16_8bit ) { metro_state *state = (metro_state *)machine->driver_data;  get_tile_info_16x16_8bit(machine, tileinfo, tile_index, 1, state->vram_1); }
static TILE_GET_INFO( get_tile_info_2_16x16_8bit ) { metro_state *state = (metro_state *)machine->driver_data;  get_tile_info_16x16_8bit(machine, tileinfo, tile_index, 2, state->vram_2); }


WRITE16_HANDLER( metro_vram_0_w ) { metro_state *state = (metro_state *)space->machine->driver_data;  metro_vram_w(space->machine, offset, data, mem_mask, 0, state->vram_0); }
WRITE16_HANDLER( metro_vram_1_w ) { metro_state *state = (metro_state *)space->machine->driver_data;  metro_vram_w(space->machine, offset, data, mem_mask, 1, state->vram_1); }
WRITE16_HANDLER( metro_vram_2_w ) { metro_state *state = (metro_state *)space->machine->driver_data;  metro_vram_w(space->machine, offset, data, mem_mask, 2, state->vram_2); }



/* Dirty the relevant tilemap when its window changes */
WRITE16_HANDLER( metro_window_w )
{
	metro_state *state = (metro_state *)space->machine->driver_data;
	COMBINE_DATA(&state->window[offset]);

}



/***************************************************************************


                            Video Init Routines


***************************************************************************/

/*
 Sprites are not tile based, so we decode their graphics at runtime.

 We can't do it at startup because drawgfx requires the tiles to be
 pre-rotated to support vertical games, and that, in turn, requires
 the tile's sizes to be known at startup - which we don't!
*/

/* Dirty tilemaps when the tiles set changes */
static void dirty_tiles( running_machine *machine, int layer, UINT16 *vram )
{
	metro_state *state = (metro_state *)machine->driver_data;
	int col, row;
	int offset = 0;

	for (row = 0; row < BIG_NY; row++)
	{
		for (col = 0; col < BIG_NX; col++)
		{
			UINT16 code = vram[offset];

			offset++;

			if (!(code & 0x8000) && state->dirtyindex[(code & 0x1ff0) >> 4])
			{
				tilemap_mark_tile_dirty(state->bg_tilemap[layer], offset);
				if (state->bg_tilemap_16x16[layer])
					tilemap_mark_tile_dirty(state->bg_tilemap_16x16[layer], offset);
			}
		}
	}
}

static STATE_POSTLOAD( metro_tile_dirty_postload )
{
	metro_state *state = (metro_state *)machine->driver_data;

	/* FIXME: this is still not enough in some games (e.g. 3kokushi) */
	memset(state->dirtyindex, 1, state->tiletable_size / 4);
	dirty_tiles(machine, 0, state->vram_0);
	dirty_tiles(machine, 1, state->vram_1);
	dirty_tiles(machine, 2, state->vram_2);

	memcpy(state->tiletable_old, state->tiletable, state->tiletable_size);
}

static void alloc_empty_tiles( running_machine *machine )
{
	metro_state *state = (metro_state *)machine->driver_data;
	int code, i;

	state->empty_tiles = auto_alloc_array(machine, UINT8, 16 * 16 * 16);

	for (code = 0; code < 0x10; code++)
		for (i = 0; i < 16 * 16; i++)
			state->empty_tiles[16 * 16 * code + i] = code;
}

VIDEO_START( metro_14100 )
{
	metro_state *state = (metro_state *)machine->driver_data;

	state->support_8bpp = 0;
	state->support_16x16 = 0;
	state->has_zoom = 0;

	alloc_empty_tiles(machine);

	state->tiletable_old = auto_alloc_array(machine, UINT16, state->tiletable_size / 2);
	state->dirtyindex = auto_alloc_array(machine, UINT8, state->tiletable_size / 4);

	state->bg_tilemap[0] = tilemap_create(machine, get_tile_info_0, tilemap_scan_rows, 8, 8, BIG_NX, BIG_NY);
	state->bg_tilemap[1] = tilemap_create(machine, get_tile_info_1, tilemap_scan_rows, 8, 8, BIG_NX, BIG_NY);
	state->bg_tilemap[2] = tilemap_create(machine, get_tile_info_2, tilemap_scan_rows, 8, 8, BIG_NX, BIG_NY);

	state->bg_tilemap_16x16[0] = NULL;
	state->bg_tilemap_16x16[1] = NULL;
	state->bg_tilemap_16x16[2] = NULL;

	state_save_register_global_pointer(machine, state->tiletable_old, state->tiletable_size / 2);
	state_save_register_postload(machine, metro_tile_dirty_postload, NULL);
}

VIDEO_START( metro_14220 )
{
	metro_state *state = (metro_state *)machine->driver_data;

	state->support_8bpp = 1;
	state->support_16x16 = 0;
	state->has_zoom = 0;

	alloc_empty_tiles(machine);

	state->tiletable_old = auto_alloc_array(machine, UINT16, state->tiletable_size / 2);
	state->dirtyindex = auto_alloc_array(machine, UINT8, state->tiletable_size / 4);

	state->bg_tilemap[0] = tilemap_create(machine, get_tile_info_0_8bit, tilemap_scan_rows, 8, 8, BIG_NX, BIG_NY);
	state->bg_tilemap[1] = tilemap_create(machine, get_tile_info_1_8bit, tilemap_scan_rows, 8, 8, BIG_NX, BIG_NY);
	state->bg_tilemap[2] = tilemap_create(machine, get_tile_info_2_8bit, tilemap_scan_rows, 8, 8, BIG_NX, BIG_NY);

	state->bg_tilemap_16x16[0] = NULL;
	state->bg_tilemap_16x16[1] = NULL;
	state->bg_tilemap_16x16[2] = NULL;

	tilemap_set_scrolldx(state->bg_tilemap[0], -2, 2);
	tilemap_set_scrolldx(state->bg_tilemap[1], -2, 2);
	tilemap_set_scrolldx(state->bg_tilemap[2], -2, 2);

	state_save_register_global_pointer(machine, state->tiletable_old, state->tiletable_size / 2);
	state_save_register_postload(machine, metro_tile_dirty_postload, NULL);
}

VIDEO_START( metro_14300 )
{
	metro_state *state = (metro_state *)machine->driver_data;

	state->support_8bpp = 1;
	state->support_16x16 = 1;
	state->has_zoom = 0;

	alloc_empty_tiles(machine);

	state->tiletable_old = auto_alloc_array(machine, UINT16, state->tiletable_size / 2);
	state->dirtyindex = auto_alloc_array(machine, UINT8, state->tiletable_size / 4);

	state->bg_tilemap[0] = tilemap_create(machine, get_tile_info_0_8bit, tilemap_scan_rows, 8, 8, BIG_NX, BIG_NY);
	state->bg_tilemap[1] = tilemap_create(machine, get_tile_info_1_8bit, tilemap_scan_rows, 8, 8, BIG_NX, BIG_NY);
	state->bg_tilemap[2] = tilemap_create(machine, get_tile_info_2_8bit, tilemap_scan_rows, 8, 8, BIG_NX, BIG_NY);

	state->bg_tilemap_16x16[0] = tilemap_create(machine, get_tile_info_0_16x16_8bit, tilemap_scan_rows, 16, 16, BIG_NX, BIG_NY);
	state->bg_tilemap_16x16[1] = tilemap_create(machine, get_tile_info_1_16x16_8bit, tilemap_scan_rows, 16, 16, BIG_NX, BIG_NY);
	state->bg_tilemap_16x16[2] = tilemap_create(machine, get_tile_info_2_16x16_8bit, tilemap_scan_rows, 16, 16, BIG_NX, BIG_NY);

	state_save_register_global_pointer(machine, state->tiletable_old, state->tiletable_size / 2);
	state_save_register_postload(machine, metro_tile_dirty_postload, NULL);
}

VIDEO_START( blzntrnd )
{
	metro_state *state = (metro_state *)machine->driver_data;

	VIDEO_START_CALL(metro_14220);

	state->has_zoom = 1;

	state->k053936_tilemap = tilemap_create(machine, metro_k053936_get_tile_info, tilemap_scan_rows, 8, 8, 256, 512);

	tilemap_set_scrolldx(state->bg_tilemap[0], 8, -8);
	tilemap_set_scrolldx(state->bg_tilemap[1], 8, -8);
	tilemap_set_scrolldx(state->bg_tilemap[2], 8, -8);
}

VIDEO_START( gstrik2 )
{
	metro_state *state = (metro_state *)machine->driver_data;

	VIDEO_START_CALL(metro_14220);

	state->has_zoom = 1;

	state->k053936_tilemap = tilemap_create(machine, metro_k053936_gstrik2_get_tile_info, tilemap_scan_gstrik2, 16, 16, 128, 256);

	tilemap_set_scrolldx(state->bg_tilemap[0], 8, -8);
	tilemap_set_scrolldx(state->bg_tilemap[1], 0, 0);
	tilemap_set_scrolldx(state->bg_tilemap[2], 8, -8);
}

/***************************************************************************

                                Video Registers


        Offset:     Bits:                   Value:

        0.w                                 Number Of Sprites To Draw
        2.w         f--- ---- ---- ----     Disable Sprites Layer Priority
                    -edc ---- ---- ----
                    ---- ba-- ---- ----     Sprites Masked Layer
                    ---- --98 ---- ----     Sprites Priority
                    ---- ---- 765- ----
                    ---- ---- ---4 3210     Sprites Masked Number
        4.w                                 Sprites Y Offset
        6.w                                 Sprites X Offset
        8.w                                 Sprites Color Codes Start

        -

        10.w        fedc ba98 76-- ----
                    ---- ---- --54 ----     Layer 2 Priority (3 backmost, 0 frontmost)
                    ---- ---- ---- 32--     Layer 1 Priority
                    ---- ---- ---- --10     Layer 0 Priority

        12.w                                Backround Color


***************************************************************************/



/***************************************************************************


                                Sprites Drawing


        Offset:     Bits:                   Value:

        0.w         fedc b--- ---- ----     Priority (0 = Max)
                    ---- -a98 7654 3210     X

        2.w         fedc ba-- ---- ----     Zoom (Both X & Y)
                    ---- --98 7654 3210     Y

        4.w         f--- ---- ---- ----     Flip X
                    -e-- ---- ---- ----     Flip Y
                    --dc b--- ---- ----     Size X *
                    ---- -a98 ---- ----     Size Y *
                    ---- ---- 7654 ----     Color
                    ---- ---- ---- 3210     Code High Bits **

        6.w                                 Code Low Bits  **

*  8 pixel increments
** 8x8 pixel increments

***************************************************************************/

/* Draw sprites */

void metro_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	metro_state *state = (metro_state *)machine->driver_data;
	UINT8 *base_gfx = memory_region(machine, "gfx1");
	UINT8 *gfx_max  = base_gfx + memory_region_length(machine, "gfx1");

	int max_x = video_screen_get_width(machine->primary_screen);
	int max_y = video_screen_get_height(machine->primary_screen);

	int max_sprites = state->spriteram_size / 8;
	int sprites     = state->videoregs[0x00/2] % max_sprites;

	int color_start = ((state->videoregs[0x08/2] & 0x0f) << 4) + 0x100;

	int i, j, pri;
	static const int primask[4] = { 0x0000, 0xff00, 0xff00 | 0xf0f0, 0xff00 | 0xf0f0 | 0xcccc };

	UINT16 *src;
	int inc;

	if (sprites == 0)
		return;

	for (i = 0; i < 0x20; i++)
	{
		gfx_element gfx;

		if (!(state->videoregs[0x02/2] & 0x8000))
		{
			src = state->spriteram + (sprites - 1) * (8 / 2);
			inc = -(8 / 2);
		} else {
			src = state->spriteram;
			inc = (8 / 2);
		}

		for (j = 0; j < sprites; j++)
		{
			int x, y, attr, code, color, flipx, flipy, zoom, curr_pri, width, height;
			UINT8 *gfxdata;

			/* Exponential zoom table extracted from daitoride */
			static const int zoomtable[0x40] =
			{	0xAAC,0x800,0x668,0x554,0x494,0x400,0x390,0x334,
				0x2E8,0x2AC,0x278,0x248,0x224,0x200,0x1E0,0x1C8,
				0x1B0,0x198,0x188,0x174,0x164,0x154,0x148,0x13C,
				0x130,0x124,0x11C,0x110,0x108,0x100,0x0F8,0x0F0,
				0x0EC,0x0E4,0x0DC,0x0D8,0x0D4,0x0CC,0x0C8,0x0C4,
				0x0C0,0x0BC,0x0B8,0x0B4,0x0B0,0x0AC,0x0A8,0x0A4,
				0x0A0,0x09C,0x098,0x094,0x090,0x08C,0x088,0x080,
				0x078,0x070,0x068,0x060,0x058,0x050,0x048,0x040	};

			x = src[0];
			curr_pri = (x & 0xf800) >> 11;

			if ((curr_pri == 0x1f) || (curr_pri != i))
			{
				src += inc;
				continue;
			}

			pri = (state->videoregs[0x02/2] & 0x0300) >> 8;

			if (!(state->videoregs[0x02/2] & 0x8000))
			{
				if (curr_pri > (state->videoregs[0x02/2] & 0x1f))
					pri = (state->videoregs[0x02/2] & 0x0c00) >> 10;
			}

			y     = src[1];
			attr  = src[2];
			code  = src[3];

			flipx =  attr & 0x8000;
			flipy =  attr & 0x4000;
			color = (attr & 0xf0) >> 4;

			zoom = zoomtable[(y & 0xfc00) >> 10] << (16 - 8);

			x = (x & 0x07ff) - state->sprite_xoffs;
			y = (y & 0x03ff) - state->sprite_yoffs;

			width  = (((attr >> 11) & 0x7) + 1) * 8;
			height = (((attr >>  8) & 0x7) + 1) * 8;

			gfxdata = base_gfx + (8 * 8 * 4 / 8) * (((attr & 0x000f) << 16) + code);

			if (state->flip_screen)
			{
				flipx = !flipx;		x = max_x - x - width;
				flipy = !flipy;		y = max_y - y - height;
			}

			if (state->support_8bpp && color == 0xf)	/* 8bpp */
			{
				/* Bounds checking */
				if ((gfxdata + width * height - 1) >= gfx_max)
					continue;

				gfx_element_build_temporary(&gfx, machine, gfxdata, width, height, width, 0, 256, 0);

				pdrawgfxzoom_transpen(	bitmap,cliprect, &gfx,
								0,
								color_start >> 4,
								flipx, flipy,
								x, y,
								zoom, zoom,
								machine->priority_bitmap,primask[pri], 255);
			}
			else
			{
				/* Bounds checking */
				if ((gfxdata + width / 2 * height - 1) >= gfx_max)
					continue;

				gfx_element_build_temporary(&gfx, machine, gfxdata, width, height, width/2, 0, 16, GFX_ELEMENT_PACKED);

				pdrawgfxzoom_transpen(	bitmap,cliprect, &gfx,
								0,
								color + color_start,
								flipx, flipy,
								x, y,
								zoom, zoom,
								machine->priority_bitmap,primask[pri], 15);
			}
#if 0
{	/* Display priority + zoom on each sprite */
	char buf[80];
	sprintf(buf, "%02X %02X", ((src[0] & 0xf800) >> 11) ^ 0x1f, ((src[1] & 0xfc00) >> 10));
	ui_draw_text(buf, x, y);
}
#endif
			src += inc;
		}
	}
}



/***************************************************************************


                                Screen Drawing


***************************************************************************/

/* copy a 'window' from the large 2048x2048 (or 4096x4096 for 16x16 tiles) tilemap */

static void draw_tilemap( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, tilemap_t *tmap, UINT32 flags, UINT32 pcode,
						 int sx, int sy, int wx, int wy )	// scroll & window values
{
	metro_state *state = (metro_state *)machine->driver_data;

	UINT16 *srcpixline;
	UINT8 *srcflgline;
	int y;

	bitmap_t *pixdata = tilemap_get_pixmap(tmap);
	bitmap_t *flgdata = tilemap_get_flagsmap(tmap);
	bitmap_t *priority_bitmap = machine->priority_bitmap;

	int width = pixdata->width;
	int height = pixdata->height;
	
	int scrwidth = bitmap->width;
	int scrheight = bitmap->height;

	int windowwidth = width >> 2;
	int windowheight = height >> 3;

	if (tilemap_get_enable(tmap)==0) return;

	if (!state->flip_screen)
	{
		sx -= tilemap_get_scrolldx(tmap);
		sy -= tilemap_get_scrolldy(tmap);
	}
	else
	{
		sx += tilemap_get_scrolldx(tmap);
		sy += tilemap_get_scrolldy(tmap);
	}

	for (y=0;y<scrheight;y++)
	{
		int scrolly = (sy+y-wy)&(windowheight-1);
		int x;
		UINT16* dst;
		UINT8 *priority_baseaddr;

		srcpixline = BITMAP_ADDR16(pixdata,(wy+scrolly)&(height-1), 0);
		srcflgline = BITMAP_ADDR8(flgdata,(wy+scrolly)&(height-1), 0);
		
		if (!state->flip_screen)
		{
			dst = BITMAP_ADDR16(bitmap, y, 0);
			priority_baseaddr = BITMAP_ADDR8(priority_bitmap, y, 0);

			for (x=0;x<scrwidth;x++)
			{
				int scrollx = (sx+x-wx)&(windowwidth-1);
				UINT16 dat;
				UINT8 flg;

				dat = srcpixline[(wx+scrollx)&(width-1)];
				flg = srcflgline[(wx+scrollx)&(width-1)];

				
				if (flg&0xf) // check category bits to see if we're 4bpp/8bpp
				{
					// 8bpp tile data
					if ((dat&0xff) != 0xff)
					{
						dst[x] = dat;
						priority_baseaddr[x] = (priority_baseaddr[x] & (pcode >> 8)) | pcode;
					}
				}
				else
				{
					// 4bpp tile data
					if ((dat&0xf) != 0xf)
					{
						dst[x] = dat;
						priority_baseaddr[x] = (priority_baseaddr[x] & (pcode >> 8)) | pcode;

					}
				}
			}
		}
		else // flipped case
		{
			dst = BITMAP_ADDR16(bitmap, scrheight-y-1, 0);
			priority_baseaddr = BITMAP_ADDR8(priority_bitmap, scrheight-y-1, 0);
			
			for (x=0;x<scrwidth;x++)
			{
				int scrollx = (sx+x-wx)&(windowwidth-1);
				UINT16 dat;
				UINT8 flg;

				dat = srcpixline[(wx+scrollx)&(width-1)];
				flg = srcflgline[(wx+scrollx)&(width-1)];

				
				if (flg&0xf) // check category bits to see if we're 4bpp/8bpp
				{
					// 8bpp tile data
					if ((dat&0xff) != 0xff)
					{
						dst[scrwidth-x-1] = dat;
						priority_baseaddr[scrwidth-x-1] = (priority_baseaddr[scrwidth-x-1] & (pcode >> 8)) | pcode;
					}
				}
				else
				{
					// 4bpp tile data
					if ((dat&0xf) != 0xf)
					{
						dst[scrwidth-x-1] = dat;
						priority_baseaddr[scrwidth-x-1] = (priority_baseaddr[scrwidth-x-1] & (pcode >> 8)) | pcode;
					}
				}
			}
		}
	}
}

/* Draw all the layers that match the given priority */
static void draw_layers( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int pri, int layers_ctrl )
{
	metro_state *state = (metro_state *)machine->driver_data;
	UINT16 layers_pri = state->videoregs[0x10 / 2];
	int layer;

	/* Draw all the layers with priority == pri */
	for (layer = 2; layer >= 0; layer--)	// tilemap[2] below?
	{
		if (pri == ((layers_pri >> (layer * 2)) & 3))
		{
			/* Scroll and Window values */
			UINT16 sy = state->scroll[layer * 2 + 0];	UINT16 sx = state->scroll[layer * 2 + 1];
			UINT16 wy = state->window[layer * 2 + 0];	UINT16 wx = state->window[layer * 2 + 1];

			if (BIT(layers_ctrl, layer))	// for debug
			{
				draw_tilemap(machine, bitmap, cliprect, state->bg_tilemap[layer], 0, 1 << (3 - pri), sx, sy, wx, wy);
				if (state->bg_tilemap_16x16[layer]) 
					draw_tilemap(machine, bitmap, cliprect, state->bg_tilemap_16x16[layer], 0, 1 << (3 - pri), sx, sy, wx, wy);
			}
		}
	}
}



VIDEO_UPDATE( metro )
{
	metro_state *state = (metro_state *)screen->machine->driver_data;
	int i, pri, layers_ctrl = -1;
	UINT16 screenctrl = *state->screenctrl;

	{
		int dirty = 0;

		memset(state->dirtyindex, 0, state->tiletable_size / 4);
		for (i = 0; i < state->tiletable_size / 4; i++)
		{
			UINT32 tile_new = (state->tiletable[2 * i + 0] << 16 ) + state->tiletable[2 * i + 1];
			UINT32 tile_old = (state->tiletable_old[2 * i + 0] << 16 ) + state->tiletable_old[2 * i + 1];

			if ((tile_new ^ tile_old) & 0x0fffffff)
			{
				state->dirtyindex[i] = 1;
				dirty = 1;
			}
		}
		memcpy(state->tiletable_old, state->tiletable, state->tiletable_size);

		if (dirty)
		{
			dirty_tiles(screen->machine, 0, state->vram_0);
			dirty_tiles(screen->machine, 1, state->vram_1);
			dirty_tiles(screen->machine, 2, state->vram_2);
		}
	}

	state->sprite_xoffs = state->videoregs[0x06 / 2] - video_screen_get_width(screen)  / 2;
	state->sprite_yoffs = state->videoregs[0x04 / 2] - video_screen_get_height(screen) / 2;

	/* The background color is selected by a register */
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, ((state->videoregs[0x12/2] & 0x0fff)) + 0x1000);

	/*  Screen Control Register:

        f--- ---- ---- ----     ?
        -edc b--- ---- ----
        ---- -a98 ---- ----     ? Leds
        ---- ---- 7--- ----     16x16 Tiles (Layer 2)
        ---- ---- -6-- ----     16x16 Tiles (Layer 1)
        ---- ---- --5- ----     16x16 Tiles (Layer 0)
        ---- ---- ---4 32--
        ---- ---- ---- --1-     ? Blank Screen
        ---- ---- ---- ---0     Flip  Screen    */
	if (screenctrl & 2)
		return 0;

	//flip_screen_set(screen->machine, screenctrl & 1);
	state->flip_screen = screenctrl & 1;

	/* If the game supports 16x16 tiles, make sure that the
       16x16 and 8x8 tilemaps of a given layer are not simultaneously
       enabled! */
	if (state->support_16x16)
	{
		int layer;

		for (layer = 0; layer < 3; layer++)
		{
			int big = screenctrl & (0x0020 << layer);

			tilemap_set_enable(state->bg_tilemap[layer],!big);
			tilemap_set_enable(state->bg_tilemap_16x16[layer], big);
		}
	}


#ifdef MAME_DEBUG
if (input_code_pressed(screen->machine, KEYCODE_Z))
{
	int msk = 0;
	if (input_code_pressed(screen->machine, KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(screen->machine, KEYCODE_W))	msk |= 2;
	if (input_code_pressed(screen->machine, KEYCODE_E))	msk |= 4;
	if (input_code_pressed(screen->machine, KEYCODE_A))	msk |= 8;
	if (msk != 0)
	{
		bitmap_fill(bitmap, cliprect, 0);
		layers_ctrl &= msk;
	}

	popmessage("l %x-%x-%x r %04x %04x %04x",
				(state->videoregs[0x10/2] & 0x30) >> 4, (state->videoregs[0x10/2] & 0xc) >> 2, state->videoregs[0x10/2] & 3,
				state->videoregs[0x02/2], state->videoregs[0x12/2],
				*state->screenctrl);
}
#endif

	if (state->has_zoom)
		k053936_zoom_draw(state->k053936, bitmap, cliprect, state->k053936_tilemap, 0, 0, 1);

	for (pri = 3; pri >= 0; pri--)
		draw_layers(screen->machine, bitmap, cliprect, pri, layers_ctrl);

	if (layers_ctrl & 0x08)
		metro_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
