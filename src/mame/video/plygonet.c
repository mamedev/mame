/*
    Polygonet Commanders (Konami, 1993)

    Video hardware emulation

    Currently contains: TTL text plane (probably not complete, needs banking? colors?)
    Needs: PSAC2 roz plane, polygons
*/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/plygonet.h"

/* TTL text plane */

static TILE_GET_INFO( ttl_get_tile_info )
{
	polygonet_state *state = machine->driver_data<polygonet_state>();
	int attr, code;

	code = state->ttl_vram[tile_index]&0xfff;

	attr = state->ttl_vram[tile_index]>>12;	/* palette in all 4 bits? */

	SET_TILE_INFO(state->ttl_gfx_index, code, attr, 0);
}

static TILE_GET_INFO( roz_get_tile_info )
{
	polygonet_state *state = machine->driver_data<polygonet_state>();
	int attr, code;

	attr = (state->roz_vram[tile_index] >> 12) + 16;	/* roz base palette is palette 16 */
	code = state->roz_vram[tile_index] & 0x3ff;

	SET_TILE_INFO(0, code, attr, 0);
}

READ32_HANDLER( polygonet_ttl_ram_r )
{
	polygonet_state *state = space->machine->driver_data<polygonet_state>();
	UINT32 *vram = (UINT32 *)state->ttl_vram;

	return vram[offset];
}

WRITE32_HANDLER( polygonet_ttl_ram_w )
{
	polygonet_state *state = space->machine->driver_data<polygonet_state>();
	UINT32 *vram = (UINT32 *)state->ttl_vram;

	COMBINE_DATA(&vram[offset]);

	tilemap_mark_tile_dirty(state->ttl_tilemap, offset*2);
	tilemap_mark_tile_dirty(state->ttl_tilemap, offset*2+1);
}

READ32_HANDLER( polygonet_roz_ram_r )
{
	polygonet_state *state = space->machine->driver_data<polygonet_state>();
	UINT32 *vram = (UINT32 *)state->roz_vram;

	return vram[offset];
}

WRITE32_HANDLER( polygonet_roz_ram_w )
{
	polygonet_state *state = space->machine->driver_data<polygonet_state>();
	UINT32 *vram = (UINT32 *)state->roz_vram;

	COMBINE_DATA(&vram[offset]);

	tilemap_mark_tile_dirty(state->roz_tilemap, offset*2);
	tilemap_mark_tile_dirty(state->roz_tilemap, offset*2+1);
}

static TILEMAP_MAPPER( plygonet_scan )
{
	return row * num_cols + (col^1);
}

static TILEMAP_MAPPER( plygonet_scan_cols )
{
	return col * num_rows + (row^1);
}

VIDEO_START( polygonet )
{
	polygonet_state *state = machine->driver_data<polygonet_state>();
	static const gfx_layout charlayout =
	{
		8, 8,	/* 8x8 */
		4096,	/* # of tiles */
		4,		/* 4bpp */
		{ 0, 1, 2, 3 },	/* plane offsets */
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },	/* X offsets */
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },	/* Y offsets */
		8*8*4
	};

	/* find first empty slot to decode gfx */
	for (state->ttl_gfx_index = 0; state->ttl_gfx_index < MAX_GFX_ELEMENTS; state->ttl_gfx_index++)
		if (machine->gfx[state->ttl_gfx_index] == 0)
			break;

	assert(state->ttl_gfx_index != MAX_GFX_ELEMENTS);

	/* decode the ttl layer's gfx */
	machine->gfx[state->ttl_gfx_index] = gfx_element_alloc(machine, &charlayout, machine->region("gfx1")->base(), machine->total_colors() / 16, 0);

	/* create the tilemap */
	state->ttl_tilemap = tilemap_create(machine, ttl_get_tile_info, plygonet_scan,  8, 8, 64, 32);

	tilemap_set_transparent_pen(state->ttl_tilemap, 0);

	/* set up the roz t-map too */
	state->roz_tilemap = tilemap_create(machine, roz_get_tile_info, plygonet_scan_cols, 16, 16, 32, 64);
	tilemap_set_transparent_pen(state->roz_tilemap, 0);

	/* save states */
	state_save_register_global(machine, state->ttl_gfx_index);
	state_save_register_global_array(machine, state->ttl_vram);
	state_save_register_global_array(machine, state->roz_vram);
}

VIDEO_UPDATE( polygonet )
{
	polygonet_state *state = screen->machine->driver_data<polygonet_state>();
	device_t *k053936 = screen->machine->device("k053936");
	bitmap_fill(screen->machine->priority_bitmap, NULL, 0);
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	k053936_zoom_draw(k053936, bitmap, cliprect, state->roz_tilemap, 0, 0, 0);

	tilemap_draw(bitmap, cliprect, state->ttl_tilemap, 0, 1<<0);
	return 0;
}

