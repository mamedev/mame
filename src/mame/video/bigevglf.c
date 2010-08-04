/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/
#include "emu.h"
#include "includes/bigevglf.h"


WRITE8_HANDLER(bigevglf_palette_w)
{
	bigevglf_state *state = space->machine->driver_data<bigevglf_state>();
	int color;

	state->paletteram[offset] = data;
	color = state->paletteram[offset & 0x3ff] | (state->paletteram[0x400 + (offset & 0x3ff)] << 8);
	palette_set_color_rgb(space->machine, offset & 0x3ff, pal4bit(color >> 4), pal4bit(color >> 0), pal4bit(color >> 8));
}

WRITE8_HANDLER( bigevglf_gfxcontrol_w )
{
	bigevglf_state *state = space->machine->driver_data<bigevglf_state>();

/* bits used: 0,1,2,3
 0 and 2 select plane,
 1 and 3 select visible plane,
*/
	state->plane_selected  = ((data & 4) >> 1) | (data & 1);
	state->plane_visible = ((data & 8) >> 2) | ((data & 2) >> 1);
}

WRITE8_HANDLER( bigevglf_vidram_addr_w )
{
	bigevglf_state *state = space->machine->driver_data<bigevglf_state>();
	state->vidram_bank = (data & 0xff) * 0x100;
}

WRITE8_HANDLER( bigevglf_vidram_w )
{
	bigevglf_state *state = space->machine->driver_data<bigevglf_state>();
	UINT32 x, y, o;
	o = state->vidram_bank + offset;
	state->vidram[o + 0x10000 * state->plane_selected] = data;
	y = o >>8;
	x = (o & 255);
	*BITMAP_ADDR16(state->tmp_bitmap[state->plane_selected], y, x) = data;
}

READ8_HANDLER( bigevglf_vidram_r )
{
	bigevglf_state *state = space->machine->driver_data<bigevglf_state>();
	return state->vidram[0x10000 * state->plane_selected + state->vidram_bank + offset];
}

VIDEO_START( bigevglf )
{
	bigevglf_state *state = machine->driver_data<bigevglf_state>();

	state->tmp_bitmap[0] = machine->primary_screen->alloc_compatible_bitmap();
	state->tmp_bitmap[1] = machine->primary_screen->alloc_compatible_bitmap();
	state->tmp_bitmap[2] = machine->primary_screen->alloc_compatible_bitmap();
	state->tmp_bitmap[3] = machine->primary_screen->alloc_compatible_bitmap();
	state_save_register_global_bitmap(machine, state->tmp_bitmap[0]);
	state_save_register_global_bitmap(machine, state->tmp_bitmap[1]);
	state_save_register_global_bitmap(machine, state->tmp_bitmap[2]);
	state_save_register_global_bitmap(machine, state->tmp_bitmap[3]);

	state->vidram = auto_alloc_array(machine, UINT8, 0x100 * 0x100 * 4);

	state_save_register_global_pointer(machine, state->vidram, 0x100 * 0x100 * 4);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	bigevglf_state *state = machine->driver_data<bigevglf_state>();
	int i, j;
	for (i = 0xc0-4; i >= 0; i-= 4)
	{
		int code, sx, sy;
		code = state->spriteram2[i + 1];
		sx = state->spriteram2[i + 3];
		sy = 200 - state->spriteram2[i];
		for (j = 0; j < 16; j++)
			drawgfx_transpen(bitmap, cliprect, machine->gfx[0],
				state->spriteram1[(code << 4) + j] + ((state->spriteram1[0x400 + (code << 4) + j] & 0xf) << 8),
				state->spriteram2[i + 2] & 0xf,
				0,0,
				sx + ((j & 1) << 3), sy + ((j >> 1) << 3), 0);
	}
}

VIDEO_UPDATE( bigevglf )
{
	bigevglf_state *state = screen->machine->driver_data<bigevglf_state>();

	copybitmap(bitmap, state->tmp_bitmap[state->plane_visible], 0, 0, 0, 0, cliprect);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
