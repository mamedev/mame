/***************************************************************************

                              -= Cave Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing:

        X/C/V/B/Z  with  Q   shows layer 0 (tiles with priority 0/1/2/3/All)
        X/C/V/B/Z  with  W   shows layer 1 (tiles with priority 0/1/2/3/All)
        X/C/V/B/Z  with  E   shows layer 2 (tiles with priority 0/1/2/3/All)
        X/C/V/B/Z  with  R   shows layer 3 (tiles with priority 0/1/2/3/All)
        X/C/V/B/Z  with  A   shows sprites (tiles with priority 0/1/2/3/All)

        Keys can be used together!

    [ 1 Layer per chip (games use as many as 4 chips) ]

        Layer Size:             512 x 512
        Tiles:                  8 x 8 & 16 x 16.

        There are 2 tilemaps in memory, one per tiles dimension.
        A bit decides which one gets displayed.
        The tiles depth varies with games, from 16 to 256 colors.

        A per layer row-scroll / row-select effect can be enabled:

        a different scroll value is fetched (from tile RAM) for each
        scan line, and a different tilemap line for each scan line

    [ 1024 Zooming Sprites ]

        There are 2 sprite RAMs. A hardware register's bit selects
        the one to display (sprites double buffering).

        The sprites are NOT tile based: the "tile" size and start address
        is selectable for each sprite with a 16 pixel granularity.

        Also note that the zoom is of a peculiar type: pixels are never
        drawn more than once. So shrinking works as usual (some pixels are
        just not drawn) while enlarging adds some transparent pixels to
        the image, uniformly, to reach the final size.

**************************************************************************/

#include "emu.h"
#include "includes/cave.h"


#define CAVE_SPRITETYPE_ZBUF        0x01
#define CAVE_SPRITETYPE_ZOOM        0x02

#define SPRITE_FLIPX_CAVE           0x01
#define SPRITE_FLIPY_CAVE           0x02
#define SPRITE_VISIBLE_CAVE         0x04

#define SWAP(X,Y) { int temp = X; X = Y; Y = temp; }

static void sprite_init_cave(running_machine *machine);
static void sprite_draw_cave(running_machine *machine, int priority);
static void sprite_draw_cave_zbuf(running_machine *machine, int priority);
static void sprite_draw_donpachi(running_machine *machine, int priority);
static void sprite_draw_donpachi_zbuf(running_machine *machine, int priority);
static STATE_POSTLOAD(cave_sprite_postload);

/***************************************************************************

                            Palette Init Routines

    Function needed for games with 4 bit sprites, rather than 8 bit,
    or games with an odd colors mapping for the layers.

***************************************************************************/

PALETTE_INIT( cave )
{
	cave_state *state = machine->driver_data<cave_state>();
	int maxpen = state->paletteram_size / 2;
	int pen;

	/* create a 1:1 palette map covering everything */
	state->palette_map = auto_alloc_array(machine, UINT16, machine->total_colors());

	for (pen = 0; pen < machine->total_colors(); pen++)
		state->palette_map[pen] = pen % maxpen;
}

PALETTE_INIT( dfeveron )
{
	cave_state *state = machine->driver_data<cave_state>();
	int color, pen;

	/* Fill the 0-3fff range, used by sprites ($40 color codes * $100 pens)
       Here sprites have 16 pens, but the sprite drawing routine always
       multiplies the color code by $100 (for consistency).
       That's why we need this function.    */

	PALETTE_INIT_CALL(cave);

	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x10; pen++)
			state->palette_map[(color << 8) | pen] = (color << 4) | pen;
}

PALETTE_INIT( ddonpach )
{
	cave_state *state = machine->driver_data<cave_state>();
	int color, pen;

	/* Fill the 8000-83ff range ($40 color codes * $10 pens) for
       layers 0 & 1 which are 4 bits deep rather than 8 bits deep
       like layer 2, but use the first 16 color of every 256 for
       any given color code. */

	PALETTE_INIT_CALL(cave);

	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x10; pen++)
			state->palette_map[0x8000 | (color << 4) | pen] = 0x4000 | (color << 8) | pen;
}

PALETTE_INIT( mazinger )
{
	cave_state *state = machine->driver_data<cave_state>();
	int color, pen;

	PALETTE_INIT_CALL(cave);

	/* sprites (encrypted) are 4 bit deep */
	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x100; pen++)
			state->palette_map[(color << 8) | pen] = (color << 4) + pen;	/* yes, PLUS, not OR */

	/* layer 0 is 6 bit deep, there are 64 color codes but only $400
       colors are actually addressable */
	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x40; pen++)
			state->palette_map[0x4400 + ((color << 6) | pen)] = 0x400 | ((color & 0x0f) << 6) | pen;
}

PALETTE_INIT( sailormn )
{
	cave_state *state = machine->driver_data<cave_state>();
	int color, pen;

	PALETTE_INIT_CALL(cave);

	/* sprites (encrypted) are 4 bit deep */
	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x100; pen++)
			state->palette_map[(color << 8) | pen] = (color << 4) + pen;	/* yes, PLUS, not OR */

	/* layer 2 is 6 bit deep, there are 64 color codes but only $400
       colors are actually addressable */
	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x40; pen++)
			state->palette_map[0x4c00 | (color << 6) | pen] = 0xc00 | ((color & 0x0f) << 6) | pen;
}

PALETTE_INIT( pwrinst2 )
{
	cave_state *state = machine->driver_data<cave_state>();
	int color, pen;

	PALETTE_INIT_CALL(cave);

	for (color = 0; color < 0x80; color++)
		for (pen = 0; pen < 0x10; pen++)
			state->palette_map[(color << 8) | pen] = (color << 4) | pen;

	for (pen = 0x8000; pen < 0xa800; pen++)
			state->palette_map[pen] = pen - 0x8000;
}

PALETTE_INIT( korokoro )
{
	cave_state *state = machine->driver_data<cave_state>();
	int color, pen;

	PALETTE_INIT_CALL(cave);

	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x10; pen++)
			state->palette_map[(color << 8) | pen] = 0x3c00 | (color << 4) | pen;
}


static void set_pens( running_machine *machine )
{
	cave_state *state = machine->driver_data<cave_state>();
	int pen;

	for (pen = 0; pen < machine->total_colors(); pen++)
	{
		UINT16 data = state->paletteram[state->palette_map[pen]];

		rgb_t color = MAKE_RGB(pal5bit(data >> 5), pal5bit(data >> 10), pal5bit(data >> 0));

		palette_set_color(machine, pen, color);
	}
}


/***************************************************************************

                                  Tiles Format

    Offset:     Bits:                   Value:

    0.w         fe-- ---- ---- ---      Priority
                --dc ba98 ---- ----     Color
                ---- ---- 7654 3210

    2.w                                 Code


    When a row-scroll / row-select effect is enabled, the scroll values are
    fetched starting from tile RAM + $1000, 4 bytes per scan line:

    Offset:     Value:

    0.w         Tilemap line to display
    2.w         X Scroll value

***************************************************************************/

INLINE void get_tile_info( running_machine *machine, tile_data *tileinfo, int tile_index, int GFX, UINT16 *VRAM, int TDIM )
{
	UINT32 code, color, pri, tile;

	if (TDIM)
	{
		tile  = (tile_index % (512 / 8)) / 2 + ((tile_index / (512 / 8)) / 2) * (512 / 16);
		code  = (VRAM[tile * 2 + 0x0000 / 2] << 16) + VRAM[tile * 2 + 0x0002 / 2];

		color	= (code & 0x3f000000) >> (32-8);
		pri   = (code & 0xc0000000) >> (32-2);
		code  = (code & 0x00ffffff) * 4;

		code += tile_index & 1;
		code += ((tile_index / (512 / 8)) & 1) * 2;
	}
	else
	{
		code  = (VRAM[tile_index * 2 + 0x4000 / 2] << 16) + VRAM[tile_index * 2 + 0x4002 / 2];

		color = (code & 0x3f000000) >> (32 - 8);
		pri   = (code & 0xc0000000) >> (32 - 2);
		code  = (code & 0x00ffffff);
	}

	SET_TILE_INFO( GFX, code, color, 0 );
	tileinfo->category = pri;
}


