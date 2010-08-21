#include "emu.h"
#include "profiler.h"
#include "includes/buggychl.h"


PALETTE_INIT( buggychl )
{
	int i;

	/* arbitrary blue shading for the sky, estimation */
	for (i = 0; i < 128; i++)
		palette_set_color(machine, i + 128, MAKE_RGB(0, 240-i, 255));
}

VIDEO_START( buggychl )
{
	buggychl_state *state = machine->driver_data<buggychl_state>();
	state->tmp_bitmap1 = machine->primary_screen->alloc_compatible_bitmap();
	state->tmp_bitmap2 = machine->primary_screen->alloc_compatible_bitmap();

	state_save_register_global_bitmap(machine, state->tmp_bitmap1);
	state_save_register_global_bitmap(machine, state->tmp_bitmap2);

	gfx_element_set_source(machine->gfx[0], state->charram);
}



WRITE8_HANDLER( buggychl_chargen_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	if (state->charram[offset] != data)
	{
		state->charram[offset] = data;
		gfx_element_mark_dirty(space->machine->gfx[0], (offset / 8) & 0xff);
	}
}

WRITE8_HANDLER( buggychl_sprite_lookup_bank_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	state->sl_bank = (data & 0x10) << 8;
}

WRITE8_HANDLER( buggychl_sprite_lookup_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	state->sprite_lookup[offset + state->sl_bank] = data;
}

WRITE8_HANDLER( buggychl_ctrl_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
/*
    bit7 = lamp
    bit6 = lockout
    bit4 = OJMODE
    bit3 = SKY OFF
    bit2 = /SN3OFF
    bit1 = HINV
    bit0 = VINV
*/

	flip_screen_y_set(space->machine, data & 0x01);
	flip_screen_x_set(space->machine, data & 0x02);

	state->bg_on = data & 0x04;
	state->sky_on = data & 0x08;

	state->sprite_color_base = (data & 0x10) ? 1 * 16 : 3 * 16;

	coin_lockout_global_w(space->machine, (~data & 0x40) >> 6);
	set_led_status(space->machine, 0, ~data & 0x80);
}

WRITE8_HANDLER( buggychl_bg_scrollx_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	state->bg_scrollx = -(data - 0x12);
}


static void draw_sky( bitmap_t *bitmap, const rectangle *cliprect )
{
	int x, y;

	for (y = 0; y < 256; y++)
		for (x = 0; x < 256; x++)
			*BITMAP_ADDR16(bitmap, y, x) = 128 + x / 2;
}


static void draw_bg( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	buggychl_state *state = machine->driver_data<buggychl_state>();
	int offs;
	int scroll[256];

	/* prevent wraparound */
	rectangle clip = *cliprect;
	if (flip_screen_x_get(machine)) clip.min_x += 8*8;
	else clip.max_x -= 8*8;

	for (offs = 0; offs < 0x400; offs++)
	{
		int code = state->videoram[0x400 + offs];

		int sx = offs % 32;
		int sy = offs / 32;

		if (flip_screen_x_get(machine))
			sx = 31 - sx;
		if (flip_screen_y_get(machine))
			sy = 31 - sy;

		drawgfx_opaque(state->tmp_bitmap1, NULL, machine->gfx[0],
				code,
				2,
				flip_screen_x_get(machine),flip_screen_y_get(machine),
				8*sx,8*sy);
	}

	/* first copy to a temp bitmap doing column scroll */
	for (offs = 0; offs < 256; offs++)
		scroll[offs] = -state->scrollv[offs / 8];

	copyscrollbitmap(state->tmp_bitmap2, state->tmp_bitmap1, 1, &state->bg_scrollx, 256, scroll, NULL);

	/* then copy to the screen doing row scroll */
	for (offs = 0; offs < 256; offs++)
		scroll[offs] = -state->scrollh[offs];

	copyscrollbitmap_trans(bitmap, state->tmp_bitmap2, 256, scroll, 0, 0, &clip, 32);
}


static void draw_fg( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	buggychl_state *state = machine->driver_data<buggychl_state>();
	int offs;

	for (offs = 0; offs < 0x400; offs++)
	{
		int sx = offs % 32;
		int sy = offs / 32;
		int flipx = flip_screen_x_get(machine);
		int flipy = flip_screen_y_get(machine);

		int code = state->videoram[offs];

		if (flipx)
			sx = 31 - sx;
		if (flipy)
			sy = 31 - sy;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				code,
				0,
				flipx,flipy,
				8*sx,8*sy,
				0);
	}
}


static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	buggychl_state *state = machine->driver_data<buggychl_state>();
	UINT8 *spriteram = state->spriteram;
	int offs;
	const UINT8 *gfx;

	g_profiler.start(PROFILER_USER1);

	gfx = memory_region(machine, "gfx2");
	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		int sx, sy, flipy, zoom, ch, x, px, y;
		const UINT8 *lookup;
		const UINT8 *zoomx_rom, *zoomy_rom;

		sx = spriteram[offs + 3] - ((spriteram[offs + 2] & 0x80) << 1);
		sy = 256 - 64 - spriteram[offs] + ((spriteram[offs + 1] & 0x80) << 1);
		flipy = spriteram[offs + 1] & 0x40;
		zoom = spriteram[offs + 1] & 0x3f;
		zoomy_rom = gfx + (zoom << 6);
		zoomx_rom = gfx + 0x2000 + (zoom << 3);

		lookup = state->sprite_lookup + ((spriteram[offs + 2] & 0x7f) << 6);

		for (y = 0; y < 64; y++)
		{
			int dy = flip_screen_y_get(machine) ? (255 - sy - y) : (sy + y);

			if ((dy & ~0xff) == 0)
			{
				int charline, base_pos;

				charline = zoomy_rom[y] & 0x07;
				base_pos = zoomy_rom[y] & 0x38;
				if (flipy)
					base_pos ^= 0x38;

				px = 0;
				for (ch = 0; ch < 4; ch++)
				{
					int pos, code, realflipy;
					const UINT8 *pendata;

					pos = base_pos + 2 * ch;
					code = 8 * (lookup[pos] | ((lookup[pos + 1] & 0x07) << 8));
					realflipy = (lookup[pos + 1] & 0x80) ? !flipy : flipy;
					code += (realflipy ? (charline ^ 7) : charline);
					pendata = gfx_element_get_data(machine->gfx[1], code);

					for (x = 0; x < 16; x++)
					{
						int col = pendata[x];
						if (col)
						{
							int dx = flip_screen_x_get(machine) ? (255 - sx - px) : (sx + px);
							if ((dx & ~0xff) == 0)
								*BITMAP_ADDR16(bitmap, dy, dx) = state->sprite_color_base + col;
						}

						/* the following line is almost certainly wrong */
						if (zoomx_rom[7 - (2 * ch + x / 8)] & (1 << (x & 7)))
							px++;
					}
				}
			}
		}
	}

	g_profiler.stop();
}


VIDEO_UPDATE( buggychl )
{
	buggychl_state *state = screen->machine->driver_data<buggychl_state>();

	if (state->sky_on)
		draw_sky(bitmap, cliprect);
	else
		bitmap_fill(bitmap, cliprect, 0);

	if (state->bg_on)
		draw_bg(screen->machine, bitmap, cliprect);

	draw_sprites(screen->machine, bitmap, cliprect);

	draw_fg(screen->machine, bitmap, cliprect);

	return 0;
}
