/***************************************************************************

   Tumblepop (bootlegs) and similar hardware
   Video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

The original uses Data East custom chip 55 for backgrounds,
custom chip 52 for sprites.  The bootlegs use generic chips to perform similar
functions

Tumblepop is one of few games to take advantage of the playfields ability
to switch between 8*8 tiles and 16*16 tiles.

***************************************************************************/

#include "emu.h"
#include "includes/tumbleb.h"

/******************************************************************************/

static void tumblepb_draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	UINT16 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0; offs < 0x400; offs += 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult;

		sprite = spriteram[offs + 1] & 0x3fff;
		if (!sprite)
			continue;

		y = spriteram[offs];
		flash = y & 0x1000;
		if (flash && (machine.primary_screen->frame_number() & 1))
			continue;

		x = spriteram[offs + 2];
		colour = (x >> 9) & 0xf;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
		x = 304 - x;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (state->m_flipscreen)
		{
			y = 240 - y;
			x = 304 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else
			mult = -16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					state->m_sprite_xoffset + x, state->m_sprite_yoffset + y + mult * multi, 0);

			multi--;
		}
	}
}

static void jumpkids_draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	UINT16 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0; offs < state->m_spriteram_size / 2; offs += 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult;

		sprite = spriteram[offs + 1] & 0x7fff;
		if (!sprite)
			continue;

		y = spriteram[offs];
		flash = y & 0x1000;
		if (flash && (machine.primary_screen->frame_number() & 1))
			continue;

		x = spriteram[offs+2];
		colour = (x >> 9) & 0xf;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
		x = 304 - x;

		//  sprite &= ~multi; /* Todo:  I bet TumblePop bootleg doesn't do this either */
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (state->m_flipscreen)
		{
			y = 240 - y;
			x = 304 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else
			mult = -16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,// x-1 for bcstory .. realign other layers?
					cliprect,machine.gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					state->m_sprite_xoffset + x, state->m_sprite_yoffset + y + mult * multi, 0);

			multi--;
		}
	}
}

static void fncywld_draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	UINT16 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0; offs < 0x400; offs += 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult;

		sprite = spriteram[offs + 1] & 0x3fff;
		if (!sprite)
			continue;

		y = spriteram[offs];
		flash = y & 0x1000;
		if (flash && (machine.primary_screen->frame_number() & 1))
			continue;

		x = spriteram[offs + 2];
		colour = (x >> 9) & 0x3f;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
		x = 304 - x;

		//  sprite &= ~multi; /* Todo:  I bet TumblePop bootleg doesn't do this either */
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (state->m_flipscreen)
		{
			y = 240 - y;
			x = 304 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else mult = -16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					state->m_sprite_xoffset + x, state->m_sprite_yoffset + y + mult * multi, 15);

			multi--;
		}
	}
}

/******************************************************************************/

WRITE16_HANDLER( bcstory_tilebank_w )
{
	tumbleb_state *state = space->machine().driver_data<tumbleb_state>();

	state->m_tilebank = data;
	tilemap_mark_all_tiles_dirty(state->m_pf1_tilemap);
	tilemap_mark_all_tiles_dirty(state->m_pf1_alt_tilemap);
	tilemap_mark_all_tiles_dirty(state->m_pf2_tilemap);
}

WRITE16_HANDLER( chokchok_tilebank_w )
{
	tumbleb_state *state = space->machine().driver_data<tumbleb_state>();

	state->m_tilebank = data << 1;
	tilemap_mark_all_tiles_dirty(state->m_pf1_tilemap);
	tilemap_mark_all_tiles_dirty(state->m_pf1_alt_tilemap);
	tilemap_mark_all_tiles_dirty(state->m_pf2_tilemap);
}

WRITE16_HANDLER( wlstar_tilebank_w )
{
	tumbleb_state *state = space->machine().driver_data<tumbleb_state>();

	/* it just writes 0000 or ffff */
	state->m_tilebank = data & 0x4000;
	tilemap_mark_all_tiles_dirty(state->m_pf1_tilemap);
	tilemap_mark_all_tiles_dirty(state->m_pf1_alt_tilemap);
	tilemap_mark_all_tiles_dirty(state->m_pf2_tilemap);
}


