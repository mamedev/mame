/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/mw8080bw.h"


#define NUM_PENS	(8)


MACHINE_START( extra_8080bw_vh )
{
	mw8080bw_state *state = machine->driver_data<mw8080bw_state>();

	state_save_register_global(machine, state->c8080bw_flip_screen);
	state_save_register_global(machine, state->color_map);
	state_save_register_global(machine, state->screen_red);

	// These two only belong to schaser, but for simplicity's sake let's waste
	// two bytes in other drivers' .sta files.
	state_save_register_global(machine, state->schaser_background_disable);
	state_save_register_global(machine, state->schaser_background_select);
}


static void invadpt2_get_pens( pen_t *pens )
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 2), pal1bit(i >> 1));
	}
}


static void sflush_get_pens( pen_t *pens )
{
	offs_t i;

	pens[0] = MAKE_RGB(0x80, 0x80, 0xff);

	for (i = 1; i < NUM_PENS; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 2), pal1bit(i >> 1));
	}
}


static void cosmo_get_pens( pen_t *pens )
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
	}
}


INLINE void set_pixel( running_machine *machine, bitmap_t *bitmap, UINT8 y, UINT8 x, pen_t *pens, UINT8 color )
{
	mw8080bw_state *state = machine->driver_data<mw8080bw_state>();

	if (y >= MW8080BW_VCOUNTER_START_NO_VBLANK)
	{
		if (state->c8080bw_flip_screen)
			*BITMAP_ADDR32(bitmap, MW8080BW_VBSTART - 1 - (y - MW8080BW_VCOUNTER_START_NO_VBLANK), MW8080BW_HPIXCOUNT - 1 - x) = pens[color];
		else
			*BITMAP_ADDR32(bitmap, y - MW8080BW_VCOUNTER_START_NO_VBLANK, x) = pens[color];
	}
}


INLINE void set_8_pixels( running_machine *machine, bitmap_t *bitmap, UINT8 y, UINT8 x, UINT8 data, pen_t *pens, UINT8 fore_color, UINT8 back_color )
{
	int i;

	for (i = 0; i < 8; i++)
	{
		set_pixel(machine, bitmap, y, x, pens, (data & 0x01) ? fore_color : back_color);

		x = x + 1;
		data = data >> 1;
	}
}


/* this is needed as this driver doesn't emulate the shift register like mw8080bw does */
static void clear_extra_columns( running_machine *machine, bitmap_t *bitmap, pen_t *pens, UINT8 color )
{
	mw8080bw_state *state = machine->driver_data<mw8080bw_state>();
	UINT8 x;

	for (x = 0; x < 4; x++)
	{
		UINT8 y;

		for (y = MW8080BW_VCOUNTER_START_NO_VBLANK; y != 0; y++)
		{
			if (state->c8080bw_flip_screen)
				*BITMAP_ADDR32(bitmap, MW8080BW_VBSTART - 1 - (y - MW8080BW_VCOUNTER_START_NO_VBLANK), MW8080BW_HPIXCOUNT - 1 - (256 + x)) = pens[color];
			else
				*BITMAP_ADDR32(bitmap, y - MW8080BW_VCOUNTER_START_NO_VBLANK, 256 + x) = pens[color];
		}
	}
}


VIDEO_UPDATE( invadpt2 )
{
	mw8080bw_state *state = screen->machine->driver_data<mw8080bw_state>();
	pen_t pens[NUM_PENS];
	offs_t offs;
	UINT8 *prom;
	UINT8 *color_map_base;

	invadpt2_get_pens(pens);

	prom = memory_region(screen->machine, "proms");
	color_map_base = state->color_map ? &prom[0x0400] : &prom[0x0000];

	for (offs = 0; offs < state->main_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		offs_t color_address = (offs >> 8 << 5) | (offs & 0x1f);

		UINT8 data = state->main_ram[offs];
		UINT8 fore_color = state->screen_red ? 1 : color_map_base[color_address] & 0x07;

		set_8_pixels(screen->machine, bitmap, y, x, data, pens, fore_color, 0);
	}

	clear_extra_columns(screen->machine, bitmap, pens, 0);

	return 0;
}


