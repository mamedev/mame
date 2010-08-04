#include "emu.h"
#include "includes/aerofgt.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_pspikes_tile_info )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	UINT16 code = state->bg1videoram[tile_index];
	int bank = (code & 0x1000) >> 12;
	SET_TILE_INFO(
			0,
			(code & 0x0fff) + (state->gfxbank[bank] << 12),
			((code & 0xe000) >> 13) + 8 * state->charpalettebank,
			0);
}

static TILE_GET_INFO( karatblz_bg1_tile_info )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	UINT16 code = state->bg1videoram[tile_index];
	SET_TILE_INFO(
			0,
			(code & 0x1fff) + (state->gfxbank[0] << 13),
			(code & 0xe000) >> 13,
			0);
}

/* also spinlbrk */
static TILE_GET_INFO( karatblz_bg2_tile_info )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	UINT16 code = state->bg2videoram[tile_index];
	SET_TILE_INFO(
			1,
			(code & 0x1fff) + (state->gfxbank[1] << 13),
			(code & 0xe000) >> 13,
			0);
}

static TILE_GET_INFO( spinlbrk_bg1_tile_info )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	UINT16 code = state->bg1videoram[tile_index];
	SET_TILE_INFO(
			0,
			(code & 0x0fff) + (state->gfxbank[0] << 12),
			(code & 0xf000) >> 12,
			0);
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	UINT16 code = state->bg1videoram[tile_index];
	int bank = (code & 0x1800) >> 11;
	SET_TILE_INFO(
			0,
			(code & 0x07ff) + (state->gfxbank[bank] << 11),
			(code & 0xe000) >> 13,
			0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	UINT16 code = state->bg2videoram[tile_index];
	int bank = 4 + ((code & 0x1800) >> 11);
	SET_TILE_INFO(
			1,
			(code & 0x07ff) + (state->gfxbank[bank] << 11),
			(code & 0xe000) >> 13,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static void aerofgt_register_state_globals( running_machine *machine )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	state_save_register_global_array(machine, state->gfxbank);
	state_save_register_global_array(machine, state->bank);
	state_save_register_global(machine, state->bg1scrollx);
	state_save_register_global(machine, state->bg1scrolly);
	state_save_register_global(machine, state->bg2scrollx);
	state_save_register_global(machine, state->bg2scrolly);
	state_save_register_global(machine, state->charpalettebank);
	state_save_register_global(machine, state->spritepalettebank);
}

VIDEO_START( pspikes )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	state->bg1_tilemap = tilemap_create(machine, get_pspikes_tile_info,tilemap_scan_rows,8,8,64,32);
	/* no bg2 in this game */

	state->sprite_gfx = 1;

	aerofgt_register_state_globals(machine);
	state_save_register_global(machine, state->spikes91_lookup);
}


VIDEO_START( karatblz )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	state->bg1_tilemap = tilemap_create(machine, karatblz_bg1_tile_info,tilemap_scan_rows,     8,8,64,64);
	state->bg2_tilemap = tilemap_create(machine, karatblz_bg2_tile_info,tilemap_scan_rows,8,8,64,64);

	tilemap_set_transparent_pen(state->bg2_tilemap, 15);
	state->spritepalettebank = 0;
	state->sprite_gfx = 2;

	aerofgt_register_state_globals(machine);
}