/* Sailormn: the lower 2 Megabytes of tiles banked */

void sailormn_tilebank_w( running_machine *machine, int bank )
{
	cave_state *state = machine->driver_data<cave_state>();
	if (state->sailormn_tilebank != bank)
	{
		state->sailormn_tilebank = bank;
		tilemap_mark_all_tiles_dirty(state->tilemap_2);
	}
}

static TILE_GET_INFO( sailormn_get_tile_info_2 )
{
	cave_state *state = machine->driver_data<cave_state>();
	UINT32 code, color, pri;

	if (state->tiledim_2)
	{
		UINT32 tile;
		tile  = (tile_index % (512 / 8)) / 2 + ((tile_index / (512 / 8)) / 2) * (512 / 16);
		code  = (state->vram_2[tile * 2 + 0x0000 / 2] << 16) + state->vram_2[tile * 2 + 0x0002 / 2];

		color = (code & 0x3f000000) >> (32 - 8);
		pri   = (code & 0xc0000000) >> (32 - 2);
		code  = (code & 0x00ffffff) * 4;

		code += tile_index & 1;
		code += ((tile_index / (512 / 8)) & 1) * 2;
	}
	else
	{
		code  = (state->vram_2[tile_index * 2 + 0x4000 / 2] << 16) + state->vram_2[tile_index * 2 + 0x4002 / 2];

		color = (code & 0x3f000000) >> (32 - 8);
		pri   = (code & 0xc0000000) >> (32 - 2);
		code  = (code & 0x00ffffff);
		if ((code < 0x10000) && (state->sailormn_tilebank))
			code += 0x40000;
	}

	SET_TILE_INFO( 2, code, color, 0 );
	tileinfo->category = pri;
}


INLINE void vram_w( UINT16 *VRAM, tilemap_t *TILEMAP, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask )
{
	if ((VRAM[offset] & mem_mask) == (data & mem_mask))
			return;

	COMBINE_DATA(&VRAM[offset]);
	offset /= 2;
	if (offset < 0x1000 / 4)	// 16x16 tilemap
	{
		offset = (offset % (512 / 16)) * 2 + (offset / (512 / 16)) * (512 / 8) * 2;
		tilemap_mark_tile_dirty(TILEMAP, offset + 0);
		tilemap_mark_tile_dirty(TILEMAP, offset + 1);
		tilemap_mark_tile_dirty(TILEMAP, offset + 0 + 512 / 8);
		tilemap_mark_tile_dirty(TILEMAP, offset + 1 + 512 / 8);
	}
	else if (offset >= 0x4000 / 4)		// 8x8 tilemap
		tilemap_mark_tile_dirty(TILEMAP, offset - 0x4000 / 4);
}

/*  Some games, that only ever use the 8x8 tiles and no line scroll,
    use mirror ram. For example in donpachi, writes to 400000-403fff
    and 408000-407fff both go to the 8x8 tilemap ram. Use this function
    in this cases. Note that the get_tile_info function looks in the
    4000-7fff range for tiles, so we have to write the data there. */
INLINE void vram_8x8_w( UINT16 *VRAM, tilemap_t *TILEMAP, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask )
{
	offset %= 0x4000 / 2;
	if ((VRAM[offset] & mem_mask) == (data & mem_mask))
		return;

	COMBINE_DATA(&VRAM[offset + 0x0000 / 2]);
	COMBINE_DATA(&VRAM[offset + 0x4000 / 2]);
	tilemap_mark_tile_dirty(TILEMAP,offset / 2);
}


static TILE_GET_INFO( get_tile_info_0 )	{ cave_state *state = machine->driver_data<cave_state>(); get_tile_info(machine, tileinfo, tile_index, 0, state->vram_0, state->tiledim_0); }
static TILE_GET_INFO( get_tile_info_1 )	{ cave_state *state = machine->driver_data<cave_state>(); get_tile_info(machine, tileinfo, tile_index, 1, state->vram_1, state->tiledim_1); }
static TILE_GET_INFO( get_tile_info_2 )	{ cave_state *state = machine->driver_data<cave_state>(); get_tile_info(machine, tileinfo, tile_index, 2, state->vram_2, state->tiledim_2); }
static TILE_GET_INFO( get_tile_info_3 )	{ cave_state *state = machine->driver_data<cave_state>(); get_tile_info(machine, tileinfo, tile_index, 3, state->vram_3, state->tiledim_3); }

WRITE16_HANDLER( cave_vram_0_w )
{
	cave_state *state = space->machine->driver_data<cave_state>();
	vram_w(state->vram_0, state->tilemap_0, offset, data, mem_mask);
}

WRITE16_HANDLER( cave_vram_0_8x8_w )
{
	cave_state *state = space->machine->driver_data<cave_state>();
	vram_8x8_w(state->vram_0, state->tilemap_0, offset, data, mem_mask);
}

WRITE16_HANDLER( cave_vram_1_w )
{
	cave_state *state = space->machine->driver_data<cave_state>();
	vram_w(state->vram_1, state->tilemap_1, offset, data, mem_mask);
}

WRITE16_HANDLER( cave_vram_1_8x8_w )
{
	cave_state *state = space->machine->driver_data<cave_state>();
	vram_8x8_w(state->vram_1, state->tilemap_1, offset, data, mem_mask);
}

WRITE16_HANDLER( cave_vram_2_w )
{
	cave_state *state = space->machine->driver_data<cave_state>();
	vram_w(state->vram_2, state->tilemap_2, offset, data, mem_mask);
}

WRITE16_HANDLER( cave_vram_2_8x8_w )
{
	cave_state *state = space->machine->driver_data<cave_state>();
	vram_8x8_w(state->vram_2, state->tilemap_2, offset, data, mem_mask);
}

WRITE16_HANDLER( cave_vram_3_w )
{
	cave_state *state = space->machine->driver_data<cave_state>();
	vram_w(state->vram_3, state->tilemap_3, offset, data, mem_mask);
}

WRITE16_HANDLER( cave_vram_3_8x8_w )
{
	cave_state *state = space->machine->driver_data<cave_state>();
	vram_8x8_w(state->vram_3, state->tilemap_3, offset, data, mem_mask);
}


/***************************************************************************

                            Video Init Routines

    Depending on the game, there can be from 1 to 4 layers and the
    tile sizes can be 8x8 or 16x16.

***************************************************************************/

