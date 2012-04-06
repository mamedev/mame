/***************************************************************************

 cosmic.c

 emulation of video hardware of cosmic machines of 1979-1980(ish)

***************************************************************************/

#include "emu.h"
#include "includes/cosmic.h"


WRITE8_MEMBER(cosmic_state::cosmic_color_register_w)
{
	m_color_registers[offset] = data ? 1 : 0;
}


static pen_t panic_map_color( running_machine &machine, UINT8 x, UINT8 y )
{
	cosmic_state *state = machine.driver_data<cosmic_state>();
	offs_t offs = (state->m_color_registers[0] << 9) | (state->m_color_registers[2] << 10) | ((x >> 4) << 5) | (y >> 3);
	pen_t pen = machine.region("user1")->base()[offs];

	if (state->m_color_registers[1])
		pen >>= 4;

	return pen & 0x0f;
}

static pen_t cosmica_map_color( running_machine &machine, UINT8 x, UINT8 y )
{
	cosmic_state *state = machine.driver_data<cosmic_state>();
	offs_t offs = (state->m_color_registers[0] << 9) | ((x >> 4) << 5) | (y >> 3);
	pen_t pen = machine.region("user1")->base()[offs];

	if (state->m_color_registers[1]) // 0 according to the schematics, but that breaks alien formation colors
		pen >>= 4;

	return pen & 0x07;
}

static pen_t cosmicg_map_color( running_machine &machine, UINT8 x, UINT8 y )
{
	cosmic_state *state = machine.driver_data<cosmic_state>();
	offs_t offs = (state->m_color_registers[0] << 8) | (state->m_color_registers[1] << 9) | ((y >> 4) << 4) | (x >> 4);
	pen_t pen = machine.region("user1")->base()[offs];

	/* the upper 4 bits are for cocktail mode support */
	return pen & 0x0f;
}

static pen_t magspot_map_color( running_machine &machine, UINT8 x, UINT8 y )
{
	cosmic_state *state = machine.driver_data<cosmic_state>();
	offs_t offs = (state->m_color_registers[0] << 9) | ((x >> 3) << 4) | (y >> 4);
	pen_t pen = machine.region("user1")->base()[offs];

	if (state->m_color_registers[1])
		pen >>= 4;

	return pen & state->m_magspot_pen_mask;
}



/*
 * Panic Color table setup
 *
 * Bit 0 = RED, Bit 1 = GREEN, Bit 2 = BLUE
 *
 * First 8 colors are normal intensities
 *
 * But, bit 3 can be used to pull Blue via a 2k resistor to 5v
 * (1k to ground) so second version of table has blue set to 2/3
 */

PALETTE_INIT( panic )
{
	cosmic_state *state = machine.driver_data<cosmic_state>();
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x10);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x10; i++)
	{
		int r = pal1bit(i >> 0);
		int g = pal1bit(i >> 1);
		int b = ((i & 0x0c) == 0x08) ? 0xaa : pal1bit(i >> 2);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* background uses colors 0x00-0x0f */
	for (i = 0; i < 0x0f; i++)
		colortable_entry_set_value(machine.colortable, i, i);

	/* sprites use colors 0x00-0x07 */
	for (i = 0x10; i < 0x30; i++)
	{
		UINT8 ctabentry = color_prom[i - 0x10] & 0x07;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}

	state->m_map_color = panic_map_color;
}


/*
 * Cosmic Alien Color table setup
 *
 * 8 colors, 16 sprite color codes
 *
 * Bit 0 = RED, Bit 1 = GREEN, Bit 2 = BLUE
 *
 */

PALETTE_INIT( cosmica )
{
	cosmic_state *state = machine.driver_data<cosmic_state>();
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x08);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x08; i++)
	{
		rgb_t color = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
		colortable_palette_set_color(machine.colortable, i, color);
	}

	/* background and sprites use colors 0x00-0x07 */
	for (i = 0; i < 0x08; i++)
		colortable_entry_set_value(machine.colortable, i, i);

	for (i = 0x08; i < 0x28; i++)
	{
		UINT8 ctabentry;

		ctabentry = (color_prom[i - 0x08] >> 0) & 0x07;
		colortable_entry_set_value(machine.colortable, i + 0x00, ctabentry);

		ctabentry = (color_prom[i - 0x08] >> 4) & 0x07;
		colortable_entry_set_value(machine.colortable, i + 0x20, ctabentry);
	}

	state->m_map_color = cosmica_map_color;
}