VIDEO_START( spinlbrk )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	int i;

	state->bg1_tilemap = tilemap_create(machine, spinlbrk_bg1_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->bg2_tilemap = tilemap_create(machine, karatblz_bg2_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	tilemap_set_transparent_pen(state->bg2_tilemap, 15);

	state->spritepalettebank = 0;
	state->sprite_gfx = 2;

	/* sprite maps are hardcoded in this game */

	/* enemy sprites use ROM instead of RAM */
	state->spriteram2 = (UINT16 *)memory_region(machine, "gfx5");
	state->spriteram2_size = 0x20000;

	/* front sprites are direct maps */
	state->spriteram1 = state->spriteram2 + state->spriteram2_size / 2;
	state->spriteram1_size = 0x4000;

	for (i = 0; i < state->spriteram1_size / 2; i++)
	{
		state->spriteram1[i] = i;
	}

	aerofgt_register_state_globals(machine);
}

VIDEO_START( turbofrc )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	state->bg1_tilemap = tilemap_create(machine, get_bg1_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->bg2_tilemap = tilemap_create(machine, get_bg2_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	tilemap_set_transparent_pen(state->bg2_tilemap, 15);

	state->spritepalettebank = 0;
	state->sprite_gfx = 2;

	aerofgt_register_state_globals(machine);
}

VIDEO_START( wbbc97 )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	state->bg1_tilemap = tilemap_create(machine, get_pspikes_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	/* no bg2 in this game */

	tilemap_set_transparent_pen(state->bg1_tilemap, 15);

	state->sprite_gfx = 1;

	aerofgt_register_state_globals(machine);

	state_save_register_global(machine, state->wbbc97_bitmap_enable);
}

/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( aerofgt_bg1videoram_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	COMBINE_DATA(&state->bg1videoram[offset]);
	tilemap_mark_tile_dirty(state->bg1_tilemap, offset);
}

WRITE16_HANDLER( aerofgt_bg2videoram_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	COMBINE_DATA(&state->bg2videoram[offset]);
	tilemap_mark_tile_dirty(state->bg2_tilemap, offset);
}


static void setbank( running_machine *machine, tilemap_t *tmap, int num, int bank )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	if (state->gfxbank[num] != bank)
	{
		state->gfxbank[num] = bank;
		tilemap_mark_all_tiles_dirty(tmap);
	}
}

WRITE16_HANDLER( pspikes_gfxbank_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	if (ACCESSING_BITS_0_7)
	{
		setbank(space->machine, state->bg1_tilemap, 0, (data & 0xf0) >> 4);
		setbank(space->machine, state->bg1_tilemap, 1, data & 0x0f);
	}
}

WRITE16_HANDLER( pspikesb_gfxbank_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	COMBINE_DATA(&state->rasterram[0x200 / 2]);

	setbank(space->machine, state->bg1_tilemap, 0, (data & 0xf000) >> 12);
	setbank(space->machine, state->bg1_tilemap, 1, (data & 0x0f00) >> 8);
}

WRITE16_HANDLER( spikes91_lookup_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	state->spikes91_lookup = data & 1;
}

WRITE16_HANDLER( karatblz_gfxbank_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	if (ACCESSING_BITS_8_15)
	{
		setbank(space->machine, state->bg1_tilemap, 0, (data & 0x0100) >> 8);
		setbank(space->machine, state->bg2_tilemap, 1, (data & 0x0800) >> 11);
	}
}

WRITE16_HANDLER( spinlbrk_gfxbank_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	if (ACCESSING_BITS_0_7)
	{
		setbank(space->machine, state->bg1_tilemap, 0, (data & 0x07));
		setbank(space->machine, state->bg2_tilemap, 1, (data & 0x38) >> 3);
	}
}

WRITE16_HANDLER( turbofrc_gfxbank_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	tilemap_t *tmap = (offset == 0) ? state->bg1_tilemap : state->bg2_tilemap;

	data = COMBINE_DATA(&state->bank[offset]);

	setbank(space->machine, tmap, 4 * offset + 0, (data >> 0) & 0x0f);
	setbank(space->machine, tmap, 4 * offset + 1, (data >> 4) & 0x0f);
	setbank(space->machine, tmap, 4 * offset + 2, (data >> 8) & 0x0f);
	setbank(space->machine, tmap, 4 * offset + 3, (data >> 12) & 0x0f);
}

WRITE16_HANDLER( aerofgt_gfxbank_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	tilemap_t *tmap = (offset < 2) ? state->bg1_tilemap : state->bg2_tilemap;

	data = COMBINE_DATA(&state->bank[offset]);

	setbank(space->machine, tmap, 2 * offset + 0, (data >> 8) & 0xff);
	setbank(space->machine, tmap, 2 * offset + 1, (data >> 0) & 0xff);
}

WRITE16_HANDLER( aerofgt_bg1scrollx_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	COMBINE_DATA(&state->bg1scrollx);
}

WRITE16_HANDLER( aerofgt_bg1scrolly_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	COMBINE_DATA(&state->bg1scrolly);
}

WRITE16_HANDLER( aerofgt_bg2scrollx_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	COMBINE_DATA(&state->bg2scrollx);
}

WRITE16_HANDLER( aerofgt_bg2scrolly_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	COMBINE_DATA(&state->bg2scrolly);
}

WRITE16_HANDLER( pspikes_palette_bank_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	if (ACCESSING_BITS_0_7)
	{
		state->spritepalettebank = data & 0x03;
		if (state->charpalettebank != (data & 0x1c) >> 2)
		{
			state->charpalettebank = (data & 0x1c) >> 2;
			tilemap_mark_all_tiles_dirty(state->bg1_tilemap);
		}
	}
}

WRITE16_HANDLER( wbbc97_bitmap_enable_w )
{
	aerofgt_state *state = space->machine->driver_data<aerofgt_state>();
	COMBINE_DATA(&state->wbbc97_bitmap_enable);
}

/***************************************************************************

  Display refresh

***************************************************************************/

static void aerofgt_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	int offs;
	priority <<= 12;

	offs = 0;
	while (offs < 0x0400 && (state->spriteram3[offs] & 0x8000) == 0)
	{
		int attr_start = 4 * (state->spriteram3[offs] & 0x03ff);

		/* is the way I handle priority correct? Or should I just check bit 13? */
		if ((state->spriteram3[attr_start + 2] & 0x3000) == priority)
		{
			int map_start;
			int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;

			ox = state->spriteram3[attr_start + 1] & 0x01ff;
			xsize = (state->spriteram3[attr_start + 1] & 0x0e00) >> 9;
			zoomx = (state->spriteram3[attr_start + 1] & 0xf000) >> 12;
			oy = state->spriteram3[attr_start + 0] & 0x01ff;
			ysize = (state->spriteram3[attr_start + 0] & 0x0e00) >> 9;
			zoomy = (state->spriteram3[attr_start + 0] & 0xf000) >> 12;
			flipx = state->spriteram3[attr_start + 2] & 0x4000;
			flipy = state->spriteram3[attr_start + 2] & 0x8000;
			color = (state->spriteram3[attr_start + 2] & 0x0f00) >> 8;
			map_start = state->spriteram3[attr_start + 3] & 0x3fff;

			ox += (xsize * zoomx + 2) / 4;
			oy += (ysize * zoomy + 2) / 4;

			zoomx = 32 - zoomx;
			zoomy = 32 - zoomy;

			for (y = 0; y <= ysize; y++)
			{
				int sx, sy;

				if (flipy)
					sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
				else
					sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

				for (x = 0; x <= xsize; x++)
				{
					int code;

					if (flipx)
						sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
					else
						sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

					if (map_start < 0x2000)
						code = state->spriteram1[map_start & 0x1fff] & 0x1fff;
					else
						code = state->spriteram2[map_start & 0x1fff] & 0x1fff;

					drawgfxzoom_transpen(bitmap,cliprect,machine->gfx[state->sprite_gfx + (map_start >= 0x2000 ? 1 : 0)],
							code,
							color,
							flipx,flipy,
							sx,sy,
							zoomx << 11, zoomy << 11,15);
					map_start++;
				}
			}
		}
		offs++;
	}
}

