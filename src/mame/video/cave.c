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

static void sprite_init_cave(running_machine &machine);
static void sprite_draw_cave(running_machine &machine, int priority);
static void sprite_draw_cave_zbuf(running_machine &machine, int priority);
static void sprite_draw_donpachi(running_machine &machine, int priority);
static void sprite_draw_donpachi_zbuf(running_machine &machine, int priority);

/***************************************************************************

                            Palette Init Routines

    Function needed for games with 4 bit sprites, rather than 8 bit,
    or games with an odd colors mapping for the layers.

***************************************************************************/

PALETTE_INIT( cave )
{
	cave_state *state = machine.driver_data<cave_state>();
	int maxpen = state->m_paletteram_size / 2;
	int pen;

	/* create a 1:1 palette map covering everything */
	state->m_palette_map = auto_alloc_array(machine, UINT16, machine.total_colors());

	for (pen = 0; pen < machine.total_colors(); pen++)
		state->m_palette_map[pen] = pen % maxpen;
}

PALETTE_INIT( dfeveron )
{
	cave_state *state = machine.driver_data<cave_state>();
	int color, pen;

	/* Fill the 0-3fff range, used by sprites ($40 color codes * $100 pens)
       Here sprites have 16 pens, but the sprite drawing routine always
       multiplies the color code by $100 (for consistency).
       That's why we need this function.    */

	PALETTE_INIT_CALL(cave);

	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x10; pen++)
			state->m_palette_map[(color << 8) | pen] = (color << 4) | pen;
}

PALETTE_INIT( ddonpach )
{
	cave_state *state = machine.driver_data<cave_state>();
	int color, pen;

	/* Fill the 8000-83ff range ($40 color codes * $10 pens) for
       layers 0 & 1 which are 4 bits deep rather than 8 bits deep
       like layer 2, but use the first 16 color of every 256 for
       any given color code. */

	PALETTE_INIT_CALL(cave);

	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x10; pen++)
			state->m_palette_map[0x8000 | (color << 4) | pen] = 0x4000 | (color << 8) | pen;
}

PALETTE_INIT( mazinger )
{
	cave_state *state = machine.driver_data<cave_state>();
	int color, pen;

	PALETTE_INIT_CALL(cave);

	/* sprites (encrypted) are 4 bit deep */
	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x100; pen++)
			state->m_palette_map[(color << 8) | pen] = (color << 4) + pen;	/* yes, PLUS, not OR */

	/* layer 0 is 6 bit deep, there are 64 color codes but only $400
       colors are actually addressable */
	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x40; pen++)
			state->m_palette_map[0x4400 + ((color << 6) | pen)] = 0x400 | ((color & 0x0f) << 6) | pen;
}

PALETTE_INIT( sailormn )
{
	cave_state *state = machine.driver_data<cave_state>();
	int color, pen;

	PALETTE_INIT_CALL(cave);

	/* sprites (encrypted) are 4 bit deep */
	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x100; pen++)
			state->m_palette_map[(color << 8) | pen] = (color << 4) + pen;	/* yes, PLUS, not OR */

	/* layer 2 is 6 bit deep, there are 64 color codes but only $400
       colors are actually addressable */
	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x40; pen++)
			state->m_palette_map[0x4c00 | (color << 6) | pen] = 0xc00 | ((color & 0x0f) << 6) | pen;
}

PALETTE_INIT( pwrinst2 )
{
	cave_state *state = machine.driver_data<cave_state>();
	int color, pen;

	PALETTE_INIT_CALL(cave);

	for (color = 0; color < 0x80; color++)
		for (pen = 0; pen < 0x10; pen++)
			state->m_palette_map[(color << 8) | pen] = (color << 4) | pen;

	for (pen = 0x8000; pen < 0xa800; pen++)
			state->m_palette_map[pen] = pen - 0x8000;
}

PALETTE_INIT( korokoro )
{
	cave_state *state = machine.driver_data<cave_state>();
	int color, pen;

	PALETTE_INIT_CALL(cave);

	for (color = 0; color < 0x40; color++)
		for (pen = 0; pen < 0x10; pen++)
			state->m_palette_map[(color << 8) | pen] = 0x3c00 | (color << 4) | pen;
}