/*
 * Cosmic guerilla table setup
 *
 * Use AA for normal, FF for Full Red
 * Bit 0 = R, bit 1 = G, bit 2 = B, bit 4 = High Red
 *
 * It's possible that the background is dark gray and not black, as the
 * resistor chain would never drop to zero, Anybody know ?
 */
PALETTE_INIT( cosmicg )
{
	cosmic_state *state = machine.driver_data<cosmic_state>();
	int i;

	for (i = 0; i < machine.total_colors(); i++)
	{
		int r = (i > 8) ? 0xff : 0xaa * ((i >> 0) & 1);
		int g = 0xaa * ((i >> 1) & 1);
		int b = 0xaa * ((i >> 2) & 1);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}

	state->m_map_color = cosmicg_map_color;
}


PALETTE_INIT( magspot )
{
	cosmic_state *state = machine.driver_data<cosmic_state>();
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x10);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x10; i++)
	{
		int r = ((i & 0x09) == 0x08) ? 0xaa : pal1bit(i >> 0);
		int g = pal1bit(i >> 1);
		int b = pal1bit(i >> 2);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* background uses colors 0x00-0x0f */
	for (i = 0; i < 0x0f; i++)
		colortable_entry_set_value(machine.colortable, i, i);

	/* sprites use colors 0x00-0x0f */
	for (i = 0x10; i < 0x30; i++)
	{
		UINT8 ctabentry = color_prom[i - 0x10] & 0x0f;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}

	state->m_map_color = magspot_map_color;
	state->m_magspot_pen_mask = 0x0f;
}


PALETTE_INIT( nomnlnd )
{
	cosmic_state *state = machine.driver_data<cosmic_state>();
	int i;

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x10);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x10; i++)
	{
		rgb_t color = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
		colortable_palette_set_color(machine.colortable, i, color);
	}

	/* background uses colors 0x00-0x07 */
	for (i = 0; i < 0x07; i++)
		colortable_entry_set_value(machine.colortable, i, i);

	/* sprites use colors 0x00-0x07 */
	for (i = 0x10; i < 0x30; i++)
	{
		UINT8 ctabentry = color_prom[i - 0x10] & 0x07;
		colortable_entry_set_value(machine.colortable, i, ctabentry);
	}

	state->m_map_color = magspot_map_color;
	state->m_magspot_pen_mask = 0x07;
}


WRITE8_MEMBER(cosmic_state::cosmic_background_enable_w)
{
	m_background_enable = data;
}


static void draw_bitmap( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	cosmic_state *state = machine.driver_data<cosmic_state>();
	offs_t offs;

	for (offs = 0; offs < state->m_videoram_size; offs++)
	{
		int i;
		UINT8 data = state->m_videoram[offs];

		UINT8 x = offs << 3;
		UINT8 y = offs >> 5;

		pen_t pen = state->m_map_color(machine, x, y);

		for (i = 0; i < 8; i++)
		{
			if (data & 0x80)
			{
				if (flip_screen_get(machine))
					bitmap.pix16(255-y, 255-x) = pen;
				else
					bitmap.pix16(y, x) = pen;
			}

			x++;
			data <<= 1;
		}
	}
}


static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int extra_sprites )
{
	cosmic_state *state = machine.driver_data<cosmic_state>();
	int offs;

	for (offs = state->m_spriteram_size - 4;offs >= 0;offs -= 4)
	{
		if (state->m_spriteram[offs] != 0)
        {
			int code, color;

			code  = ~state->m_spriteram[offs] & 0x3f;
			color = ~state->m_spriteram[offs + 3] & color_mask;

			if (extra_sprites)
				code |= (state->m_spriteram[offs + 3] & 0x08) << 3;

            if (state->m_spriteram[offs] & 0x80)
                /* 16x16 sprite */
			    drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
					    code, color,
					    0, ~state->m_spriteram[offs] & 0x40,
				    	256-state->m_spriteram[offs + 2],state->m_spriteram[offs + 1],0);
            else
                /* 32x32 sprite */
			    drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
					    code >> 2, color,
					    0, ~state->m_spriteram[offs] & 0x40,
				    	256-state->m_spriteram[offs + 2],state->m_spriteram[offs + 1],0);
        }
	}
}