static void turbofrc_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int chip, int chip_disabled_pri )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	int attr_start, base, first;
	base = chip * 0x0200;
	first = 4 * state->spriteram3[0x1fe + base];

	for (attr_start = base + 0x0200 - 8; attr_start >= first + base; attr_start -= 4)
	{
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color, pri;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(state->spriteram3[attr_start + 2] & 0x0080))
			continue;

		pri = state->spriteram3[attr_start + 2] & 0x0010;

		if ( chip_disabled_pri & !pri)
			continue;

		if ((!chip_disabled_pri) & (pri >> 4))
			continue;

		ox = state->spriteram3[attr_start + 1] & 0x01ff;
		xsize = (state->spriteram3[attr_start + 2] & 0x0700) >> 8;
		zoomx = (state->spriteram3[attr_start + 1] & 0xf000) >> 12;
		oy = state->spriteram3[attr_start + 0] & 0x01ff;
		ysize = (state->spriteram3[attr_start + 2] & 0x7000) >> 12;
		zoomy = (state->spriteram3[attr_start + 0] & 0xf000) >> 12;
		flipx = state->spriteram3[attr_start + 2] & 0x0800;
		flipy = state->spriteram3[attr_start + 2] & 0x8000;
		color = (state->spriteram3[attr_start + 2] & 0x000f) + 16 * state->spritepalettebank;

		map_start = state->spriteram3[attr_start + 3];

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		for (y = 0; y <= ysize; y++)
		{
			int sx, sy;

			if (flipy)
				sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx)
					sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = state->spriteram1[map_start % (state->spriteram1_size/2)];
				else
					code = state->spriteram2[map_start % (state->spriteram2_size/2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[state->sprite_gfx + chip],
							 code,
							 color,
							 flipx,flipy,
							 sx,sy,
							 zoomx << 11, zoomy << 11,
							 machine->priority_bitmap,pri ? 0 : 2,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

static void spinlbrk_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int chip, int chip_disabled_pri )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	int attr_start, base, first;
	base = chip * 0x0200;
	first = 4 * state->spriteram3[0x1fe + base];

	for (attr_start = base + 0x0200-8; attr_start >= first + base; attr_start -= 4)
	{
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color, pri;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(state->spriteram3[attr_start + 2] & 0x0080))
			continue;

		pri = state->spriteram3[attr_start + 2] & 0x0010;

		if ( chip_disabled_pri & !pri)
			continue;
		if ((!chip_disabled_pri) & (pri >> 4))
			continue;

		ox = state->spriteram3[attr_start + 1] & 0x01ff;
		xsize = (state->spriteram3[attr_start + 2] & 0x0700) >> 8;
		zoomx = (state->spriteram3[attr_start + 1] & 0xf000) >> 12;
		oy = state->spriteram3[attr_start + 0] & 0x01ff;
		ysize = (state->spriteram3[attr_start + 2] & 0x7000) >> 12;
		zoomy = (state->spriteram3[attr_start + 0] & 0xf000) >> 12;
		flipx = state->spriteram3[attr_start + 2] & 0x0800;
		flipy = state->spriteram3[attr_start + 2] & 0x8000;
		color = (state->spriteram3[attr_start + 2] & 0x000f) + 16 * state->spritepalettebank;

		map_start = state->spriteram3[attr_start + 3];

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		for (y = 0; y <= ysize; y++)
		{
			int sx, sy;

			if (flipy)
				sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx)
					sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = state->spriteram1[map_start % (state->spriteram1_size/2)];
				else
					code = state->spriteram2[map_start % (state->spriteram2_size/2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[state->sprite_gfx + chip],
							 code,
							 color,
							 flipx,flipy,
							 sx,sy,
							 zoomx << 11, zoomy << 11,
							 machine->priority_bitmap,pri ? 2 : 0,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

static void aerfboo2_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int chip, int chip_disabled_pri )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	int attr_start, base, first;

	base = chip * 0x0200;
//  first = 4 * state->spriteram3[0x1fe + base];
	first = 0;

	for (attr_start = base + 0x0200 - 4; attr_start >= first + base; attr_start -= 4)
	{
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color, pri;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(state->spriteram3[attr_start + 2] & 0x0080))
			continue;

		pri = state->spriteram3[attr_start + 2] & 0x0010;

		if ( chip_disabled_pri & !pri)
			continue;
		if ((!chip_disabled_pri) & (pri >> 4))
			continue;
		ox = state->spriteram3[attr_start + 1] & 0x01ff;
		xsize = (state->spriteram3[attr_start + 2] & 0x0700) >> 8;
		zoomx = (state->spriteram3[attr_start + 1] & 0xf000) >> 12;
		oy = state->spriteram3[attr_start + 0] & 0x01ff;
		ysize = (state->spriteram3[attr_start + 2] & 0x7000) >> 12;
		zoomy = (state->spriteram3[attr_start + 0] & 0xf000) >> 12;
		flipx = state->spriteram3[attr_start + 2] & 0x0800;
		flipy = state->spriteram3[attr_start + 2] & 0x8000;
		color = (state->spriteram3[attr_start + 2] & 0x000f) + 16 * state->spritepalettebank;

		map_start = state->spriteram3[attr_start + 3];

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		for (y = 0; y <= ysize; y++)
		{
			int sx, sy;

			if (flipy)
				sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx)
					sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = state->spriteram1[map_start % (state->spriteram1_size/2)];
				else
					code = state->spriteram2[map_start % (state->spriteram2_size/2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[state->sprite_gfx + chip],
							 code,
							 color,
							 flipx,flipy,
							 sx,sy,
							 zoomx << 11, zoomy << 11,
							 machine->priority_bitmap,pri ? 0 : 2,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

static void pspikesb_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	int i;

	for (i = 4; i < state->spriteram3_size / 2; i += 4)
	{
		int xpos, ypos, color, flipx, flipy, code;

		if (state->spriteram3[i + 3 - 4] & 0x8000)
			break;

		xpos = (state->spriteram3[i + 2] & 0x1ff) - 34;
		ypos = 256 - (state->spriteram3[i + 3 - 4] & 0x1ff) - 33;
		code = state->spriteram3[i + 0] & 0x1fff;
		flipy = 0;
		flipx = state->spriteram3[i + 1] & 0x0800;
		color = state->spriteram3[i + 1] & 0x000f;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[state->sprite_gfx],
				code,
				color,
				flipx,flipy,
				xpos,ypos,15);

		/* wrap around y */
		drawgfx_transpen(bitmap,cliprect,machine->gfx[state->sprite_gfx],
				code,
				color,
				flipx,flipy,
				xpos,ypos + 512,15);

	}
}

static void spikes91_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	int i;
	UINT8 *lookup;
	lookup = memory_region(machine, "user1");
	state->spritepalettebank = 1;

	for (i = state->spriteram3_size / 2 - 4; i >= 4; i -= 4)
	{
		int xpos, ypos, color, flipx, flipy, code, realcode;

		code = state->spriteram3[i + 0] & 0x1fff;

		if (!code)
			continue;

		xpos = (state->spriteram3[i + 2] & 0x01ff) - 16;
		ypos = 256 - (state->spriteram3[i + 1] & 0x00ff) - 26;
		flipy = 0;
		flipx = state->spriteram3[i + 3] & 0x8000;
		color = ((state->spriteram3[i + 3] & 0x00f0) >> 4);

		code |= state->spikes91_lookup * 0x2000;

		realcode = (lookup[code] << 8) + lookup[0x10000 + code];

		drawgfx_transpen(bitmap,cliprect,machine->gfx[state->sprite_gfx],
				realcode,
				color,
				flipx,flipy,
				xpos,ypos,15);

		/* wrap around y */
		drawgfx_transpen(bitmap,cliprect,machine->gfx[state->sprite_gfx],
				realcode,
				color,
				flipx,flipy,
				xpos,ypos + 512,15);
	}
}

static void aerfboot_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	int attr_start, last;

	last = ((state->rasterram[0x404 / 2] << 5) - 0x8000) / 2;

	for (attr_start = state->spriteram3_size / 2 - 4; attr_start >= last; attr_start -= 4)
	{
		int code;
		int ox, oy, sx, sy, zoomx, zoomy, flipx, flipy, color, pri;

		ox = state->spriteram3[attr_start + 1] & 0x01ff;
		oy = state->spriteram3[attr_start + 0] & 0x01ff;
		flipx = state->spriteram3[attr_start + 2] & 0x0800;
		flipy = state->spriteram3[attr_start + 2] & 0x8000;
		color = state->spriteram3[attr_start + 2] & 0x000f;

		zoomx = (state->spriteram3[attr_start + 1] & 0xf000) >> 12;
		zoomy = (state->spriteram3[attr_start + 0] & 0xf000) >> 12;
		pri = state->spriteram3[attr_start + 2] & 0x0010;
		code = state->spriteram3[attr_start + 3] & 0x1fff;

		if (!(state->spriteram3[attr_start + 2] & 0x0040))
			code |= 0x2000;

		zoomx = 32 + zoomx;
		zoomy = 32 + zoomy;

		sy = ((oy + 16 - 1) & 0x1ff) - 16;

		sx = ((ox + 16 + 3) & 0x1ff) - 16;

		pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[state->sprite_gfx + (code >= 0x1000 ? 0 : 1)],
				code,
				color,
				flipx,flipy,
				sx,sy,
				zoomx << 11,zoomy << 11,
				machine->priority_bitmap,pri ? 0 : 2,15);

	}

	last = ((state->rasterram[0x402 / 2] << 5) - 0x8000) / 2;

	for (attr_start = ((state->spriteram3_size / 2) / 2) - 4; attr_start >= last; attr_start -= 4)
	{
		int code;
		int ox, oy, sx, sy, zoomx, zoomy, flipx, flipy, color, pri;

		ox = state->spriteram3[attr_start + 1] & 0x01ff;
		oy = state->spriteram3[attr_start + 0] & 0x01ff;
		flipx = state->spriteram3[attr_start + 2] & 0x0800;
		flipy = state->spriteram3[attr_start + 2] & 0x8000;
		color = state->spriteram3[attr_start + 2] & 0x000f;

		zoomx = (state->spriteram3[attr_start + 1] & 0xf000) >> 12;
		zoomy = (state->spriteram3[attr_start + 0] & 0xf000) >> 12;
		pri = state->spriteram3[attr_start + 2] & 0x0010;
		code = state->spriteram3[attr_start + 3] & 0x1fff;

		if (!(state->spriteram3[attr_start + 2] & 0x0040))
			code |= 0x2000;

		zoomx = 32 + zoomx;
		zoomy = 32 + zoomy;

		sy = ((oy + 16 - 1) & 0x1ff) - 16;

		sx = ((ox + 16 + 3) & 0x1ff) - 16;

		pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[state->sprite_gfx + (code >= 0x1000 ? 0 : 1)],
				code,
				color,
				flipx,flipy,
				sx,sy,
				zoomx << 11,zoomy << 11,
				machine->priority_bitmap,pri ? 0 : 2,15);

	}
}

