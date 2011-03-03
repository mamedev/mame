/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/wiz.h"


static const rectangle spritevisiblearea =
{
	2*8, 32*8-1,
	2*8, 30*8-1
};

static const rectangle spritevisibleareaflipx =
{
	0*8, 30*8-1,
	2*8, 30*8-1
};


VIDEO_START( wiz )
{
	wiz_state *state = machine->driver_data<wiz_state>();
	state_save_register_global_array(machine, state->char_bank);
	state_save_register_global_array(machine, state->palbank);
	state_save_register_global(machine, state->flipx);
	state_save_register_global(machine, state->flipy);
	state_save_register_global(machine, state->bgpen);
}

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Stinger has three 256x4 palette PROMs (one per gun).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 100 ohm resistor  -- RED/GREEN/BLUE
        -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 1  kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
PALETTE_INIT( wiz )
{
	int i;


	for (i = 0;i < machine->total_colors();i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;
		bit0 = (color_prom[machine->total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[machine->total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[machine->total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[machine->total_colors()] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;
		bit0 = (color_prom[2*machine->total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[2*machine->total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[2*machine->total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[2*machine->total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		color_prom++;
	}
}

WRITE8_HANDLER( wiz_palettebank_w )
{
	wiz_state *state = space->machine->driver_data<wiz_state>();
	state->palbank[offset] = data & 1;
	state->palette_bank = state->palbank[0] + 2 * state->palbank[1];
}

WRITE8_HANDLER( wiz_bgcolor_w )
{
	wiz_state *state = space->machine->driver_data<wiz_state>();
	state->bgpen = data;
}

WRITE8_HANDLER( wiz_char_bank_select_w )
{
	wiz_state *state = space->machine->driver_data<wiz_state>();
	state->char_bank[offset] = data & 1;
}

WRITE8_HANDLER( wiz_flipx_w )
{
	wiz_state *state = space->machine->driver_data<wiz_state>();
	state->flipx = data;
}


WRITE8_HANDLER( wiz_flipy_w )
{
	wiz_state *state = space->machine->driver_data<wiz_state>();
	state->flipy = data;
}

static void draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int bank, int colortype)
{
	wiz_state *state = machine->driver_data<wiz_state>();
	UINT8 *videoram = state->videoram;
	int offs;

	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */

	for (offs = 0x400 - 1; offs >= 0; offs--)
	{
		int scroll,sx,sy,col;

		sx = offs % 32;
		sy = offs / 32;

		if (colortype)
		{
			col = (state->attributesram[2 * sx + 1] & 0x07);
		}
		else
		{
			col = (state->attributesram[2 * (offs % 32) + 1] & 0x04) + (videoram[offs] & 3);
		}

		scroll = (8*sy + 256 - state->attributesram[2 * sx]) % 256;
		if (state->flipy)
		{
		   scroll = (248 - scroll) % 256;
		}
		if (state->flipx) sx = 31 - sx;


		drawgfx_transpen(bitmap,cliprect,machine->gfx[bank],
			videoram[offs],
			col + 8 * state->palette_bank,
			state->flipx,state->flipy,
			8*sx,scroll,0);
	}
}

static void draw_foreground(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int colortype)
{
	wiz_state *state = machine->driver_data<wiz_state>();
	int offs;

	/* draw the frontmost playfield. They are characters, but draw them as sprites. */
	for (offs = 0x400 - 1; offs >= 0; offs--)
	{
		int scroll,sx,sy,col;


		sx = offs % 32;
		sy = offs / 32;

		if (colortype)
		{
			col = (state->attributesram2[2 * sx + 1] & 0x07);
		}
		else
		{
			col = (state->colorram2[offs] & 0x07);
		}

		scroll = (8*sy + 256 - state->attributesram2[2 * sx]) % 256;
		if (state->flipy)
		{
		   scroll = (248 - scroll) % 256;
		}
		if (state->flipx) sx = 31 - sx;


		drawgfx_transpen(bitmap,cliprect,machine->gfx[state->char_bank[1]],
			state->videoram2[offs],
			col + 8 * state->palette_bank,
			state->flipx,state->flipy,
			8*sx,scroll,0);
	}
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,
						 const rectangle *cliprect, UINT8* sprite_ram,
                         int bank)
{
	wiz_state *state = machine->driver_data<wiz_state>();
	int offs;

	for (offs = machine->generic.spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int sx,sy;


		sx = sprite_ram[offs + 3];
		sy = sprite_ram[offs];

		if (!sx || !sy) continue;

		if ( state->flipx) sx = 240 - sx;
		if (!state->flipy) sy = 240 - sy;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[bank],
				sprite_ram[offs + 1],
				(sprite_ram[offs + 2] & 0x07) + 8 * state->palette_bank,
				state->flipx,state->flipy,
				sx,sy,0);
	}
}


SCREEN_UPDATE( kungfut )
{
	wiz_state *state = screen->machine->driver_data<wiz_state>();
	bitmap_fill(bitmap,cliprect,state->bgpen);
	draw_background(screen->machine, bitmap, cliprect, 2 + state->char_bank[0] , 0);
	draw_foreground(screen->machine, bitmap, cliprect, 0);
	draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.spriteram2.u8, 4);
	draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.spriteram.u8  , 5);
	return 0;
}

SCREEN_UPDATE( wiz )
{
	wiz_state *state = screen->machine->driver_data<wiz_state>();
	int bank;
	const rectangle* visible_area;

	bitmap_fill(bitmap,cliprect,state->bgpen);
	draw_background(screen->machine, bitmap, cliprect, 2 + ((state->char_bank[0] << 1) | state->char_bank[1]), 0);
	draw_foreground(screen->machine, bitmap, cliprect, 0);

	visible_area = state->flipx ? &spritevisibleareaflipx : &spritevisiblearea;

	bank = 7 + *state->sprite_bank;

	draw_sprites(screen->machine, bitmap, visible_area, screen->machine->generic.spriteram2.u8, 6);
	draw_sprites(screen->machine, bitmap, visible_area, screen->machine->generic.spriteram.u8  , bank);
	return 0;
}


SCREEN_UPDATE( stinger )
{
	wiz_state *state = screen->machine->driver_data<wiz_state>();
	bitmap_fill(bitmap,cliprect,state->bgpen);
	draw_background(screen->machine, bitmap, cliprect, 2 + state->char_bank[0], 1);
	draw_foreground(screen->machine, bitmap, cliprect, 1);
	draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.spriteram2.u8, 4);
	draw_sprites(screen->machine, bitmap, cliprect, screen->machine->generic.spriteram.u8  , 5);
	return 0;
}