static void cosmica_draw_starfield( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 y = 0;
	UINT8 map = 0;
	UINT8 *PROM = screen.machine().region("user2")->base();

	while (1)
	{
		int va  =  y       & 0x01;
		int vb  = (y >> 1) & 0x01;

		UINT8 x = 0;

		while (1)
		{
			UINT8 x1;
			int hc, hb_;

			if (flip_screen_get(screen.machine()))
				x1 = x - screen.frame_number();
			else
				x1 = x + screen.frame_number();

			hc  = (x1 >> 2) & 0x01;
			hb_ = (x  >> 5) & 0x01;  /* not a bug, this one is the real x */

			if ((x1 & 0x1f) == 0)
				// flip-flop at IC11 is clocked
				map = PROM[(x1 >> 5) | (y >> 1 << 3)];

			if (((!(hc & va)) & (vb ^ hb_)) &&			/* right network */
			    (((x1 ^ map) & (hc | 0x1e)) == 0x1e))	/* left network */
			{
				/* RGB order is reversed -- bit 7=R, 6=G, 5=B */
				int col = (map >> 7) | ((map >> 5) & 0x02) | ((map >> 3) & 0x04);

				bitmap.pix16(y, x) = col;
			}

			x++;
			if (x == 0)  break;
		}

		y++;
		if (y == 0)  break;
	}
}


static void devzone_draw_grid( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 y;
	UINT8 *horz_PROM = machine.region("user2")->base();
	UINT8 *vert_PROM = machine.region("user3")->base();
	offs_t horz_addr = 0;

	UINT8 count = 0;
	UINT8 horz_data = 0;
	UINT8 vert_data;

	for (y = 32; y < 224; y++)
	{
		UINT8 x = 0;

		while (1)
		{
			int x1;

			/* for the vertical lines, each bit indicates
               if there should be a line at the x position */
			vert_data = vert_PROM[x >> 3];

			/* the horizontal (perspective) lines are RLE encoded.
               When the screen is flipped, the address should be
               decrementing.  But since it's just a mirrored image,
               this is easier. */
			if (count == 0)
				count = horz_PROM[horz_addr++];

			count++;

			if (count == 0)
				horz_data = horz_PROM[horz_addr++];

			for (x1 = 0; x1 < 8; x1++)
			{
				if (!(vert_data & horz_data & 0x80))	/* NAND gate */
				{
					/* blue */
					if (flip_screen_get(machine))
						bitmap.pix16(255-y, 255-x) = 4;
					else
						bitmap.pix16(y, x) = 4;
				}

				horz_data = (horz_data << 1) | 0x01;
				vert_data = (vert_data << 1) | 0x01;

				x++;
			}

			if (x == 0)  break;
		}
	}
}