static void wbbc97_draw_bitmap( running_machine *machine, bitmap_t *bitmap )
{
	aerofgt_state *state = machine->driver_data<aerofgt_state>();
	int x, y, count;

	count = 16; // weird, the bitmap doesn't start at 0?
	for (y = 0; y < 256; y++)
		for (x = 0; x < 512; x++)
		{
			int color = state->bitmapram[count] >> 1;

			/* data is GRB; convert to RGB */
			rgb_t pen = MAKE_RGB(pal5bit((color & 0x3e0) >> 5), pal5bit((color & 0x7c00) >> 10), pal5bit(color & 0x1f));
			*BITMAP_ADDR32(bitmap, y, (10 + x - state->rasterram[(y & 0x7f)]) & 0x1ff) = pen;

			count++;
			count &= 0x1ffff;
		}
}


VIDEO_UPDATE( pspikes )
{
	aerofgt_state *state = screen->machine->driver_data<aerofgt_state>();
	int i, scrolly;

	tilemap_set_scroll_rows(state->bg1_tilemap, 256);
	scrolly = state->bg1scrolly;
	for (i = 0; i < 256; i++)
		tilemap_set_scrollx(state->bg1_tilemap, (i + scrolly) & 0xff, state->rasterram[i]);
	tilemap_set_scrolly(state->bg1_tilemap, 0, scrolly);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, 0, 0);
	turbofrc_draw_sprites(screen->machine, bitmap, cliprect, 0, -1);
	turbofrc_draw_sprites(screen->machine, bitmap, cliprect, 0, 0);
	return 0;
}

