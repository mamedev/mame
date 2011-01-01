#include "emu.h"
#include "video/konicdev.h"
#include "includes/tmnt.h"

static TILE_GET_INFO( glfgreat_get_roz_tile_info )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	UINT8 *rom = machine->region("user1")->base();
	int code;

	tile_index += 0x40000 * state->glfgreat_roz_rom_bank;

	code = rom[tile_index + 0x80000] + 256 * rom[tile_index] + 256 * 256 * ((rom[tile_index / 4 + 0x100000] >> (2 * (tile_index & 3))) & 3);

	SET_TILE_INFO(0, code & 0x3fff, code >> 14, 0);
}

static TILE_GET_INFO( prmrsocr_get_roz_tile_info )
{
	UINT8 *rom = machine->region("user1")->base();
	int code = rom[tile_index + 0x20000] + 256 * rom[tile_index];

	SET_TILE_INFO(0, code & 0x1fff, code >> 13, 0);
}



/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

/* Missing in Action */

void mia_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	*flags = (*color & 0x04) ? TILE_FLIPX : 0;
	if (layer == 0)
	{
		*code |= ((*color & 0x01) << 8);
		*color = state->layer_colorbase[layer] + ((*color & 0x80) >> 5) + ((*color & 0x10) >> 1);
	}
	else
	{
		*code |= ((*color & 0x01) << 8) | ((*color & 0x18) << 6) | (bank << 11);
		*color = state->layer_colorbase[layer] + ((*color & 0xe0) >> 5);
	}
}

void cuebrick_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();

	if ((k052109_get_rmrd_line(state->k052109) == CLEAR_LINE) && (layer == 0))
	{
		*code |= ((*color & 0x01) << 8);
		*color = state->layer_colorbase[layer]  + ((*color & 0x80) >> 5) + ((*color & 0x10) >> 1);
	}
	else
	{
		*code |= ((*color & 0xf) << 8);
		*color = state->layer_colorbase[layer] + ((*color & 0xe0) >> 5);
	}
}

void tmnt_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = state->layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

void ssbl_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	if (layer == 0)
	{
		*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	}
	else
	{
		*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
//      mame_printf_debug("L%d: bank %d code %x color %x\n", layer, bank, *code, *color);
	}

	*color = state->layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

void blswhstl_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();

	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x01) << 8) | ((*color & 0x10) << 5) | ((*color & 0x0c) << 8) | (bank << 12) | state->blswhstl_rombank << 14;
	*color = state->layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}



/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void mia_sprite_callback( running_machine *machine, int *code, int *color, int *priority, int *shadow )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	*color = state->sprite_colorbase + (*color & 0x0f);
}

void tmnt_sprite_callback( running_machine *machine, int *code, int *color, int *priority, int *shadow )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	*code |= (*color & 0x10) << 9;
	*color = state->sprite_colorbase + (*color & 0x0f);
}

void punkshot_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask, int *shadow )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= state->layerpri[2])
		*priority_mask = 0;
	else if (pri > state->layerpri[2] && pri <= state->layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > state->layerpri[1] && pri <= state->layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*code |= (*color & 0x10) << 9;
	*color = state->sprite_colorbase + (*color & 0x0f);
}

void thndrx2_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask, int *shadow )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= state->layerpri[2])
		*priority_mask = 0;
	else if (pri > state->layerpri[2] && pri <= state->layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > state->layerpri[1] && pri <= state->layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*color = state->sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K053245

***************************************************************************/

void lgtnfght_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= state->layerpri[2])
		*priority_mask = 0;
	else if (pri > state->layerpri[2] && pri <= state->layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > state->layerpri[1] && pri <= state->layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*color = state->sprite_colorbase + (*color & 0x1f);
}

void blswhstl_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
#if 0
if (input_code_pressed(machine, KEYCODE_Q) && (*color & 0x20)) *color = rand();
if (input_code_pressed(machine, KEYCODE_W) && (*color & 0x40)) *color = rand();
if (input_code_pressed(machine, KEYCODE_E) && (*color & 0x80)) *color = rand();
#endif
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= state->layerpri[2])
		*priority_mask = 0;
	else if (pri > state->layerpri[2] && pri <= state->layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > state->layerpri[1] && pri <= state->layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*color = state->sprite_colorbase + (*color & 0x1f);
}

