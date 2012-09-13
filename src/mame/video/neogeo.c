/***************************************************************************

    Neo-Geo hardware

****************************************************************************/

#include "emu.h"
#include "includes/neogeo.h"
#include "video/resnet.h"

#define NUM_PENS	(0x1000)

#define VERBOSE 	(0)

/*************************************
 *
 *  Video RAM access
 *
 *************************************/

static void set_videoram_offset( running_machine &machine, UINT16 data )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_videoram_offset = data;

	/* the read happens right away */
	state->m_videoram_read_buffer = state->m_videoram[state->m_videoram_offset];
}


static UINT16 get_videoram_data( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	return state->m_videoram_read_buffer;
}


static void set_videoram_data( running_machine &machine, UINT16 data)
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_videoram[state->m_videoram_offset] = data;

	/* auto increment/decrement the current offset - A15 is NOT effected */
	state->m_videoram_offset = (state->m_videoram_offset & 0x8000) | ((state->m_videoram_offset + state->m_videoram_modulo) & 0x7fff);

	/* read next value right away */
	state->m_videoram_read_buffer = state->m_videoram[state->m_videoram_offset];
}


static void set_videoram_modulo( running_machine &machine, UINT16 data)
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_videoram_modulo = data;
}


static UINT16 get_videoram_modulo( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	return state->m_videoram_modulo;
}



/*************************************
 *
 *  Palette handling
 *
 *************************************/

static void compute_rgb_weights( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	static const int resistances[] = { 220, 470, 1000, 2200, 3900 };

	/* compute four sets of weights - with or without the pulldowns -
       ensuring that we use the same scaler for all */

	double scaler = compute_resistor_weights(0, 0xff, -1,
							 5, resistances, state->m_rgb_weights_normal, 0, 0,
							 0, 0, 0, 0, 0,
							 0, 0, 0, 0, 0);

	compute_resistor_weights(0, 0xff, scaler,
							 5, resistances, state->m_rgb_weights_normal_bit15, 8200, 0,
							 0, 0, 0, 0, 0,
							 0, 0, 0, 0, 0);

	compute_resistor_weights(0, 0xff, scaler,
							 5, resistances, state->m_rgb_weights_dark, 150, 0,
							 0, 0, 0, 0, 0,
							 0, 0, 0, 0, 0);

	compute_resistor_weights(0, 0xff, scaler,
							 5, resistances, state->m_rgb_weights_dark_bit15, 1 / ((1.0 / 8200) + (1.0 / 150)), 0,
							 0, 0, 0, 0, 0,
							 0, 0, 0, 0, 0);
}


static pen_t get_pen( running_machine &machine, UINT16 data )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	double *weights;
	UINT8 r, g, b;

	if (state->m_screen_dark)
	{
		if (data & 0x8000)
			weights = state->m_rgb_weights_dark_bit15;
		else
			weights = state->m_rgb_weights_dark;
	}
	else
	{
		if (data & 0x8000)
			weights = state->m_rgb_weights_normal_bit15;
		else
			weights = state->m_rgb_weights_normal;
	}

	r = combine_5_weights(weights,
						  (data >> 11) & 0x01,
						  (data >> 10) & 0x01,
						  (data >>  9) & 0x01,
						  (data >>  8) & 0x01,
						  (data >> 14) & 0x01);

	g = combine_5_weights(weights,
						  (data >>  7) & 0x01,
						  (data >>  6) & 0x01,
						  (data >>  5) & 0x01,
						  (data >>  4) & 0x01,
						  (data >> 13) & 0x01);

	b = combine_5_weights(weights,
						  (data >>  3) & 0x01,
						  (data >>  2) & 0x01,
						  (data >>  1) & 0x01,
						  (data >>  0) & 0x01,
						  (data >> 12) & 0x01);

	return MAKE_RGB(r, g, b);
}


static void regenerate_pens(running_machine &machine)
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	int i;

	for (i = 0; i < NUM_PENS; i++)
		state->m_pens[i] = get_pen(machine, state->m_palettes[state->m_palette_bank][i]);
}


void neogeo_set_palette_bank( running_machine &machine, UINT8 data )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	if (data != state->m_palette_bank)
	{
		state->m_palette_bank = data;

		regenerate_pens(machine);
	}
}


void neogeo_set_screen_dark( running_machine &machine, UINT8 data )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	if (data != state->m_screen_dark)
	{
		state->m_screen_dark = data;

		regenerate_pens(machine);
	}
}