VIDEO_UPDATE( pspikesb )
{
	aerofgt_state *state = screen->machine->driver_data<aerofgt_state>();
	int i, scrolly;

	tilemap_set_scroll_rows(state->bg1_tilemap, 256);
	scrolly = state->bg1scrolly;
	for (i = 0; i < 256; i++)
		tilemap_set_scrollx(state->bg1_tilemap, (i + scrolly) & 0xff, state->rasterram[i] + 22);
	tilemap_set_scrolly(state->bg1_tilemap, 0, scrolly);

	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, 0, 0);
	pspikesb_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

VIDEO_UPDATE( spikes91 )
{
	aerofgt_state *state = screen->machine->driver_data<aerofgt_state>();
	int i, scrolly;
	int y, x;
	int count;
	const gfx_element *gfx = screen->machine->gfx[0];

	tilemap_set_scroll_rows(state->bg1_tilemap, 256);
	scrolly = state->bg1scrolly;

	for (i = 0; i < 256; i++)
		tilemap_set_scrollx(state->bg1_tilemap, (i + scrolly) & 0xff, state->rasterram[i + 0x01f0 / 2] + 0x96 + 0x16);
	tilemap_set_scrolly(state->bg1_tilemap, 0, scrolly);

	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, 0, 0);
	spikes91_draw_sprites(screen->machine, bitmap, cliprect);

	/* we could use a tilemap, but it's easier to just do it here */
	count = 0;
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 64; x++)
		{
			UINT16 tileno = state->tx_tilemap_ram[count] & 0x1fff;
			UINT16 colour = state->tx_tilemap_ram[count] & 0xe000;
			drawgfx_transpen(bitmap,cliprect,gfx,
					tileno,
					colour>>13,
					0,0,
					(x*8)+24,(y*8)+8,15);

			count++;

		}

	}

	return 0;
}