void prmrsocr_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= state->layerpri[2])
		*priority_mask = 0;
	else if (pri > state->layerpri[2] && pri <= state->layerpri[1])
		*priority_mask = 0xf0;
	else if (pri > state->layerpri[1] && pri <= state->layerpri[0])
		*priority_mask = 0xf0 | 0xcc;
	else
		*priority_mask = 0xf0 | 0xcc | 0xaa;

	*code |= state->prmrsocr_sprite_bank << 14;

	*color = state->sprite_colorbase + (*color & 0x1f);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( cuebrick )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	state->layer_colorbase[0] = 0;
	state->layer_colorbase[1] = 32;
	state->layer_colorbase[2] = 40;
	state->sprite_colorbase = 16;
}

VIDEO_START( mia )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	state->layer_colorbase[0] = 0;
	state->layer_colorbase[1] = 32;
	state->layer_colorbase[2] = 40;
	state->sprite_colorbase = 16;

	state->tmnt_priorityflag = 0;
	state_save_register_global(machine, state->tmnt_priorityflag);
}

VIDEO_START( tmnt )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	state->layer_colorbase[0] = 0;
	state->layer_colorbase[1] = 32;
	state->layer_colorbase[2] = 40;
	state->sprite_colorbase = 16;

	state->tmnt_priorityflag = 0;
	state_save_register_global(machine, state->tmnt_priorityflag);

	palette_set_shadow_factor(machine,0.75);
}

VIDEO_START( lgtnfght )	/* also tmnt2, ssriders */
{
	tmnt_state *state = machine->driver_data<tmnt_state>();

	k05324x_set_z_rejection(state->k053245, 0);

	state->dim_c = state->dim_v = state->lastdim = state->lasten = 0;

	state_save_register_global(machine, state->dim_c);
	state_save_register_global(machine, state->dim_v);
	state_save_register_global(machine, state->lastdim);
	state_save_register_global(machine, state->lasten);
}

VIDEO_START( glfgreat )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();

	state->roz_tilemap = tilemap_create(machine, glfgreat_get_roz_tile_info, tilemap_scan_rows, 16, 16, 512, 512);
	tilemap_set_transparent_pen(state->roz_tilemap,0);

	state->glfgreat_roz_rom_bank = 0;
	state->glfgreat_roz_char_bank = 0;
	state->glfgreat_roz_rom_mode = 0;
	state_save_register_global(machine, state->glfgreat_roz_rom_bank);
	state_save_register_global(machine, state->glfgreat_roz_char_bank);
	state_save_register_global(machine, state->glfgreat_roz_rom_mode);
}

VIDEO_START( prmrsocr )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();

	state->roz_tilemap = tilemap_create(machine, prmrsocr_get_roz_tile_info, tilemap_scan_rows, 16, 16, 512, 256);
	tilemap_set_transparent_pen(state->roz_tilemap,0);

	state->prmrsocr_sprite_bank = 0;
	state->glfgreat_roz_char_bank = 0;
	state_save_register_global(machine, state->prmrsocr_sprite_bank);
	state_save_register_global(machine, state->glfgreat_roz_char_bank);
}

VIDEO_START( blswhstl )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();

	state->blswhstl_rombank = -1;
	state_save_register_global(machine, state->blswhstl_rombank);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( tmnt_paletteram_word_w )
{
	COMBINE_DATA(space->machine->generic.paletteram.u16 + offset);
	offset &= ~1;

	data = (space->machine->generic.paletteram.u16[offset] << 8) | space->machine->generic.paletteram.u16[offset + 1];
	palette_set_color_rgb(space->machine, offset / 2, pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));
}



WRITE16_HANDLER( tmnt_0a0000_w )
{
	tmnt_state *state = space->machine->driver_data<tmnt_state>();

	if (ACCESSING_BITS_0_7)
	{
		/* bit 0/1 = coin counters */
		coin_counter_w(space->machine, 0, data & 0x01);
		coin_counter_w(space->machine, 1, data & 0x02);	/* 2 players version */

		/* bit 3 high then low triggers irq on sound CPU */
		if (state->last == 0x08 && (data & 0x08) == 0)
			cpu_set_input_line_and_vector(state->audiocpu, 0, HOLD_LINE, 0xff);

		state->last = data & 0x08;

		/* bit 5 = irq enable */
		interrupt_enable_w(space, 0, data & 0x20);

		/* bit 7 = enable char ROM reading through the video RAM */
		k052109_set_rmrd_line(state->k052109, (data & 0x80) ? ASSERT_LINE : CLEAR_LINE);

		/* other bits unused */
	}
}