READ16_MEMBER(neogeo_state::neogeo_paletteram_r)
{
	return m_palettes[m_palette_bank][offset];
}


WRITE16_MEMBER(neogeo_state::neogeo_paletteram_w)
{
	UINT16 *addr = &m_palettes[m_palette_bank][offset];

	COMBINE_DATA(addr);

	m_pens[offset] = get_pen(machine(), *addr);
}



/*************************************
 *
 *  Auto animation
 *
 *************************************/

static void set_auto_animation_speed( running_machine &machine, UINT8 data)
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_auto_animation_speed = data;
}


static void set_auto_animation_disabled( running_machine &machine, UINT8 data)
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_auto_animation_disabled = data;
}


UINT8 neogeo_get_auto_animation_counter( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	return state->m_auto_animation_counter;
}


static TIMER_CALLBACK( auto_animation_timer_callback )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	if (state->m_auto_animation_frame_counter == 0)
	{
		state->m_auto_animation_frame_counter = state->m_auto_animation_speed;
		state->m_auto_animation_counter += 1;
	}
	else
		state->m_auto_animation_frame_counter = state->m_auto_animation_frame_counter - 1;

	state->m_auto_animation_timer->adjust(machine.primary_screen->time_until_pos(NEOGEO_VSSTART));
}


static void create_auto_animation_timer( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_auto_animation_timer = machine.scheduler().timer_alloc(FUNC(auto_animation_timer_callback));
}


static void start_auto_animation_timer( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_auto_animation_timer->adjust(machine.primary_screen->time_until_pos(NEOGEO_VSSTART));
}



/*************************************
 *
 *  Fixed layer
 *
 *************************************/

void neogeo_set_fixed_layer_source( running_machine &machine, UINT8 data )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_fixed_layer_source = data;
}


static void draw_fixed_layer( running_machine &machine, bitmap_rgb32 &bitmap, int scanline )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	int x;

	UINT8* gfx_base = state->memregion(state->m_fixed_layer_source ? "fixed" : "fixedbios")->base();
	UINT32 addr_mask = state->memregion(state->m_fixed_layer_source ? "fixed" : "fixedbios")->bytes() - 1;
	UINT16 *video_data = &state->m_videoram[0x7000 | (scanline >> 3)];
	UINT32 *pixel_addr = &bitmap.pix32(scanline, NEOGEO_HBEND);

	int garouoffsets[32];
	int banked = state->m_fixed_layer_source && (addr_mask > 0x1ffff);

	/* thanks to Mr K for the garou & kof2000 banking info */
	/* Build line banking table for Garou & MS3 before starting render */
	if (banked && state->m_fixed_layer_bank_type == 1)
	{
		int garoubank = 0;
		int k = 0;
		int y = 0;
		while (y < 32)
		{
			if (state->m_videoram[0x7500 + k] == 0x0200 && (state->m_videoram[0x7580 + k] & 0xff00) == 0xff00)
			{
				garoubank = state->m_videoram[0x7580 + k] & 3;
				garouoffsets[y++] = garoubank;
			}
			garouoffsets[y++] = garoubank;
			k += 2;
		}
	}

	for (x = 0; x < 40; x++)
	{
		UINT16 code_and_palette = *video_data;
		UINT16 code = code_and_palette & 0x0fff;

		if (banked)
		{
			int y = scanline >> 3;
			switch (state->m_fixed_layer_bank_type)
			{
			case 1:
				/* Garou, MSlug 3 */
				code += 0x1000 * (garouoffsets[(y - 2) & 31] ^ 3);
				break;
			case 2:
				code += 0x1000 * (((state->m_videoram[0x7500 + ((y - 1) & 31) + 32 * (x / 6)] >> (5 - (x % 6)) * 2) & 3) ^ 3);
				break;
			}
		}

		{
			UINT8 data;
			int i;

			UINT8 *gfx = &gfx_base[((code << 5) | (scanline & 0x07)) & addr_mask];
			pen_t *char_pens = &state->m_pens[code_and_palette >> 12 << 4];

			for (i = 0; i < 4; i++)
			{
				static const UINT32 pix_offsets[] = { 0x10, 0x18, 0x00, 0x08 };

				data = gfx[pix_offsets[i]];

				if (data & 0x0f)
					*pixel_addr = char_pens[data & 0x0f];
				pixel_addr++;

				if (data & 0xf0)
					*pixel_addr = char_pens[(data & 0xf0) >> 4];
				pixel_addr++;
			}
		}

		video_data = video_data + 0x20;
	}
}