WRITE16_HANDLER( suprtrio_tilebank_w )
{
	tumbleb_state *state = space->machine().driver_data<tumbleb_state>();

	state->m_tilebank = data << 14; // shift it here, makes using bcstory_tilebank easier
	tilemap_mark_all_tiles_dirty(state->m_pf1_tilemap);
	tilemap_mark_all_tiles_dirty(state->m_pf1_alt_tilemap);
	tilemap_mark_all_tiles_dirty(state->m_pf2_tilemap);
}


WRITE16_HANDLER( tumblepb_pf1_data_w )
{
	tumbleb_state *state = space->machine().driver_data<tumbleb_state>();

	COMBINE_DATA(&state->m_pf1_data[offset]);
	tilemap_mark_tile_dirty(state->m_pf1_tilemap, offset);
	tilemap_mark_tile_dirty(state->m_pf1_alt_tilemap, offset);
}

WRITE16_HANDLER( tumblepb_pf2_data_w )
{
	tumbleb_state *state = space->machine().driver_data<tumbleb_state>();

	COMBINE_DATA(&state->m_pf2_data[offset]);
	tilemap_mark_tile_dirty(state->m_pf2_tilemap, offset);

	if (state->m_pf2_alt_tilemap)
		tilemap_mark_tile_dirty(state->m_pf2_alt_tilemap, offset);
}

WRITE16_HANDLER( fncywld_pf1_data_w )
{
	tumbleb_state *state = space->machine().driver_data<tumbleb_state>();

	COMBINE_DATA(&state->m_pf1_data[offset]);
	tilemap_mark_tile_dirty(state->m_pf1_tilemap, offset / 2);
	tilemap_mark_tile_dirty(state->m_pf1_alt_tilemap, offset / 2);
}

WRITE16_HANDLER( fncywld_pf2_data_w )
{
	tumbleb_state *state = space->machine().driver_data<tumbleb_state>();

	COMBINE_DATA(&state->m_pf2_data[offset]);
	tilemap_mark_tile_dirty(state->m_pf2_tilemap, offset / 2);
}

WRITE16_HANDLER( tumblepb_control_0_w )
{
	tumbleb_state *state = space->machine().driver_data<tumbleb_state>();
	COMBINE_DATA(&state->m_control_0[offset]);
}


WRITE16_HANDLER( pangpang_pf1_data_w )
{
	tumbleb_state *state = space->machine().driver_data<tumbleb_state>();

	COMBINE_DATA(&state->m_pf1_data[offset]);
	tilemap_mark_tile_dirty(state->m_pf1_tilemap, offset / 2);
	tilemap_mark_tile_dirty(state->m_pf1_alt_tilemap, offset / 2);
}

WRITE16_HANDLER( pangpang_pf2_data_w )
{
	tumbleb_state *state = space->machine().driver_data<tumbleb_state>();

	COMBINE_DATA(&state->m_pf2_data[offset]);
	tilemap_mark_tile_dirty(state->m_pf2_tilemap, offset / 2);

	if (state->m_pf2_alt_tilemap)
		tilemap_mark_tile_dirty(state->m_pf2_alt_tilemap, offset / 2);
}

/******************************************************************************/

static TILEMAP_MAPPER( tumblep_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x60) << 5);
}

INLINE void get_bg_tile_info( running_machine &machine, tile_data *tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base)
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = gfx_base[tile_index];

	SET_TILE_INFO(
			gfx_bank,
			(data & 0x0fff) | (state->m_tilebank >> 2),
			data >> 12,
			0);
}

static TILE_GET_INFO( get_bg1_tile_info ) { tumbleb_state *state = machine.driver_data<tumbleb_state>();	get_bg_tile_info(machine, tileinfo, tile_index, 2, state->m_pf1_data); }
static TILE_GET_INFO( get_bg2_tile_info ) { tumbleb_state *state = machine.driver_data<tumbleb_state>();	get_bg_tile_info(machine, tileinfo, tile_index, 1, state->m_pf2_data); }

static TILE_GET_INFO( get_fg_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf1_data[tile_index];

	SET_TILE_INFO(
			0,
			(data & 0x0fff) | state->m_tilebank,
			data >> 12,
			0);
}