WRITE16_HANDLER( punkshot_0a0020_w )
{
	tmnt_state *state = space->machine->driver_data<tmnt_state>();

	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 = coin counter */
		coin_counter_w(space->machine, 0, data & 0x01);

		/* bit 2 = trigger irq on sound CPU */
		if (state->last == 0x04 && (data & 0x04) == 0)
			cpu_set_input_line_and_vector(state->audiocpu, 0, HOLD_LINE, 0xff);

		state->last = data & 0x04;

		/* bit 3 = enable char ROM reading through the video RAM */
		k052109_set_rmrd_line(state->k052109, (data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	}
}

WRITE16_HANDLER( lgtnfght_0a0018_w )
{
	tmnt_state *state = space->machine->driver_data<tmnt_state>();

	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(space->machine, 0, data & 0x01);
		coin_counter_w(space->machine, 1, data & 0x02);

		/* bit 2 = trigger irq on sound CPU */
		if (state->last == 0x00 && (data & 0x04) == 0x04)
			cpu_set_input_line_and_vector(state->audiocpu, 0, HOLD_LINE, 0xff);

		state->last = data & 0x04;

		/* bit 3 = enable char ROM reading through the video RAM */
		k052109_set_rmrd_line(state->k052109, (data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	}
}

WRITE16_HANDLER( blswhstl_700300_w )
{
	tmnt_state *state = space->machine->driver_data<tmnt_state>();

	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(space->machine, 0,data & 0x01);
		coin_counter_w(space->machine, 1,data & 0x02);

		/* bit 3 = enable char ROM reading through the video RAM */
		k052109_set_rmrd_line(state->k052109, (data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 7 = select char ROM bank */
		if (state->blswhstl_rombank != ((data & 0x80) >> 7))
		{
			state->blswhstl_rombank = (data & 0x80) >> 7;
			tilemap_mark_all_tiles_dirty_all(space->machine);
		}

		/* other bits unknown */
	}
}


READ16_HANDLER( glfgreat_rom_r )
{
	tmnt_state *state = space->machine->driver_data<tmnt_state>();

	if (state->glfgreat_roz_rom_mode)
		return space->machine->region("gfx3")->base()[state->glfgreat_roz_char_bank * 0x80000 + offset];
	else if (offset < 0x40000)
	{
		UINT8 *usr = space->machine->region("user1")->base();
		return usr[offset + 0x80000 + state->glfgreat_roz_rom_bank * 0x40000] + 256 * usr[offset + state->glfgreat_roz_rom_bank * 0x40000];
	}
	else
		return space->machine->region("user1")->base()[((offset & 0x3ffff) >> 2) + 0x100000 + state->glfgreat_roz_rom_bank * 0x10000];
}

WRITE16_HANDLER( glfgreat_122000_w )
{
	tmnt_state *state = space->machine->driver_data<tmnt_state>();

	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(space->machine, 0, data & 0x01);
		coin_counter_w(space->machine, 1, data & 0x02);

		/* bit 4 = enable char ROM reading through the video RAM */
		k052109_set_rmrd_line(state->k052109, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 5 = 53596 tile rom bank selection */
		if (state->glfgreat_roz_rom_bank != (data & 0x20) >> 5)
		{
			state->glfgreat_roz_rom_bank = (data & 0x20) >> 5;
			tilemap_mark_all_tiles_dirty(state->roz_tilemap);
		}

		/* bit 6,7 = 53596 char bank selection for ROM test */
		state->glfgreat_roz_char_bank = (data & 0xc0) >> 6;

		/* other bits unknown */
	}
	if (ACCESSING_BITS_8_15)
	{
		/* bit 8 = 53596 char/rom selection for ROM test */
		state->glfgreat_roz_rom_mode = data & 0x100;
	}
}


WRITE16_HANDLER( ssriders_eeprom_w )
{
	tmnt_state *state = space->machine->driver_data<tmnt_state>();

	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 is data */
		/* bit 1 is cs (active low) */
		/* bit 2 is clock (active high) */
		input_port_write(space->machine, "EEPROMOUT", data, 0xff);

		/* bits 3-4 control palette dimming */
		/* 4 = DIMPOL = when set, negate SHAD */
		/* 3 = DIMMOD = when set, or BRIT with [negated] SHAD */
		state->dim_c = data & 0x18;

		/* bit 5 selects sprite ROM for testing in TMNT2 (bits 5-7, actually, according to the schematics) */
		k053244_bankselect(state->k053245, ((data & 0x20) >> 5) << 2);
	}
}

WRITE16_HANDLER( ssriders_1c0300_w )
{
	tmnt_state *state = space->machine->driver_data<tmnt_state>();

	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(space->machine, 0, data & 0x01);
		coin_counter_w(space->machine, 1, data & 0x02);

		/* bit 3 = enable char ROM reading through the video RAM */
		k052109_set_rmrd_line(state->k052109, (data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

		/* bits 4-6 control palette dimming (DIM0-DIM2) */
		state->dim_v = (data & 0x70) >> 4;
	}
}

WRITE16_HANDLER( prmrsocr_122000_w )
{
	tmnt_state *state = space->machine->driver_data<tmnt_state>();

	if (ACCESSING_BITS_0_7)
	{
		/* bit 0,1 = coin counter */
		coin_counter_w(space->machine, 0, data & 0x01);
		coin_counter_w(space->machine, 1, data & 0x02);

		/* bit 4 = enable char ROM reading through the video RAM */
		k052109_set_rmrd_line(state->k052109, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 6 = sprite ROM bank */
		state->prmrsocr_sprite_bank = (data & 0x40) >> 6;
		k053244_bankselect(state->k053245, state->prmrsocr_sprite_bank << 2);

		/* bit 7 = 53596 region selector for ROM test */
		state->glfgreat_roz_char_bank = (data & 0x80) >> 7;

		/* other bits unknown (unused?) */
	}
}

READ16_HANDLER( prmrsocr_rom_r )
{
	tmnt_state *state = space->machine->driver_data<tmnt_state>();

	if(state->glfgreat_roz_char_bank)
		return space->machine->region("gfx3")->base()[offset];
	else
	{
		UINT8 *usr = space->machine->region("user1")->base();
		return 256 * usr[offset] + usr[offset + 0x020000];
	}
}

WRITE16_HANDLER( tmnt_priority_w )
{
	tmnt_state *state = space->machine->driver_data<tmnt_state>();

	if (ACCESSING_BITS_0_7)
	{
		/* bit 2/3 = priority; other bits unused */
		/* bit2 = PRI bit3 = PRI2
              sprite/playfield priority is controlled by these two bits, by bit 3
              of the background tile color code, and by the SHADOW sprite
              attribute bit.
              Priorities are encoded in a PROM (G19 for TMNT). However, in TMNT,
              the PROM only takes into account the PRI and SHADOW bits.
              PRI  Priority
               0   bg fg spr text
               1   bg spr fg text
              The SHADOW bit, when set, torns a sprite into a shadow which makes
              color below it darker (this is done by turning off three resistors
              in parallel with the RGB output).

              Note: the background color (color used when all of the four layers
              are 0) is taken from the *foreground* palette, not the background
              one as would be more intuitive.
        */
		state->tmnt_priorityflag = (data & 0x0c) >> 2;
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( mia )
{
	tmnt_state *state = screen->machine->driver_data<tmnt_state>();

	k052109_tilemap_update(state->k052109);

	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE,0);
	if ((state->tmnt_priorityflag & 1) == 1) k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, 0);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, 0, 0);
	if ((state->tmnt_priorityflag & 1) == 0) k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, 0);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 0, 0, 0);

	return 0;
}

VIDEO_UPDATE( tmnt )
{
	tmnt_state *state = screen->machine->driver_data<tmnt_state>();

	k052109_tilemap_update(state->k052109);

	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE,0);
	if ((state->tmnt_priorityflag & 1) == 1) k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, 0);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, 0, 0);
	if ((state->tmnt_priorityflag & 1) == 0) k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, 0);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 0, 0, 0);

	return 0;
}


