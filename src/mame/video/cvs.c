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
	machine.colortable = colortable_alloc(machine, 0x10);

	/* color mapping PROM */
	for (attr = 0; attr < 0x100; attr++)
	{
		for (i = 0; i < 8; i++)
		{
			UINT8 ctabentry = color_prom[(i << 8) | attr] & 0x07;

			/* bits 0 and 2 are swapped */
			ctabentry = BITSWAP8(ctabentry,7,6,5,4,3,0,1,2);

			colortable_entry_set_value(machine.colortable, (attr << 3) | i, ctabentry);
		}
	}

	/* background collision map */
	for (i = 0; i < 8; i++)
	{
		colortable_entry_set_value(machine.colortable, 0x800 + i, 0);
		colortable_entry_set_value(machine.colortable, 0x808 + i, i & 0x04);
		colortable_entry_set_value(machine.colortable, 0x810 + i, i & 0x02);
		colortable_entry_set_value(machine.colortable, 0x818 + i, i & 0x06);
	}

	/* sprites */
	for (i = 0; i < 8; i++)
		colortable_entry_set_value(machine.colortable, SPRITE_PEN_BASE + i, i | 0x08);

	/* bullet */
	colortable_entry_set_value(machine.colortable, BULLET_STAR_PEN, 7);
}


static void set_pens( running_machine &machine )
{
	cvs_state *state = machine.driver_data<cvs_state>();
	int i;

	for (i = 0; i < 0x10; i++)
	{
		int r = pal2bit(~state->m_palette_ram[i] >> 0);
		int g = pal3bit(~state->m_palette_ram[i] >> 2);
		int b = pal3bit(~state->m_palette_ram[i] >> 5);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}
}



WRITE8_MEMBER(cvs_state::cvs_video_fx_w)
{

	if (data & 0xce)
		logerror("%4x : CVS: Unimplemented CVS video fx = %2x\n",cpu_get_pc(&space.device()), data & 0xce);

	m_stars_on = data & 0x01;

	if (data & 0x02)   logerror("           SHADE BRIGHTER TO RIGHT\n");
	if (data & 0x04)   logerror("           SCREEN ROTATE\n");
	if (data & 0x08)   logerror("           SHADE BRIGHTER TO LEFT\n");

	set_led_status(machine(), 1, data & 0x10);	/* lamp 1 */
	set_led_status(machine(), 2, data & 0x20);	/* lamp 2 */

	if (data & 0x40)   logerror("           SHADE BRIGHTER TO BOTTOM\n");
	if (data & 0x80)   logerror("           SHADE BRIGHTER TO TOP\n");
}



READ8_MEMBER(cvs_state::cvs_collision_r)
{
	return m_collision_register;
}

READ8_MEMBER(cvs_state::cvs_collision_clear)
{
	m_collision_register = 0;
	return 0;
}


WRITE8_MEMBER(cvs_state::cvs_scroll_w)
{
	m_scroll_reg = 255 - data;
}


VIDEO_START( cvs )
{
	cvs_state *state = machine.driver_data<cvs_state>();

	cvs_init_stars(machine);

	/* create helper bitmaps */
	machine.primary_screen->register_screen_bitmap(state->m_background_bitmap);
	machine.primary_screen->register_screen_bitmap(state->m_collision_background);
	machine.primary_screen->register_screen_bitmap(state->m_scrolled_collision_background);

	/* register save */
	state->save_item(NAME(state->m_background_bitmap));
	state->save_item(NAME(state->m_collision_background));
	state->save_item(NAME(state->m_scrolled_collision_background));
}


