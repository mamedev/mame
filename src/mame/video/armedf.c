#include "emu.h"
#include "includes/armedf.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( armedf_scan_type1 )
{	/* col: 0..63; row: 0..31 */
	/* armed formation */
	return col * 32 + row;
}

static TILEMAP_MAPPER( armedf_scan_type2 )
{	/* col: 0..63; row: 0..31 */
	return 32 * (31 - row) + (col & 0x1f) + 0x800 * (col / 32);
}

static TILEMAP_MAPPER( armedf_scan_type3 )
{	/* col: 0..63; row: 0..31 */
	/* legion & legiono */
	return (col & 0x1f) * 32 + row + 0x800 * (col / 32);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	armedf_state *state = machine->driver_data<armedf_state>();
	int tile_number = state->text_videoram[tile_index] & 0xff;
	int attributes;

	if (state->scroll_type == 1)
		attributes = state->text_videoram[tile_index + 0x800] & 0xff;
	else
		attributes = state->text_videoram[tile_index + 0x400] & 0xff;

	SET_TILE_INFO(
			0,
			tile_number + 256 * (attributes & 0x3),
			attributes >> 4,
			0);
}

static TILE_GET_INFO( get_legion_tx_tile_info )
{
	armedf_state *state = machine->driver_data<armedf_state>();

	int tile_number = state->text_videoram[tile_index] & 0xff;

	if(tile_index<0x10) tile_number=0x20;

	int attributes;

	if (state->scroll_type == 1)
		attributes = state->text_videoram[tile_index + 0x800] & 0xff;
	else
		attributes = state->text_videoram[tile_index + 0x400] & 0xff;


	tileinfo->category = 0;

	if((attributes & 0x3) == 3)
	{
		tileinfo->category = 1;
	}

	SET_TILE_INFO(
			0,
			tile_number + 256 * (attributes & 0x3),
			attributes >> 4,
			0);
}


static TILE_GET_INFO( get_fg_tile_info )
{
	armedf_state *state = machine->driver_data<armedf_state>();
	int data = state->fg_videoram[tile_index];
	SET_TILE_INFO(
			1,
			data&0x7ff,
			data>>11,
			0);
}


static TILE_GET_INFO( get_bg_tile_info )
{
	armedf_state *state = machine->driver_data<armedf_state>();
	int data = state->bg_videoram[tile_index];
	SET_TILE_INFO(
			2,
			data & 0x3ff,
			data >> 11,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( armedf )
{
	armedf_state *state = machine->driver_data<armedf_state>();

	if (state->scroll_type == 4 || /* cclimbr2 */
		state->scroll_type == 3 || /* legion */
		state->scroll_type == 6 )  /* legiono */
	{
		state->sprite_offy = 0;
	}
	else
		state->sprite_offy = 128;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 64, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols, 16, 16, 64, 32);

	switch (state->scroll_type)
	{
	case 1: /* armed formation */
		state->tx_tilemap = tilemap_create(machine, get_tx_tile_info, armedf_scan_type1, 8, 8, 64, 32);
		break;

	case 3: /* legion */
	case 6: /* legiono */
		state->tx_tilemap = tilemap_create(machine, get_legion_tx_tile_info, armedf_scan_type3, 8, 8, 64, 32);
		break;

	default:
		state->tx_tilemap = tilemap_create(machine, get_tx_tile_info, armedf_scan_type2, 8, 8, 64, 32);
		break;
	}

	tilemap_set_transparent_pen(state->bg_tilemap, 0xf);
	tilemap_set_transparent_pen(state->fg_tilemap, 0xf);
	tilemap_set_transparent_pen(state->tx_tilemap, 0xf);

	if (state->scroll_type != 1)
		tilemap_set_scrollx(state->tx_tilemap, 0, -128);
}

/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( armedf_text_videoram_w )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();
	COMBINE_DATA(&state->text_videoram[offset]);
	if (state->scroll_type == 1)
		tilemap_mark_tile_dirty(state->tx_tilemap, offset & 0x7ff);
	else
		tilemap_mark_tile_dirty(state->tx_tilemap, offset & 0xbff);
/*
  if (offset < 0x10)
        logerror("%04x %04x %04x %04x %04x %04x %04x %04x-%04x %04x %04x %04x %04x %04x %04x %04x (%04x)\n",
            state->text_videoram[0], state->text_videoram[1], state->text_videoram[2],
            state->text_videoram[3], state->text_videoram[4], state->text_videoram[5],
            state->text_videoram[6], state->text_videoram[7], state->text_videoram[8],
            state->text_videoram[9], state->text_videoram[10], state->text_videoram[11],
            state->text_videoram[12], state->text_videoram[13], state->text_videoram[14],
            state->text_videoram[15], offset);
*/
}