VIDEO_UPDATE( punkshot )
{
	tmnt_state *state = screen->machine->driver_data<tmnt_state>();

	state->sprite_colorbase = k053251_get_palette_index(state->k053251, K053251_CI1);
	state->layer_colorbase[0] = k053251_get_palette_index(state->k053251, K053251_CI2);
	state->layer_colorbase[1] = k053251_get_palette_index(state->k053251, K053251_CI4);
	state->layer_colorbase[2] = k053251_get_palette_index(state->k053251, K053251_CI3);

	k052109_tilemap_update(state->k052109);

	state->sorted_layer[0] = 0;
	state->layerpri[0] = k053251_get_priority(state->k053251, K053251_CI2);
	state->sorted_layer[1] = 1;
	state->layerpri[1] = k053251_get_priority(state->k053251, K053251_CI4);
	state->sorted_layer[2] = 2;
	state->layerpri[2] = k053251_get_priority(state->k053251, K053251_CI3);

	konami_sortlayers3(state->sorted_layer, state->layerpri);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, state->sorted_layer[0], TILEMAP_DRAW_OPAQUE, 1);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, state->sorted_layer[1], 0, 2);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, state->sorted_layer[2], 0, 4);

	k051960_sprites_draw(state->k051960, bitmap, cliprect, -1, -1);
	return 0;
}


