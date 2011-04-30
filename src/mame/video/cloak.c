/***************************************************************************

    Atari Cloak & Dagger hardware

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/cloak.h"


#define NUM_PENS	(0x40)

/***************************************************************************

  CLOAK & DAGGER uses RAM to dynamically
  create the palette. The resolution is 9 bit (3 bits per gun). The palette
  contains 64 entries, but it is accessed through a memory windows 128 bytes
  long: writing to the first 64 bytes sets the MSB of the red component to 0,
  while writing to the last 64 bytes sets it to 1.

  Colors 0-15  Character mapped graphics
  Colors 16-31 Bitmapped graphics (2 palettes selected by 128H)
  Colors 32-47 Sprites
  Colors 48-63 not used

  These are the exact resistor values from the schematics:

  bit 8 -- diode |< -- pullup 1 kohm -- 2.2 kohm resistor -- pulldown 100 pf -- RED
        -- diode |< -- pullup 1 kohm -- 4.7 kohm resistor -- pulldown 100 pf -- RED
        -- diode |< -- pullup 1 kohm -- 10  kohm resistor -- pulldown 100 pf -- RED
        -- diode |< -- pullup 1 kohm -- 2.2 kohm resistor -- pulldown 100 pf -- GREEN
        -- diode |< -- pullup 1 kohm -- 4.7 kohm resistor -- pulldown 100 pf -- GREEN
        -- diode |< -- pullup 1 kohm -- 10  kohm resistor -- pulldown 100 pf -- GREEN
        -- diode |< -- pullup 1 kohm -- 2.2 kohm resistor -- pulldown 100 pf -- BLUE
        -- diode |< -- pullup 1 kohm -- 4.7 kohm resistor -- pulldown 100 pf -- BLUE
  bit 0 -- diode |< -- pullup 1 kohm -- 10  kohm resistor -- pulldown 100 pf -- BLUE

***************************************************************************/

WRITE8_HANDLER( cloak_paletteram_w )
{
	cloak_state *state = space->machine().driver_data<cloak_state>();
	state->m_palette_ram[offset & 0x3f] = ((offset & 0x40) << 2) | data;
}