static void cave_vh_start( running_machine *machine, int num )
{
	cave_state *state = machine->driver_data<cave_state>();

	assert(state->palette_map != NULL);

	state->tilemap_0 = 0;
	state->tilemap_1 = 0;
	state->tilemap_2 = 0;
	state->tilemap_3 = 0;

	state->tiledim_0 = 0;
	state->tiledim_1 = 0;
	state->tiledim_2 = 0;
	state->tiledim_3 = 0;

	state->old_tiledim_0 = 0;
	state->old_tiledim_1 = 0;
	state->old_tiledim_2 = 0;
	state->old_tiledim_3 = 0;

	assert((num >= 1) && (num <= 4));

	switch (num)
	{
		case 4:
			state->tilemap_3 = tilemap_create(machine, get_tile_info_3, tilemap_scan_rows, 8, 8, 512 / 8, 512 / 8);
			tilemap_set_transparent_pen(state->tilemap_3, 0);
			tilemap_set_scroll_rows(state->tilemap_3, 1);
			tilemap_set_scroll_cols(state->tilemap_3, 1);
			state_save_register_global(machine, state->tiledim_3);
			state_save_register_global(machine, state->old_tiledim_3);

		case 3:
			state->tilemap_2 = tilemap_create(machine, get_tile_info_2, tilemap_scan_rows, 8, 8, 512 / 8, 512 / 8);
			tilemap_set_transparent_pen(state->tilemap_2, 0);
			tilemap_set_scroll_rows(state->tilemap_2, 1);
			tilemap_set_scroll_cols(state->tilemap_2, 1);
			state_save_register_global(machine, state->tiledim_2);
			state_save_register_global(machine, state->old_tiledim_2);

		case 2:
			state->tilemap_1 = tilemap_create(machine, get_tile_info_1, tilemap_scan_rows, 8, 8, 512 / 8, 512 / 8);
			tilemap_set_transparent_pen(state->tilemap_1, 0);
			tilemap_set_scroll_rows(state->tilemap_1, 1);
			tilemap_set_scroll_cols(state->tilemap_1, 1);
			state_save_register_global(machine, state->tiledim_1);
			state_save_register_global(machine, state->old_tiledim_1);

		case 1:
			state->tilemap_0 = tilemap_create(machine, get_tile_info_0, tilemap_scan_rows, 8, 8, 512 / 8, 512 / 8);
			tilemap_set_transparent_pen(state->tilemap_0, 0);
			tilemap_set_scroll_rows(state->tilemap_0, 1);
			tilemap_set_scroll_cols(state->tilemap_0, 1);
			state_save_register_global(machine, state->tiledim_0);
			state_save_register_global(machine, state->old_tiledim_0);

			break;
	}

	sprite_init_cave(machine);

	state->layers_offs_x = 0x13;
	state->layers_offs_y = -0x12;

	state->row_effect_offs_n = -1;
	state->row_effect_offs_f = 1;

	state->background_color = machine->config->m_gfxdecodeinfo[0].color_codes_start +
					(machine->config->m_gfxdecodeinfo[0].total_color_codes - 1) *
						machine->gfx[0]->color_granularity;

	switch (state->kludge)
	{
		case 1:	/* sailormn */
			state->row_effect_offs_n = -1;
			state->row_effect_offs_f = -1;
			break;
		case 2:	/* uopoko dfeveron */
			state->background_color = 0x3f00;
			break;
		case 4:	/* pwrinst2 */
			state->background_color = 0x7f00;
			state->layers_offs_y++;
			break;
	}
}

VIDEO_START( cave_1_layer  )	{	cave_vh_start(machine, 1);	}
VIDEO_START( cave_2_layers )	{	cave_vh_start(machine, 2);	}
VIDEO_START( cave_3_layers )	{	cave_vh_start(machine, 3);	}
VIDEO_START( cave_4_layers )	{	cave_vh_start(machine, 4);	}


VIDEO_START( sailormn_3_layers )
{
	cave_state *state = machine->driver_data<cave_state>();
	cave_vh_start(machine, 2);

	/* Layer 2 (8x8) needs to be handled differently */
	state->tilemap_2 = tilemap_create(machine, sailormn_get_tile_info_2, tilemap_scan_rows, 8, 8, 512 / 8, 512 / 8 );

	tilemap_set_transparent_pen(state->tilemap_2, 0);
	tilemap_set_scroll_rows(state->tilemap_2, 1);
	tilemap_set_scroll_cols(state->tilemap_2, 1);
}

/***************************************************************************

                                Sprites Drawing

    Offset:     Bits:                   Value:

    00.w        fedc ba98 76-- ----     X Position
                ---- ---- --54 3210

    02.w        fedc ba98 76-- ----     Y Position
                ---- ---- --54 3210

    04.w        fe-- ---- ---- ----
                --dc ba98 ---- ----     Color
                ---- ---- 76-- ----
                ---- ---- --54 ----     Priority
                ---- ---- ---- 3---     Flip X
                ---- ---- ---- -2--     Flip Y
                ---- ---- ---- --10     Code High Bit(s?)

    06.w                                Code Low Bits

    08/0A.w                             Zoom X / Y

    0C.w        fedc ba98 ---- ----     Tile Size X
                ---- ---- 7654 3210     Tile Size Y

    0E.w                                Unused


***************************************************************************/

static void get_sprite_info_cave( running_machine *machine )
{
	cave_state *state = machine->driver_data<cave_state>();
	pen_t base_pal = 0;
	const UINT8 *base_gfx = memory_region(machine, "sprites");
	int code_max = memory_region_length(machine, "sprites") / (16*16);

	UINT16 *source;
	UINT16 *finish;
	struct sprite_cave *sprite = state->sprite;

	int glob_flipx = state->videoregs[0] & 0x8000;
	int glob_flipy = state->videoregs[1] & 0x8000;

	int max_x = machine->primary_screen->width();
	int max_y = machine->primary_screen->height();

	source = state->spriteram + ((state->spriteram_size / 2) / 2) * state->spriteram_bank;

	if (state->videoregs[4] & 0x02)
		if (state->spriteram_2)
			source = state->spriteram_2 + ((state->spriteram_size / 2) / 2) * state->spriteram_bank;

	finish = source + ((state->spriteram_size / 2) / 2);


	for (; source < finish; source += 8)
	{
		int x, y, attr, code, zoomx, zoomy, size, flipx, flipy;
		int total_width_f, total_height_f;

		if (state->spritetype[0] == 2)	/* Hot Dog Storm */
		{
			x = (source[0] & 0x3ff) << 8;
			y = (source[1] & 0x3ff) << 8;
		}
		else						/* all others */
		{
			x = source[0] << 2;
			y = source[1] << 2;
		}
		attr  = source[2];
		code  = source[3] + ((attr & 3) << 16);
		zoomx = source[4];
		zoomy = source[5];
		size  = source[6];

		sprite->tile_width  = ((size >> 8) & 0x1f) * 16;
		sprite->tile_height = ((size >> 0) & 0x1f) * 16;

		if (!sprite->tile_width || !sprite->tile_height)
			continue;

		/* Bound checking */
		code %= code_max;
		sprite->pen_data = base_gfx + (16 * 16) * code;

		flipx = attr & 0x0008;
		flipy = attr & 0x0004;

		sprite->total_width  = (total_width_f  = sprite->tile_width  * zoomx) / 0x100;
		sprite->total_height = (total_height_f = sprite->tile_height * zoomy) / 0x100;

		if (sprite->total_width <= 1)
		{
			sprite->total_width = 1;
			sprite->zoomx_re = sprite->tile_width << 16;
			sprite->xcount0 = sprite->zoomx_re / 2;
			x -= 0x80;
		}
		else
		{
			sprite->zoomx_re = 0x1000000 / zoomx;
			sprite->xcount0 = sprite->zoomx_re - 1;
		}

		if (sprite->total_height <= 1)
		{
			sprite->total_height = 1;
			sprite->zoomy_re = sprite->tile_height << 16;
			sprite->ycount0 = sprite->zoomy_re / 2;
			y -= 0x80;
		}
		else
		{
			sprite->zoomy_re = 0x1000000 / zoomy;
			sprite->ycount0 = sprite->zoomy_re - 1;
		}

		if (state->spritetype[0] == 2)
		{
			x >>= 8;
			y >>= 8;
			if (flipx && (zoomx != 0x100)) x += sprite->tile_width - sprite->total_width;
			if (flipy && (zoomy != 0x100)) y += sprite->tile_height - sprite->total_height;
		}
		else
		{
			if (flipx && (zoomx != 0x100)) x += (sprite->tile_width << 8) - total_width_f - 0x80;
			if (flipy && (zoomy != 0x100)) y += (sprite->tile_height << 8) - total_height_f - 0x80;
			x >>= 8;
			y >>= 8;
		}

		if (x > 0x1ff)	x -= 0x400;
		if (y > 0x1ff)	y -= 0x400;

		if (x + sprite->total_width <= 0 || x >= max_x || y + sprite->total_height <= 0 || y >= max_y )
		{continue;}

		sprite->priority    = (attr & 0x0030) >> 4;
		sprite->flags       = SPRITE_VISIBLE_CAVE;
		sprite->line_offset = sprite->tile_width;
		sprite->base_pen    = base_pal + (attr & 0x3f00);	// first 0x4000 colors

		if (glob_flipx)	{ x = max_x - x - sprite->total_width;	flipx = !flipx; }
		if (glob_flipy)	{ y = max_y - y - sprite->total_height;	flipy = !flipy; }

		sprite->x = x;
		sprite->y = y;

		if (flipx)	sprite->flags |= SPRITE_FLIPX_CAVE;
		if (flipy)	sprite->flags |= SPRITE_FLIPY_CAVE;

		sprite++;
	}
	state->num_sprites = sprite - state->sprite;
}