/*************************************
 *
 *  Sprite hardware
 *
 *************************************/

#define MAX_SPRITES_PER_SCREEN    (381)
#define MAX_SPRITES_PER_LINE      (96)


/* horizontal zoom table - verified on real hardware */
static const int zoom_x_tables[][16] =
{
	{ 0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0 },
	{ 0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0 },
	{ 0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0 },
	{ 0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0 },
	{ 0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0 },
	{ 0,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0 },
	{ 0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0 },
	{ 1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0 },
	{ 1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0 },
	{ 1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0 },
	{ 1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1 },
	{ 1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1 },
	{ 1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1 },
	{ 1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1 },
	{ 1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1 },
	{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 }
};


INLINE int rows_to_height(int rows)
{
	if ((rows == 0) || (rows > 0x20))
		rows = 0x20;

	return rows * 0x10;
}


INLINE int sprite_on_scanline(int scanline, int y, int rows)
{
	/* check if the current scanline falls inside this sprite,
       two possible scenerios, wrap around or not */
	int max_y = (y + rows_to_height(rows) - 1) & 0x1ff;

	return (((max_y >= y) &&  (scanline >= y) && (scanline <= max_y)) ||
			((max_y <  y) && ((scanline >= y) || (scanline <= max_y))));
}


static void draw_sprites( running_machine &machine, bitmap_rgb32 &bitmap, int scanline )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	int sprite_index;
	int max_sprite_index;

	int y = 0;
	int x = 0;
	int rows = 0;
	int zoom_y = 0;
	int zoom_x = 0;
	UINT16 *sprite_list;

	/* select the active list */
	if (scanline & 0x01)
		sprite_list = &state->m_videoram[0x8680];
	else
		sprite_list = &state->m_videoram[0x8600];

	/* optimization -- find last non-zero entry and only draw that many +1
       sprite.  This is not 100% correct as the hardware will keep drawing
       the #0 sprite over and over, but we need the speed */
	for (max_sprite_index = (MAX_SPRITES_PER_LINE - 1); max_sprite_index >= 0; max_sprite_index--)
	{
		if (sprite_list[max_sprite_index] != 0)
			break;
	}

	/* add the +1 now, just in case the 0 at the end is real sprite */
	if (max_sprite_index != (MAX_SPRITES_PER_LINE - 1))
		max_sprite_index = max_sprite_index + 1;

	for (sprite_index = 0; sprite_index <= max_sprite_index; sprite_index++)
	{
		UINT16 sprite_number = sprite_list[sprite_index] & 0x01ff;
		UINT16 y_control = state->m_videoram[0x8200 | sprite_number];
		UINT16 zoom_control = state->m_videoram[0x8000 | sprite_number];

		/* if chained, go to next X coordinate and get new X zoom */
		if (y_control & 0x40)
		{
			x = (x + zoom_x + 1) & 0x01ff;
			zoom_x = (zoom_control >> 8) & 0x0f;
		}
		/* new block */
		else
		{
			y = 0x200 - (y_control >> 7);
			x = state->m_videoram[0x8400 | sprite_number] >> 7;
			zoom_y = zoom_control & 0xff;
			zoom_x = (zoom_control >> 8) & 0x0f;
			rows = y_control & 0x3f;
		}

		/* skip if falls completely outside the screen */
		if ((x >= 0x140) && (x <= 0x1f0))
			continue;

		/* double check the Y coordinate, in case somebody modified the sprite coordinate
           since we buffered it */
		if (sprite_on_scanline(scanline, y, rows))
		{
			int sprite_y;
			int tile;
			UINT8 sprite_y_and_tile;
			offs_t attr_and_code_offs;
			UINT16 attr;
			UINT32 code;
			const int *zoom_x_table;
			UINT8 *gfx;
			pen_t *line_pens;
			int x_inc;

			int sprite_line = (scanline - y) & 0x1ff;
			int zoom_line = sprite_line & 0xff;
			int invert = sprite_line & 0x100;

			if (invert)
				zoom_line ^= 0xff;

			if (rows > 0x20)
			{
				zoom_line = zoom_line % ((zoom_y + 1) << 1);

				if (zoom_line > zoom_y)
				{
					zoom_line = ((zoom_y + 1) << 1) - 1 - zoom_line;
					invert = !invert;
				}
			}

			sprite_y_and_tile = state->m_region_zoomy[(zoom_y << 8) | zoom_line];
			sprite_y = sprite_y_and_tile & 0x0f;
			tile = sprite_y_and_tile >> 4;

			if (invert)
			{
				sprite_y ^= 0x0f;
				tile ^= 0x1f;
			}

			attr_and_code_offs = (sprite_number << 6) | (tile << 1);
			attr = state->m_videoram[attr_and_code_offs + 1];
			code = ((attr << 12) & 0x70000) | state->m_videoram[attr_and_code_offs];

			/* substitute auto animation bits */
			if (!state->m_auto_animation_disabled)
			{
				if (attr & 0x0008)
					code = (code & ~0x07) | (state->m_auto_animation_counter & 0x07);
				else if (attr & 0x0004)
					code = (code & ~0x03) | (state->m_auto_animation_counter & 0x03);
			}

			/* vertical flip? */
			if (attr & 0x0002)
				sprite_y ^= 0x0f;

			zoom_x_table = zoom_x_tables[zoom_x];

			/* compute offset in gfx ROM and mask it to the number of bits available */
			gfx = &state->m_sprite_gfx[((code << 8) | (sprite_y << 4)) & state->m_sprite_gfx_address_mask];

			line_pens = &state->m_pens[attr >> 8 << 4];

			/* horizontal flip? */
			if (attr & 0x0001)
			{
				gfx = gfx + 0x0f;
				x_inc = -1;
			}
			else
				x_inc = 1;

			/* draw the line - no wrap-around */
			if (x <= 0x01f0)
			{
				int i;

				UINT32 *pixel_addr = &bitmap.pix32(scanline, x + NEOGEO_HBEND);

				for (i = 0; i < 0x10; i++)
				{
					if (*zoom_x_table)
					{
						if (*gfx)
							*pixel_addr = line_pens[*gfx];

						pixel_addr++;
					}

					zoom_x_table++;
					gfx += x_inc;
				}
			}
			/* wrap-around */
			else
			{
				int i;

				int x_save = x;
				UINT32 *pixel_addr = &bitmap.pix32(scanline, NEOGEO_HBEND);

				for (i = 0; i < 0x10; i++)
				{
					if (*zoom_x_table)
					{
						if (x >= 0x200)
						{
							if (*gfx)
								*pixel_addr = line_pens[*gfx];

							pixel_addr++;
						}

						x++;
					}

					zoom_x_table++;
					gfx += x_inc;
				}

				x = x_save;
			}
		}
	}
}


