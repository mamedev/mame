/***************************************************************************

  video\cvs.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "includes/cvs.h"
#include "video/s2636.h"


#define SPRITE_PEN_BASE		(0x820)
#define BULLET_STAR_PEN		(0x828)

#define STARS_COLOR_BASE	16


/******************************************************
 * Convert Colour prom to format for Mame Colour Map  *
 *                                                    *
 * There is a prom used for colour mapping and plane  *
 * priority. This is converted to a colour table here *
 *                                                    *
 * colours are taken from SRAM and are programmable   *
 ******************************************************/

PALETTE_INIT( cvs )
{
	int i, attr;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x10);

	/* color mapping PROM */
	for (attr = 0; attr < 0x100; attr++)
	{
		for (i = 0; i < 8; i++)
		{
			UINT8 ctabentry = color_prom[(i << 8) | attr] & 0x07;

			/* bits 0 and 2 are swapped */
			ctabentry = BITSWAP8(ctabentry,7,6,5,4,3,0,1,2);

			colortable_entry_set_value(machine->colortable, (attr << 3) | i, ctabentry);
		}
	}

	/* background collision map */
	for (i = 0; i < 8; i++)
	{
		colortable_entry_set_value(machine->colortable, 0x800 + i, 0);
		colortable_entry_set_value(machine->colortable, 0x808 + i, i & 0x04);
		colortable_entry_set_value(machine->colortable, 0x810 + i, i & 0x02);
		colortable_entry_set_value(machine->colortable, 0x818 + i, i & 0x06);
	}

	/* sprites */
	for (i = 0; i < 8; i++)
		colortable_entry_set_value(machine->colortable, SPRITE_PEN_BASE + i, i | 0x08);

	/* bullet */
	colortable_entry_set_value(machine->colortable, BULLET_STAR_PEN, 7);
}


static void set_pens( running_machine *machine )
{
	cvs_state *state = (cvs_state *)machine->driver_data;
	int i;

	for (i = 0; i < 0x10; i++)
	{
		int r = pal2bit(~state->palette_ram[i] >> 0);
		int g = pal3bit(~state->palette_ram[i] >> 2);
		int b = pal3bit(~state->palette_ram[i] >> 5);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}
}



WRITE8_HANDLER( cvs_video_fx_w )
{
	cvs_state *state = (cvs_state *)space->machine->driver_data;

	if (data & 0xce)
		logerror("%4x : CVS: Unimplemented CVS video fx = %2x\n",cpu_get_pc(space->cpu), data & 0xce);

	state->stars_on = data & 0x01;

	if (data & 0x02)   logerror("           SHADE BRIGHTER TO RIGHT\n");
	if (data & 0x04)   logerror("           SCREEN ROTATE\n");
	if (data & 0x08)   logerror("           SHADE BRIGHTER TO LEFT\n");

	set_led_status(space->machine, 1, data & 0x10);	/* lamp 1 */
	set_led_status(space->machine, 2, data & 0x20);	/* lamp 2 */

	if (data & 0x40)   logerror("           SHADE BRIGHTER TO BOTTOM\n");
	if (data & 0x80)   logerror("           SHADE BRIGHTER TO TOP\n");
}



READ8_HANDLER( cvs_collision_r )
{
	cvs_state *state = (cvs_state *)space->machine->driver_data;
	return state->collision_register;
}

READ8_HANDLER( cvs_collision_clear )
{
	cvs_state *state = (cvs_state *)space->machine->driver_data;
	state->collision_register = 0;
	return 0;
}


WRITE8_HANDLER( cvs_scroll_w )
{
	cvs_state *state = (cvs_state *)space->machine->driver_data;
	state->scroll_reg = 255 - data;
}


VIDEO_START( cvs )
{
	cvs_state *state = (cvs_state *)machine->driver_data;
	int generator = 0;
	int y;

	/* precalculate the star background */

	state->total_stars = 0;

	for (y = 255; y >= 0; y--)
	{
		int x;

		for (x = 511; x >= 0; x--)
		{
			int bit1, bit2;

			generator <<= 1;
			bit1 = (~generator >> 17) & 1;
			bit2 = (generator >> 5) & 1;

			if (bit1 ^ bit2)
				generator |= 1;

			if (((~generator >> 16) & 1) && (generator & 0xfe) == 0xfe)
			{
				if(((~(generator >> 12)) & 0x01) && ((~(generator >> 13)) & 0x01))
				{
					if (state->total_stars < CVS_MAX_STARS)
					{
						state->stars[state->total_stars].x = x;
						state->stars[state->total_stars].y = y;
						state->stars[state->total_stars].code = 1;

						state->total_stars++;
					}
				}
			}
		}
	}

	/* create helper bitmaps */
	state->background_bitmap = machine->primary_screen->alloc_compatible_bitmap();
	state->collision_background = machine->primary_screen->alloc_compatible_bitmap();
	state->scrolled_collision_background = machine->primary_screen->alloc_compatible_bitmap();

	/* register save */
	state_save_register_global_bitmap(machine, state->background_bitmap);
	state_save_register_global_bitmap(machine, state->collision_background);
	state_save_register_global_bitmap(machine, state->scrolled_collision_background);
}


void cvs_scroll_stars( running_machine *machine )
{
	cvs_state *state = (cvs_state *)machine->driver_data;
	state->stars_scroll++;
}