INLINE void get_fncywld_bg_tile_info( running_machine &machine, tile_data *tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base)
{
	int data = gfx_base[tile_index * 2];
	int attr = gfx_base[tile_index * 2 + 1];

	SET_TILE_INFO(
			gfx_bank,
			data & 0x1fff,
			attr & 0x1f,
			0);
}

static TILE_GET_INFO( get_fncywld_bg1_tile_info ) { tumbleb_state *state = machine.driver_data<tumbleb_state>();	get_fncywld_bg_tile_info(machine, tileinfo, tile_index, 2, state->m_pf1_data); }
static TILE_GET_INFO( get_fncywld_bg2_tile_info ) { tumbleb_state *state = machine.driver_data<tumbleb_state>();	get_fncywld_bg_tile_info(machine, tileinfo, tile_index, 1, state->m_pf2_data); }

static TILE_GET_INFO( get_fncywld_fg_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf1_data[tile_index * 2];
	int attr = state->m_pf1_data[tile_index * 2 + 1];

	SET_TILE_INFO(
			0,
			data & 0x1fff,
			attr & 0x1f,
			0);
}


/* jump pop */
static TILE_GET_INFO( get_jumppop_bg1_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf1_data[tile_index];

	SET_TILE_INFO(
			2,
			data & 0x3fff,
			0,
			0);
}

static TILE_GET_INFO( get_jumppop_bg2_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf2_data[tile_index];

	SET_TILE_INFO(
			1,
			data & 0x1fff,
			1,
			0);
}

static TILE_GET_INFO( get_jumppop_bg2_alt_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf2_data[tile_index];

	SET_TILE_INFO(
			0,
			data & 0x7fff,
			1,
			0);
}


static TILE_GET_INFO( get_jumppop_fg_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf1_data[tile_index];

	SET_TILE_INFO(
			0,
			data & 0x7fff,
			0,
			0);
}

INLINE void pangpang_get_bg_tile_info( running_machine &machine, tile_data *tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base )
{
	int data = gfx_base[tile_index * 2 + 1];
	int attr = gfx_base[tile_index * 2];

	SET_TILE_INFO(
			gfx_bank,
			data & 0x1fff,
			(attr >>12) & 0xf,
			0);
}

INLINE void pangpang_get_bg2x_tile_info( running_machine &machine, tile_data *tileinfo, int tile_index, int gfx_bank, UINT16 *gfx_base )
{
	int data = gfx_base[tile_index * 2 + 1];
	int attr = gfx_base[tile_index * 2];

	SET_TILE_INFO(
			gfx_bank,
			(data & 0xfff) + 0x1000,
			(attr >>12) & 0xf,
			0);
}


static TILE_GET_INFO( pangpang_get_bg1_tile_info ) { tumbleb_state *state = machine.driver_data<tumbleb_state>();	pangpang_get_bg_tile_info(machine, tileinfo, tile_index, 2, state->m_pf1_data); }
static TILE_GET_INFO( pangpang_get_bg2_tile_info ) { tumbleb_state *state = machine.driver_data<tumbleb_state>();	pangpang_get_bg2x_tile_info(machine, tileinfo, tile_index, 1, state->m_pf2_data); }

static TILE_GET_INFO( pangpang_get_fg_tile_info )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();
	int data = state->m_pf1_data[tile_index * 2 + 1];
	int attr = state->m_pf1_data[tile_index * 2];

	SET_TILE_INFO(
			0,
			data & 0x1fff,
			(attr >> 12)& 0x1f,
			0);
}


static void tumbleb_tilemap_redraw(running_machine &machine)
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	tilemap_mark_all_tiles_dirty(state->m_pf1_tilemap);
	tilemap_mark_all_tiles_dirty(state->m_pf1_alt_tilemap);
	tilemap_mark_all_tiles_dirty(state->m_pf2_tilemap);
	if (state->m_pf2_alt_tilemap)
		tilemap_mark_all_tiles_dirty(state->m_pf2_alt_tilemap);
}