VIDEO_UPDATE( karatblz )
{
	aerofgt_state *state = screen->machine->driver_data<aerofgt_state>();
	tilemap_set_scrollx(state->bg1_tilemap, 0, state->bg1scrollx - 8);
	tilemap_set_scrolly(state->bg1_tilemap, 0, state->bg1scrolly);
	tilemap_set_scrollx(state->bg2_tilemap, 0, state->bg2scrollx - 4);
	tilemap_set_scrolly(state->bg2_tilemap, 0, state->bg2scrolly);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg2_tilemap, 0, 0);

	/* we use the priority buffer so sprites are drawn front to back */
	turbofrc_draw_sprites(screen->machine, bitmap, cliprect, 1, -1);
	turbofrc_draw_sprites(screen->machine, bitmap, cliprect, 1, 0);
	turbofrc_draw_sprites(screen->machine, bitmap, cliprect, 0, -1);
	turbofrc_draw_sprites(screen->machine, bitmap, cliprect, 0, 0);
	return 0;
}

VIDEO_UPDATE( spinlbrk )
{
	aerofgt_state *state = screen->machine->driver_data<aerofgt_state>();
	int i, scrolly;

	tilemap_set_scroll_rows(state->bg1_tilemap, 512);
	scrolly = 0;
	for (i = 0; i < 256; i++)
		tilemap_set_scrollx(state->bg1_tilemap, (i + scrolly) & 0x1ff, state->rasterram[i] - 8);
//  tilemap_set_scrolly(state->bg1_tilemap, 0, state->bg1scrolly);
	tilemap_set_scrollx(state->bg2_tilemap, 0, state->bg2scrollx - 4);
//  tilemap_set_scrolly(state->bg2_tilemap, 0, state->bg2scrolly);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg2_tilemap, 0, 1);

	/* we use the priority buffer so sprites are drawn front to back */
	spinlbrk_draw_sprites(screen->machine, bitmap, cliprect, 0, 0);
	spinlbrk_draw_sprites(screen->machine, bitmap, cliprect, 0, -1);
	spinlbrk_draw_sprites(screen->machine, bitmap, cliprect, 1, 0);
	spinlbrk_draw_sprites(screen->machine, bitmap, cliprect, 1, -1);
	return 0;
}