VIDEO_UPDATE( lgtnfght )
{
	tmnt_state *state = screen->machine->driver_data<tmnt_state>();
	int bg_colorbase;

	bg_colorbase = k053251_get_palette_index(state->k053251, K053251_CI0);
	state->sprite_colorbase = k053251_get_palette_index(state->k053251, K053251_CI1);
	state->layer_colorbase[0] = k053251_get_palette_index(state->k053251, K053251_CI2);
	state->layer_colorbase[1] = k053251_get_palette_index(state->k053251, K053251_CI4);
	state->layer_colorbase[2] = k053251_get_palette_index(state->k053251, K053251_CI3);

	k052109_tilemap_update(state->k052109);

	state->sorted_layer[0] = 0;
	state->layerpri[0] = k053251_get_priority(state->k053251, K053251_CI2);
	state->sorted_layer[1] = 1;
	state->layerpri[1] = k053251_get_priority(state->k053251, K053251_CI4);
	state->sorted_layer[2] = 2;
	state->layerpri[2] = k053251_get_priority(state->k053251, K053251_CI3);

	konami_sortlayers3(state->sorted_layer, state->layerpri);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 16 * bg_colorbase);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, state->sorted_layer[0], 0, 1);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, state->sorted_layer[1], 0, 2);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, state->sorted_layer[2], 0, 4);

	k053245_sprites_draw(state->k053245, bitmap, cliprect);
	return 0;
}


READ16_HANDLER( glfgreat_ball_r )
{
	tmnt_state *state = space->machine->driver_data<tmnt_state>();

#ifdef MAME_DEBUG
popmessage("%04x", state->glfgreat_pixel);
#endif
	/* if out of the ROZ layer palette range, it's in the water - return 0 */
	if (state->glfgreat_pixel < 0x400 || state->glfgreat_pixel >= 0x500)
		return 0;
	else
		return state->glfgreat_pixel & 0xff;
}

VIDEO_UPDATE( glfgreat )
{
	tmnt_state *state = screen->machine->driver_data<tmnt_state>();
	int bg_colorbase;

	bg_colorbase = k053251_get_palette_index(state->k053251, K053251_CI0);
	state->sprite_colorbase  = k053251_get_palette_index(state->k053251, K053251_CI1);
	state->layer_colorbase[0] = k053251_get_palette_index(state->k053251, K053251_CI2);
	state->layer_colorbase[1] = k053251_get_palette_index(state->k053251, K053251_CI3) + 8;	/* weird... */
	state->layer_colorbase[2] = k053251_get_palette_index(state->k053251, K053251_CI4);

	k052109_tilemap_update(state->k052109);

	state->sorted_layer[0] = 0;
	state->layerpri[0] = k053251_get_priority(state->k053251, K053251_CI2);
	state->sorted_layer[1] = 1;
	state->layerpri[1] = k053251_get_priority(state->k053251, K053251_CI3);
	state->sorted_layer[2] = 2;
	state->layerpri[2] = k053251_get_priority(state->k053251, K053251_CI4);

	konami_sortlayers3(state->sorted_layer, state->layerpri);

	/* not sure about the 053936 priority, but it seems to work */

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect,16 * bg_colorbase);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, state->sorted_layer[0], 0, 1);

	if (state->layerpri[0] >= 0x30 && state->layerpri[1] < 0x30)
	{
		k053936_zoom_draw(state->k053936, bitmap, cliprect, state->roz_tilemap, 0, 1, 1);
		state->glfgreat_pixel = *BITMAP_ADDR16(bitmap, 0x80, 0x105);
	}

	k052109_tilemap_draw(state->k052109, bitmap, cliprect, state->sorted_layer[1], 0, 2);

	if (state->layerpri[1] >= 0x30 && state->layerpri[2] < 0x30)
	{
		k053936_zoom_draw(state->k053936, bitmap, cliprect, state->roz_tilemap, 0, 1, 1);
		state->glfgreat_pixel = *BITMAP_ADDR16(bitmap, 0x80, 0x105);
	}

	k052109_tilemap_draw(state->k052109, bitmap, cliprect, state->sorted_layer[2], 0, 4);

	if (state->layerpri[2] >= 0x30)
	{
		k053936_zoom_draw(state->k053936, bitmap, cliprect, state->roz_tilemap, 0, 1, 1);
		state->glfgreat_pixel = *BITMAP_ADDR16(bitmap, 0x80, 0x105);
	}

	k053245_sprites_draw(state->k053245, bitmap, cliprect);
	return 0;
}