static void parse_sprites( running_machine &machine, int scanline )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	UINT16 sprite_number;
	int y = 0;
	int rows = 0;
	UINT16 *sprite_list;

	int active_sprite_count = 0;

	/* select the active list */
	if (scanline & 0x01)
		sprite_list = &state->m_videoram[0x8680];
	else
		sprite_list = &state->m_videoram[0x8600];

	/* scan all sprites */
	for (sprite_number = 0; sprite_number < MAX_SPRITES_PER_SCREEN; sprite_number++)
	{
		UINT16 y_control = state->m_videoram[0x8200 | sprite_number];

		/* if not chained, get Y position and height, otherwise use previous values */
		if (~y_control & 0x40)
		{
			y = 0x200 - (y_control >> 7);
			rows = y_control & 0x3f;
		}

		/* skip sprites with 0 rows */
		if (rows == 0)
			continue;

		if (!sprite_on_scanline(scanline, y, rows))
			continue;

		/* sprite is on this scanline, add it to active list */
		*sprite_list = sprite_number;

		sprite_list++;

		/* increment sprite count, and if we reached the max, bail out */
		active_sprite_count++;

		if (active_sprite_count == MAX_SPRITES_PER_LINE)
			break;
	}

	/* fill the rest of the sprite list with 0, including one extra entry */
	memset(sprite_list, 0, sizeof(sprite_list[0]) * (MAX_SPRITES_PER_LINE - active_sprite_count + 1));
}


static TIMER_CALLBACK( sprite_line_timer_callback )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	int scanline = param;

	/* we are at the beginning of a scanline -
       we need to draw the previous scanline and parse the sprites on the current one */
	if (scanline != 0)
		machine.primary_screen->update_partial(scanline - 1);

	parse_sprites(machine, scanline);

	/* let's come back at the beginning of the next line */
	scanline = (scanline + 1) % NEOGEO_VTOTAL;

	state->m_sprite_line_timer->adjust(machine.primary_screen->time_until_pos(scanline), scanline);
}