VIDEO_UPDATE( turbofrc )
{
	aerofgt_state *state = screen->machine->driver_data<aerofgt_state>();
	int i, scrolly;

	tilemap_set_scroll_rows(state->bg1_tilemap, 512);
	scrolly = state->bg1scrolly + 2;
	for (i = 0; i < 256; i++)
//      tilemap_set_scrollx(state->bg1_tilemap, (i + scrolly) & 0x1ff, state->rasterram[i] - 11);
		tilemap_set_scrollx(state->bg1_tilemap, (i + scrolly) & 0x1ff, state->rasterram[7] - 11);
	tilemap_set_scrolly(state->bg1_tilemap, 0, scrolly);
	tilemap_set_scrollx(state->bg2_tilemap, 0, state->bg2scrollx - 7);
	tilemap_set_scrolly(state->bg2_tilemap, 0, state->bg2scrolly + 2);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg2_tilemap, 0, 1);

	/* we use the priority buffer so sprites are drawn front to back */
	turbofrc_draw_sprites(screen->machine, bitmap, cliprect, 1, -1); //ship
	turbofrc_draw_sprites(screen->machine, bitmap, cliprect, 1, 0); //intro
	turbofrc_draw_sprites(screen->machine, bitmap, cliprect, 0, -1); //enemy
	turbofrc_draw_sprites(screen->machine, bitmap, cliprect, 0, 0); //enemy
	return 0;
}

