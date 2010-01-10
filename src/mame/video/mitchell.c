/***************************************************************************

 Pang Video Hardware

***************************************************************************/

#include "emu.h"
#include "sound/okim6295.h"
#include "includes/mitchell.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	mitchell_state *state = (mitchell_state *)machine->driver_data;
	UINT8 attr = state->colorram[tile_index];
	int code = state->videoram[2 * tile_index] + (state->videoram[2 * tile_index + 1] << 8);
	SET_TILE_INFO(
			0,
			code,
			attr & 0x7f,
			(attr & 0x80) ? TILE_FLIPX : 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( pang )
{
	mitchell_state *state = (mitchell_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	tilemap_set_transparent_pen(state->bg_tilemap, 15);

	/* OBJ RAM */
	state->objram = auto_alloc_array_clear(machine, UINT8, state->videoram_size);

	/* Palette RAM */
	machine->generic.paletteram.u8 = auto_alloc_array_clear(machine, UINT8, 2 * machine->config->total_colors);

	state_save_register_global_pointer(machine, state->objram, state->videoram_size);
	state_save_register_global_pointer(machine, machine->generic.paletteram.u8, 2 * machine->config->total_colors);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

/***************************************************************************
  OBJ / CHAR RAM HANDLERS (BANK 0 = CHAR, BANK 1=OBJ)
***************************************************************************/

WRITE8_HANDLER( pang_video_bank_w )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;

	/* Bank handler (sets base pointers for video write) (doesn't apply to mgakuen) */
	state->video_bank = data;
}

WRITE8_HANDLER( mstworld_video_bank_w )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;

	/* Monsters World seems to freak out if more bits are used.. */
	state->video_bank = data & 1;
}


WRITE8_HANDLER( mgakuen_videoram_w )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

READ8_HANDLER( mgakuen_videoram_r )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;
	return state->videoram[offset];
}

WRITE8_HANDLER( mgakuen_objram_w )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;
	state->objram[offset] = data;
}

READ8_HANDLER( mgakuen_objram_r )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;
	return state->objram[offset];
}

WRITE8_HANDLER( pang_videoram_w )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;

	if (state->video_bank)
		mgakuen_objram_w(space, offset, data);
	else
		mgakuen_videoram_w(space, offset, data);
}

READ8_HANDLER( pang_videoram_r )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;

	if (state->video_bank)
		return mgakuen_objram_r(space, offset);
	else
		return mgakuen_videoram_r(space, offset);
}

/***************************************************************************
  COLOUR RAM
****************************************************************************/

WRITE8_HANDLER( pang_colorram_w )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

READ8_HANDLER( pang_colorram_r )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;
	return state->colorram[offset];
}

/***************************************************************************
  PALETTE HANDLERS (COLOURS: BANK 0 = 0x00-0x3f BANK 1=0x40-0xff)
****************************************************************************/

WRITE8_HANDLER( pang_gfxctrl_w )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;

logerror("PC %04x: pang_gfxctrl_w %02x\n",cpu_get_pc(space->cpu),data);
{
#if 0
	char baf[40];
	sprintf(baf,"%02x",data);
	popmessage(baf);
#endif
}

	/* bit 0 is unknown (used, maybe back color enable?) */

	/* bit 1 is coin counter */
	coin_counter_w(space->machine, 0, data & 2);

	/* bit 2 is flip screen */
	if (state->flipscreen != (data & 0x04))
	{
		state->flipscreen = data & 0x04;
		tilemap_set_flip_all(space->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	}

	/* bit 3 is unknown (used, e.g. marukin pulses it on the title screen) */

	/* bit 4 selects OKI M6295 bank */
	if (state->oki != NULL && sound_get_type(state->oki) == SOUND_OKIM6295)
		okim6295_set_bank_base(state->oki, (data & 0x10) ? 0x40000 : 0x00000);

	/* bit 5 is palette RAM bank selector (doesn't apply to mgakuen) */
	state->paletteram_bank = data & 0x20;

	/* bits 6 and 7 are unknown, used in several places. At first I thought */
	/* they were bg and sprites enable, but this screws up spang (screen flickers */
	/* every time you pop a bubble). However, not using them as enable bits screws */
	/* up marukin - you can see partially built up screens during attract mode. */
}

WRITE8_HANDLER( mstworld_gfxctrl_w )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;

logerror("PC %04x: pang_gfxctrl_w %02x\n",cpu_get_pc(space->cpu),data);
{
	char baf[40];
	sprintf(baf,"%02x",data);
//  popmessage(baf);
}

	/* bit 0 is unknown (used, maybe back color enable?) */

	/* bit 1 is coin counter */
	coin_counter_w(space->machine, 0, data & 2);

	/* bit 2 is flip screen */
	if (state->flipscreen != (data & 0x04))
	{
		state->flipscreen = data & 0x04;
		tilemap_set_flip_all(space->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	}

	/* bit 3 is unknown (used, e.g. marukin pulses it on the title screen) */

	/* bit 4 here does not select OKI M6295 bank: mstworld has its own z80 + sound banking */

	/* bit 5 is palette RAM bank selector (doesn't apply to mgakuen) */
	state->paletteram_bank = data & 0x20;

	/* bits 6 and 7 are unknown, used in several places. At first I thought */
	/* they were bg and sprites enable, but this screws up spang (screen flickers */
	/* every time you pop a bubble). However, not using them as enable bits screws */
	/* up marukin - you can see partially built up screens during attract mode. */
}

WRITE8_HANDLER( pang_paletteram_w )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;

	if (state->paletteram_bank)
		paletteram_xxxxRRRRGGGGBBBB_le_w(space, offset + 0x800, data);
	else
		paletteram_xxxxRRRRGGGGBBBB_le_w(space, offset, data);
}

READ8_HANDLER( pang_paletteram_r )
{
	mitchell_state *state = (mitchell_state *)space->machine->driver_data;

	if (state->paletteram_bank)
		return space->machine->generic.paletteram.u8[offset + 0x800];

	return space->machine->generic.paletteram.u8[offset];
}

WRITE8_HANDLER( mgakuen_paletteram_w )
{
	paletteram_xxxxRRRRGGGGBBBB_le_w(space, offset, data);
}

READ8_HANDLER( mgakuen_paletteram_r )
{
	return space->machine->generic.paletteram.u8[offset];
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	mitchell_state *state = (mitchell_state *)machine->driver_data;
	int offs, sx, sy;

	/* the last entry is not a sprite, we skip it otherwise spang shows a bubble */
	/* moving diagonally across the screen */
	for (offs = 0x1000 - 0x40; offs >= 0; offs -= 0x20)
	{
		int code = state->objram[offs];
		int attr = state->objram[offs + 1];
		int color = attr & 0x0f;
		sx = state->objram[offs + 3] + ((attr & 0x10) << 4);
		sy = ((state->objram[offs + 2] + 8) & 0xff) - 8;
		code += (attr & 0xe0) << 3;
		if (state->flipscreen)
		{
			sx = 496 - sx;
			sy = 240 - sy;
		}
		drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				 code,
				 color,
				 state->flipscreen, state->flipscreen,
				 sx,sy,15);
	}
}

VIDEO_UPDATE( pang )
{
	mitchell_state *state = (mitchell_state *)screen->machine->driver_data;

	bitmap_fill(bitmap, cliprect, 0);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