static void nomnlnd_draw_background( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 y = 0;
	UINT8 water = screen.frame_number();
	UINT8 *PROM = screen.machine().region("user2")->base();

	/* all positioning is via logic gates:

       tree is displayed where
       __          __
       HD' ^ HC' ^ HB'

       and
        __          __              __
       (VB' ^ VC' ^ VD')  X  (VB' ^ VC' ^ VD')

       water is displayed where
             __         __
       HD' ^ HC' ^ HB ^ HA'

       and vertically the same equation as the trees,
       but the final result is inverted.


       The colors are coded in logic gates:

       trees:
                                P1 P2  BGR
         R = Plane1 ^ Plane2     0  0  000
         G = Plane2              0  1  010
         B = Plane1 ^ ~Plane2    1  0  100
                                 1  1  011
       water:
                                P1 P2  BGR or
         R = Plane1 ^ Plane2     0  0  100 000
         G = Plane2 v Plane2     0  1  110 010
         B = ~Plane1 or          1  0  010 010
             0 based oh HD       1  1  011 011

         Not sure about B, the logic seems convulated for such
         a simple result.
    */

	while (1)
	{
		int vb_ = (y >> 5) & 0x01;
		int vc_ = (y >> 6) & 0x01;
		int vd_ =  y >> 7;

		UINT8 x = 0;

		while (1)
		{
			int color = 0;

			int hd  = (x >> 3) & 0x01;
			int ha_ = (x >> 4) & 0x01;
			int hb_ = (x >> 5) & 0x01;
			int hc_ = (x >> 6) & 0x01;
			int hd_ =  x >> 7;

			if (((!vb_) & vc_ & (!vd_)) ^ (vb_ & (!vc_) & vd_))
			{
				/* tree */
				if ((!hd_) & hc_ & (!hb_))
				{
					offs_t offs = ((x >> 3) & 0x03) | ((y & 0x1f) << 2) |
					              (flip_screen_get(screen.machine()) ? 0x80 : 0);

					UINT8 plane1 = PROM[offs         ] << (x & 0x07);
					UINT8 plane2 = PROM[offs | 0x0400] << (x & 0x07);

					plane1 >>= 7;
					plane2 >>= 7;

					color = (plane1 & plane2)       |	// R
					        (plane2 		)  << 1 |	// G
					        (plane1 & !plane2) << 2;	// B
				}
			}
			else
			{
				/* water */
				if (hd_ & !hc_ & hb_ & !ha_)
				{
					offs_t offs = hd | (water << 1) | 0x0200;

					UINT8 plane1 = PROM[offs         ] << (x & 0x07);
					UINT8 plane2 = PROM[offs | 0x0400] << (x & 0x07);

					plane1 >>= 7;
					plane2 >>= 7;

					color = ( plane1 & plane2)      |	// R
					        ( plane1 | plane2) << 1 |	// G
					        ((!plane1) & hd)     << 2;	// B - see above
				}
			}

			if (color != 0)
			{
				if (flip_screen_get(screen.machine()))
					bitmap.pix16(255-y, 255-x) = color;
				else
					bitmap.pix16(y, x) = color;
			}

			x++;
			if (x == 0)  break;
		}


		// this is obviously wrong
//      if (vb_)
			water++;

		y++;
		if (y == 0)  break;
	}
}


SCREEN_UPDATE_IND16( cosmicg )
{
	bitmap.fill(0, cliprect);
	draw_bitmap(screen.machine(), bitmap, cliprect);
	return 0;
}


SCREEN_UPDATE_IND16( panic )
{
	bitmap.fill(0, cliprect);
	draw_bitmap(screen.machine(), bitmap, cliprect);
	draw_sprites(screen.machine(), bitmap, cliprect, 0x07, 1);
	return 0;
}


SCREEN_UPDATE_IND16( cosmica )
{
	bitmap.fill(0, cliprect);
	cosmica_draw_starfield(screen, bitmap, cliprect);
	draw_bitmap(screen.machine(), bitmap, cliprect);
	draw_sprites(screen.machine(), bitmap, cliprect, 0x0f, 0);
	return 0;
}


SCREEN_UPDATE_IND16( magspot )
{
	bitmap.fill(0, cliprect);
	draw_bitmap(screen.machine(), bitmap, cliprect);
	draw_sprites(screen.machine(), bitmap, cliprect, 0x07, 0);
	return 0;
}


SCREEN_UPDATE_IND16( devzone )
{
	cosmic_state *state = screen.machine().driver_data<cosmic_state>();

	bitmap.fill(0, cliprect);

	if (state->m_background_enable)
		devzone_draw_grid(screen.machine(), bitmap, cliprect);

	draw_bitmap(screen.machine(), bitmap, cliprect);
	draw_sprites(screen.machine(), bitmap, cliprect, 0x07, 0);
	return 0;
}


SCREEN_UPDATE_IND16( nomnlnd )
{
	cosmic_state *state = screen.machine().driver_data<cosmic_state>();

	/* according to the video summation logic on pg4, the trees and river
       have the highest priority */

	bitmap.fill(0, cliprect);
	draw_bitmap(screen.machine(), bitmap, cliprect);
	draw_sprites(screen.machine(), bitmap, cliprect, 0x07, 0);

	if (state->m_background_enable)
		nomnlnd_draw_background(screen, bitmap, cliprect);

	return 0;
}