static void set_pens(running_machine &machine)
{
	cloak_state *state = machine.driver_data<cloak_state>();
	UINT16 *palette_ram = state->m_palette_ram;
	static const int resistances[3] = { 10000, 4700, 2200 };
	double weights[3];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
							 3, resistances, weights, 0, 1000,
							 0, 0, 0, 0, 0,
							 0, 0, 0, 0, 0);

	for (i = 0; i < NUM_PENS; i++)
	{
		int r, g, b;
		int bit0, bit1, bit2;

		/* red component */
		bit0 = (~palette_ram[i] >> 6) & 0x01;
		bit1 = (~palette_ram[i] >> 7) & 0x01;
		bit2 = (~palette_ram[i] >> 8) & 0x01;
		r = combine_3_weights(weights, bit0, bit1, bit2);

		/* green component */
		bit0 = (~palette_ram[i] >> 3) & 0x01;
		bit1 = (~palette_ram[i] >> 4) & 0x01;
		bit2 = (~palette_ram[i] >> 5) & 0x01;
		g = combine_3_weights(weights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (~palette_ram[i] >> 0) & 0x01;
		bit1 = (~palette_ram[i] >> 1) & 0x01;
		bit2 = (~palette_ram[i] >> 2) & 0x01;
		b = combine_3_weights(weights, bit0, bit1, bit2);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


static void set_current_bitmap_videoram_pointer(cloak_state *state)
{
	state->m_current_bitmap_videoram_accessed  = state->m_bitmap_videoram_selected ? state->m_bitmap_videoram1 : state->m_bitmap_videoram2;
	state->m_current_bitmap_videoram_displayed = state->m_bitmap_videoram_selected ? state->m_bitmap_videoram2 : state->m_bitmap_videoram1;
}

WRITE8_HANDLER( cloak_clearbmp_w )
{
	cloak_state *state = space->machine().driver_data<cloak_state>();

	space->machine().primary_screen->update_now();
	state->m_bitmap_videoram_selected = data & 0x01;
	set_current_bitmap_videoram_pointer(state);

	if (data & 0x02)	/* clear */
		memset(state->m_current_bitmap_videoram_accessed, 0, 256*256);
}

static void adjust_xy(cloak_state *state, int offset)
{
	switch (offset)
	{
		case 0x00:  state->m_bitmap_videoram_address_x--; state->m_bitmap_videoram_address_y++; break;
		case 0x01:								 state->m_bitmap_videoram_address_y--; break;
		case 0x02:  state->m_bitmap_videoram_address_x--;							  break;
		case 0x04:  state->m_bitmap_videoram_address_x++; state->m_bitmap_videoram_address_y++; break;
		case 0x05:								 state->m_bitmap_videoram_address_y++; break;
		case 0x06:  state->m_bitmap_videoram_address_x++;							  break;
	}
}

READ8_HANDLER( graph_processor_r )
{
	cloak_state *state = space->machine().driver_data<cloak_state>();
	UINT8 ret = state->m_current_bitmap_videoram_displayed[(state->m_bitmap_videoram_address_y << 8) | state->m_bitmap_videoram_address_x];

	adjust_xy(state, offset);

	return ret;
}

WRITE8_HANDLER( graph_processor_w )
{
	cloak_state *state = space->machine().driver_data<cloak_state>();

	switch (offset)
	{
		case 0x03: state->m_bitmap_videoram_address_x = data; break;
		case 0x07: state->m_bitmap_videoram_address_y = data; break;
		default:
			state->m_current_bitmap_videoram_accessed[(state->m_bitmap_videoram_address_y << 8) | state->m_bitmap_videoram_address_x] = data & 0x0f;

			adjust_xy(state, offset);
			break;
	}
}

WRITE8_HANDLER( cloak_videoram_w )
{
	cloak_state *state = space->machine().driver_data<cloak_state>();
	UINT8 *videoram = state->m_videoram;

	videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

WRITE8_HANDLER( cloak_flipscreen_w )
{
	flip_screen_set(space->machine(), data & 0x80);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	cloak_state *state = machine.driver_data<cloak_state>();
	UINT8 *videoram = state->m_videoram;
	int code = videoram[tile_index];

	SET_TILE_INFO(0, code, 0, 0);
}

VIDEO_START( cloak )
{
	cloak_state *state = machine.driver_data<cloak_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_bitmap_videoram1 = auto_alloc_array(machine, UINT8, 256*256);
	state->m_bitmap_videoram2 = auto_alloc_array(machine, UINT8, 256*256);
	state->m_palette_ram = auto_alloc_array(machine, UINT16, NUM_PENS);

	set_current_bitmap_videoram_pointer(state);

	state->save_item(NAME(state->m_bitmap_videoram_address_x));
	state->save_item(NAME(state->m_bitmap_videoram_address_y));
	state->save_item(NAME(state->m_bitmap_videoram_selected));
	state->save_pointer(NAME(state->m_bitmap_videoram1), 256*256);
	state->save_pointer(NAME(state->m_bitmap_videoram2), 256*256);
	state->save_pointer(NAME(state->m_palette_ram), NUM_PENS);
	machine.save().register_postload(save_prepost_delegate(FUNC(set_current_bitmap_videoram_pointer), state));
}

static void draw_bitmap(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	cloak_state *state = machine.driver_data<cloak_state>();
	int x, y;

	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			pen_t pen = state->m_current_bitmap_videoram_displayed[(y << 8) | x] & 0x07;

			if (pen)
				*BITMAP_ADDR16(bitmap, y, (x - 6) & 0xff) = 0x10 | ((x & 0x80) >> 4) | pen;
		}
}

static void draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	cloak_state *state = machine.driver_data<cloak_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = (0x100 / 4) - 1; offs >= 0; offs--)
	{
		int code = spriteram[offs + 64] & 0x7f;
		int flipx = spriteram[offs + 64] & 0x80;
		int flipy = 0;
		int sx = spriteram[offs + 192];
		int sy = 240 - spriteram[offs];

		if (flip_screen_get(machine))
		{
			sx -= 9;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, machine.gfx[1], code, 0, flipx, flipy,	sx, sy, 0);
	}
}

SCREEN_UPDATE( cloak )
{
	cloak_state *state = screen->machine().driver_data<cloak_state>();
	set_pens(screen->machine());
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	draw_bitmap(screen->machine(), bitmap, cliprect);
	draw_sprites(screen->machine(), bitmap, cliprect);
	return 0;
}