static void get_sprite_info_donpachi( running_machine *machine )
{
	cave_state *state = machine->driver_data<cave_state>();
	pen_t base_pal = 0;
	const UINT8 *base_gfx = memory_region(machine, "sprites");
	int code_max = memory_region_length(machine, "sprites") / (16*16);

	UINT16 *source;
	UINT16 *finish;

	struct sprite_cave *sprite = state->sprite;

	int glob_flipx = state->videoregs[0] & 0x8000;
	int glob_flipy = state->videoregs[1] & 0x8000;

	int max_x = machine->primary_screen->width();
	int max_y = machine->primary_screen->height();

	source = state->spriteram + ((state->spriteram_size / 2) / 2) * state->spriteram_bank;

	if (state->videoregs[4] & 0x02)
		if (state->spriteram_2)
			source = state->spriteram_2 + ((state->spriteram_size / 2) / 2) * state->spriteram_bank;

	finish = source + ((state->spriteram_size / 2) / 2);

	for (; source < finish; source += 8)
	{
		int x, y, attr, code, size, flipx, flipy;

		attr = source[0];
		code = source[1] + ((attr & 3) << 16);
		x    = source[2] & 0x3ff;

		if (state->spritetype[0] == 3)	/* pwrinst2 */
			y = (source[3] + 1) & 0x3ff;
		else
			y = source[3] & 0x3ff;

		size = source[4];

		sprite->tile_width  = sprite->total_width  = ((size >> 8) & 0x1f) * 16;
		sprite->tile_height = sprite->total_height = ((size >> 0) & 0x1f) * 16;

		/* Bound checking */
		code %= code_max;
		sprite->pen_data = base_gfx + (16*16) * code;

		if (x > 0x1ff)	x -= 0x400;
		if (y > 0x1ff)	y -= 0x400;

		if (!sprite->tile_width || !sprite->tile_height ||
			x + sprite->total_width <= 0 || x >= max_x || y + sprite->total_height <= 0 || y >= max_y )
		{continue;}

		flipx	 = attr & 0x0008;
		flipy	 = attr & 0x0004;

		if (state->spritetype[0] == 3)	/* pwrinst2 */
		{
			sprite->priority = ((attr & 0x0010) >> 4) + 2;
			sprite->base_pen = base_pal + (attr & 0x3f00) + 0x4000 * ((attr & 0x0020) >> 5);
		}
		else
		{
			sprite->priority = (attr & 0x0030) >> 4;
			sprite->base_pen = base_pal + (attr & 0x3f00);	// first 0x4000 colors
		}

		sprite->flags = SPRITE_VISIBLE_CAVE;
		sprite->line_offset = sprite->tile_width;

		if (glob_flipx)	{ x = max_x - x - sprite->total_width;	flipx = !flipx; }
		if (glob_flipy)	{ y = max_y - y - sprite->total_height;	flipy = !flipy; }

		sprite->x = x;
		sprite->y = y;

		if (flipx)	sprite->flags |= SPRITE_FLIPX_CAVE;
		if (flipy)	sprite->flags |= SPRITE_FLIPY_CAVE;

		sprite++;
	}
	state->num_sprites = sprite - state->sprite;
}


static void sprite_init_cave( running_machine *machine )
{
	cave_state *state = machine->driver_data<cave_state>();
	int screen_width = machine->primary_screen->width();
	int screen_height = machine->primary_screen->height();

	if (state->spritetype[0] == 0 || state->spritetype[0] == 2)	// most of the games
	{
		state->get_sprite_info = get_sprite_info_cave;
		state->spritetype[1] = CAVE_SPRITETYPE_ZOOM;
	}
	else						// donpachi ddonpach
	{
		state->get_sprite_info = get_sprite_info_donpachi;
		state->spritetype[1] = 0;
	}

	state->sprite_zbuf_baseval = 0x10000 - MAX_SPRITE_NUM;
	state->sprite_zbuf = auto_bitmap_alloc(machine, screen_width, screen_height, BITMAP_FORMAT_INDEXED16);
	state->blit.baseaddr_zbuf = (UINT8 *)state->sprite_zbuf->base;
	state->blit.line_offset_zbuf = state->sprite_zbuf->rowpixels * state->sprite_zbuf->bpp / 8;

	state->num_sprites = state->spriteram_size / 0x10 / 2;
	state->sprite = auto_alloc_array_clear(machine, struct sprite_cave, state->num_sprites);

	memset(state->sprite_table, 0, sizeof(state->sprite_table));
	state->sprite_draw = sprite_draw_donpachi;

	state_save_register_global_bitmap(machine, state->sprite_zbuf);
	state_save_register_global(machine, state->sprite_zbuf_baseval);
	state_save_register_global(machine, state->num_sprites);
	state_save_register_global(machine, state->spriteram_bank);
	state_save_register_global(machine, state->spriteram_bank_delay);

	state_save_register_global(machine, state->blit.clip_left);
	state_save_register_global(machine, state->blit.clip_right);
	state_save_register_global(machine, state->blit.clip_top);
	state_save_register_global(machine, state->blit.clip_bottom);

	state_save_register_postload(machine, cave_sprite_postload, NULL);
}

static void cave_sprite_check( screen_device &screen, const rectangle *clip )
{
	cave_state *state = screen.machine->driver_data<cave_state>();

	{	/* set clip */
		int left = clip->min_x;
		int top = clip->min_y;
		int right = clip->max_x + 1;
		int bottom = clip->max_y + 1;

		state->blit.clip_left = left;
		state->blit.clip_top = top;
		state->blit.clip_right = right;
		state->blit.clip_bottom = bottom;
	}

	{	/* check priority & sprite type */
		struct sprite_cave *sprite = state->sprite;
		const struct sprite_cave *finish = &sprite[state->num_sprites];
		int i[4] = {0,0,0,0};
		int priority_check = 0;
		int spritetype = state->spritetype[1];
		const rectangle &visarea = screen.visible_area();

		while (sprite < finish)
		{
			if (sprite->x + sprite->total_width  > state->blit.clip_left && sprite->x < state->blit.clip_right  &&
				sprite->y + sprite->total_height > state->blit.clip_top  && sprite->y < state->blit.clip_bottom    )
			{
				state->sprite_table[sprite->priority][i[sprite->priority]++] = sprite;

				if(!(spritetype & CAVE_SPRITETYPE_ZBUF))
				{
					if (priority_check > sprite->priority)
						spritetype |= CAVE_SPRITETYPE_ZBUF;
					else
						priority_check = sprite->priority;
				}
			}
			sprite++;
		}

		state->sprite_table[0][i[0]] = 0;
		state->sprite_table[1][i[1]] = 0;
		state->sprite_table[2][i[2]] = 0;
		state->sprite_table[3][i[3]] = 0;

		switch (spritetype)
		{
			case CAVE_SPRITETYPE_ZOOM:
				state->sprite_draw = sprite_draw_cave;
				break;

			case CAVE_SPRITETYPE_ZOOM | CAVE_SPRITETYPE_ZBUF:
				state->sprite_draw = sprite_draw_cave_zbuf;
				if (clip->min_y == visarea.min_y)
				{
					if(!(state->sprite_zbuf_baseval += MAX_SPRITE_NUM))
						bitmap_fill(state->sprite_zbuf, &visarea, 0);
				}
				break;

			case CAVE_SPRITETYPE_ZBUF:
				state->sprite_draw = sprite_draw_donpachi_zbuf;
				if (clip->min_y == visarea.min_y)
				{
					if(!(state->sprite_zbuf_baseval += MAX_SPRITE_NUM))
						bitmap_fill(state->sprite_zbuf,&visarea,0);
				}
				break;

			default:
			case 0:
				state->sprite_draw = sprite_draw_donpachi;
		}
	}
}