static void create_sprite_line_timer( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_sprite_line_timer = machine.scheduler().timer_alloc(FUNC(sprite_line_timer_callback));
}


static void start_sprite_line_timer( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();
	state->m_sprite_line_timer->adjust(machine.primary_screen->time_until_pos(0));
}


static void optimize_sprite_data( running_machine &machine )
{
	neogeo_state *state = machine.driver_data<neogeo_state>();

	/* convert the sprite graphics data into a format that
       allows faster blitting */
	int i;
	int len;
	UINT8 *src;
	UINT8 *dest;
	UINT32 bit;

	/* get mask based on the length rounded up to the nearest
       power of 2 */
	state->m_sprite_gfx_address_mask = 0xffffffff;

	len = state->memregion("sprites")->bytes();

	for (bit = 0x80000000; bit != 0; bit >>= 1)
	{
		if (((len * 2) - 1) & bit)
			break;

		state->m_sprite_gfx_address_mask >>= 1;
	}

	state->m_sprite_gfx = auto_alloc_array_clear(machine, UINT8, state->m_sprite_gfx_address_mask + 1);

	src = machine.root_device().memregion("sprites")->base();
	dest = state->m_sprite_gfx;

	for (i = 0; i < len; i += 0x80, src += 0x80)
	{
		int y;

		for (y = 0; y < 0x10; y++)
		{
			int x;

			for (x = 0; x < 8; x++)
			{
				*(dest++) = (((src[0x43 | (y << 2)] >> x) & 0x01) << 3) |
						    (((src[0x41 | (y << 2)] >> x) & 0x01) << 2) |
							(((src[0x42 | (y << 2)] >> x) & 0x01) << 1) |
							(((src[0x40 | (y << 2)] >> x) & 0x01) << 0);
			}

			for (x = 0; x < 8; x++)
			{
				*(dest++) = (((src[0x03 | (y << 2)] >> x) & 0x01) << 3) |
						    (((src[0x01 | (y << 2)] >> x) & 0x01) << 2) |
							(((src[0x02 | (y << 2)] >> x) & 0x01) << 1) |
							(((src[0x00 | (y << 2)] >> x) & 0x01) << 0);
			}
		}
	}
}



/*************************************
 *
 *  Video control
 *
 *************************************/

static UINT16 get_video_control( running_machine &machine )
{
	UINT16 ret;
	UINT16 v_counter;

	/*
        The format of this very important location is:  AAAA AAAA A??? BCCC

        A is the raster line counter. mosyougi relies solely on this to do the
          raster effects on the title screen; sdodgeb loops waiting for the top
          bit to be 1; zedblade heavily depends on it to work correctly (it
          checks the top bit in the IRQ2 handler).
        B is definitely a PAL/NTSC flag. Evidence:
          1) trally changes the position of the speed indicator depending on
             it (0 = lower 1 = higher).
          2) samsho3 sets a variable to 60 when the bit is 0 and 50 when it's 1.
             This is obviously the video refresh rate in Hz.
          3) samsho3 sets another variable to 256 or 307. This could be the total
             screen height (including vblank), or close to that.
          Some games (e.g. lstbld2, samsho3) do this (or similar):
          bclr    #$0, $3c000e.l
          when the bit is set, so 3c000e (whose function is unknown) has to be
          related
        C animation counter lower 3 bits
    */

	/* the vertical counter chain goes from 0xf8 - 0x1ff */
	v_counter = machine.primary_screen->vpos() + 0x100;

	if (v_counter >= 0x200)
		v_counter = v_counter - NEOGEO_VTOTAL;

	ret = (v_counter << 7) | (neogeo_get_auto_animation_counter(machine) & 0x0007);

	if (VERBOSE) logerror("%s: video_control read (%04x)\n", machine.describe_context(), ret);

	return ret;
}


static void set_video_control( running_machine &machine, UINT16 data )
{
	/* this does much more than this, but I'm not sure exactly what */
	if (VERBOSE) logerror("%s: video control write %04x\n", machine.describe_context(), data);

	set_auto_animation_speed(machine, data >> 8);
	set_auto_animation_disabled(machine, data & 0x0008);

	neogeo_set_display_position_interrupt_control(machine, data & 0x00f0);
}