VIDEO_START( pangpang )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap =     tilemap_create(machine, pangpang_get_fg_tile_info,  tilemap_scan_rows, 8,  8, 64, 32);
	state->m_pf1_alt_tilemap = tilemap_create(machine, pangpang_get_bg1_tile_info, tumblep_scan,     16, 16, 64, 32);
	state->m_pf2_tilemap =     tilemap_create(machine, pangpang_get_bg2_tile_info, tumblep_scan,     16, 16, 64, 32);

	tilemap_set_transparent_pen(state->m_pf1_tilemap, 0);
	tilemap_set_transparent_pen(state->m_pf1_alt_tilemap, 0);

	state->m_sprite_xoffset = -1;
	state->m_sprite_yoffset = 0;

	machine.save().register_postload(save_prepost_delegate(FUNC(tumbleb_tilemap_redraw), &machine));
}


VIDEO_START( tumblepb )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap =     tilemap_create(machine, get_fg_tile_info,  tilemap_scan_rows, 8,  8, 64, 32);
	state->m_pf1_alt_tilemap = tilemap_create(machine, get_bg1_tile_info, tumblep_scan,     16, 16, 64, 32);
	state->m_pf2_tilemap =     tilemap_create(machine, get_bg2_tile_info, tumblep_scan,     16, 16, 64, 32);

	tilemap_set_transparent_pen(state->m_pf1_tilemap, 0);
	tilemap_set_transparent_pen(state->m_pf1_alt_tilemap, 0);

	state->m_sprite_xoffset = -1;
	state->m_sprite_yoffset = 0;

	machine.save().register_postload(save_prepost_delegate(FUNC(tumbleb_tilemap_redraw), &machine));
}

VIDEO_START( sdfight )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap =     tilemap_create(machine, get_fg_tile_info,  tilemap_scan_rows, 8,  8, 64, 64); // 64*64 to prevent bad tilemap wrapping? - check real behavior
	state->m_pf1_alt_tilemap = tilemap_create(machine, get_bg1_tile_info, tumblep_scan,     16, 16, 64, 32);
	state->m_pf2_tilemap =     tilemap_create(machine, get_bg2_tile_info, tumblep_scan,     16, 16, 64, 32);

	tilemap_set_transparent_pen(state->m_pf1_tilemap, 0);
	tilemap_set_transparent_pen(state->m_pf1_alt_tilemap, 0);

	/* aligned to monitor test */
	state->m_sprite_xoffset = 0;
	state->m_sprite_yoffset = 1;

	machine.save().register_postload(save_prepost_delegate(FUNC(tumbleb_tilemap_redraw), &machine));
}

VIDEO_START( fncywld )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap =     tilemap_create(machine, get_fncywld_fg_tile_info,  tilemap_scan_rows, 8,  8, 64, 32);
	state->m_pf1_alt_tilemap = tilemap_create(machine, get_fncywld_bg1_tile_info, tumblep_scan,     16, 16, 64, 32);
	state->m_pf2_tilemap =     tilemap_create(machine, get_fncywld_bg2_tile_info, tumblep_scan,     16, 16, 64, 32);

	tilemap_set_transparent_pen(state->m_pf1_tilemap, 15);
	tilemap_set_transparent_pen(state->m_pf1_alt_tilemap, 15);

	state->m_sprite_xoffset = -1;
	state->m_sprite_yoffset = 0;

	machine.save().register_postload(save_prepost_delegate(FUNC(tumbleb_tilemap_redraw), &machine));
}

VIDEO_START( jumppop )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap =     tilemap_create(machine, get_jumppop_fg_tile_info,      tilemap_scan_rows,     8,  8, 128, 64);
	state->m_pf1_alt_tilemap = tilemap_create(machine, get_jumppop_bg1_tile_info,     tilemap_scan_rows,    16, 16,  64, 64);
	state->m_pf2_tilemap =     tilemap_create(machine, get_jumppop_bg2_tile_info,     tilemap_scan_rows,    16, 16,  64, 64);
	state->m_pf2_alt_tilemap = tilemap_create(machine, get_jumppop_bg2_alt_tile_info, tilemap_scan_rows,     8,  8, 128, 64);

	tilemap_set_transparent_pen(state->m_pf1_tilemap, 0);
	tilemap_set_transparent_pen(state->m_pf1_alt_tilemap, 0);

	tilemap_set_flip(state->m_pf1_tilemap, TILEMAP_FLIPX);
	tilemap_set_flip(state->m_pf1_alt_tilemap, TILEMAP_FLIPX);
	tilemap_set_flip(state->m_pf2_tilemap, TILEMAP_FLIPX);
	tilemap_set_flip(state->m_pf2_alt_tilemap, TILEMAP_FLIPX);

	state->m_sprite_xoffset = -1;
	state->m_sprite_yoffset = 0;

	machine.save().register_postload(save_prepost_delegate(FUNC(tumbleb_tilemap_redraw), &machine));
}