static void do_blit_zoom16_cave( running_machine *machine, const struct sprite_cave *sprite )
{
	/*  assumes SPRITE_LIST_RAW_DATA flag is set */
	cave_state *state = machine->driver_data<cave_state>();
	int x1, x2, y1, y2, dx, dy;
	int xcount0 = 0x10000 + sprite->xcount0, ycount0 = 0x10000 + sprite->ycount0;

	if (sprite->flags & SPRITE_FLIPX_CAVE)
	{
		x2 = sprite->x;
		x1 = x2 + sprite->total_width;
		dx = -1;
		if (x2 < state->blit.clip_left)
			x2 = state->blit.clip_left;

		if (x1 > state->blit.clip_right)
		{
			xcount0 += (x1 - state->blit.clip_right) * sprite->zoomx_re;
			x1 = state->blit.clip_right;
			while ((xcount0 & 0xffff) >= sprite->zoomx_re)
			{
				xcount0 += sprite->zoomx_re;
				x1--;
			}
		}

		if (x2 >= x1)
			return;
		x1--; x2--;
	}
	else
	{
		x1 = sprite->x;
		x2 = x1 + sprite->total_width;
		dx = 1;
		if (x1 < state->blit.clip_left)
		{
			xcount0 += (state->blit.clip_left - x1) * sprite->zoomx_re;
			x1 = state->blit.clip_left;
			while ((xcount0 & 0xffff) >= sprite->zoomx_re)
			{
				xcount0 += sprite->zoomx_re;
				x1++;
			}
		}
		if (x2 > state->blit.clip_right)
			x2 = state->blit.clip_right;
		if (x1 >= x2)
			return;
	}

	if (sprite->flags & SPRITE_FLIPY_CAVE )
	{
		y2 = sprite->y;
		y1 = y2 + sprite->total_height;
		dy = -1;
		if (y2 < state->blit.clip_top)
			y2 = state->blit.clip_top;
		if (y1 > state->blit.clip_bottom)
		{
			ycount0 += (y1 - state->blit.clip_bottom) * sprite->zoomy_re;
			y1 = state->blit.clip_bottom;
			while ((ycount0 & 0xffff) >= sprite->zoomy_re)
			{
				ycount0 += sprite->zoomy_re;
				y1--;
			}
		}
		if (y2 >= y1)
			return;
		y1--; y2--;
	}
	else
	{
		y1 = sprite->y;
		y2 = y1 + sprite->total_height;
		dy = 1;
		if (y1 < state->blit.clip_top)
		{
			ycount0 += (state->blit.clip_top - y1) * sprite->zoomy_re;
			y1 = state->blit.clip_top;
			while ((ycount0 & 0xffff) >= sprite->zoomy_re)
			{
				ycount0 += sprite->zoomy_re;
				y1++;
			}
		}
		if (y2 > state->blit.clip_bottom )
			y2 = state->blit.clip_bottom;
		if (y1 >= y2)
			return;
	}

	{
		const UINT8 *pen_data = sprite->pen_data -1 -sprite->line_offset;
		pen_t base_pen = sprite->base_pen;
		int x, y;
		UINT8 pen;
		int pitch = state->blit.line_offset * dy / 2;
		UINT16 *dest = (UINT16 *)(state->blit.baseaddr + state->blit.line_offset * y1);
		int ycount = ycount0;

		for (y = y1; y != y2; y += dy)
		{
			int xcount;
			const UINT8 *source;

			if (ycount & 0xffff0000)
			{
				xcount = xcount0;
				pen_data += sprite->line_offset * (ycount >> 16);
				ycount &= 0xffff;
				source = pen_data;
				for (x = x1; x != x2; x += dx)
				{
					if (xcount & 0xffff0000)
					{
						source += xcount >> 16;
						xcount &= 0xffff;
						pen = *source;
						if (pen)
							dest[x] = base_pen + pen;
					}
					xcount += sprite->zoomx_re;
				}
			}
			ycount += sprite->zoomy_re;
			dest += pitch;
		}
	}
}


static void do_blit_zoom16_cave_zb( running_machine *machine, const struct sprite_cave *sprite )
{
	/*  assumes SPRITE_LIST_RAW_DATA flag is set */
	cave_state *state = machine->driver_data<cave_state>();
	int x1, x2, y1, y2, dx, dy;
	int xcount0 = 0x10000 + sprite->xcount0, ycount0 = 0x10000 + sprite->ycount0;

	if (sprite->flags & SPRITE_FLIPX_CAVE)
	{
		x2 = sprite->x;
		x1 = x2 + sprite->total_width;
		dx = -1;
		if (x2 < state->blit.clip_left)
			x2 = state->blit.clip_left;
		if (x1 > state->blit.clip_right)
		{
			xcount0 += (x1 - state->blit.clip_right) * sprite->zoomx_re;
			x1 = state->blit.clip_right;
			while ((xcount0 & 0xffff) >= sprite->zoomx_re)
			{
				xcount0 += sprite->zoomx_re;
				x1--;
			}
		}
		if (x2 >= x1)
			return;
		x1--; x2--;
	}
	else
	{
		x1 = sprite->x;
		x2 = x1 + sprite->total_width;
		dx = 1;
		if (x1 < state->blit.clip_left)
		{
			xcount0 += (state->blit.clip_left - x1) * sprite->zoomx_re;
			x1 = state->blit.clip_left;
			while ((xcount0 & 0xffff) >= sprite->zoomx_re)
			{
				xcount0 += sprite->zoomx_re;
				x1++;
			}
		}
		if (x2 > state->blit.clip_right)
			x2 = state->blit.clip_right;
		if (x1 >= x2)
			return;
	}
	if (sprite->flags & SPRITE_FLIPY_CAVE)
	{
		y2 = sprite->y;
		y1 = y2 + sprite->total_height;
		dy = -1;
		if (y2 < state->blit.clip_top)
			y2 = state->blit.clip_top;
		if (y1 > state->blit.clip_bottom)
		{
			ycount0 += (y1 - state->blit.clip_bottom) * sprite->zoomy_re;
			y1 = state->blit.clip_bottom;
			while ((ycount0 & 0xffff) >= sprite->zoomy_re)
			{
				ycount0 += sprite->zoomy_re;
				y1--;
			}
		}
		if (y2 >= y1)
			return;
		y1--; y2--;
	}
	else
	{
		y1 = sprite->y;
		y2 = y1 + sprite->total_height;
		dy = 1;
		if (y1 < state->blit.clip_top)
		{
			ycount0 += (state->blit.clip_top - y1) * sprite->zoomy_re;
			y1 = state->blit.clip_top;
			while ((ycount0 & 0xffff) >= sprite->zoomy_re)
			{
				ycount0 += sprite->zoomy_re;
				y1++;
			}
		}
		if (y2 > state->blit.clip_bottom)
			y2 = state->blit.clip_bottom;
		if (y1 >= y2)
			return;
	}

	{
		const UINT8 *pen_data = sprite->pen_data - 1 - sprite->line_offset;
		pen_t base_pen = sprite->base_pen;
		int x, y;
		UINT8 pen;
		int pitch = state->blit.line_offset * dy / 2;
		UINT16 *dest = (UINT16 *)(state->blit.baseaddr + state->blit.line_offset * y1);
		int pitchz = state->blit.line_offset_zbuf * dy / 2;
		UINT16 *zbf = (UINT16 *)(state->blit.baseaddr_zbuf + state->blit.line_offset_zbuf * y1);
		UINT16 pri_sp = (UINT16)(sprite - state->sprite) + state->sprite_zbuf_baseval;
		int ycount = ycount0;

		for (y = y1; y != y2; y += dy)
		{
			int xcount;
			const UINT8 *source;

			if (ycount & 0xffff0000)
			{
				xcount = xcount0;
				pen_data += sprite->line_offset * (ycount >> 16);
				ycount &= 0xffff;
				source = pen_data;
				for (x = x1; x != x2; x += dx)
				{
					if (xcount & 0xffff0000)
					{
						source += xcount >> 16;
						xcount &= 0xffff;
						pen = *source;
						if (pen && (zbf[x] <= pri_sp))
						{
							dest[x] = base_pen + pen;
							zbf[x] = pri_sp;
						}
					}
					xcount += sprite->zoomx_re;
				}
			}
			ycount += sprite->zoomy_re;
			dest += pitch;
			zbf += pitchz;
		}
	}
}

