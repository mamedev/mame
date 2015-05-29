// license:???
// copyright-holders:Stefan Jokisch
/***************************************************************************

    video/liberatr.c

  Functions to emulate the video hardware of the machine.

   Liberator's screen is 256 pixels by 256 pixels.  The
     round planet in the middle of the screen is 128 pixels
     tall by 96 equivalent (192 at double pixel rate).  The
     emulator needs to account for the aspect ratio of 4/3
     from the arcade video system in order to make the planet
     appear round.

***************************************************************************/

#include "emu.h"
#include "includes/liberatr.h"


#define NUM_PENS    (0x18)




WRITE8_MEMBER( liberatr_state::bitmap_xy_w )
{
	m_videoram[(*m_ycoord << 8) | *m_xcoord] = data & 0xe0;
}


READ8_MEMBER( liberatr_state::bitmap_xy_r )
{
	return m_videoram[(*m_ycoord << 8) | *m_xcoord];
}


WRITE8_MEMBER( liberatr_state::bitmap_w )
{
	UINT8 x, y;

	m_bitmapram[offset] = data;

	x = (offset & 0x3f) << 2;
	y = offset >> 6;

	data = data & 0xe0;

	m_videoram[(y << 8) | x | 0] = data;
	m_videoram[(y << 8) | x | 1] = data;
	m_videoram[(y << 8) | x | 2] = data;
	m_videoram[(y << 8) | x | 3] = data;
}



/********************************************************************************************
  liberatr_init_planet()

  The data for the planet is stored in ROM using a run-length type of encoding.  This
  function does the conversion to the above structures and then a smaller
  structure which is quicker to use in real time.

  Its a multi-step process, reflecting the history of the code.  Not quite as efficient
  as it might be, but this is not realtime stuff, so who cares...
 ********************************************************************************************/