SCREEN_UPDATE_IND16( cvs )
{
	cvs_state *state = screen.machine().driver_data<cvs_state>();
	static const int ram_based_char_start_indices[] = { 0xe0, 0xc0, 0x100, 0x80 };
	offs_t offs;
	int scroll[8];

	set_pens(screen.machine());

	/* draw the background */
	for (offs = 0; offs < 0x0400; offs++)
	{
		int collision_color = 0x100;
		UINT8 code = state->m_video_ram[offs];
		UINT8 color = state->m_color_ram[offs];

		UINT8 x = offs << 3;
		UINT8 y = offs >> 5 << 3;

		int gfxnum = (code < ram_based_char_start_indices[state->m_character_banking_mode]) ? 0 : 1;

		drawgfx_opaque(state->m_background_bitmap, state->m_background_bitmap.cliprect(), screen.machine().gfx[gfxnum],
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

		drawgfx_opaque(state->m_collision_background, state->m_collision_background.cliprect(), screen.machine().gfx[gfxnum],
				code, collision_color,
				0, 0,
				x, y);
	}


	/* Update screen - 8 regions, fixed scrolling area */
	scroll[0] = 0;
	scroll[1] = state->m_scroll_reg;
	scroll[2] = state->m_scroll_reg;
	scroll[3] = state->m_scroll_reg;
	scroll[4] = state->m_scroll_reg;
	scroll[5] = state->m_scroll_reg;
	scroll[6] = 0;
	scroll[7] = 0;

	copyscrollbitmap(bitmap, state->m_background_bitmap, 0, 0, 8, scroll, cliprect);
	copyscrollbitmap(state->m_scrolled_collision_background, state->m_collision_background, 0, 0, 8, scroll, cliprect);

	/* update the S2636 chips */
	bitmap_ind16 &s2636_0_bitmap = s2636_update(state->m_s2636_0, cliprect);
	bitmap_ind16 &s2636_1_bitmap = s2636_update(state->m_s2636_1, cliprect);
	bitmap_ind16 &s2636_2_bitmap = s2636_update(state->m_s2636_2, cliprect);

	/* Bullet Hardware */
	for (offs = 8; offs < 256; offs++ )
	{
		if (state->m_bullet_ram[offs] != 0)
		{
			int ct;
			for (ct = 0; ct < 4; ct++)
			{
				int bx = 255 - 7 - state->m_bullet_ram[offs] - ct;

				/* Bullet/Object Collision */
				if ((s2636_0_bitmap.pix16(offs, bx) != 0) ||
					(s2636_1_bitmap.pix16(offs, bx) != 0) ||
					(s2636_2_bitmap.pix16(offs, bx) != 0))
					state->m_collision_register |= 0x08;

				/* Bullet/Background Collision */
				if (colortable_entry_get_value(screen.machine().colortable, state->m_scrolled_collision_background.pix16(offs, bx)))
					state->m_collision_register |= 0x80;

				bitmap.pix16(offs, bx) = BULLET_STAR_PEN;
			}
		}
	}


	/* mix and copy the S2636 images into the main bitmap, also check for collision */
	{
		int y;

		for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			int x;

			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				int pixel0 = s2636_0_bitmap.pix16(y, x);
				int pixel1 = s2636_1_bitmap.pix16(y, x);
				int pixel2 = s2636_2_bitmap.pix16(y, x);

				int pixel = pixel0 | pixel1 | pixel2;

				if (S2636_IS_PIXEL_DRAWN(pixel))
				{
					bitmap.pix16(y, x) = SPRITE_PEN_BASE + S2636_PIXEL_COLOR(pixel);

					/* S2636 vs. S2636 collision detection */
					if (S2636_IS_PIXEL_DRAWN(pixel0) && S2636_IS_PIXEL_DRAWN(pixel1)) state->m_collision_register |= 0x01;
					if (S2636_IS_PIXEL_DRAWN(pixel1) && S2636_IS_PIXEL_DRAWN(pixel2)) state->m_collision_register |= 0x02;
					if (S2636_IS_PIXEL_DRAWN(pixel0) && S2636_IS_PIXEL_DRAWN(pixel2)) state->m_collision_register |= 0x04;

					/* S2636 vs. background collision detection */
					if (colortable_entry_get_value(screen.machine().colortable, state->m_scrolled_collision_background.pix16(y, x)))
					{
						if (S2636_IS_PIXEL_DRAWN(pixel0)) state->m_collision_register |= 0x10;
						if (S2636_IS_PIXEL_DRAWN(pixel1)) state->m_collision_register |= 0x20;
						if (S2636_IS_PIXEL_DRAWN(pixel2)) state->m_collision_register |= 0x40;
					}
				}
			}
		}
	}

	/* stars circuit */
	if (state->m_stars_on)
		cvs_update_stars(screen.machine(), bitmap, cliprect, BULLET_STAR_PEN, 0);

	return 0;
}



/* cvs stars hardware */

void cvs_scroll_stars( running_machine &machine )
{
	cvs_state *state = machine.driver_data<cvs_state>();
	state->m_stars_scroll++;
}

void cvs_init_stars( running_machine &machine )
{
	cvs_state *state = machine.driver_data<cvs_state>();
	int generator = 0;
	int x, y;

	/* precalculate the star background */

	state->m_total_stars = 0;

	for (y = 255; y >= 0; y--)
	{
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
					if (state->m_total_stars < CVS_MAX_STARS)
					{
						state->m_stars[state->m_total_stars].x = x;
						state->m_stars[state->m_total_stars].y = y;
						state->m_stars[state->m_total_stars].code = 1;

						state->m_total_stars++;
					}
				}
			}
		}
	}
}

void cvs_update_stars(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, const pen_t star_pen, bool update_always)
{
	cvs_state *state = machine.driver_data<cvs_state>();
	for (int offs = 0; offs < state->m_total_stars; offs++)
	{
		UINT8 x = (state->m_stars[offs].x + state->m_stars_scroll) >> 1;
		UINT8 y = state->m_stars[offs].y + ((state->m_stars_scroll + state->m_stars[offs].x) >> 9);

		if ((y & 1) ^ ((x >> 4) & 1))
		{
			if (flip_screen_x_get(machine))
				x = ~x;

			if (flip_screen_y_get(machine))
				y = ~y;

			if ((y >= cliprect.min_y) && (y <= cliprect.max_y) &&
				(update_always || (colortable_entry_get_value(machine.colortable, bitmap.pix16(y, x)) == 0)))
				bitmap.pix16(y, x) = star_pen;
		}
	}
}
