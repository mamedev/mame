/***************************************************************************
  Functions to emulate similar video hardware on these Taito games:

  - rastan
  - operation wolf
  - rainbow islands
  - jumping (bootleg)

***************************************************************************/

#include "emu.h"
#include "video/taitoic.h"

#include "includes/opwolf.h"
#include "includes/rastan.h"
#include "includes/rainbow.h"

/***************************************************************************/

VIDEO_START( jumping )
{
	rainbow_state *state = (rainbow_state *)machine->driver_data;

	pc080sn_set_trans_pen(state->pc080sn, 1, 15);

	state->sprite_ctrl = 0;
	state->sprites_flipscreen = 0;

	/* not 100% sure Jumping needs to save both... */
	state_save_register_global(machine, state->sprite_ctrl);
	state_save_register_global(machine, state->sprites_flipscreen);
}


WRITE16_HANDLER( rastan_spritectrl_w )
{
	rastan_state *state = (rastan_state *)space->machine->driver_data;

	/* bits 5-7 are the sprite palette bank */
	pc090oj_set_sprite_ctrl(state->pc090oj, (data & 0xe0) >> 5);

	/* bit 4 unused */

	/* bits 0 and 1 are coin lockout */
	coin_lockout_w(space->machine, 1, ~data & 0x01);
	coin_lockout_w(space->machine, 0, ~data & 0x02);

	/* bits 2 and 3 are the coin counters */
	coin_counter_w(space->machine, 1, data & 0x04);
	coin_counter_w(space->machine, 0, data & 0x08);
}

WRITE16_HANDLER( rainbow_spritectrl_w )
{
	rainbow_state *state = (rainbow_state *)space->machine->driver_data;

	if (offset == 0)
	{
		/* bits 0 and 1 always set */
		/* bits 5-7 are the sprite palette bank */
		/* other bits unknown */

		pc090oj_set_sprite_ctrl(state->pc090oj, (data & 0xe0) >> 5);
	}
}

WRITE16_HANDLER( opwolf_spritectrl_w )
{
	opwolf_state *state = (opwolf_state *)space->machine->driver_data;

	if (offset == 0)
	{
		/* bits 0 and 1 always set */
		/* bits 5-7 are the sprite palette bank */
		/* other bits unknown */

		pc090oj_set_sprite_ctrl(state->pc090oj, (data & 0xe0) >> 5);

		/* If data = 4, the Piston Motor is off, otherwise it's on. */
		if (data == 4)
		{
			output_set_value("Player1_Recoil_Piston", 0);
		}
		else
		{
			output_set_value("Player1_Recoil_Piston", 1);
		}
	}
}

WRITE16_HANDLER( jumping_spritectrl_w )
{
	rainbow_state *state = (rainbow_state *)space->machine->driver_data;

	if (offset == 0)
	{
		/* bits 0 and 1 are set after 15 seconds */
		/* bits 5-7 are the sprite palette bank */
		/* other bits unknown */

		state->sprite_ctrl = data;
	}
}



/***************************************************************************/

VIDEO_UPDATE( rastan )
{
	rastan_state *state = (rastan_state *)screen->machine->driver_data;
	int layer[2];

	pc080sn_tilemap_update(state->pc080sn);

	layer[0] = 0;
	layer[1] = 1;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	pc080sn_tilemap_draw(state->pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	pc080sn_tilemap_draw(state->pc080sn, bitmap, cliprect, layer[1], 0, 2);

	pc090oj_draw_sprites(state->pc090oj, bitmap, cliprect, 0);
	return 0;
}

/***************************************************************************/

VIDEO_UPDATE( opwolf )
{
	opwolf_state *state = (opwolf_state *)screen->machine->driver_data;
	int layer[2];

	pc080sn_tilemap_update(state->pc080sn);

	layer[0] = 0;
	layer[1] = 1;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	pc080sn_tilemap_draw(state->pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	pc080sn_tilemap_draw(state->pc080sn, bitmap, cliprect, layer[1], 0, 2);

	pc090oj_draw_sprites(state->pc090oj, bitmap, cliprect, 1);

//  if (input_port_read(machine, "P1X"))
//  popmessage("%d %d", input_port_read(machine, "P1X"), input_port_read(machine, "P1Y"));

	return 0;
}

/***************************************************************************/

VIDEO_UPDATE( rainbow )
{
	rainbow_state *state = (rainbow_state *)screen->machine->driver_data;
	int layer[2];

	pc080sn_tilemap_update(state->pc080sn);

	layer[0] = 0;
	layer[1] = 1;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	pc080sn_tilemap_draw(state->pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);
	pc080sn_tilemap_draw(state->pc080sn, bitmap, cliprect, layer[1], 0, 2);

	pc090oj_draw_sprites(state->pc090oj, bitmap, cliprect, 1);
	return 0;
}

/***************************************************************************

Jumping uses different sprite controller
than rainbow island. - values are remapped
at address 0x2EA in the code. Apart from
physical layout, the main change is that
the Y settings are active low.

*/

VIDEO_UPDATE( jumping )
{
	rainbow_state *state = (rainbow_state *)screen->machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int offs, layer[2];
	int sprite_colbank = (state->sprite_ctrl & 0xe0) >> 1;

	pc080sn_tilemap_update(state->pc080sn);

	/* Override values, or foreground layer is in wrong position */
	pc080sn_set_scroll(state->pc080sn, 1, 16, 0);

	layer[0] = 0;
	layer[1] = 1;

	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	pc080sn_tilemap_draw(state->pc080sn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);

	/* Draw the sprites. 128 sprites in total */
	for (offs = state->spriteram_size / 2 - 8; offs >= 0; offs -= 8)
	{
		int tile = spriteram[offs];
		if (tile < screen->machine->gfx[1]->total_elements)
		{
			int sx,sy,color,data1;

			sy = ((spriteram[offs + 1] - 0xfff1) ^ 0xffff) & 0x1ff;
			if (sy > 400) sy = sy - 512;
			sx = (spriteram[offs + 2] - 0x38) & 0x1ff;
			if (sx > 400) sx = sx - 512;

			data1 = spriteram[offs + 3];
			color = (spriteram[offs + 4] & 0x0f) | sprite_colbank;

			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],
					tile,
					color,
					data1 & 0x40, data1 & 0x80,
					sx,sy+1,15);
		}
	}

	pc080sn_tilemap_draw(state->pc080sn, bitmap, cliprect, layer[1], 0, 0);

#if 0
	{
		char buf[80];
		sprintf(buf,"sprite_ctrl: %04x", state->sprite_ctrl);
		popmessage(buf);
	}
#endif
	return 0;
}
