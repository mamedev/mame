#include "emu.h"
#include "video/konicdev.h"
#include "includes/ultraman.h"

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void ultraman_sprite_callback( running_machine *machine, int *code, int *color, int *priority, int *shadow )
{
	ultraman_state *state = (ultraman_state *)machine->driver_data;

	*priority = (*color & 0x80) >> 7;
	*color = state->sprite_colorbase + ((*color & 0x7e) >> 1);
	*shadow = 0;
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void ultraman_zoom_callback_0(running_machine *machine, int *code, int *color, int *flags )
{
	ultraman_state *state = (ultraman_state *)machine->driver_data;
	*code |= ((*color & 0x07) << 8) | (state->bank0 << 11);
	*color = state->zoom_colorbase[0] + ((*color & 0xf8) >> 3);
}

void ultraman_zoom_callback_1(running_machine *machine, int *code, int *color, int *flags )
{
	ultraman_state *state = (ultraman_state *)machine->driver_data;
	*code |= ((*color & 0x07) << 8) | (state->bank1 << 11);
	*color = state->zoom_colorbase[1] + ((*color & 0xf8) >> 3);
}

void ultraman_zoom_callback_2(running_machine *machine, int *code, int *color, int *flags )
{
	ultraman_state *state = (ultraman_state *)machine->driver_data;
	*code |= ((*color & 0x07) << 8) | (state->bank2 << 11);
	*color = state->zoom_colorbase[2] + ((*color & 0xf8) >> 3);
}



/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( ultraman )
{
	ultraman_state *state = (ultraman_state *)machine->driver_data;
	state->sprite_colorbase = 192;
	state->zoom_colorbase[0] = 0;
	state->zoom_colorbase[1] = 64;
	state->zoom_colorbase[2] = 128;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( ultraman_gfxctrl_w )
{
	ultraman_state *state = (ultraman_state *)space->machine->driver_data;

	if (ACCESSING_BITS_0_7)
	{
		/*  bit 0: enable wraparound for scr #1
            bit 1: msb of code for scr #1
            bit 2: enable wraparound for scr #2
            bit 3: msb of code for scr #2
            bit 4: enable wraparound for scr #3
            bit 5: msb of code for scr #3
            bit 6: coin counter 1
            bit 7: coin counter 2 */

		k051316_wraparound_enable(state->k051316_1, data & 0x01);

		if (state->bank0 != ((data & 0x02) >> 1))
		{
			state->bank0 = (data & 0x02) >> 1;
			tilemap_mark_all_tiles_dirty_all(space->machine);	/* should mark only zoom0 */
		}

		k051316_wraparound_enable(state->k051316_2, data & 0x04);

		if (state->bank1 != ((data & 0x08) >> 3))
		{
			state->bank1 = (data & 0x08) >> 3;
			tilemap_mark_all_tiles_dirty_all(space->machine);	/* should mark only zoom1 */
		}

		k051316_wraparound_enable(state->k051316_3, data & 0x10);

		if (state->bank2 != ((data & 0x20) >> 5))
		{
			state->bank2 = (data & 0x20) >> 5;
			tilemap_mark_all_tiles_dirty_all(space->machine);	/* should mark only zoom2 */
		}

		coin_counter_w(space->machine, 0, data & 0x40);
		coin_counter_w(space->machine, 1, data & 0x80);
	}
}



/***************************************************************************

    Display Refresh

***************************************************************************/

VIDEO_UPDATE( ultraman )
{
	ultraman_state *state = (ultraman_state *)screen->machine->driver_data;

	k051316_zoom_draw(state->k051316_3, bitmap, cliprect, 0, 0);
	k051316_zoom_draw(state->k051316_2, bitmap, cliprect, 0, 0);
	k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, 0);
	k051316_zoom_draw(state->k051316_1, bitmap, cliprect, 0, 0);
	k051960_sprites_draw(state->k051960, bitmap, cliprect, 1, 1);
	return 0;
}
