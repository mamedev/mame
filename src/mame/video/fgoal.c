/***************************************************************************

    Taito Field Goal video emulation

***************************************************************************/

#include "emu.h"
#include "includes/fgoal.h"


WRITE8_HANDLER( fgoal_color_w )
{
	fgoal_state *state = space->machine->driver_data<fgoal_state>();
	state->current_color = data & 3;
}


WRITE8_HANDLER( fgoal_ypos_w )
{
	fgoal_state *state = space->machine->driver_data<fgoal_state>();
	state->ypos = data;
}


WRITE8_HANDLER( fgoal_xpos_w )
{
	fgoal_state *state = space->machine->driver_data<fgoal_state>();
	state->xpos = data;
}


VIDEO_START( fgoal )
{
	fgoal_state *state = machine->driver_data<fgoal_state>();
	state->fgbitmap = machine->primary_screen->alloc_compatible_bitmap();
	state->bgbitmap = machine->primary_screen->alloc_compatible_bitmap();

	state_save_register_global_bitmap(machine, state->fgbitmap);
	state_save_register_global_bitmap(machine, state->bgbitmap);
}


VIDEO_UPDATE( fgoal )
{
	fgoal_state *state = screen->machine->driver_data<fgoal_state>();
	const UINT8* VRAM = state->video_ram;

	int x;
	int y;
	int n;

	/* draw color overlay foreground and background */

	if (state->fgoal_player == 1 && (input_port_read(screen->machine, "IN1") & 0x40))
	{
		drawgfxzoom_opaque(state->fgbitmap, cliprect, screen->machine->gfx[0],
			0, (state->fgoal_player << 2) | state->current_color,
			1, 1,
			0, 16,
			0x40000,
			0x40000);

		drawgfxzoom_opaque(state->bgbitmap, cliprect, screen->machine->gfx[1],
			0, 0,
			1, 1,
			0, 16,
			0x40000,
			0x40000);
	}
	else
	{
		drawgfxzoom_opaque(state->fgbitmap, cliprect, screen->machine->gfx[0],
			0, (state->fgoal_player << 2) | state->current_color,
			0, 0,
			0, 0,
			0x40000,
			0x40000);

		drawgfxzoom_opaque(state->bgbitmap, cliprect, screen->machine->gfx[1],
			0, 0,
			0, 0,
			0, 0,
			0x40000,
			0x40000);
	}

	/* the ball has a fixed color */

	for (y = state->ypos; y < state->ypos + 8; y++)
	{
		for (x = state->xpos; x < state->xpos + 8; x++)
		{
			if (y < 256 && x < 256)
			{
				*BITMAP_ADDR16(state->fgbitmap, y, x) = 128 + 16;
			}
		}
	}

	/* draw bitmap layer */

	for (y = 0; y < 256; y++)
	{
		UINT16* p = BITMAP_ADDR16(bitmap, y, 0);

		const UINT16* FG = BITMAP_ADDR16(state->fgbitmap, y, 0);
		const UINT16* BG = BITMAP_ADDR16(state->bgbitmap, y, 0);

		for (x = 0; x < 256; x += 8)
		{
			UINT8 v = *VRAM++;

			for (n = 0; n < 8; n++)
			{
				if (v & (1 << n))
				{
					p[x + n] = FG[x + n];
				}
				else
				{
					p[x + n] = BG[x + n];
				}
			}
		}
	}
	return 0;
}