VIDEO_UPDATE( cvs )
{
	cvs_state *state = (cvs_state *)screen->machine->driver_data;
	static const int ram_based_char_start_indices[] = { 0xe0, 0xc0, 0x100, 0x80 };
	offs_t offs;
	int scroll[8];
	bitmap_t *s2636_0_bitmap, *s2636_1_bitmap, *s2636_2_bitmap;

	set_pens(screen->machine);

	/* draw the background */
	for (offs = 0; offs < 0x0400; offs++)
	{
		int collision_color = 0x100;
		UINT8 code = state->video_ram[offs];
		UINT8 color = state->color_ram[offs];

		UINT8 x = offs << 3;
		UINT8 y = offs >> 5 << 3;

		int gfxnum = (code < ram_based_char_start_indices[state->character_banking_mode]) ? 0 : 1;

		drawgfx_opaque(state->background_bitmap, 0, screen->machine->gfx[gfxnum],
				code, color,
				0, 0,
				x, y);

		/* foreground for collision detection */
		if (color & 0x80)
			collision_color = 0x103;
		else
		{
			if ((color & 0x03) == 0x03)
				collision_color = 0x101;
			else if ((color & 0x01) == 0)
				collision_color = 0x102;
		}

		drawgfx_opaque(state->collision_background, 0, screen->machine->gfx[gfxnum],
				code, collision_color,
				0, 0,
				x, y);
	}


	/* Update screen - 8 regions, fixed scrolling area */
	scroll[0] = 0;
	scroll[1] = state->scroll_reg;
	scroll[2] = state->scroll_reg;
	scroll[3] = state->scroll_reg;
	scroll[4] = state->scroll_reg;
	scroll[5] = state->scroll_reg;
	scroll[6] = 0;
	scroll[7] = 0;

	copyscrollbitmap(bitmap, state->background_bitmap, 0, 0, 8, scroll, cliprect);
	copyscrollbitmap(state->scrolled_collision_background, state->collision_background, 0, 0, 8, scroll, cliprect);

	/* update the S2636 chips */
	s2636_0_bitmap = s2636_update(state->s2636_0, cliprect);
	s2636_1_bitmap = s2636_update(state->s2636_1, cliprect);
	s2636_2_bitmap = s2636_update(state->s2636_2, cliprect);

	/* Bullet Hardware */
	for (offs = 8; offs < 256; offs++ )
	{
		if (state->bullet_ram[offs] != 0)
		{
			int ct;
			for (ct = 0; ct < 4; ct++)
			{
				int bx = 255 - 7 - state->bullet_ram[offs] - ct;

				/* Bullet/Object Collision */
				if ((*BITMAP_ADDR16(s2636_0_bitmap, offs, bx) != 0) ||
					(*BITMAP_ADDR16(s2636_1_bitmap, offs, bx) != 0) ||
					(*BITMAP_ADDR16(s2636_2_bitmap, offs, bx) != 0))
					state->collision_register |= 0x08;

				/* Bullet/Background Collision */
				if (colortable_entry_get_value(screen->machine->colortable, *BITMAP_ADDR16(state->scrolled_collision_background, offs, bx)))
					state->collision_register |= 0x80;

				*BITMAP_ADDR16(bitmap, offs, bx) = BULLET_STAR_PEN;
			}
		}
	}


	/* mix and copy the S2636 images into the main bitmap, also check for collision */
	{
		int y;

		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			int x;

			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				int pixel0 = *BITMAP_ADDR16(s2636_0_bitmap, y, x);
				int pixel1 = *BITMAP_ADDR16(s2636_1_bitmap, y, x);
				int pixel2 = *BITMAP_ADDR16(s2636_2_bitmap, y, x);

				int pixel = pixel0 | pixel1 | pixel2;

				if (S2636_IS_PIXEL_DRAWN(pixel))
				{
					*BITMAP_ADDR16(bitmap, y, x) = SPRITE_PEN_BASE + S2636_PIXEL_COLOR(pixel);

					/* S2636 vs. S2636 collision detection */
					if (S2636_IS_PIXEL_DRAWN(pixel0) && S2636_IS_PIXEL_DRAWN(pixel1)) state->collision_register |= 0x01;
					if (S2636_IS_PIXEL_DRAWN(pixel1) && S2636_IS_PIXEL_DRAWN(pixel2)) state->collision_register |= 0x02;
					if (S2636_IS_PIXEL_DRAWN(pixel0) && S2636_IS_PIXEL_DRAWN(pixel2)) state->collision_register |= 0x04;

					/* S2636 vs. background collision detection */
					if (colortable_entry_get_value(screen->machine->colortable, *BITMAP_ADDR16(state->scrolled_collision_background, y, x)))
					{
						if (S2636_IS_PIXEL_DRAWN(pixel0)) state->collision_register |= 0x10;
						if (S2636_IS_PIXEL_DRAWN(pixel1)) state->collision_register |= 0x20;
						if (S2636_IS_PIXEL_DRAWN(pixel2)) state->collision_register |= 0x40;
					}
				}
			}
		}
	}

	/* stars circuit */
	if (state->stars_on)
	{
		for (offs = 0; offs < state->total_stars; offs++)
		{
			UINT8 x = (state->stars[offs].x + state->stars_scroll) >> 1;
			UINT8 y = state->stars[offs].y + ((state->stars_scroll + state->stars[offs].x) >> 9);

			if ((y & 1) ^ ((x >> 4) & 1))
			{
				if (flip_screen_x_get(screen->machine))
					x = ~x;

				if (flip_screen_y_get(screen->machine))
					y = ~y;

				if ((y >= cliprect->min_y) && (y <= cliprect->max_y) &&
					(colortable_entry_get_value(screen->machine->colortable, *BITMAP_ADDR16(bitmap, y, x)) == 0))
					*BITMAP_ADDR16(bitmap, y, x) = BULLET_STAR_PEN;
			}
		}
	}

	return 0;
}