static void do_blit_16_cave( running_machine *machine, const struct sprite_cave *sprite )
{
	/*  assumes SPRITE_LIST_RAW_DATA flag is set */
	cave_state *state = machine->driver_data<cave_state>();
	int x1, x2, y1, y2, dx, dy;
	int xcount0 = 0, ycount0 = 0;

	if (sprite->flags & SPRITE_FLIPX_CAVE)
	{
		x2 = sprite->x;
		x1 = x2 + sprite->total_width;
		dx = -1;
		if (x2 < state->blit.clip_left)
			x2 = state->blit.clip_left;
		if (x1 > state->blit.clip_right)
		{
			xcount0 = x1 - state->blit.clip_right;
			x1 = state->blit.clip_right;
		}
		if (x2 >= x1)
			return;
		x1--; x2--;
	}
	else
	{
		x1 = sprite->x;
		x2 = x1 + sprite->total_width;
		dx = 1;
		if (x1 < state->blit.clip_left)
		{
			xcount0 = state->blit.clip_left - x1;
			x1 = state->blit.clip_left;
		}
		if (x2 > state->blit.clip_right)
			x2 = state->blit.clip_right;
		if (x1 >= x2)
			return;
	}
	if (sprite->flags & SPRITE_FLIPY_CAVE)
	{
		y2 = sprite->y;
		y1 = y2 + sprite->total_height;
		dy = -1;
		if (y2 < state->blit.clip_top)
			y2 = state->blit.clip_top;
		if (y1 > state->blit.clip_bottom)
		{
			ycount0 = y1 - state->blit.clip_bottom;
			y1 = state->blit.clip_bottom;
		}
		if (y2 >= y1)
			return;
		y1--; y2--;
	}
	else {
		y1 = sprite->y;
		y2 = y1 + sprite->total_height;
		dy = 1;
		if (y1 < state->blit.clip_top )
		{
			ycount0 = state->blit.clip_top - y1;
			y1 = state->blit.clip_top;
		}
		if (y2 > state->blit.clip_bottom)
			y2 = state->blit.clip_bottom;
		if (y1 >= y2)
			return;
	}

	{
		const UINT8 *pen_data = sprite->pen_data;
		pen_t base_pen = sprite->base_pen;
		int x, y;
		UINT8 pen;
		int pitch = state->blit.line_offset * dy / 2;
		UINT16 *dest = (UINT16 *)(state->blit.baseaddr + state->blit.line_offset * y1);

		pen_data += sprite->line_offset * ycount0 + xcount0;
		for (y = y1; y != y2; y += dy)
		{
			const UINT8 *source;
			source = pen_data;
			for (x = x1; x != x2; x += dx)
			{
				pen = *source;
				if (pen)
					dest[x] = base_pen + pen;
				source++;
			}
			pen_data += sprite->line_offset;
			dest += pitch;
		}
	}
}


static void do_blit_16_cave_zb( running_machine *machine,  const struct sprite_cave *sprite )
{
	/*  assumes SPRITE_LIST_RAW_DATA flag is set */
	cave_state *state = machine->driver_data<cave_state>();
	int x1, x2, y1, y2, dx, dy;
	int xcount0 = 0, ycount0 = 0;

	if (sprite->flags & SPRITE_FLIPX_CAVE)
	{
		x2 = sprite->x;
		x1 = x2 + sprite->total_width;
		dx = -1;
		if (x2 < state->blit.clip_left)
			x2 = state->blit.clip_left;
		if (x1 > state->blit.clip_right)
		{
			xcount0 = x1 - state->blit.clip_right;
			x1 = state->blit.clip_right;
		}
		if (x2 >= x1)
			return;
		x1--; x2--;
	}
	else
	{
		x1 = sprite->x;
		x2 = x1 + sprite->total_width;
		dx = 1;
		if (x1 < state->blit.clip_left)
		{
			xcount0 = state->blit.clip_left - x1;
			x1 = state->blit.clip_left;
		}
		if (x2 > state->blit.clip_right)
			x2 = state->blit.clip_right;
		if (x1 >= x2)
			return;
	}
	if (sprite->flags & SPRITE_FLIPY_CAVE)
	{
		y2 = sprite->y;
		y1 = y2 + sprite->total_height;
		dy = -1;
		if (y2 < state->blit.clip_top)
			y2 = state->blit.clip_top;
		if (y1 > state->blit.clip_bottom)
		{
			ycount0 = y1 - state->blit.clip_bottom;
			y1 = state->blit.clip_bottom;
		}
		if (y2 >= y1)
			return;
		y1--; y2--;
	}
	else
	{
		y1 = sprite->y;
		y2 = y1 + sprite->total_height;
		dy = 1;
		if (y1 < state->blit.clip_top)
		{
			ycount0 = state->blit.clip_top - y1;
			y1 = state->blit.clip_top;
		}
		if (y2 > state->blit.clip_bottom)
			y2 = state->blit.clip_bottom;
		if (y1 >= y2)
			return;
	}

	{
		const UINT8 *pen_data = sprite->pen_data;
		pen_t base_pen = sprite->base_pen;
		int x, y;
		UINT8 pen;
		int pitch = state->blit.line_offset * dy / 2;
		UINT16 *dest = (UINT16 *)(state->blit.baseaddr + state->blit.line_offset * y1);
		int pitchz = state->blit.line_offset_zbuf * dy / 2;
		UINT16 *zbf = (UINT16 *)(state->blit.baseaddr_zbuf + state->blit.line_offset_zbuf * y1);
		UINT16 pri_sp = (UINT16)(sprite - state->sprite) + state->sprite_zbuf_baseval;

		pen_data += sprite->line_offset * ycount0 + xcount0;
		for (y = y1; y != y2; y += dy)
		{
			const UINT8 *source;
			source = pen_data;
			for (x = x1; x != x2; x += dx)
			{
				pen = *source;
				if (pen && (zbf[x] <= pri_sp))
				{
					dest[x] = base_pen + pen;
					zbf[x] = pri_sp;
				}
				source++;
			}
			pen_data += sprite->line_offset;
			dest += pitch;
			zbf += pitchz;
		}
	}
}


static void sprite_draw_cave( running_machine *machine, int priority )
{
	cave_state *state = machine->driver_data<cave_state>();
	int i = 0;
	while (state->sprite_table[priority][i])
	{
		const struct sprite_cave *sprite = state->sprite_table[priority][i++];
		if ((sprite->tile_width == sprite->total_width) && (sprite->tile_height == sprite->total_height))
			do_blit_16_cave(machine, sprite);
		else
			do_blit_zoom16_cave(machine, sprite);
	}
}

static void sprite_draw_cave_zbuf( running_machine *machine, int priority )
{
	cave_state *state = machine->driver_data<cave_state>();
	int i = 0;
	while (state->sprite_table[priority][i])
	{
		const struct sprite_cave *sprite = state->sprite_table[priority][i++];
		if ((sprite->tile_width == sprite->total_width) && (sprite->tile_height == sprite->total_height))
			do_blit_16_cave_zb(machine, sprite);
		else
			do_blit_zoom16_cave_zb(machine, sprite);
	}
}

static void sprite_draw_donpachi( running_machine *machine, int priority )
{
	cave_state *state = machine->driver_data<cave_state>();
	int i = 0;
	while (state->sprite_table[priority][i])
		do_blit_16_cave(machine, state->sprite_table[priority][i++]);
}

static void sprite_draw_donpachi_zbuf( running_machine *machine, int priority )
{
	cave_state *state = machine->driver_data<cave_state>();
	int i = 0;
	while (state->sprite_table[priority][i])
		do_blit_16_cave_zb(machine, state->sprite_table[priority][i++]);
}