void liberatr_state::init_planet(planet &liberatr_planet, UINT8 *planet_rom)
{
	UINT16 longitude;

	const UINT8 *latitude_scale = memregion("user1")->base();
	const UINT8 *longitude_scale = memregion("user2")->base();

	/* for each starting longitude */
	for (longitude = 0; longitude < 0x100; longitude++)
	{
		UINT8 i, latitude, start_segment, segment_count;
		UINT8 *buffer;

		planet_frame frame;
		planet_frame_line *line = 0;

		UINT16 total_segment_count = 0;

		/* for each latitude */
		for (latitude = 0; latitude < 0x80; latitude++)
		{
			UINT8 segment, longitude_scale_factor, latitude_scale_factor, color, x=0;
			UINT8 x_array[32], color_array[32], visible_array[32];

			/* point to the structure which will hold the data for this line */
			line = &frame.lines[latitude];

			latitude_scale_factor = latitude_scale[latitude];

			/* for this latitude, load the 32 segments into the arrays */
			for (segment = 0; segment < 0x20; segment++)
			{
				UINT16 length, planet_data, address;

				/*
				   read the planet picture ROM and get the
				   latitude and longitude scaled from the scaling PROMS
				*/
				address = (latitude << 5) + segment;
				planet_data = (planet_rom[address] << 8) | planet_rom[address + 0x1000];

				color  =  (planet_data >> 8) & 0x0f;
				length = ((planet_data << 1) & 0x1fe) + ((planet_data >> 15) & 0x01);


				/* scale the longitude limit (adding the starting longitude) */
				address = longitude + ( length >> 1 ) + ( length & 1 );     /* shift with rounding */
				visible_array[segment] = (( address & 0x100 ) ? 1 : 0);
				if (address & 0x80)
				{
					longitude_scale_factor = 0xff;
				}
				else
				{
					address = ((address & 0x7f) << 1) + (((length & 1) || visible_array[segment]) ? 0 : 1);
					longitude_scale_factor = longitude_scale[address];
				}

				x_array[segment] = (((UINT16)latitude_scale_factor * (UINT16)longitude_scale_factor) + 0x80) >> 8;  /* round it */
				color_array[segment] = color;
			}

			/*
			   determine which segment is the western horizon and
			     leave 'segment' indexing it.
			*/
			for (segment = 0; segment < 0x1f; segment++)    /* if not found, 'segment' = 0x1f */
				if (visible_array[segment]) break;

			/* transfer from the temporary arrays to the structure */
			line->max_x = (latitude_scale_factor * 0xc0) >> 8;
			if (line->max_x & 1)
				line->max_x += 1;               /* make it even */

			/*
			   as part of the quest to reduce memory usage (and to a lesser degree
			     execution time), stitch together segments that have the same color
			*/
			segment_count = 0;
			i = 0;
			start_segment = segment;

			do
			{
				color = color_array[segment];
				while (color == color_array[segment])
				{
					x = x_array[segment];
					segment = (segment+1) & 0x1f;
					if (segment == start_segment)
						break;
				}

				line->color_array[i] = color;
				line->x_array[i] = (x > line->max_x) ? line->max_x : x;
				i++;
				segment_count++;
			} while ((i < 32) && (x <= line->max_x));

			total_segment_count += segment_count;
			line->segment_count = segment_count;
		}

		/* now that the all the lines have been processed, and we know how
		   many segments it will take to store the description, allocate the
		   space for it and copy the data to it.
		*/
		buffer = auto_alloc_array(machine(), UINT8, 2*(128 + total_segment_count));

		liberatr_planet.frames[longitude] = buffer;

		for (latitude = 0; latitude < 0x80; latitude++)
		{
			UINT8 last_x;


			line = &frame.lines[latitude];
			segment_count = line->segment_count;
			*buffer++ = segment_count;
			last_x = 0;

			/* calculate the bitmap's x coordinate for the western horizon
			   center of bitmap - (the number of planet pixels) / 4 */
			*buffer++ = (m_screen->width() / 2) - ((line->max_x + 2) / 4);

			for (i = 0; i < segment_count; i++)
			{
				UINT8 current_x = (line->x_array[i] + 1) / 2;

				*buffer++ = line->color_array[i];
				*buffer++ = current_x - last_x;

				last_x = current_x;
			}
		}
	}
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void liberatr_state::video_start()
{
	// for each planet in the planet ROMs
	init_planet(m_planets[0], &memregion("gfx1")->base()[0x2000]);
	init_planet(m_planets[1], &memregion("gfx1")->base()[0x0000]);
}


void liberatr_state::get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		UINT8 r,g,b;

		/* handle the hardware flip of the bit order from 765 to 576 that
		   hardware does between vram and color ram */
		static const offs_t penmap[] = { 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
									0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
									0x10, 0x12, 0x14, 0x16, 0x11, 0x13, 0x15, 0x17 };

		UINT8 data = m_colorram[i];

		/* scale it from 0x00-0xff */
		r = ((~data >> 3) & 0x07) * 0x24 + 3;  if (r == 3)  r = 0;
		g = ((~data >> 0) & 0x07) * 0x24 + 3;  if (g == 3)  g = 0;
		b = ((~data >> 5) & 0x06) * 0x24 + 3;  if (b == 3)  b = 0;

		pens[penmap[i]] = rgb_t(r, g, b);
	}
}


void liberatr_state::draw_planet(bitmap_rgb32 &bitmap, pen_t *pens)
{
	UINT8 latitude;

	UINT8 *buffer = m_planets[(*m_planet_select >> 4) & 0x01].frames[*m_planet_frame];

	/* for each latitude */
	for (latitude = 0; latitude < 0x80; latitude++)
	{
		UINT8 segment;

		/* grab the color value for the base (if any) at this latitude */
		UINT8 base_color = m_base_ram[latitude >> 3] ^ 0x0f;

		UINT8 segment_count = *buffer++;
		UINT8 x = *buffer++;
		UINT8 y = 64 + latitude;

		/* run through the segments, drawing its color until its x_array value comes up. */
		for (segment = 0; segment < segment_count; segment++)
		{
			UINT8 i;

			UINT8 color = *buffer++;
			UINT8 segment_length = *buffer++;

			if ((color & 0x0c) == 0x0c)
				color = base_color;

			for (i = 0; i < segment_length; i++, x++)
				bitmap.pix32(y, x) = pens[color];
		}
	}
}


void liberatr_state::draw_bitmap(bitmap_rgb32 &bitmap, pen_t *pens)
{
	offs_t offs;

	for (offs = 0; offs < 0x10000; offs++)
	{
		UINT8 data = m_videoram[offs];

		UINT8 y = offs >> 8;
		UINT8 x = offs & 0xff;

		if (data)
			bitmap.pix32(y, x) = pens[(data >> 5) | 0x10];
	}
}


UINT32 liberatr_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[NUM_PENS];
	get_pens(pens);

	bitmap.fill(rgb_t::black, cliprect);
	draw_planet(bitmap, pens);
	draw_bitmap(bitmap, pens);

	return 0;
}