READ16_MEMBER(neogeo_state::neogeo_video_register_r)
{
	UINT16 ret;

	/* accessing the LSB only is not mapped */
	if (mem_mask == 0x00ff)
		ret = neogeo_unmapped_r(space, 0, 0xffff) & 0x00ff;
	else
	{
		switch (offset)
		{
		default:
		case 0x00:
		case 0x01: ret = get_videoram_data(machine()); break;
		case 0x02: ret = get_videoram_modulo(machine()); break;
		case 0x03: ret = get_video_control(machine()); break;
		}
	}

	return ret;
}


WRITE16_MEMBER(neogeo_state::neogeo_video_register_w)
{
	/* accessing the LSB only is not mapped */
	if (mem_mask != 0x00ff)
	{
		/* accessing the MSB only stores same data in MSB and LSB */
		if (mem_mask == 0xff00)
			data = (data & 0xff00) | (data >> 8);

		switch (offset)
		{
		case 0x00: set_videoram_offset(machine(), data); break;
		case 0x01: set_videoram_data(machine(), data); break;
		case 0x02: set_videoram_modulo(machine(), data); break;
		case 0x03: set_video_control(machine(), data); break;
		case 0x04: neogeo_set_display_counter_msb(&space, data); break;
		case 0x05: neogeo_set_display_counter_lsb(&space, data); break;
		case 0x06: neogeo_acknowledge_interrupt(machine(), data); break;
		case 0x07: break; /* unknown, see get_video_control */
		}
	}
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void neogeo_state::video_start()
{

	/* allocate memory not directly mapped */
	m_palettes[0] = auto_alloc_array(machine(), UINT16, NUM_PENS);
	m_palettes[1] = auto_alloc_array(machine(), UINT16, NUM_PENS);
	m_pens = auto_alloc_array(machine(), pen_t, NUM_PENS);
	m_videoram = auto_alloc_array(machine(), UINT16, 0x20000/2);

	/* clear allocated memory */
	memset(m_palettes[0], 0x00, NUM_PENS * sizeof(UINT16));
	memset(m_palettes[1], 0x00, NUM_PENS * sizeof(UINT16));
	memset(m_pens, 0x00, NUM_PENS * sizeof(pen_t));
	memset(m_videoram, 0x00, 0x20000);

	compute_rgb_weights(machine());
	create_sprite_line_timer(machine());
	create_auto_animation_timer(machine());
	optimize_sprite_data(machine());

	/* initialize values that are not modified on a reset */
	m_videoram_read_buffer = 0;
	m_videoram_offset = 0;
	m_videoram_modulo = 0;
	m_auto_animation_speed = 0;
	m_auto_animation_disabled = 0;
	m_auto_animation_counter = 0;
	m_auto_animation_frame_counter = 0;

	/* register for state saving */
	save_pointer(NAME(m_palettes[0]), NUM_PENS);
	save_pointer(NAME(m_palettes[1]), NUM_PENS);
	save_pointer(NAME(m_videoram), 0x20000/2);
	save_item(NAME(m_videoram_read_buffer));
	save_item(NAME(m_videoram_modulo));
	save_item(NAME(m_videoram_offset));
	save_item(NAME(m_fixed_layer_source));
	save_item(NAME(m_screen_dark));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_auto_animation_speed));
	save_item(NAME(m_auto_animation_disabled));
	save_item(NAME(m_auto_animation_counter));
	save_item(NAME(m_auto_animation_frame_counter));

	machine().save().register_postload(save_prepost_delegate(FUNC(regenerate_pens), &machine()));

	m_region_zoomy = memregion("zoomy")->base();
}



/*************************************
 *
 *  Video system reset
 *
 *************************************/

void neogeo_state::video_reset()
{
	start_sprite_line_timer(machine());
	start_auto_animation_timer(machine());
	optimize_sprite_data(machine());
}



/*************************************
 *
 *  Video update
 *
 *************************************/

SCREEN_UPDATE_RGB32( neogeo )
{
	neogeo_state *state = screen.machine().driver_data<neogeo_state>();

	/* fill with background color first */
	bitmap.fill(state->m_pens[0x0fff], cliprect);

	draw_sprites(screen.machine(), bitmap, cliprect.min_y);

	draw_fixed_layer(screen.machine(), bitmap, cliprect.min_y);

	return 0;
}