WRITE16_HANDLER( armedf_fg_videoram_w )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();
	COMBINE_DATA(&state->fg_videoram[offset]);
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE16_HANDLER( armedf_bg_videoram_w )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();
	COMBINE_DATA(&state->bg_videoram[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE16_HANDLER( terraf_fg_scrollx_w )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();
	if (ACCESSING_BITS_8_15)
	{
		state->fg_scrollx = data >> 8;
		state->waiting_msb = 1;
	}
}

WRITE16_HANDLER( terraf_fg_scrolly_w )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();
	if (ACCESSING_BITS_8_15)
	{
		if (state->waiting_msb)
			state->scroll_msb = data >> 8;
		else
			state->fg_scrolly = data >> 8;
	}
}

WRITE16_HANDLER( terraf_fg_scroll_msb_arm_w )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();
	if (ACCESSING_BITS_8_15)
		state->waiting_msb = 0;
}

WRITE16_HANDLER( armedf_fg_scrollx_w )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();
	COMBINE_DATA(&state->fg_scrollx);
}

WRITE16_HANDLER( armedf_fg_scrolly_w )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();
	COMBINE_DATA(&state->fg_scrolly);
}

WRITE16_HANDLER( armedf_bg_scrollx_w )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();
	COMBINE_DATA(&state->bg_scrollx);
	tilemap_set_scrollx(state->bg_tilemap, 0, state->bg_scrollx);
}

WRITE16_HANDLER( armedf_bg_scrolly_w )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();
	COMBINE_DATA(&state->bg_scrolly);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->bg_scrolly);
}

WRITE16_HANDLER( armedf_mcu_cmd )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();
	COMBINE_DATA(&state->mcu_mode);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	UINT16 *buffered_spriteram = machine->generic.buffered_spriteram.u16;
	armedf_state *state = machine->driver_data<armedf_state>();
	int offs;

	for (offs = 0; offs < machine->generic.spriteram_size / 2; offs += 4)
	{
		int code = buffered_spriteram[offs + 1]; /* ??YX?TTTTTTTTTTT */
		int flipx = code & 0x2000;
		int flipy = code & 0x1000;
		int color = (buffered_spriteram[offs + 2] >> 8) & 0x1f;
		int sx = buffered_spriteram[offs + 3];
		int sy = state->sprite_offy + 240 - (buffered_spriteram[offs + 0] & 0x1ff);

		if (flip_screen_get(machine))
		{
			sx = 320 - sx + 176;	/* don't ask where 176 comes from, just tried it out */
			sy = 240 - sy + 1;	/* don't ask where 1 comes from, just tried it out */
			flipx = !flipx;		/* the values seem to result in pixel-correct placement */
			flipy = !flipy;		/* in all the games supported by this driver */
		}

		if (((buffered_spriteram[offs + 0] & 0x3000) >> 12) == priority)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
				code & 0xfff,
				color,
				flipx,flipy,
				sx,sy,15);
		}
	}
}

static void copy_textmap(running_machine *machine, int index)
{
	/*
        (not simulated)
        1st half of the MCU ROM contains various strings and
        gfx elements (copied by MCU to textram)


        (partially simulated)
        2nd half of the MCu external ROM contains text tilemaps:

         4 - title screen
         5 - bottom layer gfx, visible  in later levels, during boss fight
         6 - test mode screen (not hooked up)
         7 - portraits (title)

    */

	armedf_state *state = machine->driver_data<armedf_state>();
	UINT8 * data = (UINT8 *)machine->region("gfx5")->base();

	for(int i=0;i<0x400;++i)
	{
		if(i<0x10) continue;

		int tile=data[0x800*index+i];
		int bank=data[0x800*index+i+0x400]&3;

		if( (tile|(bank<<8))!=0x20)
		{
			state->text_videoram[i]=tile;
			state->text_videoram[i+0x400]=data[0x800*index+i+0x400];
		}

	}

	tilemap_mark_all_tiles_dirty(state->tx_tilemap);

}