/***************************************************************************

                                Screen Drawing


                Layers Control Registers (cave_vctrl_0..2)


        Offset:     Bits:                   Value:

        0.w         f--- ---- ---- ----     0 = Layer Flip X
                    -e-- ---- ---- ----     Activate Row-scroll
                    --d- ---- ---- ----
                    ---c ba9- ---- ----
                    ---- ---8 7654 3210     Scroll X

        2.w         f--- ---- ---- ----     0 = Layer Flip Y
                    -e-- ---- ---- ----     Activate Row-select
                    --d- ---- ---- ----     0 = 8x8 tiles, 1 = 16x16 tiles
                    ---c ba9- ---- ----
                    ---- ---8 7654 3210     Scroll Y

        4.w         fedc ba98 765- ----
                    ---- ---- ---4 ----     Layer Disable
                    ---- ---- ---- 32--
                    ---- ---- ---- --10     Layer Priority (decides the order
                                            of the layers for tiles with the
                                            same tile priority)


        Row-scroll / row-select data is fetched from tile RAM + $1000.

        Row-select:     a tilemap line is specified for each scan line.
        Row-scroll:     a different scroll value is specified for each scan line.


                    Sprites Registers (cave_videoregs)


    Offset:     Bits:                   Value:

        0.w     f--- ---- ---- ----     Sprites Flip X
                -edc ba98 7654 3210     Sprites Offset X

        2.w     f--- ---- ---- ----     Sprites Flip Y
                -edc ba98 7654 3210     Sprites Offset Y

        ..

        8.w     fedc ba98 7654 321-
                ---- ---- ---- ---0     Sprite RAM Bank

        There are more!

***************************************************************************/

INLINE void cave_tilemap_draw(
	running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect,
	tilemap_t *TILEMAP, UINT16 *VRAM, UINT16 *VCTRL,
	UINT32 flags, UINT32 priority, UINT32 priority2 )
{
	cave_state *state = machine->driver_data<cave_state>();
	int sx, sy, flipx, flipy, offs_x, offs_y, offs_row;

	/* Bail out if ... */

	if	( (!TILEMAP) ||							/* no tilemap; */
		  ((VCTRL[2] & 0x0003) != priority2) ||	/* tilemap's global priority not the requested one; */
		  ((VCTRL[2] & 0x0010))	)				/* tilemap's disabled. */
		return;

	flipx = ~VCTRL[0] & 0x8000;
	flipy = ~VCTRL[1] & 0x8000;
	tilemap_set_flip(TILEMAP, (flipx ? TILEMAP_FLIPX : 0) | (flipy ? TILEMAP_FLIPY : 0) );

	offs_x	=	state->layers_offs_x;
	offs_y	=	state->layers_offs_y;

	offs_row =  flipy ? state->row_effect_offs_f : state->row_effect_offs_n;

	/* An additional 8 pixel offset for layers with 8x8 tiles. Plus
       Layer 0 is displaced by 1 pixel wrt Layer 1, so is Layer 2 wrt
       Layer 1 */
	if		(TILEMAP == state->tilemap_0)	offs_x -= (state->tiledim_0 ? 1 : (1 + 8));
	else if	(TILEMAP == state->tilemap_1)	offs_x -= (state->tiledim_1 ? 2 : (2 + 8));
	else if	(TILEMAP == state->tilemap_2)	offs_x -= (state->tiledim_2 ? 3 : (3 + 8));
	else if	(TILEMAP == state->tilemap_3)	offs_x -= (state->tiledim_3 ? 4 : (4 + 8));

	sx = VCTRL[0] - state->videoregs[0] + (flipx ? (offs_x + 2) : -offs_x);
	sy = VCTRL[1] - state->videoregs[1] + (flipy ? (offs_y + 2) : -offs_y);

	if (VCTRL[1] & 0x4000)	// row-select
	{
		rectangle clip;
		int startline, endline, vramdata0, vramdata1;

		/*
            Row-select:

            A tilemap line is specified for each scan line. This is handled
            using many horizontal clipping regions (slices) and calling
            tilemap_draw multiple times.
        */

		clip.min_x = cliprect->min_x;
		clip.max_x = cliprect->max_x;

		for (startline = cliprect->min_y; startline <= cliprect->max_y;)
		{
			/* Find the largest slice */
			vramdata0 = (vramdata1 = VRAM[(0x1002 + (((sy + offs_row + startline) * 4) & 0x7ff)) / 2]);
			for(endline = startline + 1; endline <= cliprect->max_y; endline++)
				if((++vramdata1) != VRAM[(0x1002 + (((sy + offs_row + endline) * 4) & 0x7ff)) / 2]) break;

			tilemap_set_scrolly(TILEMAP, 0, vramdata0 - startline);

			if (VCTRL[0] & 0x4000)	// row-scroll, row-select
			{
				int line;

				/*
                    Row-scroll:

                    A different scroll value is specified for each scan line.
                    This is handled using tilemap_set_scroll_rows and calling
                    tilemap_draw just once.
                */

				tilemap_set_scroll_rows(TILEMAP, 512);
				for(line = startline; line < endline; line++)
					tilemap_set_scrollx(TILEMAP, (vramdata0 - startline + line) & 511,
										sx + VRAM[(0x1000 + (((sy + offs_row + line) * 4) & 0x7ff)) / 2]);
			}
			else					// no row-scroll, row-select
			{
				tilemap_set_scroll_rows(TILEMAP, 1);
				tilemap_set_scrollx(TILEMAP, 0, sx);
			}

			if (flipy)
			{
				clip.min_y = cliprect->max_y - (endline - 1 - cliprect->min_y);
				clip.max_y = cliprect->max_y - (startline - cliprect->min_y);
			}
			else
			{
				clip.min_y = startline;
				clip.max_y = endline - 1;
			}

			tilemap_draw(bitmap, &clip, TILEMAP, flags, priority);

			startline = endline;
		}
	}
	else if (VCTRL[0] & 0x4000)	// row-scroll, no row-select
	{
		int line;
		tilemap_set_scroll_rows(TILEMAP, 512);
		for(line = cliprect->min_y; line <= cliprect->max_y; line++)
			tilemap_set_scrollx(TILEMAP, (line + sy) & 511,
							sx + VRAM[(0x1000+(((sy + offs_row + line) * 4) & 0x7ff)) / 2] );
		tilemap_set_scrolly(TILEMAP, 0, sy);
		tilemap_draw(bitmap, cliprect, TILEMAP, flags, priority);
	}
	else
	{
		/* DEF_STR( Normal ) scrolling */
		tilemap_set_scroll_rows(TILEMAP, 1);
		tilemap_set_scroll_cols(TILEMAP, 1);
		tilemap_set_scrollx(TILEMAP, 0, sx);
		tilemap_set_scrolly(TILEMAP, 0, sy);
		tilemap_draw(bitmap, cliprect, TILEMAP, flags, priority);
	}
}

static void cave_tilemap_0_draw( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT32 flags, UINT32 priority, UINT32 priority2 )
{
	cave_state *state = machine->driver_data<cave_state>();
	cave_tilemap_draw(machine, bitmap, cliprect, state->tilemap_0, state->vram_0, state->vctrl_0, flags, priority, priority2);
}

static void cave_tilemap_1_draw( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT32 flags, UINT32 priority, UINT32 priority2 )
{
	cave_state *state = machine->driver_data<cave_state>();
	cave_tilemap_draw(machine, bitmap, cliprect, state->tilemap_1, state->vram_1, state->vctrl_1, flags, priority, priority2);
}

static void cave_tilemap_2_draw( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT32 flags, UINT32 priority, UINT32 priority2 )
{
	cave_state *state = machine->driver_data<cave_state>();
	cave_tilemap_draw(machine, bitmap, cliprect, state->tilemap_2, state->vram_2, state->vctrl_2, flags, priority, priority2);
}

static void cave_tilemap_3_draw( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT32 flags, UINT32 priority, UINT32 priority2 )
{
	cave_state *state = machine->driver_data<cave_state>();
	cave_tilemap_draw(machine, bitmap, cliprect, state->tilemap_3, state->vram_3, state->vctrl_3, flags, priority, priority2);
}