static void set_pens( running_machine &machine )
{
	cave_state *state = machine.driver_data<cave_state>();
	int pen;

	for (pen = 0; pen < machine.total_colors(); pen++)
	{
		UINT16 data = state->m_paletteram[state->m_palette_map[pen]];

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

INLINE void get_tile_info( running_machine &machine, tile_data &tileinfo, int tile_index, int GFX )
{
	cave_state *state = machine.driver_data<cave_state>();
	UINT16 *VRAM = state->m_vram[GFX];
	int TDIM = state->m_tiledim[GFX];
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
	tileinfo.category = pri;
}


/* Sailormn: the lower 2 Megabytes of tiles banked */

void sailormn_tilebank_w( running_machine &machine, int bank )
{
	cave_state *state = machine.driver_data<cave_state>();
	if (state->m_sailormn_tilebank != bank)
	{
		state->m_sailormn_tilebank = bank;
		state->m_tilemap[2]->mark_all_dirty();
	}
}

static TILE_GET_INFO( sailormn_get_tile_info_2 )
{
	cave_state *state = machine.driver_data<cave_state>();
	UINT32 code, color, pri;

	if (state->m_tiledim[2])
	{
		UINT32 tile;
		tile  = (tile_index % (512 / 8)) / 2 + ((tile_index / (512 / 8)) / 2) * (512 / 16);
		code  = (state->m_vram[2][tile * 2 + 0x0000 / 2] << 16) + state->m_vram[2][tile * 2 + 0x0002 / 2];

		color = (code & 0x3f000000) >> (32 - 8);
		pri   = (code & 0xc0000000) >> (32 - 2);
		code  = (code & 0x00ffffff) * 4;

		code += tile_index & 1;
		code += ((tile_index / (512 / 8)) & 1) * 2;
	}
	else
	{
		code  = (state->m_vram[2][tile_index * 2 + 0x4000 / 2] << 16) + state->m_vram[2][tile_index * 2 + 0x4002 / 2];

		color = (code & 0x3f000000) >> (32 - 8);
		pri   = (code & 0xc0000000) >> (32 - 2);
		code  = (code & 0x00ffffff);
		if ((code < 0x10000) && (state->m_sailormn_tilebank))
			code += 0x40000;
	}

	SET_TILE_INFO( 2, code, color, 0 );
	tileinfo.category = pri;
}


INLINE void vram_w( address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask, int GFX )
{
	cave_state *state = space->machine().driver_data<cave_state>();
	UINT16 *VRAM = state->m_vram[GFX];
	tilemap_t *TILEMAP = state->m_tilemap[GFX];

	if ((VRAM[offset] & mem_mask) == (data & mem_mask))
		return;

	COMBINE_DATA(&VRAM[offset]);
	offset /= 2;
	if (offset < 0x1000 / 4)	// 16x16 tilemap
	{
		offset = (offset % (512 / 16)) * 2 + (offset / (512 / 16)) * (512 / 8) * 2;
		TILEMAP->mark_tile_dirty(offset + 0);
		TILEMAP->mark_tile_dirty(offset + 1);
		TILEMAP->mark_tile_dirty(offset + 0 + 512 / 8);
		TILEMAP->mark_tile_dirty(offset + 1 + 512 / 8);
	}
	else if (offset >= 0x4000 / 4)		// 8x8 tilemap
		TILEMAP->mark_tile_dirty(offset - 0x4000 / 4);
}

/*  Some games, that only ever use the 8x8 tiles and no line scroll,
    use mirror ram. For example in donpachi, writes to 400000-403fff
    and 408000-407fff both go to the 8x8 tilemap ram. Use this function
    in this cases. Note that the get_tile_info function looks in the
    4000-7fff range for tiles, so we have to write the data there. */
INLINE void vram_8x8_w( address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask, int GFX )
{
	cave_state *state = space->machine().driver_data<cave_state>();
	UINT16 *VRAM = state->m_vram[GFX];
	tilemap_t *TILEMAP = state->m_tilemap[GFX];

	offset %= 0x4000 / 2;
	if ((VRAM[offset] & mem_mask) == (data & mem_mask))
		return;

	COMBINE_DATA(&VRAM[offset + 0x0000 / 2]);
	COMBINE_DATA(&VRAM[offset + 0x4000 / 2]);
	TILEMAP->mark_tile_dirty(offset / 2);
}


static TILE_GET_INFO( get_tile_info_0 )	{ get_tile_info(machine, tileinfo, tile_index, 0); }
static TILE_GET_INFO( get_tile_info_1 )	{ get_tile_info(machine, tileinfo, tile_index, 1); }
static TILE_GET_INFO( get_tile_info_2 )	{ get_tile_info(machine, tileinfo, tile_index, 2); }
static TILE_GET_INFO( get_tile_info_3 )	{ get_tile_info(machine, tileinfo, tile_index, 3); }

WRITE16_MEMBER(cave_state::cave_vram_0_w){ vram_w(&space, offset, data, mem_mask, 0); }
WRITE16_MEMBER(cave_state::cave_vram_1_w){ vram_w(&space, offset, data, mem_mask, 1); }
WRITE16_MEMBER(cave_state::cave_vram_2_w){ vram_w(&space, offset, data, mem_mask, 2); }
WRITE16_MEMBER(cave_state::cave_vram_3_w){ vram_w(&space, offset, data, mem_mask, 3); }

WRITE16_MEMBER(cave_state::cave_vram_0_8x8_w){ vram_8x8_w(&space, offset, data, mem_mask, 0); }
WRITE16_MEMBER(cave_state::cave_vram_1_8x8_w){ vram_8x8_w(&space, offset, data, mem_mask, 1); }
WRITE16_MEMBER(cave_state::cave_vram_2_8x8_w){ vram_8x8_w(&space, offset, data, mem_mask, 2); }
WRITE16_MEMBER(cave_state::cave_vram_3_8x8_w){ vram_8x8_w(&space, offset, data, mem_mask, 3); }


/***************************************************************************

                            Video Init Routines

    Depending on the game, there can be from 1 to 4 layers and the
    tile sizes can be 8x8 or 16x16.

***************************************************************************/

static void cave_vh_start( running_machine &machine, int num )
{
	cave_state *state = machine.driver_data<cave_state>();

	assert(state->m_palette_map != NULL);

	state->m_tilemap[0] = 0;
	state->m_tilemap[1] = 0;
	state->m_tilemap[2] = 0;
	state->m_tilemap[3] = 0;

	state->m_tiledim[0] = 0;
	state->m_tiledim[1] = 0;
	state->m_tiledim[2] = 0;
	state->m_tiledim[3] = 0;

	state->m_old_tiledim[0] = 0;
	state->m_old_tiledim[1] = 0;
	state->m_old_tiledim[2] = 0;
	state->m_old_tiledim[3] = 0;

	assert((num >= 1) && (num <= 4));

	switch (num)
	{
		case 4:
			state->m_tilemap[3] = tilemap_create(machine, get_tile_info_3, tilemap_scan_rows, 8, 8, 512 / 8, 512 / 8);
			state->m_tilemap[3]->set_transparent_pen(0);
			state->m_tilemap[3]->set_scroll_rows(1);
			state->m_tilemap[3]->set_scroll_cols(1);
			state->save_item(NAME(state->m_tiledim[3]));
			state->save_item(NAME(state->m_old_tiledim[3]));

		case 3:
			state->m_tilemap[2] = tilemap_create(machine, get_tile_info_2, tilemap_scan_rows, 8, 8, 512 / 8, 512 / 8);
			state->m_tilemap[2]->set_transparent_pen(0);
			state->m_tilemap[2]->set_scroll_rows(1);
			state->m_tilemap[2]->set_scroll_cols(1);
			state->save_item(NAME(state->m_tiledim[2]));
			state->save_item(NAME(state->m_old_tiledim[2]));

		case 2:
			state->m_tilemap[1] = tilemap_create(machine, get_tile_info_1, tilemap_scan_rows, 8, 8, 512 / 8, 512 / 8);
			state->m_tilemap[1]->set_transparent_pen(0);
			state->m_tilemap[1]->set_scroll_rows(1);
			state->m_tilemap[1]->set_scroll_cols(1);
			state->save_item(NAME(state->m_tiledim[1]));
			state->save_item(NAME(state->m_old_tiledim[1]));

		case 1:
			state->m_tilemap[0] = tilemap_create(machine, get_tile_info_0, tilemap_scan_rows, 8, 8, 512 / 8, 512 / 8);
			state->m_tilemap[0]->set_transparent_pen(0);
			state->m_tilemap[0]->set_scroll_rows(1);
			state->m_tilemap[0]->set_scroll_cols(1);
			state->save_item(NAME(state->m_tiledim[0]));
			state->save_item(NAME(state->m_old_tiledim[0]));

			break;
	}

	sprite_init_cave(machine);

	state->m_layers_offs_x = 0x13;
	state->m_layers_offs_y = -0x12;

	state->m_row_effect_offs_n = -1;
	state->m_row_effect_offs_f = 1;

	state->m_background_color = machine.config().m_gfxdecodeinfo[0].color_codes_start +
					(machine.config().m_gfxdecodeinfo[0].total_color_codes - 1) *
						machine.gfx[0]->color_granularity;

	switch (state->m_kludge)
	{
		case 1:	/* sailormn */
			state->m_row_effect_offs_n = -1;
			state->m_row_effect_offs_f = -1;
			break;
		case 2:	/* uopoko dfeveron */
			state->m_background_color = 0x3f00;
			break;
		case 4:	/* pwrinst2 */
			state->m_background_color = 0x7f00;
			state->m_layers_offs_y++;
			break;
	}
}

VIDEO_START( cave_1_layer  )	{	cave_vh_start(machine, 1);	}
VIDEO_START( cave_2_layers )	{	cave_vh_start(machine, 2);	}
VIDEO_START( cave_3_layers )	{	cave_vh_start(machine, 3);	}
VIDEO_START( cave_4_layers )	{	cave_vh_start(machine, 4);	}


VIDEO_START( sailormn_3_layers )
{
	cave_state *state = machine.driver_data<cave_state>();
	cave_vh_start(machine, 2);

	/* Layer 2 (8x8) needs to be handled differently */
	state->m_tilemap[2] = tilemap_create(machine, sailormn_get_tile_info_2, tilemap_scan_rows, 8, 8, 512 / 8, 512 / 8 );

	state->m_tilemap[2]->set_transparent_pen(0);
	state->m_tilemap[2]->set_scroll_rows(1);
	state->m_tilemap[2]->set_scroll_cols(1);
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

static void get_sprite_info_cave( running_machine &machine )
{
	cave_state *state = machine.driver_data<cave_state>();
	pen_t base_pal = 0;
	const UINT8 *base_gfx = machine.region("sprites")->base();
	int code_max = machine.region("sprites")->bytes() / (16*16);

	UINT16 *source;
	UINT16 *finish;
	struct sprite_cave *sprite = state->m_sprite;

	int glob_flipx = state->m_videoregs[0] & 0x8000;
	int glob_flipy = state->m_videoregs[1] & 0x8000;

	int max_x = machine.primary_screen->width();
	int max_y = machine.primary_screen->height();

	source = state->m_spriteram + ((state->m_spriteram_size / 2) / 2) * state->m_spriteram_bank;

	if (state->m_videoregs[4] & 0x02)
		if (state->m_spriteram_2)
			source = state->m_spriteram_2 + ((state->m_spriteram_size / 2) / 2) * state->m_spriteram_bank;

	finish = source + ((state->m_spriteram_size / 2) / 2);


	for (; source < finish; source += 8)
	{
		int x, y, attr, code, zoomx, zoomy, size, flipx, flipy;
		int total_width_f, total_height_f;

		if (state->m_spritetype[0] == 2)	/* Hot Dog Storm */
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

		if (state->m_spritetype[0] == 2)
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
	state->m_num_sprites = sprite - state->m_sprite;
}

static void get_sprite_info_donpachi( running_machine &machine )
{
	cave_state *state = machine.driver_data<cave_state>();
	pen_t base_pal = 0;
	const UINT8 *base_gfx = machine.region("sprites")->base();
	int code_max = machine.region("sprites")->bytes() / (16*16);

	UINT16 *source;
	UINT16 *finish;

	struct sprite_cave *sprite = state->m_sprite;

	int glob_flipx = state->m_videoregs[0] & 0x8000;
	int glob_flipy = state->m_videoregs[1] & 0x8000;

	int max_x = machine.primary_screen->width();
	int max_y = machine.primary_screen->height();

	source = state->m_spriteram + ((state->m_spriteram_size / 2) / 2) * state->m_spriteram_bank;

	if (state->m_videoregs[4] & 0x02)
		if (state->m_spriteram_2)
			source = state->m_spriteram_2 + ((state->m_spriteram_size / 2) / 2) * state->m_spriteram_bank;

	finish = source + ((state->m_spriteram_size / 2) / 2);

	for (; source < finish; source += 8)
	{
		int x, y, attr, code, size, flipx, flipy;

		attr = source[0];
		code = source[1] + ((attr & 3) << 16);
		x    = source[2] & 0x3ff;

		if (state->m_spritetype[0] == 3)	/* pwrinst2 */
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

		if (state->m_spritetype[0] == 3)	/* pwrinst2 */
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
	state->m_num_sprites = sprite - state->m_sprite;
}


static void sprite_init_cave( running_machine &machine )
{
	cave_state *state = machine.driver_data<cave_state>();

	if (state->m_spritetype[0] == 0 || state->m_spritetype[0] == 2)	// most of the games
	{
		state->m_get_sprite_info = get_sprite_info_cave;
		state->m_spritetype[1] = CAVE_SPRITETYPE_ZOOM;
	}
	else						// donpachi ddonpach
	{
		state->m_get_sprite_info = get_sprite_info_donpachi;
		state->m_spritetype[1] = 0;
	}

	state->m_sprite_zbuf_baseval = 0x10000 - MAX_SPRITE_NUM;
	machine.primary_screen->register_screen_bitmap(state->m_sprite_zbuf);

	state->m_num_sprites = state->m_spriteram_size / 0x10 / 2;
	state->m_sprite = auto_alloc_array_clear(machine, struct sprite_cave, state->m_num_sprites);

	memset(state->m_sprite_table, 0, sizeof(state->m_sprite_table));
	state->m_sprite_draw = sprite_draw_donpachi;

	state->save_item(NAME(state->m_sprite_zbuf));
	state->save_item(NAME(state->m_sprite_zbuf_baseval));
	state->save_item(NAME(state->m_num_sprites));
	state->save_item(NAME(state->m_spriteram_bank));
	state->save_item(NAME(state->m_spriteram_bank_delay));

	state->save_item(NAME(state->m_blit.clip_left));
	state->save_item(NAME(state->m_blit.clip_right));
	state->save_item(NAME(state->m_blit.clip_top));
	state->save_item(NAME(state->m_blit.clip_bottom));

	machine.save().register_postload(save_prepost_delegate(FUNC(cave_get_sprite_info), &machine));
}

static void cave_sprite_check( screen_device &screen, const rectangle &clip )
{
	cave_state *state = screen.machine().driver_data<cave_state>();

	{	/* set clip */
		int left = clip.min_x;
		int top = clip.min_y;
		int right = clip.max_x + 1;
		int bottom = clip.max_y + 1;

		state->m_blit.clip_left = left;
		state->m_blit.clip_top = top;
		state->m_blit.clip_right = right;
		state->m_blit.clip_bottom = bottom;
	}

	{	/* check priority & sprite type */
		struct sprite_cave *sprite = state->m_sprite;
		const struct sprite_cave *finish = &sprite[state->m_num_sprites];
		int i[4] = {0,0,0,0};
		int priority_check = 0;
		int spritetype = state->m_spritetype[1];
		const rectangle &visarea = screen.visible_area();

		while (sprite < finish)
		{
			if (sprite->x + sprite->total_width  > state->m_blit.clip_left && sprite->x < state->m_blit.clip_right  &&
				sprite->y + sprite->total_height > state->m_blit.clip_top  && sprite->y < state->m_blit.clip_bottom    )
			{
				state->m_sprite_table[sprite->priority][i[sprite->priority]++] = sprite;

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

		state->m_sprite_table[0][i[0]] = 0;
		state->m_sprite_table[1][i[1]] = 0;
		state->m_sprite_table[2][i[2]] = 0;
		state->m_sprite_table[3][i[3]] = 0;

		switch (spritetype)
		{
			case CAVE_SPRITETYPE_ZOOM:
				state->m_sprite_draw = sprite_draw_cave;
				break;

			case CAVE_SPRITETYPE_ZOOM | CAVE_SPRITETYPE_ZBUF:
				state->m_sprite_draw = sprite_draw_cave_zbuf;
				if (clip.min_y == visarea.min_y)
				{
					if(!(state->m_sprite_zbuf_baseval += MAX_SPRITE_NUM))
						state->m_sprite_zbuf.fill(0, visarea);
				}
				break;

			case CAVE_SPRITETYPE_ZBUF:
				state->m_sprite_draw = sprite_draw_donpachi_zbuf;
				if (clip.min_y == visarea.min_y)
				{
					if(!(state->m_sprite_zbuf_baseval += MAX_SPRITE_NUM))
						state->m_sprite_zbuf.fill(0, visarea);
				}
				break;

			default:
			case 0:
				state->m_sprite_draw = sprite_draw_donpachi;
		}
	}
}

static void do_blit_zoom16_cave( running_machine &machine, const struct sprite_cave *sprite )
{
	/*  assumes SPRITE_LIST_RAW_DATA flag is set */
	cave_state *state = machine.driver_data<cave_state>();
	int x1, x2, y1, y2, dx, dy;
	int xcount0 = 0x10000 + sprite->xcount0, ycount0 = 0x10000 + sprite->ycount0;

	if (sprite->flags & SPRITE_FLIPX_CAVE)
	{
		x2 = sprite->x;
		x1 = x2 + sprite->total_width;
		dx = -1;
		if (x2 < state->m_blit.clip_left)
			x2 = state->m_blit.clip_left;

		if (x1 > state->m_blit.clip_right)
		{
			xcount0 += (x1 - state->m_blit.clip_right) * sprite->zoomx_re;
			x1 = state->m_blit.clip_right;
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
		if (x1 < state->m_blit.clip_left)
		{
			xcount0 += (state->m_blit.clip_left - x1) * sprite->zoomx_re;
			x1 = state->m_blit.clip_left;
			while ((xcount0 & 0xffff) >= sprite->zoomx_re)
			{
				xcount0 += sprite->zoomx_re;
				x1++;
			}
		}
		if (x2 > state->m_blit.clip_right)
			x2 = state->m_blit.clip_right;
		if (x1 >= x2)
			return;
	}

	if (sprite->flags & SPRITE_FLIPY_CAVE )
	{
		y2 = sprite->y;
		y1 = y2 + sprite->total_height;
		dy = -1;
		if (y2 < state->m_blit.clip_top)
			y2 = state->m_blit.clip_top;
		if (y1 > state->m_blit.clip_bottom)
		{
			ycount0 += (y1 - state->m_blit.clip_bottom) * sprite->zoomy_re;
			y1 = state->m_blit.clip_bottom;
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
		if (y1 < state->m_blit.clip_top)
		{
			ycount0 += (state->m_blit.clip_top - y1) * sprite->zoomy_re;
			y1 = state->m_blit.clip_top;
			while ((ycount0 & 0xffff) >= sprite->zoomy_re)
			{
				ycount0 += sprite->zoomy_re;
				y1++;
			}
		}
		if (y2 > state->m_blit.clip_bottom )
			y2 = state->m_blit.clip_bottom;
		if (y1 >= y2)
			return;
	}

	{
		const UINT8 *pen_data = sprite->pen_data -1 -sprite->line_offset;
		pen_t base_pen = sprite->base_pen;
		int x, y;
		UINT8 pen;
		int pitch = state->m_blit.line_offset * dy / 2;
		UINT16 *dest = (UINT16 *)(state->m_blit.baseaddr + state->m_blit.line_offset * y1);
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


static void do_blit_zoom16_cave_zb( running_machine &machine, const struct sprite_cave *sprite )
{
	/*  assumes SPRITE_LIST_RAW_DATA flag is set */
	cave_state *state = machine.driver_data<cave_state>();
	int x1, x2, y1, y2, dx, dy;
	int xcount0 = 0x10000 + sprite->xcount0, ycount0 = 0x10000 + sprite->ycount0;

	if (sprite->flags & SPRITE_FLIPX_CAVE)
	{
		x2 = sprite->x;
		x1 = x2 + sprite->total_width;
		dx = -1;
		if (x2 < state->m_blit.clip_left)
			x2 = state->m_blit.clip_left;
		if (x1 > state->m_blit.clip_right)
		{
			xcount0 += (x1 - state->m_blit.clip_right) * sprite->zoomx_re;
			x1 = state->m_blit.clip_right;
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
		if (x1 < state->m_blit.clip_left)
		{
			xcount0 += (state->m_blit.clip_left - x1) * sprite->zoomx_re;
			x1 = state->m_blit.clip_left;
			while ((xcount0 & 0xffff) >= sprite->zoomx_re)
			{
				xcount0 += sprite->zoomx_re;
				x1++;
			}
		}
		if (x2 > state->m_blit.clip_right)
			x2 = state->m_blit.clip_right;
		if (x1 >= x2)
			return;
	}
	if (sprite->flags & SPRITE_FLIPY_CAVE)
	{
		y2 = sprite->y;
		y1 = y2 + sprite->total_height;
		dy = -1;
		if (y2 < state->m_blit.clip_top)
			y2 = state->m_blit.clip_top;
		if (y1 > state->m_blit.clip_bottom)
		{
			ycount0 += (y1 - state->m_blit.clip_bottom) * sprite->zoomy_re;
			y1 = state->m_blit.clip_bottom;
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
		if (y1 < state->m_blit.clip_top)
		{
			ycount0 += (state->m_blit.clip_top - y1) * sprite->zoomy_re;
			y1 = state->m_blit.clip_top;
			while ((ycount0 & 0xffff) >= sprite->zoomy_re)
			{
				ycount0 += sprite->zoomy_re;
				y1++;
			}
		}
		if (y2 > state->m_blit.clip_bottom)
			y2 = state->m_blit.clip_bottom;
		if (y1 >= y2)
			return;
	}

	{
		const UINT8 *pen_data = sprite->pen_data - 1 - sprite->line_offset;
		pen_t base_pen = sprite->base_pen;
		int x, y;
		UINT8 pen;
		int pitch = state->m_blit.line_offset * dy / 2;
		UINT16 *dest = (UINT16 *)(state->m_blit.baseaddr + state->m_blit.line_offset * y1);
		int pitchz = state->m_blit.line_offset_zbuf * dy / 2;
		UINT16 *zbf = (UINT16 *)(state->m_blit.baseaddr_zbuf + state->m_blit.line_offset_zbuf * y1);
		UINT16 pri_sp = (UINT16)(sprite - state->m_sprite) + state->m_sprite_zbuf_baseval;
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

static void do_blit_16_cave( running_machine &machine, const struct sprite_cave *sprite )
{
	/*  assumes SPRITE_LIST_RAW_DATA flag is set */
	cave_state *state = machine.driver_data<cave_state>();
	int x1, x2, y1, y2, dx, dy;
	int xcount0 = 0, ycount0 = 0;

	if (sprite->flags & SPRITE_FLIPX_CAVE)
	{
		x2 = sprite->x;
		x1 = x2 + sprite->total_width;
		dx = -1;
		if (x2 < state->m_blit.clip_left)
			x2 = state->m_blit.clip_left;
		if (x1 > state->m_blit.clip_right)
		{
			xcount0 = x1 - state->m_blit.clip_right;
			x1 = state->m_blit.clip_right;
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
		if (x1 < state->m_blit.clip_left)
		{
			xcount0 = state->m_blit.clip_left - x1;
			x1 = state->m_blit.clip_left;
		}
		if (x2 > state->m_blit.clip_right)
			x2 = state->m_blit.clip_right;
		if (x1 >= x2)
			return;
	}
	if (sprite->flags & SPRITE_FLIPY_CAVE)
	{
		y2 = sprite->y;
		y1 = y2 + sprite->total_height;
		dy = -1;
		if (y2 < state->m_blit.clip_top)
			y2 = state->m_blit.clip_top;
		if (y1 > state->m_blit.clip_bottom)
		{
			ycount0 = y1 - state->m_blit.clip_bottom;
			y1 = state->m_blit.clip_bottom;
		}
		if (y2 >= y1)
			return;
		y1--; y2--;
	}
	else {
		y1 = sprite->y;
		y2 = y1 + sprite->total_height;
		dy = 1;
		if (y1 < state->m_blit.clip_top )
		{
			ycount0 = state->m_blit.clip_top - y1;
			y1 = state->m_blit.clip_top;
		}
		if (y2 > state->m_blit.clip_bottom)
			y2 = state->m_blit.clip_bottom;
		if (y1 >= y2)
			return;
	}

	{
		const UINT8 *pen_data = sprite->pen_data;
		pen_t base_pen = sprite->base_pen;
		int x, y;
		UINT8 pen;
		int pitch = state->m_blit.line_offset * dy / 2;
		UINT16 *dest = (UINT16 *)(state->m_blit.baseaddr + state->m_blit.line_offset * y1);

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


static void do_blit_16_cave_zb( running_machine &machine,  const struct sprite_cave *sprite )
{
	/*  assumes SPRITE_LIST_RAW_DATA flag is set */
	cave_state *state = machine.driver_data<cave_state>();
	int x1, x2, y1, y2, dx, dy;
	int xcount0 = 0, ycount0 = 0;

	if (sprite->flags & SPRITE_FLIPX_CAVE)
	{
		x2 = sprite->x;
		x1 = x2 + sprite->total_width;
		dx = -1;
		if (x2 < state->m_blit.clip_left)
			x2 = state->m_blit.clip_left;
		if (x1 > state->m_blit.clip_right)
		{
			xcount0 = x1 - state->m_blit.clip_right;
			x1 = state->m_blit.clip_right;
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
		if (x1 < state->m_blit.clip_left)
		{
			xcount0 = state->m_blit.clip_left - x1;
			x1 = state->m_blit.clip_left;
		}
		if (x2 > state->m_blit.clip_right)
			x2 = state->m_blit.clip_right;
		if (x1 >= x2)
			return;
	}
	if (sprite->flags & SPRITE_FLIPY_CAVE)
	{
		y2 = sprite->y;
		y1 = y2 + sprite->total_height;
		dy = -1;
		if (y2 < state->m_blit.clip_top)
			y2 = state->m_blit.clip_top;
		if (y1 > state->m_blit.clip_bottom)
		{
			ycount0 = y1 - state->m_blit.clip_bottom;
			y1 = state->m_blit.clip_bottom;
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
		if (y1 < state->m_blit.clip_top)
		{
			ycount0 = state->m_blit.clip_top - y1;
			y1 = state->m_blit.clip_top;
		}
		if (y2 > state->m_blit.clip_bottom)
			y2 = state->m_blit.clip_bottom;
		if (y1 >= y2)
			return;
	}

	{
		const UINT8 *pen_data = sprite->pen_data;
		pen_t base_pen = sprite->base_pen;
		int x, y;
		UINT8 pen;
		int pitch = state->m_blit.line_offset * dy / 2;
		UINT16 *dest = (UINT16 *)(state->m_blit.baseaddr + state->m_blit.line_offset * y1);
		int pitchz = state->m_blit.line_offset_zbuf * dy / 2;
		UINT16 *zbf = (UINT16 *)(state->m_blit.baseaddr_zbuf + state->m_blit.line_offset_zbuf * y1);
		UINT16 pri_sp = (UINT16)(sprite - state->m_sprite) + state->m_sprite_zbuf_baseval;

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


static void sprite_draw_cave( running_machine &machine, int priority )
{
	cave_state *state = machine.driver_data<cave_state>();
	int i = 0;
	while (state->m_sprite_table[priority][i])
	{
		const struct sprite_cave *sprite = state->m_sprite_table[priority][i++];
		if ((sprite->tile_width == sprite->total_width) && (sprite->tile_height == sprite->total_height))
			do_blit_16_cave(machine, sprite);
		else
			do_blit_zoom16_cave(machine, sprite);
	}
}

static void sprite_draw_cave_zbuf( running_machine &machine, int priority )
{
	cave_state *state = machine.driver_data<cave_state>();
	int i = 0;
	while (state->m_sprite_table[priority][i])
	{
		const struct sprite_cave *sprite = state->m_sprite_table[priority][i++];
		if ((sprite->tile_width == sprite->total_width) && (sprite->tile_height == sprite->total_height))
			do_blit_16_cave_zb(machine, sprite);
		else
			do_blit_zoom16_cave_zb(machine, sprite);
	}
}

static void sprite_draw_donpachi( running_machine &machine, int priority )
{
	cave_state *state = machine.driver_data<cave_state>();
	int i = 0;
	while (state->m_sprite_table[priority][i])
		do_blit_16_cave(machine, state->m_sprite_table[priority][i++]);
}

static void sprite_draw_donpachi_zbuf( running_machine &machine, int priority )
{
	cave_state *state = machine.driver_data<cave_state>();
	int i = 0;
	while (state->m_sprite_table[priority][i])
		do_blit_16_cave_zb(machine, state->m_sprite_table[priority][i++]);
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
	running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect,
	UINT32 flags, UINT32 priority, UINT32 priority2, int GFX )
{
	cave_state *state = machine.driver_data<cave_state>();
	tilemap_t *TILEMAP = state->m_tilemap[GFX];
	UINT16 *VRAM = state->m_vram[GFX];
	UINT16 *VCTRL = state->m_vctrl[GFX];
	int sx, sy, flipx, flipy, offs_x, offs_y, offs_row;

	/* Bail out if ... */

	if	( (!TILEMAP) ||							/* no tilemap; */
		  ((VCTRL[2] & 0x0003) != priority2) ||	/* tilemap's global priority not the requested one; */
		  ((VCTRL[2] & 0x0010))	)				/* tilemap's disabled. */
		return;

	flipx = ~VCTRL[0] & 0x8000;
	flipy = ~VCTRL[1] & 0x8000;
	TILEMAP->set_flip((flipx ? TILEMAP_FLIPX : 0) | (flipy ? TILEMAP_FLIPY : 0) );

	offs_x	=	state->m_layers_offs_x;
	offs_y	=	state->m_layers_offs_y;

	offs_row =  flipy ? state->m_row_effect_offs_f : state->m_row_effect_offs_n;

	/* An additional 8 pixel offset for layers with 8x8 tiles. Plus
       Layer 0 is displaced by 1 pixel wrt Layer 1, so is Layer 2 wrt
       Layer 1 */
	if		(TILEMAP == state->m_tilemap[0])	offs_x -= (state->m_tiledim[0] ? 1 : (1 + 8));
	else if	(TILEMAP == state->m_tilemap[1])	offs_x -= (state->m_tiledim[1] ? 2 : (2 + 8));
	else if	(TILEMAP == state->m_tilemap[2])	offs_x -= (state->m_tiledim[2] ? 3 : (3 + 8));
	else if	(TILEMAP == state->m_tilemap[3])	offs_x -= (state->m_tiledim[3] ? 4 : (4 + 8));

	sx = VCTRL[0] - state->m_videoregs[0] + (flipx ? (offs_x + 2) : -offs_x);
	sy = VCTRL[1] - state->m_videoregs[1] + (flipy ? (offs_y + 2) : -offs_y);

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

		clip.min_x = cliprect.min_x;
		clip.max_x = cliprect.max_x;

		for (startline = cliprect.min_y; startline <= cliprect.max_y;)
		{
			/* Find the largest slice */
			vramdata0 = (vramdata1 = VRAM[(0x1002 + (((sy + offs_row + startline) * 4) & 0x7ff)) / 2]);
			for(endline = startline + 1; endline <= cliprect.max_y; endline++)
				if((++vramdata1) != VRAM[(0x1002 + (((sy + offs_row + endline) * 4) & 0x7ff)) / 2]) break;

			TILEMAP->set_scrolly(0, vramdata0 - startline);

			if (VCTRL[0] & 0x4000)	// row-scroll, row-select
			{
				int line;

				/*
                    Row-scroll:

                    A different scroll value is specified for each scan line.
                    This is handled using tilemap_set_scroll_rows and calling
                    tilemap_draw just once.
                */

				TILEMAP->set_scroll_rows(512);
				for(line = startline; line < endline; line++)
					TILEMAP->set_scrollx((vramdata0 - startline + line) & 511,
										sx + VRAM[(0x1000 + (((sy + offs_row + line) * 4) & 0x7ff)) / 2]);
			}
			else					// no row-scroll, row-select
			{
				TILEMAP->set_scroll_rows(1);
				TILEMAP->set_scrollx(0, sx);
			}

			if (flipy)
			{
				clip.min_y = cliprect.max_y - (endline - 1 - cliprect.min_y);
				clip.max_y = cliprect.max_y - (startline - cliprect.min_y);
			}
			else
			{
				clip.min_y = startline;
				clip.max_y = endline - 1;
			}

			TILEMAP->draw(bitmap, clip, flags, priority);

			startline = endline;
		}
	}
	else if (VCTRL[0] & 0x4000)	// row-scroll, no row-select
	{
		int line;
		TILEMAP->set_scroll_rows(512);
		for(line = cliprect.min_y; line <= cliprect.max_y; line++)
			TILEMAP->set_scrollx((line + sy) & 511,
							sx + VRAM[(0x1000+(((sy + offs_row + line) * 4) & 0x7ff)) / 2] );
		TILEMAP->set_scrolly(0, sy);
		TILEMAP->draw(bitmap, cliprect, flags, priority);
	}
	else
	{
		/* DEF_STR( Normal ) scrolling */
		TILEMAP->set_scroll_rows(1);
		TILEMAP->set_scroll_cols(1);
		TILEMAP->set_scrollx(0, sx);
		TILEMAP->set_scrolly(0, sy);
		TILEMAP->draw(bitmap, cliprect, flags, priority);
	}
}


SCREEN_UPDATE_IND16( cave )
{
	cave_state *state = screen.machine().driver_data<cave_state>();
	int pri, pri2, GFX;
	int layers_ctrl = -1;

	set_pens(screen.machine());

	state->m_blit.baseaddr = reinterpret_cast<UINT8 *>(bitmap.raw_pixptr(0));
	state->m_blit.line_offset = bitmap.rowbytes();
	state->m_blit.baseaddr_zbuf = reinterpret_cast<UINT8 *>(state->m_sprite_zbuf.raw_pixptr(0));
	state->m_blit.line_offset_zbuf = state->m_sprite_zbuf.rowbytes();

	/* Choose the tilemap to display (8x8 tiles or 16x16 tiles) */
	for (GFX = 0; GFX < 4; GFX++)
	{
		if (state->m_tilemap[GFX])
		{
			state->m_tiledim[GFX] = state->m_vctrl[GFX][1] & 0x2000;
			if (state->m_tiledim[GFX] != state->m_old_tiledim[GFX])
				state->m_tilemap[GFX]->mark_all_dirty();
			state->m_old_tiledim[GFX] = state->m_tiledim[GFX];
		}
	}

#ifdef MAME_DEBUG
{
	if ( screen.machine().input().code_pressed(KEYCODE_Z) || screen.machine().input().code_pressed(KEYCODE_X) || screen.machine().input().code_pressed(KEYCODE_C) ||
    	 screen.machine().input().code_pressed(KEYCODE_V) || screen.machine().input().code_pressed(KEYCODE_B) )
	{
		int msk = 0, val = 0;

		if (screen.machine().input().code_pressed(KEYCODE_X))	val = 1;	// priority 0 only
		if (screen.machine().input().code_pressed(KEYCODE_C))	val = 2;	// ""       1
		if (screen.machine().input().code_pressed(KEYCODE_V))	val = 4;	// ""       2
		if (screen.machine().input().code_pressed(KEYCODE_B))	val = 8;	// ""       3
		if (screen.machine().input().code_pressed(KEYCODE_Z))	val = 1|2|4|8;	// All of the above priorities

		if (screen.machine().input().code_pressed(KEYCODE_Q))	msk |= val <<  0;	// for layer 0
		if (screen.machine().input().code_pressed(KEYCODE_W))	msk |= val <<  4;	// for layer 1
		if (screen.machine().input().code_pressed(KEYCODE_E))	msk |= val <<  8;	// for layer 2
		if (screen.machine().input().code_pressed(KEYCODE_R))	msk |= val << 12;	// for layer 3
		if (screen.machine().input().code_pressed(KEYCODE_A))	msk |= val << 16;	// for sprites
		if (msk != 0) layers_ctrl &= msk;

#if 1
		/* Show the video registers (cave_videoregs) */
		popmessage("%04X %04X %04X %04X %04X %04X %04X %04X",
			state->m_videoregs[0], state->m_videoregs[1], state->m_videoregs[2], state->m_videoregs[3],
			state->m_videoregs[4], state->m_videoregs[5], state->m_videoregs[6], state->m_videoregs[7] );
#endif
		/* Show the scroll / flags registers of the selected layer */
		if ((state->m_tilemap[0]) && (msk & 0x000f))	popmessage("x:%04X y:%04X f:%04X", state->m_vctrl[0][0],state->m_vctrl[0][1],state->m_vctrl[0][2]);
		if ((state->m_tilemap[1]) && (msk & 0x00f0))	popmessage("x:%04X y:%04X f:%04X", state->m_vctrl[1][0],state->m_vctrl[1][1],state->m_vctrl[1][2]);
		if ((state->m_tilemap[2]) && (msk & 0x0f00))	popmessage("x:%04X y:%04X f:%04X", state->m_vctrl[2][0],state->m_vctrl[2][1],state->m_vctrl[2][2]);
		if ((state->m_tilemap[3]) && (msk & 0xf000))	popmessage("x:%04X y:%04X f:%04X", state->m_vctrl[3][0],state->m_vctrl[3][1],state->m_vctrl[3][2]);
	}

	/* Show the row / "column" scroll enable flags, when they change state */
	state->m_rasflag = 0;
	for (GFX = 0; GFX < 4; GFX++)
	{
		if (state->m_tilemap[GFX])
		{
			state->m_rasflag |= (state->m_vctrl[GFX][0] & 0x4000) ? 0x0001 << (4*GFX) : 0;
			state->m_rasflag |= (state->m_vctrl[GFX][1] & 0x4000) ? 0x0002 << (4*GFX) : 0;
		}
	}

	if (state->m_rasflag != state->m_old_rasflag)
	{
		popmessage("Line Effect: 0:%c%c 1:%c%c 2:%c%c 3:%c%c",
			(state->m_rasflag & 0x0001) ? 'x' : ' ', (state->m_rasflag & 0x0002) ? 'y' : ' ',
			(state->m_rasflag & 0x0010) ? 'x' : ' ', (state->m_rasflag & 0x0020) ? 'y' : ' ',
			(state->m_rasflag & 0x0100) ? 'x' : ' ', (state->m_rasflag & 0x0200) ? 'y' : ' ',
			(state->m_rasflag & 0x1000) ? 'x' : ' ', (state->m_rasflag & 0x2000) ? 'y' : ' ' );
		state->m_old_rasflag = state->m_rasflag;
	}
}
#endif

	cave_sprite_check(screen, cliprect);

	bitmap.fill(state->m_background_color, cliprect);

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
		if (layers_ctrl & (1 << (pri + 16)))	(*state->m_sprite_draw)(screen.machine(), pri);

		for (pri2 = 0; pri2 <= 3; pri2++)	// priority of the whole layer
		{
			if (layers_ctrl & (1 << (pri +  0)))	cave_tilemap_draw(screen.machine(), bitmap, cliprect, pri, 0, pri2, 0);
			if (layers_ctrl & (1 << (pri +  4)))	cave_tilemap_draw(screen.machine(), bitmap, cliprect, pri, 0, pri2, 1);
			if (layers_ctrl & (1 << (pri +  8)))	cave_tilemap_draw(screen.machine(), bitmap, cliprect, pri, 0, pri2, 2);
			if (layers_ctrl & (1 << (pri + 12)))	cave_tilemap_draw(screen.machine(), bitmap, cliprect, pri, 0, pri2, 3);
		}
	}
	return 0;
}



/**************************************************************/

void cave_get_sprite_info( running_machine &machine )
{
	cave_state *state = machine.driver_data<cave_state>();
	if (state->m_kludge == 3)	/* mazinger metmqstr */
	{
		if (machine.video().skip_this_frame() == 0)
		{
			state->m_spriteram_bank = state->m_spriteram_bank_delay;
			(*state->m_get_sprite_info)(machine);
		}
		state->m_spriteram_bank_delay = state->m_videoregs[4] & 1;
	}
	else
	{
		if (machine.video().skip_this_frame() == 0)
		{
			state->m_spriteram_bank = state->m_videoregs[4] & 1;
			(*state->m_get_sprite_info)(machine);
		}
	}
}