VIDEO_UPDATE( armedf )
{
	armedf_state *state = screen->machine->driver_data<armedf_state>();
	int sprite_enable = state->vreg & 0x200;

	tilemap_set_enable(state->bg_tilemap, state->vreg & 0x800);
	tilemap_set_enable(state->fg_tilemap, state->vreg & 0x400);
	tilemap_set_enable(state->tx_tilemap, state->vreg & 0x100);

	if ((state->scroll_type == 0)||(state->scroll_type == 5 ))
	{
		if (state->old_mcu_mode != state->mcu_mode)
		{
			if ((state->mcu_mode & 0x000f) == 0x0004)
			{	// transparent tx
				tilemap_set_transparent_pen(state->tx_tilemap, 0x0f);
				tilemap_mark_all_tiles_dirty(state->tx_tilemap);
				//logerror("? Transparent TX 0x0f\n");
			}
			if ((state->mcu_mode & 0x000f) == 0x000f)
			{		// opaque tx
				tilemap_set_transparent_pen(state->tx_tilemap, 0x10);
				tilemap_mark_all_tiles_dirty(state->tx_tilemap);
				//logerror("? Opaque TX\n");
			}

			state->old_mcu_mode = state->mcu_mode;
			//logerror("MCU Change => %04x\n", state->mcu_mode);
		}
	}

	switch (state->scroll_type)
	{
		case 0: /* terra force */
			tilemap_set_scrollx(state->fg_tilemap, 0, state->fg_scrolly + ((state->scroll_msb >> 4) & 3) * 256);
			tilemap_set_scrolly(state->fg_tilemap, 0, state->fg_scrollx + ((state->scroll_msb) & 3) * 256);
			break;

		case 1: /* armed formation */
			tilemap_set_scrollx(state->fg_tilemap, 0, state->fg_scrollx);
			tilemap_set_scrolly(state->fg_tilemap, 0, state->fg_scrolly);
			break;

		case 6: /* legiono */
			tilemap_set_scrollx(state->fg_tilemap, 0, (state->legion_cmd[13] & 0xff) | ((state->legion_cmd[14] & 0x3) << 8));
			tilemap_set_scrolly(state->fg_tilemap, 0, (state->legion_cmd[11] & 0xff) | ((state->legion_cmd[12] & 0x3) << 8));
			break;
		case 2: /* kodure ookami */
		case 3:
		case 4: /* crazy climber 2 */
			{
				int scrollx, scrolly;

				/* scrolling is handled by the protection mcu */
				scrollx = (state->text_videoram[13] & 0xff) | (state->text_videoram[14] << 8);
				scrolly = (state->text_videoram[11] & 0xff) | (state->text_videoram[12] << 8);
				tilemap_set_scrollx(state->fg_tilemap, 0, scrollx);
				tilemap_set_scrolly(state->fg_tilemap, 0, scrolly);

			}
			break;
		case 5: /* terra force (US) */
			tilemap_set_scrollx(state->fg_tilemap, 0, (state->text_videoram[13] & 0xff) | ((state->text_videoram[14] & 0x3) << 8));
			tilemap_set_scrolly(state->fg_tilemap, 0, (state->text_videoram[11] & 0xff) | ((state->text_videoram[12] & 0x3) << 8));
			break;

	}


	bitmap_fill(bitmap, cliprect , 0xff);

	if(state->scroll_type == 3 || state->scroll_type == 6) /* legion / legiono */
	{
		tilemap_draw(bitmap, cliprect, state->tx_tilemap, 1, 0);
	}

	if (state->vreg & 0x0800)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	/*
    if(state->vreg & 0x0800)
    {
        tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
    }
    else
    {
        bitmap_fill(bitmap, cliprect , get_black_pen(screen->machine) & 0x0f);
    }*/



	if ((state->mcu_mode & 0x0030) == 0x0030)
		tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);

	if (sprite_enable)
		draw_sprites(screen->machine, bitmap, cliprect, 2);

	if ((state->mcu_mode & 0x0030) == 0x0020)
		tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);

	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);

	if ((state->mcu_mode & 0x0030) == 0x0010)
		tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);

	if (sprite_enable)
		draw_sprites(screen->machine, bitmap, cliprect, 1);

	if ((state->mcu_mode & 0x0030) == 0x0000)
		tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);

	if (sprite_enable)
		draw_sprites(screen->machine, bitmap, cliprect, 0);

	if(state->scroll_type == 3) /* legion */
	{
		static int oldmode=-1;

		int mode=state->text_videoram[1]&0xff;

		if (mode != oldmode)
		{
			oldmode=mode;
			switch(mode)
			{
				case 0x01: copy_textmap(screen->machine, 4); break; /* title screen */
				case 0x06: copy_textmap(screen->machine, 7); break; /* portraits on title screen */
				case 0x1c: copy_textmap(screen->machine, 5); break; /* bottom, in-game layer */
				default: logerror("unknown mode %d\n", mode); break;
			}
		}

	}


	return 0;
}


VIDEO_EOF( armedf )
{
	address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram16_w(space, 0, 0, 0xffff);
}