VIDEO_UPDATE( ballbomb )
{
	mw8080bw_state *state = screen->machine->driver_data<mw8080bw_state>();
	pen_t pens[NUM_PENS];
	offs_t offs;
	UINT8 *color_map_base;
	UINT8 *prom;

	invadpt2_get_pens(pens);

	prom = memory_region(screen->machine, "proms");
	color_map_base = state->color_map ? &prom[0x0400] : &prom[0x0000];

	for (offs = 0; offs < state->main_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		offs_t color_address = (offs >> 8 << 5) | (offs & 0x1f);

		UINT8 data = state->main_ram[offs];
		UINT8 fore_color = state->screen_red ? 1 : color_map_base[color_address] & 0x07;

		/* blue background */
		set_8_pixels(screen->machine, bitmap, y, x, data, pens, fore_color, 2);
	}

	clear_extra_columns(screen->machine, bitmap, pens, 2);

	return 0;
}


VIDEO_UPDATE( schaser )
{
	mw8080bw_state *state = screen->machine->driver_data<mw8080bw_state>();
	pen_t pens[NUM_PENS];
	offs_t offs;
	UINT8 *background_map_base;

	invadpt2_get_pens(pens);

	background_map_base = memory_region(screen->machine, "proms");

	for (offs = 0; offs < state->main_ram_size; offs++)
	{
		UINT8 back_color = 0;

		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = state->main_ram[offs];
		UINT8 fore_color = state->colorram[offs & 0x1f9f] & 0x07;

		if (!state->schaser_background_disable)
		{
			offs_t back_address = (offs >> 8 << 5) | (offs & 0x1f);

			UINT8 back_data = background_map_base[back_address];

			/* the equations derived from the schematics don't appear to produce
               the right colors, but this one does, at least for this PROM */
			back_color = (((back_data & 0x0c) == 0x0c) && state->schaser_background_select) ? 4 : 2;
		}

		set_8_pixels(screen->machine, bitmap, y, x, data, pens, fore_color, back_color);
	}

	clear_extra_columns(screen->machine, bitmap, pens, state->schaser_background_disable ? 0 : 2);

	return 0;
}


VIDEO_UPDATE( schasercv )
{
	mw8080bw_state *state = screen->machine->driver_data<mw8080bw_state>();
	pen_t pens[NUM_PENS];
	offs_t offs;

	invadpt2_get_pens(pens);

	for (offs = 0; offs < state->main_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = state->main_ram[offs];
		UINT8 fore_color = state->colorram[offs & 0x1f9f] & 0x07;

		/* blue background */
		set_8_pixels(screen->machine, bitmap, y, x, data, pens, fore_color, 2);
	}

	clear_extra_columns(screen->machine, bitmap, pens, 2);

	return 0;
}


VIDEO_UPDATE( rollingc )
{
	mw8080bw_state *state = screen->machine->driver_data<mw8080bw_state>();
	pen_t pens[NUM_PENS];
	offs_t offs;

	invadpt2_get_pens(pens);

	for (offs = 0; offs < state->main_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = state->main_ram[offs];
		UINT8 fore_color = state->colorram[offs & 0x1f1f] & 0x07;

		set_8_pixels(screen->machine, bitmap, y, x, data, pens, fore_color, 0);
	}

	clear_extra_columns(screen->machine, bitmap, pens, 0);

	return 0;
}


VIDEO_UPDATE( polaris )
{
	mw8080bw_state *state = screen->machine->driver_data<mw8080bw_state>();
	pen_t pens[NUM_PENS];
	offs_t offs;
	UINT8 *color_map_base;
	UINT8 *cloud_gfx;

	invadpt2_get_pens(pens);

	color_map_base = memory_region(screen->machine, "proms");
	cloud_gfx = memory_region(screen->machine, "user1");

	for (offs = 0; offs < state->main_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = state->main_ram[offs];

		offs_t color_address = (offs >> 8 << 5) | (offs & 0x1f);

		/* for the background color, bit 0 of the map PROM is connected to green gun.
           red is 0 and blue is 1, giving cyan and blue for the background.  This
           is different from what the schematics shows, but it's supported
           by screenshots.  Bit 3 is connected to cloud enable, while
           bits 1 and 2 are marked 'not use' (sic) */

		UINT8 back_color = (color_map_base[color_address] & 0x01) ? 6 : 2;
		UINT8 fore_color = ~state->colorram[offs & 0x1f9f] & 0x07;

		UINT8 cloud_y = y - state->polaris_cloud_pos;

		if ((color_map_base[color_address] & 0x08) || (cloud_y >= 64))
		{
			set_8_pixels(screen->machine, bitmap, y, x, data, pens, fore_color, back_color);
		}
		else
		{
			/* cloud appears in this part of the screen */
			int i;

			for (i = 0; i < 8; i++)
			{
				UINT8 color;

				if (data & 0x01)
				{
					color = fore_color;
				}
				else
				{
					int bit = 1 << (~x & 0x03);
					offs_t cloud_gfx_offs = ((x >> 2) & 0x03) | ((~cloud_y & 0x3f) << 2);

					color = (cloud_gfx[cloud_gfx_offs] & bit) ? 7 : back_color;
				}

				set_pixel(screen->machine, bitmap, y, x, pens, color);

				x = x + 1;
				data = data >> 1;
			}
		}
	}

	clear_extra_columns(screen->machine, bitmap, pens, 6);

	return 0;
}