VIDEO_UPDATE( tmnt2 )
{
	tmnt_state *state = screen->machine->driver_data<tmnt_state>();
	double brt;
	int i, newdim, newen, cb, ce;

	newdim = state->dim_v | ((~state->dim_c & 0x10) >> 1);
	newen  = (k053251_get_priority(state->k053251, 5) && k053251_get_priority(state->k053251, 5) != 0x3e);

	if (newdim != state->lastdim || newen != state->lasten)
	{
		brt = 1.0;
		if (newen)
			brt -= (1.0 - PALETTE_DEFAULT_SHADOW_FACTOR) * newdim / 8;
		state->lastdim = newdim;
		state->lasten = newen;

		/*
            Only affect the background and sprites, not text layer.
            Instead of dimming each layer we dim the entire palette
            except text colors because palette bases may change
            anytime and there's no guarantee a dimmed color will be
            reset properly.
        */

		// find the text layer's palette range
		cb = state->layer_colorbase[state->sorted_layer[2]] << 4;
		ce = cb + 128;

		// dim all colors before it
		for (i = 0; i < cb; i++)
			palette_set_pen_contrast(screen->machine, i, brt);

		// reset all colors in range
		for (i = cb; i < ce; i++)
			palette_set_pen_contrast(screen->machine, i, 1.0);

		// dim all colors after it
		for (i = ce; i < 2048; i++)
			palette_set_pen_contrast(screen->machine, i, brt);

		// toggle shadow/highlight
		if (~state->dim_c & 0x10)
			palette_set_shadow_mode(screen->machine, 1);
		else
			palette_set_shadow_mode(screen->machine, 0);
	}

	VIDEO_UPDATE_CALL(lgtnfght);
	return 0;
}


VIDEO_UPDATE( thndrx2 )
{
	tmnt_state *state = screen->machine->driver_data<tmnt_state>();
	int bg_colorbase;

	bg_colorbase = k053251_get_palette_index(state->k053251, K053251_CI0);
	state->sprite_colorbase = k053251_get_palette_index(state->k053251, K053251_CI1);
	state->layer_colorbase[0] = k053251_get_palette_index(state->k053251, K053251_CI2);
	state->layer_colorbase[1] = k053251_get_palette_index(state->k053251, K053251_CI4);
	state->layer_colorbase[2] = k053251_get_palette_index(state->k053251, K053251_CI3);

	k052109_tilemap_update(state->k052109);

	state->sorted_layer[0] = 0;
	state->layerpri[0] = k053251_get_priority(state->k053251, K053251_CI2);
	state->sorted_layer[1] = 1;
	state->layerpri[1] = k053251_get_priority(state->k053251, K053251_CI4);
	state->sorted_layer[2] = 2;
	state->layerpri[2] = k053251_get_priority(state->k053251, K053251_CI3);

	konami_sortlayers3(state->sorted_layer, state->layerpri);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 16 * bg_colorbase);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, state->sorted_layer[0], 0, 1);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, state->sorted_layer[1], 0, 2);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, state->sorted_layer[2], 0, 4);

	k051960_sprites_draw(state->k051960, bitmap, cliprect, -1, -1);
	return 0;
}



/***************************************************************************

  Housekeeping

***************************************************************************/

VIDEO_EOF( blswhstl )
{
	tmnt_state *state = machine->driver_data<tmnt_state>();
	k053245_clear_buffer(state->k053245);
}