VIDEO_UPDATE( cave )
{
	cave_state *state = screen->machine->driver_data<cave_state>();
	int pri, pri2;
	int layers_ctrl = -1;

	set_pens(screen->machine);

	state->blit.baseaddr = (UINT8 *)bitmap->base;
	state->blit.line_offset = bitmap->rowpixels * bitmap->bpp / 8;

	/* Choose the tilemap to display (8x8 tiles or 16x16 tiles) */
	if (state->tilemap_0)
	{
		state->tiledim_0 = state->vctrl_0[1] & 0x2000;
		if (state->tiledim_0 != state->old_tiledim_0)
			tilemap_mark_all_tiles_dirty(state->tilemap_0);
		state->old_tiledim_0 = state->tiledim_0;
	}

	if (state->tilemap_1)
	{
		state->tiledim_1 = state->vctrl_1[1] & 0x2000;
		if (state->tiledim_1 != state->old_tiledim_1)
			tilemap_mark_all_tiles_dirty(state->tilemap_1);
		state->old_tiledim_1 = state->tiledim_1;
	}

	if (state->tilemap_2)
	{
		state->tiledim_2 = state->vctrl_2[1] & 0x2000;
		if (state->tiledim_2 != state->old_tiledim_2)
			tilemap_mark_all_tiles_dirty(state->tilemap_2);
		state->old_tiledim_2 = state->tiledim_2;
	}

	if (state->tilemap_3)
	{
		state->tiledim_3 = state->vctrl_3[1] & 0x2000;
		if (state->tiledim_3 != state->old_tiledim_3)
			tilemap_mark_all_tiles_dirty(state->tilemap_3);
		state->old_tiledim_3 = state->tiledim_3;
	}


#ifdef MAME_DEBUG
{
	static int rasflag, old_rasflag;
	if ( input_code_pressed(screen->machine, KEYCODE_Z) || input_code_pressed(screen->machine, KEYCODE_X) || input_code_pressed(screen->machine, KEYCODE_C) ||
    	 input_code_pressed(screen->machine, KEYCODE_V) || input_code_pressed(screen->machine, KEYCODE_B) )
	{
		int msk = 0, val = 0;

		if (input_code_pressed(screen->machine, KEYCODE_X))	val = 1;	// priority 0 only
		if (input_code_pressed(screen->machine, KEYCODE_C))	val = 2;	// ""       1
		if (input_code_pressed(screen->machine, KEYCODE_V))	val = 4;	// ""       2
		if (input_code_pressed(screen->machine, KEYCODE_B))	val = 8;	// ""       3
		if (input_code_pressed(screen->machine, KEYCODE_Z))	val = 1|2|4|8;	// All of the above priorities

		if (input_code_pressed(screen->machine, KEYCODE_Q))	msk |= val <<  0;	// for layer 0
		if (input_code_pressed(screen->machine, KEYCODE_W))	msk |= val <<  4;	// for layer 1
		if (input_code_pressed(screen->machine, KEYCODE_E))	msk |= val <<  8;	// for layer 2
		if (input_code_pressed(screen->machine, KEYCODE_R))	msk |= val << 12;	// for layer 3
		if (input_code_pressed(screen->machine, KEYCODE_A))	msk |= val << 16;	// for sprites
		if (msk != 0) layers_ctrl &= msk;

#if 1
		/* Show the video registers (cave_videoregs) */
		popmessage("%04X %04X %04X %04X %04X %04X %04X %04X",
			state->videoregs[0], state->videoregs[1], state->videoregs[2], state->videoregs[3],
			state->videoregs[4], state->videoregs[5], state->videoregs[6], state->videoregs[7] );
#endif
		/* Show the scroll / flags registers of the selected layer */
		if ((state->tilemap_0) && (msk & 0x000f))	popmessage("x:%04X y:%04X f:%04X", state->vctrl_0[0],state->vctrl_0[1],state->vctrl_0[2]);
		if ((state->tilemap_1) && (msk & 0x00f0))	popmessage("x:%04X y:%04X f:%04X", state->vctrl_1[0],state->vctrl_1[1],state->vctrl_1[2]);
		if ((state->tilemap_2) && (msk & 0x0f00))	popmessage("x:%04X y:%04X f:%04X", state->vctrl_2[0],state->vctrl_2[1],state->vctrl_2[2]);
		if ((state->tilemap_3) && (msk & 0xf000))	popmessage("x:%04X y:%04X f:%04X", state->vctrl_3[0],state->vctrl_3[1],state->vctrl_3[2]);
	}

	/* Show the row / "column" scroll enable flags, when they change state */
	rasflag = 0;
	if (state->tilemap_0)
	{
		rasflag |= (state->vctrl_0[0] & 0x4000) ? 0x0001 : 0;
		rasflag |= (state->vctrl_0[1] & 0x4000) ? 0x0002 : 0;
	}
	if (state->tilemap_1)
	{
		rasflag |= (state->vctrl_1[0] & 0x4000) ? 0x0010 : 0;
		rasflag |= (state->vctrl_1[1] & 0x4000) ? 0x0020 : 0;
	}
	if (state->tilemap_2)
	{
		rasflag |= (state->vctrl_2[0] & 0x4000) ? 0x0100 : 0;
		rasflag |= (state->vctrl_2[1] & 0x4000) ? 0x0200 : 0;
	}
	if (state->tilemap_3)
	{
		rasflag |= (state->vctrl_3[0] & 0x4000) ? 0x1000 : 0;
		rasflag |= (state->vctrl_3[1] & 0x4000) ? 0x2000 : 0;
	}
	if (rasflag != old_rasflag)
	{
		popmessage("Line Effect: 0:%c%c 1:%c%c 2:%c%c 3:%c%c",
			(rasflag & 0x0001) ? 'x' : ' ', (rasflag & 0x0002) ? 'y' : ' ',
			(rasflag & 0x0010) ? 'x' : ' ', (rasflag & 0x0020) ? 'y' : ' ',
			(rasflag & 0x0100) ? 'x' : ' ', (rasflag & 0x0200) ? 'y' : ' ',
			(rasflag & 0x1000) ? 'x' : ' ', (rasflag & 0x2000) ? 'y' : ' ' );
		old_rasflag = rasflag;
	}
}
#endif

	cave_sprite_check(*screen, cliprect);

	bitmap_fill(bitmap, cliprect, state->background_color);

	/*
        Tiles and sprites are ordered by priority (0 back, 3 front) with
        sprites going below tiles of their same priority.

        Sprites with the same priority are ordered by their place in
        sprite RAM (last sprite is the frontmost).

        Tiles with the same priority are ordered by the priority of their layer.

        Tiles with the same priority *and* the same priority of their layer
        are ordered by layer (0 back, 2 front)
    */
	for (pri = 0; pri <= 3; pri++)	// tile / sprite priority
	{
			if (layers_ctrl & (1 << (pri + 16)))	(*state->sprite_draw)(screen->machine, pri);

		for (pri2 = 0; pri2 <= 3; pri2++)	// priority of the whole layer
		{
			if (layers_ctrl & (1 << (pri +  0)))	cave_tilemap_0_draw(screen->machine, bitmap, cliprect, pri, 0, pri2);
			if (layers_ctrl & (1 << (pri +  4)))	cave_tilemap_1_draw(screen->machine, bitmap, cliprect, pri, 0, pri2);
			if (layers_ctrl & (1 << (pri +  8)))	cave_tilemap_2_draw(screen->machine, bitmap, cliprect, pri, 0, pri2);
			if (layers_ctrl & (1 << (pri + 12)))	cave_tilemap_3_draw(screen->machine, bitmap, cliprect, pri, 0, pri2);
		}
	}
	return 0;
}



/**************************************************************/

void cave_get_sprite_info( running_machine *machine )
{
	cave_state *state = machine->driver_data<cave_state>();
	if (state->kludge == 3)	/* mazinger metmqstr */
	{
		if (video_skip_this_frame() == 0)
		{
			state->spriteram_bank = state->spriteram_bank_delay;
			(*state->get_sprite_info)(machine);
		}
		state->spriteram_bank_delay = state->videoregs[4] & 1;
	}
	else
	{
		if (video_skip_this_frame() == 0)
		{
			state->spriteram_bank = state->videoregs[4] & 1;
			(*state->get_sprite_info)(machine);
		}
	}
}

static STATE_POSTLOAD( cave_sprite_postload )
{
	/* FIXME: this is probably not enough to restore sprites, but anyway saves do not
    currently work due to pending timers hence it is hard to test what is missing */
	cave_get_sprite_info(machine);
}