VIDEO_UPDATE( lupin3 )
{
	mw8080bw_state *state = screen->machine->driver_data<mw8080bw_state>();
	pen_t pens[NUM_PENS];
	offs_t offs;

	invadpt2_get_pens(pens);

	for (offs = 0; offs < state->main_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = state->main_ram[offs];
		UINT8 fore_color = ~state->colorram[offs & 0x1f9f] & 0x07;

		set_8_pixels(screen->machine, bitmap, y, x, data, pens, fore_color, 0);
	}

	clear_extra_columns(screen->machine, bitmap, pens, 0);

	return 0;
}


VIDEO_UPDATE( cosmo )
{
	mw8080bw_state *state = screen->machine->driver_data<mw8080bw_state>();
	pen_t pens[NUM_PENS];
	offs_t offs;

	cosmo_get_pens(pens);

	for (offs = 0; offs < state->main_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		offs_t color_address = (offs >> 8 << 5) | (offs & 0x1f);

		UINT8 data = state->main_ram[offs];
		UINT8 fore_color = state->colorram[color_address] & 0x07;

		set_8_pixels(screen->machine, bitmap, y, x, data, pens, fore_color, 0);
	}

	clear_extra_columns(screen->machine, bitmap, pens, 0);

	return 0;
}


VIDEO_UPDATE( indianbt )
{
	mw8080bw_state *state = screen->machine->driver_data<mw8080bw_state>();
	pen_t pens[NUM_PENS];
	offs_t offs;
	UINT8 *color_map_base;
	UINT8 *prom;

	cosmo_get_pens(pens);

	prom = memory_region(screen->machine, "proms");
	color_map_base = state->color_map ? &prom[0x0400] : &prom[0x0000];

	for (offs = 0; offs < state->main_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		offs_t color_address = (offs >> 8 << 5) | (offs & 0x1f);

		UINT8 data = state->main_ram[offs];
		UINT8 fore_color = color_map_base[color_address] & 0x07;

		set_8_pixels(screen->machine, bitmap, y, x, data, pens, fore_color, 0);
	}

	clear_extra_columns(screen->machine, bitmap, pens, 0);

	return 0;
}


VIDEO_UPDATE( shuttlei )
{
	mw8080bw_state *state = screen->machine->driver_data<mw8080bw_state>();
	pen_t pens[2] = { RGB_BLACK, RGB_WHITE };
	offs_t offs;

	for (offs = 0; offs < state->main_ram_size; offs++)
	{
		int i;

		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = state->main_ram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? RGB_WHITE : RGB_BLACK;
			*BITMAP_ADDR32(bitmap, y, x) = pen;

			x = x + 1;
			data = data << 1;
		}
	}

	clear_extra_columns(screen->machine, bitmap, pens, 0);

	return 0;
}


VIDEO_UPDATE( sflush )
{
	mw8080bw_state *state = screen->machine->driver_data<mw8080bw_state>();
	pen_t pens[NUM_PENS];
	offs_t offs;

	sflush_get_pens(pens);

	for (offs = 0; offs < state->main_ram_size; offs++)
	{
		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data = state->main_ram[offs];
		UINT8 fore_color = state->colorram[offs & 0x1f9f] & 0x07;

		set_8_pixels(screen->machine, bitmap, y, x, data, pens, fore_color, 0);
	}

	clear_extra_columns(screen->machine, bitmap, pens, 0);

	return 0;
}