VIDEO_UPDATE( aerofgt )
{
	aerofgt_state *state = screen->machine->driver_data<aerofgt_state>();
	tilemap_set_scrollx(state->bg1_tilemap, 0, state->rasterram[0x0000] - 18);
	tilemap_set_scrolly(state->bg1_tilemap, 0, state->bg1scrolly);
	tilemap_set_scrollx(state->bg2_tilemap, 0, state->rasterram[0x0200] - 20);
	tilemap_set_scrolly(state->bg2_tilemap, 0, state->bg2scrolly);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, 0, 0);

	aerofgt_draw_sprites(screen->machine, bitmap, cliprect, 0);
	aerofgt_draw_sprites(screen->machine, bitmap, cliprect, 1);

	tilemap_draw(bitmap, cliprect, state->bg2_tilemap, 0, 0);

	aerofgt_draw_sprites(screen->machine, bitmap, cliprect, 2);
	aerofgt_draw_sprites(screen->machine, bitmap, cliprect, 3);
	return 0;
}


VIDEO_UPDATE( aerfboot )
{
	aerofgt_state *state = screen->machine->driver_data<aerofgt_state>();
	int i, scrolly;

	tilemap_set_scroll_rows(state->bg1_tilemap, 512);
	scrolly = state->bg1scrolly + 2;
	for (i = 0; i < 256; i++)
		tilemap_set_scrollx(state->bg1_tilemap, (i + scrolly) & 0x1ff, state->rasterram[7] + 174);
	tilemap_set_scrolly(state->bg1_tilemap, 0, scrolly);
	tilemap_set_scrollx(state->bg2_tilemap, 0, state->bg2scrollx + 172);
	tilemap_set_scrolly(state->bg2_tilemap, 0, state->bg2scrolly + 2);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg2_tilemap, 0, 1);

	/* we use the priority buffer so sprites are drawn front to back */
	aerfboot_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

VIDEO_UPDATE( aerfboo2 )
{
	aerofgt_state *state = screen->machine->driver_data<aerofgt_state>();
	int i, scrolly;

	tilemap_set_scroll_rows(state->bg1_tilemap, 512);
	scrolly = state->bg1scrolly + 2;
	for (i = 0; i < 256; i++)
//      tilemap_set_scrollx(state->bg1_tilemap, (i + scrolly) & 0x1ff, state->rasterram[i] - 11);
		tilemap_set_scrollx(state->bg1_tilemap, (i + scrolly) & 0x1ff, state->rasterram[7] - 11);
	tilemap_set_scrolly(state->bg1_tilemap, 0, scrolly);
	tilemap_set_scrollx(state->bg2_tilemap, 0, state->bg2scrollx - 7);
	tilemap_set_scrolly(state->bg2_tilemap, 0, state->bg2scrolly + 2);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg2_tilemap, 0, 1);

	/* we use the priority buffer so sprites are drawn front to back */
	aerfboo2_draw_sprites(screen->machine, bitmap, cliprect, 1, -1); //ship
	aerfboo2_draw_sprites(screen->machine, bitmap, cliprect, 1, 0); //intro
	aerfboo2_draw_sprites(screen->machine, bitmap, cliprect, 0, -1); //enemy
	aerfboo2_draw_sprites(screen->machine, bitmap, cliprect, 0, 0); //enemy
	return 0;
}

VIDEO_UPDATE( wbbc97 )
{
	aerofgt_state *state = screen->machine->driver_data<aerofgt_state>();
	int i, scrolly;

	tilemap_set_scroll_rows(state->bg1_tilemap, 256);
	scrolly = state->bg1scrolly;
	for (i = 0; i < 256; i++)
		tilemap_set_scrollx(state->bg1_tilemap, (i + scrolly) & 0xff, state->rasterram[i]);
	tilemap_set_scrolly(state->bg1_tilemap, 0, scrolly);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	if (state->wbbc97_bitmap_enable)
	{
		wbbc97_draw_bitmap(screen->machine, bitmap);
		tilemap_draw(bitmap, cliprect, state->bg1_tilemap, 0, 0);
	}
	else
	{
		tilemap_draw(bitmap, cliprect, state->bg1_tilemap, TILEMAP_DRAW_OPAQUE, 0);
	}

	turbofrc_draw_sprites(screen->machine, bitmap, cliprect, 0, -1);
	turbofrc_draw_sprites(screen->machine, bitmap, cliprect, 0, 0);
	return 0;
}