VIDEO_START( suprtrio )
{
	tumbleb_state *state = machine.driver_data<tumbleb_state>();

	state->m_pf1_tilemap =     tilemap_create(machine, get_fg_tile_info,  tilemap_scan_rows, 8,  8, 64, 32);
	state->m_pf1_alt_tilemap = tilemap_create(machine, get_bg1_tile_info, tumblep_scan,     16, 16, 64, 32);
	state->m_pf2_tilemap =     tilemap_create(machine, get_bg2_tile_info, tumblep_scan,     16, 16, 64, 32);

	tilemap_set_transparent_pen(state->m_pf1_alt_tilemap, 0);

	machine.save().register_postload(save_prepost_delegate(FUNC(tumbleb_tilemap_redraw), &machine));
}

/******************************************************************************/


SCREEN_UPDATE( tumblepb )
{
	tumbleb_state *state = screen->machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	tilemap_set_flip_all(screen->machine(), state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (state->m_flipscreen)
		offs = 1;
	else
		offs = -1;

	if (state->m_flipscreen)
		offs2 = -3;
	else
		offs2 = -5;

	tilemap_set_scrollx(state->m_pf1_tilemap,     0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_tilemap,     0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf1_alt_tilemap, 0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_alt_tilemap, 0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf2_tilemap,     0, state->m_control_0[3] + offs);
	tilemap_set_scrolly(state->m_pf2_tilemap,     0, state->m_control_0[4]);

	tilemap_draw(bitmap, cliprect, state->m_pf2_tilemap, 0, 0);

	if (state->m_control_0[6] & 0x80)
		tilemap_draw(bitmap, cliprect, state->m_pf1_tilemap, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->m_pf1_alt_tilemap, 0, 0);

	tumblepb_draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE( jumpkids )
{
	tumbleb_state *state = screen->machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	tilemap_set_flip_all(screen->machine(), state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (state->m_flipscreen)
		offs = 1;
	else
		offs = -1;

	if (state->m_flipscreen)
		offs2 = -3;
	else
		offs2 = -5;

	tilemap_set_scrollx(state->m_pf1_tilemap,     0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_tilemap,     0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf1_alt_tilemap, 0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_alt_tilemap, 0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf2_tilemap,     0, state->m_control_0[3] + offs);
	tilemap_set_scrolly(state->m_pf2_tilemap,     0, state->m_control_0[4]);

	tilemap_draw(bitmap, cliprect, state->m_pf2_tilemap, 0, 0);

	if (state->m_control_0[6] & 0x80)
		tilemap_draw(bitmap, cliprect, state->m_pf1_tilemap, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->m_pf1_alt_tilemap, 0, 0);

	jumpkids_draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE( semicom )
{
	tumbleb_state *state = screen->machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	tilemap_set_flip_all(screen->machine(), state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (state->m_flipscreen)
		offs = 1;
	else
		offs = -1;

	if (state->m_flipscreen)
		offs2 = -3;
	else
		offs2 = -5;

	tilemap_set_scrollx(state->m_pf1_tilemap,     0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_tilemap,     0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf1_alt_tilemap, 0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_alt_tilemap, 0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf2_tilemap,     0, state->m_control_0[3] + offs);
	tilemap_set_scrolly(state->m_pf2_tilemap,     0, state->m_control_0[4]);

	tilemap_draw(bitmap, cliprect, state->m_pf2_tilemap, 0, 0);

	if (state->m_control_0[6] & 0x80)
		tilemap_draw(bitmap, cliprect, state->m_pf1_tilemap, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->m_pf1_alt_tilemap, 0, 0);

	jumpkids_draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE( semicom_altoffsets )
{
	tumbleb_state *state = screen->machine().driver_data<tumbleb_state>();
	int offsx, offsy, offsx2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;

	offsx = -1;
	offsy = 2;
	offsx2 = -5;

	tilemap_set_scrollx(state->m_pf1_tilemap,     0, state->m_control_0[1] + offsx2);
	tilemap_set_scrolly(state->m_pf1_tilemap,     0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf1_alt_tilemap, 0, state->m_control_0[1] + offsx2);
	tilemap_set_scrolly(state->m_pf1_alt_tilemap, 0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf2_tilemap,     0, state->m_control_0[3] + offsx);
	tilemap_set_scrolly(state->m_pf2_tilemap,     0, state->m_control_0[4] + offsy);

	tilemap_draw(bitmap, cliprect, state->m_pf2_tilemap, 0, 0);

	if (state->m_control_0[6] & 0x80)
		tilemap_draw(bitmap, cliprect, state->m_pf1_tilemap, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->m_pf1_alt_tilemap, 0, 0);

	jumpkids_draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE( bcstory )
{
	tumbleb_state *state = screen->machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	tilemap_set_flip_all(screen->machine(), state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* not sure of this */
	if (state->m_flipscreen)
		offs = 1;
	else
		offs = 8;

	/* not sure of this */
	if (state->m_flipscreen)
		offs2 = -3;
	else
		offs2 = 8;

	tilemap_set_scrollx(state->m_pf1_tilemap,     0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_tilemap,     0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf1_alt_tilemap, 0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_alt_tilemap, 0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf2_tilemap,     0, state->m_control_0[3] + offs);
	tilemap_set_scrolly(state->m_pf2_tilemap,     0, state->m_control_0[4]);

	tilemap_draw(bitmap, cliprect, state->m_pf2_tilemap, 0, 0);

	if (state->m_control_0[6] & 0x80)
		tilemap_draw(bitmap, cliprect, state->m_pf1_tilemap, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->m_pf1_alt_tilemap, 0, 0);

	jumpkids_draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE( semibase )
{
	tumbleb_state *state = screen->machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	tilemap_set_flip_all(screen->machine(), state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	offs = -1;
	offs2 = -2;

	/* sprites need an offset too */
	tilemap_set_scrollx(state->m_pf1_tilemap,     0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_tilemap,     0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf1_alt_tilemap, 0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_alt_tilemap, 0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf2_tilemap,     0, state->m_control_0[3] + offs);
	tilemap_set_scrolly(state->m_pf2_tilemap,     0, state->m_control_0[4]);

	tilemap_draw(bitmap, cliprect, state->m_pf2_tilemap, 0, 0);

	if (state->m_control_0[6] & 0x80)
		tilemap_draw(bitmap, cliprect, state->m_pf1_tilemap, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->m_pf1_alt_tilemap, 0, 0);

	jumpkids_draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE( sdfight )
{
	tumbleb_state *state = screen->machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	tilemap_set_flip_all(screen->machine(), state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	offs = -1;
	offs2 = -5; // foreground scroll..

	/* sprites need an offset too */
	tilemap_set_scrollx(state->m_pf1_tilemap,     0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_tilemap,     0, state->m_control_0[2] - 16); // needed for the ground ...
	tilemap_set_scrollx(state->m_pf1_alt_tilemap, 0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_alt_tilemap, 0, state->m_control_0[2] - 16);
	tilemap_set_scrollx(state->m_pf2_tilemap,     0, state->m_control_0[3] + offs);
	tilemap_set_scrolly(state->m_pf2_tilemap,     0, state->m_control_0[4]);

	tilemap_draw(bitmap, cliprect, state->m_pf2_tilemap, 0, 0);
	if (state->m_control_0[6] & 0x80)
		tilemap_draw(bitmap, cliprect, state->m_pf1_tilemap, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->m_pf1_alt_tilemap, 0, 0);

	jumpkids_draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}



SCREEN_UPDATE( fncywld )
{
	tumbleb_state *state = screen->machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	tilemap_set_flip_all(screen->machine(), state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (state->m_flipscreen)
		offs = 1;
	else
		offs = -1;

	if (state->m_flipscreen)
		offs2 = -3;
	else
		offs2 = -5;

	tilemap_set_scrollx(state->m_pf1_tilemap,     0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_tilemap,     0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf1_alt_tilemap, 0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_alt_tilemap, 0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf2_tilemap,     0, state->m_control_0[3] + offs);
	tilemap_set_scrolly(state->m_pf2_tilemap,     0, state->m_control_0[4]);

	tilemap_draw(bitmap, cliprect, state->m_pf2_tilemap, 0, 0);

	if (state->m_control_0[6] & 0x80)
		tilemap_draw(bitmap, cliprect, state->m_pf1_tilemap, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->m_pf1_alt_tilemap, 0, 0);

	fncywld_draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}


SCREEN_UPDATE( jumppop )
{
	tumbleb_state *state = screen->machine().driver_data<tumbleb_state>();

	//  bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine()));

	tilemap_set_scrollx(state->m_pf1_tilemap, 0, state->m_control[2] - 0x3a0);
	tilemap_set_scrolly(state->m_pf1_tilemap, 0, state->m_control[3]);
	tilemap_set_scrollx(state->m_pf1_alt_tilemap, 0, state->m_control[2] - 0x3a0);
	tilemap_set_scrolly(state->m_pf1_alt_tilemap, 0, state->m_control[3]);
	tilemap_set_scrollx(state->m_pf2_tilemap, 0, state->m_control[0] - 0x3a2);
	tilemap_set_scrolly(state->m_pf2_tilemap, 0, state->m_control[1]);
	tilemap_set_scrollx(state->m_pf2_alt_tilemap, 0, state->m_control[0] - 0x3a2);
	tilemap_set_scrolly(state->m_pf2_alt_tilemap, 0, state->m_control[1]);

	if (state->m_control[7] & 1)
		tilemap_draw(bitmap, cliprect, state->m_pf2_tilemap, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->m_pf2_alt_tilemap, 0, 0);

	if (state->m_control[7] & 2)
		tilemap_draw(bitmap, cliprect, state->m_pf1_alt_tilemap, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->m_pf1_tilemap, 0, 0);

//popmessage("%04x %04x %04x %04x %04x %04x %04x %04x", state->m_control[0],state->m_control[1],state->m_control[2],state->m_control[3],state->m_control[4],state->m_control[5],state->m_control[6],state->m_control[7]);

	jumpkids_draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}


SCREEN_UPDATE( suprtrio )
{
	tumbleb_state *state = screen->machine().driver_data<tumbleb_state>();

	tilemap_set_scrollx(state->m_pf1_alt_tilemap, 0, -state->m_control[1] - 6);
	tilemap_set_scrolly(state->m_pf1_alt_tilemap, 0, -state->m_control[2]);
	tilemap_set_scrollx(state->m_pf2_tilemap, 0, -state->m_control[3] - 2);
	tilemap_set_scrolly(state->m_pf2_tilemap, 0, -state->m_control[4]);

	tilemap_draw(bitmap, cliprect, state->m_pf2_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->m_pf1_alt_tilemap, 0, 0);

	jumpkids_draw_sprites(screen->machine(), bitmap, cliprect);
#if 0
popmessage("%04x %04x %04x %04x %04x %04x %04x %04x",
 state->m_control[0],
 state->m_control[1],
 state->m_control[2],
 state->m_control[3],
 state->m_control[4],
 state->m_control[5],
 state->m_control[6],
 state->m_control[7]);
#endif

	return 0;
}

SCREEN_UPDATE( pangpang )
{
	tumbleb_state *state = screen->machine().driver_data<tumbleb_state>();
	int offs, offs2;

	state->m_flipscreen = state->m_control_0[0] & 0x80;
	tilemap_set_flip_all(screen->machine(), state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (state->m_flipscreen)
		offs = 1;
	else
		offs = -1;

	if (state->m_flipscreen)
		offs2 = -3;
	else
		offs2 = -5;

	tilemap_set_scrollx(state->m_pf1_tilemap,     0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_tilemap,     0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf1_alt_tilemap, 0, state->m_control_0[1] + offs2);
	tilemap_set_scrolly(state->m_pf1_alt_tilemap, 0, state->m_control_0[2]);
	tilemap_set_scrollx(state->m_pf2_tilemap,     0, state->m_control_0[3] + offs);
	tilemap_set_scrolly(state->m_pf2_tilemap,     0, state->m_control_0[4]);

	tilemap_draw(bitmap, cliprect, state->m_pf2_tilemap, 0, 0);

	if (state->m_control_0[6] & 0x80)
		tilemap_draw(bitmap, cliprect, state->m_pf1_tilemap, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->m_pf1_alt_tilemap, 0, 0);

	jumpkids_draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}
