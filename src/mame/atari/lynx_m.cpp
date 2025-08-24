// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
 PeT mess@utanet.at 2000,2001
******************************************************************************/

#include "emu.h"
#include "lynx.h"
#include "cpu/m6502/g65sc02.h"

#include "corestr.h"
#include "render.h"

#define PAD_UP      0x80
#define PAD_DOWN    0x40
#define PAD_LEFT    0x20
#define PAD_RIGHT   0x10


/****************************************

    Graphics Drawing

****************************************/

/* modes from blitter command */
enum {
	BACKGROUND = 0,
	BACKGROUND_NO_COLL,
	BOUNDARY_SHADOW,
	BOUNDARY,
	NORMAL_SPRITE,
	NO_COLL,
	XOR_SPRITE,
	SHADOW
};

/* The pen numbers range from '0' to 'F. Pen numbers '1' through 'D' are always collidable and opaque. The other
ones have different behavior depending on the sprite type: there are 8 types of sprites, each has different
characteristics relating to some or all of their pen numbers.

* Shadow Error: The hardware is missing an inverter in the 'shadow' generator. This causes sprite types that
did not invoke shadow to now invoke it and vice versa. The only actual functionality loss is that 'exclusive or'
sprites and 'background' sprites will have shadow enabled.

The sprite types relate to specific hardware functions according to the following table:


   -------------------------- SHADOW
  |   ----------------------- BOUNDARY_SHADOW
  |  |   -------------------- NORMAL_SPRITE
  |  |  |   ----------------- BOUNDARY
  |  |  |  |   -------------- BACKGROUND (+ shadow, due to bug in 'E' pen)
  |  |  |  |  |   ----------- BACKGROUND_NO_COLL
  |  |  |  |  |  |   -------- NO_COLL
  |  |  |  |  |  |  |   ----- XOR_SPRITE (+ shadow, due to bug in 'E' pen)
  |  |  |  |  |  |  |  |
  1  0  1  0  1  1  1  1      F is opaque
  0  0  1  1  0  0  0  0      E is collideable
  0  0  1  1  0  0  0  0      0 is opaque and collideable
  1  1  1  1  0  0  0  1      allow collision detect
  1  1  1  1  1  0  0  1      allow coll. buffer access
  0  0  0  0  0  0  0  1      exclusive-or the data
*/

inline void lynx_state::plot_pixel(const s16 x, const s16 y, const u8 color)
{
	u8 back;

	m_blitter.everon = true;
	const u16 screen = m_blitter.screen + y * 80 + x / 2;
	const u16 colbuf = m_blitter.colbuf + y * 80 + x / 2;

	/* a note on timing: The hardware packs the pixel data and updates the screen and collision buffer a byte at a time.
	Thus the buffer update for two pixels takes 3 memory accesses for a normal sprite (write to screen buffer, read/write to collision buffer).
	+1 memory access for palette fetch?
	*/

	switch (m_blitter.mode & 0x7)
	{
		case NORMAL_SPRITE:
		/* A sprite may be set to 'normal'. This means that pen number '0' will be transparent and
		non-collideable. All other pens will be opaque and collideable */
			if (color == 0)
				break;
			if (!(x & 0x01))        /* Upper nibble */
			{
				dram_byte_w(screen, color << 4, 0xf0);
				m_blitter.memory_accesses++;

				if (m_blitter.SPRITE_COLLIDE())
				{
					back = BIT(dram_byte_r(colbuf), 4, 4);
					if (back > m_blitter.fred)
						m_blitter.fred = back;
					dram_byte_w(colbuf, m_blitter.spritenr << 4, 0xf0);
					m_blitter.memory_accesses += 2;
				}
				m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				dram_byte_w(screen, color, 0x0f);

				if (m_blitter.SPRITE_COLLIDE())
				{
					back = BIT(dram_byte_r(colbuf), 0, 4);
					if (back > m_blitter.fred)
						m_blitter.fred = back;
					dram_byte_w(colbuf, m_blitter.spritenr, 0x0f);
				}
			}
			break;

		case BOUNDARY:
		/* A sprite may be set to 'boundary'. This is a 'normal' sprite with the exception that pen
		number 'F' is transparent (and still collideable). */
			if (color == 0)
				break;
			if (!(x & 0x01))        /* Upper nibble */
			{
				if (color != 0x0f)
				{
					dram_byte_w(screen, color << 4, 0xf0);
					m_blitter.memory_accesses++;
				}
				if (m_blitter.SPRITE_COLLIDE())
				{
					back = BIT(dram_byte_r(colbuf), 4, 4);
					if (back > m_blitter.fred)
						m_blitter.fred = back;
					dram_byte_w(colbuf, m_blitter.spritenr << 4, 0xf0);
					m_blitter.memory_accesses += 2;
				}
				m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				if (color != 0x0f)
				{
					dram_byte_w(screen, color, 0x0f);
				}
				if (m_blitter.SPRITE_COLLIDE())
				{
					back = BIT(dram_byte_r(colbuf), 0, 4);
					if (back > m_blitter.fred)
						m_blitter.fred = back;
					dram_byte_w(colbuf, m_blitter.spritenr, 0x0f);
				}
			}
			break;

		case SHADOW:
		/* A sprite may be set to 'shadow'. This is a 'normal' sprite with the exception that pen
		number 'E' is non-collideable (but still opaque) */
			if (color == 0)
				break;
			if (!(x & 0x01))        /* Upper nibble */
			{
				dram_byte_w(screen, color << 4, 0xf0);
				m_blitter.memory_accesses++;

				if (m_blitter.SPRITE_COLLIDE() && (color != 0x0e))
				{
					back = BIT(dram_byte_r(colbuf), 4, 4);
					if (back > m_blitter.fred)
						m_blitter.fred = back;
					dram_byte_w(colbuf, m_blitter.spritenr << 4, 0xf0);
					m_blitter.memory_accesses += 2;
				}
				m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				dram_byte_w(screen, color, 0x0f);

				if (m_blitter.SPRITE_COLLIDE() && (color != 0x0e))
				{
					back = BIT(dram_byte_r(colbuf), 0, 4);
					if (back > m_blitter.fred)
						m_blitter.fred = back;
					dram_byte_w(colbuf, m_blitter.spritenr, 0x0f);
				}
			}
			break;

		case BOUNDARY_SHADOW:
		/* This sprite is a 'normal' sprite with the characteristics of both 'boundary'
		and 'shadow'. That is, pen number 'F' is transparent (and still collideable) and
		pen number 'E' is non-collideable (but still opaque). */
			if (color == 0)
				break;
			if (!(x & 0x01))        /* Upper nibble */
			{
				if (color != 0x0f)
				{
					dram_byte_w(screen, color << 4, 0xf0);
					m_blitter.memory_accesses++;
				}
				if (m_blitter.SPRITE_COLLIDE() && (color != 0x0e))
				{
					back = BIT(dram_byte_r(colbuf), 4, 4);
					if (back > m_blitter.fred)
						m_blitter.fred = back;
					dram_byte_w(colbuf, m_blitter.spritenr << 4, 0xf0);
					m_blitter.memory_accesses += 2;
				}
				m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				if (color != 0x0f)
				{
					dram_byte_w(screen, color, 0x0f);
				}
				if (m_blitter.SPRITE_COLLIDE() && (color != 0x0e))
				{
					back = BIT(dram_byte_r(colbuf), 0, 4);
					if (back > m_blitter.fred)
						m_blitter.fred = back;
					dram_byte_w(colbuf, m_blitter.spritenr, 0x0f);
				}
			}
			break;

		case BACKGROUND:
		/* A sprite may be set to 'background'. This sprite will overwrite the contents of the video and
		collision buffers. Pens '0' and 'F' are no longer transparent. This sprite is used to initialize
		the buffers at the start of a 'painting'. Additionally, no collision detection is done, and no write
		to the collision depository occurs. The 'E' error will cause the pen number 'E' to be non-collideable
		and therefore not clear the collision buffer */
			if (!(x & 0x01))        /* Upper nibble */
			{
				dram_byte_w(screen, color << 4, 0xf0);
				m_blitter.memory_accesses++;

				if (m_blitter.SPRITE_COLLIDE() && (color != 0x0e))
				{
					dram_byte_w(colbuf, m_blitter.spritenr << 4, 0xf0);
					m_blitter.memory_accesses++;
				}
				m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				dram_byte_w(screen, color, 0x0f);

				if (m_blitter.SPRITE_COLLIDE() && (color != 0x0e))
				{
					dram_byte_w(colbuf, m_blitter.spritenr, 0x0f);
				}
			}
			break;

		case BACKGROUND_NO_COLL:
		/* This is a 'background' sprite with the exception that no activity occurs in the collision buffer */
			if (!(x & 0x01))        /* Upper nibble */
			{
				dram_byte_w(screen, color << 4, 0xf0);
				m_blitter.memory_accesses++;
				m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				dram_byte_w(screen, color, 0x0f);
			}
			break;

		case NO_COLL:
		/* A sprite may be set to 'non-collideable'. This means that it will have no affect on the contents of
		the collision buffer and all other collision activities are overridden (pen 'F' is not collideable). */
			if (color == 0)
				break;
			if (!(x & 0x01))        /* Upper nibble */
			{
				dram_byte_w(screen, color << 4, 0xf0);
				m_blitter.memory_accesses++;
				m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				dram_byte_w(screen, color, 0x0f);
			}
			break;

		case XOR_SPRITE:
		/* This is a 'normal' sprite with the exception that the data from the video buffer is exclusive-ored
		with the sprite data and written back out to the video buffer. Collision activity is 'normal'. The 'E'
		error will cause the pen number 'E' to be non-collideable and therefore not react with the collision
		buffer */
			if (color == 0)
				break;
			if (!(x & 0x01))        /* Upper nibble */
			{
				dram_byte_w(screen, dram_byte_r(screen) ^ (color << 4), 0xf0);
				m_blitter.memory_accesses += 2;
				if (m_blitter.SPRITE_COLLIDE() && (color != 0x0e))
				{
					back = BIT(dram_byte_r(colbuf), 4, 4);
					if (back > m_blitter.fred)
						m_blitter.fred = back;
					dram_byte_w(colbuf, m_blitter.spritenr << 4, 0xf0);
					m_blitter.memory_accesses += 2;
				}
				m_blitter.memory_accesses++;
			}
			else                    /* Lower nibble */
			{
				dram_byte_w(screen, dram_byte_r(screen) ^ color, 0x0f);
				if (m_blitter.SPRITE_COLLIDE() && (color != 0x0e))
				{
					back = BIT(dram_byte_r(colbuf), 0, 4);
					if (back > m_blitter.fred)
						m_blitter.fred = back;
					dram_byte_w(colbuf, m_blitter.spritenr, 0x0f);
				}
			}
			break;
	}
}

void lynx_state::blit_do_work(const s16 y, const int xdir, const int bits_per_pixel, const u8 mask)
{
	int i,j;
	int xi, bits, color;
	u16 buffer;

	const int next_line_addr = dram_byte_r(m_blitter.bitmap); // offset to second sprite line
	u16 width_accum = (xdir == 1) ? m_blitter.width_offset : 0;
	m_blitter.memory_accesses++;

	for (xi = m_blitter.x_pos - m_blitter.xoff, bits = 0, buffer = 0, j = 1; j < next_line_addr; j++)
	{
		buffer = (buffer << 8) | dram_byte_r(m_blitter.bitmap + j);
		bits += 8; // current bits in buffer
		m_blitter.memory_accesses++;

		for (; bits > bits_per_pixel; bits -= bits_per_pixel) // last data packet at end of scanline is not rendered (qix, blulght)
		{
			color = m_blitter.color[(buffer >> (bits - bits_per_pixel)) & mask];
			width_accum += m_blitter.width;
			for (i = 0; i < (width_accum >> 8); i++, xi += xdir)
			{
				if ((xi >= 0) && (xi < 160))
				{
					plot_pixel(xi, y, color);
				}
			}
			width_accum &= 0xff;
		}
	}
}

void lynx_state::blit_rle_do_work(const s16 y, const int xdir, const int bits_per_pixel, const u8 mask)
{
	int i;
	int xi;
	int buffer, bits, j;
	int literal_data, count, color;

	const int next_line_addr = dram_byte_r(m_blitter.bitmap); // offset to second sprite line
	u16 width_accum = (xdir == 1) ? m_blitter.width_offset : 0;
	for (bits = 0, j = 0, buffer = 0, xi = m_blitter.x_pos - m_blitter.xoff; ;)      /* through the rle entries */
	{
		if (bits < 5 + bits_per_pixel) /* under 7 bits no complete entry */
		{
			j++;
			if (j >= next_line_addr)
				return;

			bits += 8;
			buffer = (buffer << 8) | dram_byte_r(m_blitter.bitmap + j);
			m_blitter.memory_accesses++;
		}

		literal_data = (buffer >> (bits - 1)) & 0x01;
		bits--;
		count = (buffer >> (bits - 4)) & 0x0f; // repeat count (packed) or pixel count (literal)
		bits -= 4;

		if (literal_data)       /* count of different pixels */
		{
			for (; count >= 0; count--)
			{
				if (bits < bits_per_pixel)
				{
					j++;
					if (j >= next_line_addr)
						return;
					bits += 8;
					buffer = (buffer << 8) | dram_byte_r(m_blitter.bitmap + j);
					m_blitter.memory_accesses++;
				}

				color = m_blitter.color[(buffer >> (bits - bits_per_pixel)) & mask];
				bits -= bits_per_pixel;
				width_accum += m_blitter.width;
				for (i = 0; i < (width_accum >> 8); i++, xi += xdir)
				{
					if ((xi >= 0) && (xi < 160))
						plot_pixel(xi, y, color);
				}
				width_accum &= 0xff;
			}
		}
		else        /* count of same pixels */
		{
			if (count == 0) // 4 bit count value of zero indicates end-of-line in a packed sprite
				return;

			if (bits < bits_per_pixel)
			{
				j++;
				if (j >= next_line_addr)
					return;
				bits += 8;
				buffer = (buffer << 8) | dram_byte_r(m_blitter.bitmap + j);
				m_blitter.memory_accesses++;
			}

			color = m_blitter.color[(buffer >> (bits - bits_per_pixel)) & mask];
			bits -= bits_per_pixel;

			for (; count >= 0; count--)
			{
				width_accum += m_blitter.width;
				for (i = 0; i < (width_accum >> 8); i++, xi += xdir)
				{
					if ((xi >= 0) && (xi < 160))
						plot_pixel(xi, y, color);
				}
				width_accum &= 0xff;
			}
		}
	}
}

void lynx_state::blit_lines()
{
	static const u8 lynx_color_masks[4] = { 0x01, 0x03, 0x07, 0x0f };
	s16 y;
	int i;
	int ydir = 0, xdir = 0;
	int flip = 0;

	m_blitter.everon = false;

	switch (m_blitter.DRAW_ORIGIN())   /* Initial drawing direction */
	{
		case 0: // Down/Right (quadrant 0)
			xdir = 1;
			ydir = 1;
			flip = 0;
			break;
		case 1: // Down/Left (blockout) (quadrant 3)
			xdir = -1;
			ydir = 1;
			flip = 3;
			break;
		case 2: // Up/Right (fat bobby) (quadrant 1)
			xdir = 1;
			ydir = -1;
			flip = 1;
			break;
		case 3: // Up/Left (quadrant 2)
			xdir = -1;
			ydir = -1;
			flip = 2;
			break;
	}

	if (m_blitter.HFLIP())   /* Horizontal Flip */
	{
		xdir *= -1;
	}

	if (m_blitter.VFLIP())   /* Vertical Flip */
	{
		ydir *= -1;
	}

	// Set height accumulator based on drawing direction
	m_blitter.height_accumulator = (ydir == 1) ? m_blitter.height_offset : 0x00;

	// loop through lines, next line offset of zero indicates end of sprite
	for (y = m_blitter.y_pos - m_blitter.yoff; (i = dram_byte_r(m_blitter.bitmap)); m_blitter.bitmap += i)
	{
		m_blitter.memory_accesses++;

		if (i == 1) // draw next quadrant
		{
			// centered sprites sprdemo3, fat bobby, blockout
			switch (flip & 0x03)
			{
				case 0:
				case 2:
					ydir *= -1;
					m_blitter.y_pos += ydir;
					break;
				case 1:
				case 3:
					xdir *= -1;
					m_blitter.x_pos += xdir;
					break;
			}
			flip++;
			y = m_blitter.y_pos - m_blitter.yoff;
			m_blitter.height_accumulator = (ydir == 1) ? m_blitter.height_offset : 0x00;
			continue;
		}

		m_blitter.height_accumulator += m_blitter.height;
		for (int j = 0; j < (m_blitter.height_accumulator >> 8); j++, y += ydir)
		{
			if (y >= 0 && y < 102)
			{
				if (m_blitter.use_rle)
					blit_rle_do_work(y, xdir, m_blitter.line_color + 1, lynx_color_masks[m_blitter.line_color]);
				else
					blit_do_work(y, xdir, m_blitter.line_color + 1, lynx_color_masks[m_blitter.line_color]);
			}
			m_blitter.width += (s16)m_blitter.stretch;
			if (m_blitter.vstretch) // doesn't seem to be used
			{
				m_blitter.height += (s16)m_blitter.stretch;
				logerror("vertical stretch enabled");
			}
			m_blitter.tilt_accumulator += m_blitter.tilt;
			m_blitter.x_pos += (m_blitter.tilt_accumulator >> 8);
			m_blitter.tilt_accumulator &= 0xff;
		}
		m_blitter.height_accumulator &= 0xff;
	}
}

TIMER_CALLBACK_MEMBER(lynx_state::blitter_timer)
{
	m_blitter.busy = false; // blitter finished
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}

/*
  control 0
   bit 7,6: 00 2 color
            01 4 color
            11 8 colors?
            11 16 color
   bit 5,4: 00 right down
            01 right up
            10 left down
            11 left up

#define SHADOW         (0x07)
#define XORSHADOW      (0x06)
#define NONCOLLIDABLE  (0x05)
#define NORMAL         (0x04)
#define BOUNDARY       (0x03)
#define BOUNDARYSHADOW (0x02)
#define BKGRNDNOCOL    (0x01)
#define BKGRND         (0x00)

  control 1
   bit 7: 0 bitmap rle encoded
          1 not encoded
   bit 3: 0 color info with command
          1 no color info with command

#define RELHVST        (0x30)
#define RELHVS         (0x20)
#define RELHV          (0x10)

#define SKIPSPRITE     (0x04)

#define DUP            (0x02)
#define DDOWN          (0x00)
#define DLEFT          (0x01)
#define DRIGHT         (0x00)


  coll
#define DONTCOLLIDE    (0x20)

  word next
  word data
  word x
  word y
  word width
  word height

  pixel c0 90 20 0000 datapointer x y 0100 0100 color (8 colorbytes)
  4 bit direct?
  datapointer 2 10 0
  98 (0 colorbytes)

  box c0 90 20 0000 datapointer x y width height color
  datapointer 2 10 0

  c1 98 00 4 bit direct without color bytes (raycast)

  40 10 20 4 bit rle (sprdemo2)

  line c1 b0 20 0000 datapointer x y 0100 0100 stretch tilt:x/y color (8 color bytes)
  or
  line c0 b0 20 0000 datapointer x y 0100 0100 stretch tilt:x/y color
  datapointer 2 11 0

  text ?(04) 90 20 0000 datapointer x y width height color
  datapointer 2 10 0

  stretch: hsize adder
  tilt: hpos adder

*/

void lynx_state::blitter()
{
	static const u8 lynx_colors[4] = { 2, 4, 8, 16 };
	u8 palette_offset;

	m_blitter.busy = 1; // blitter working
	m_blitter.memory_accesses = 0;

	// Last SCB is indicated by zero in the high byte of SCBNEXT
	while (m_blitter.scb_next & 0xff00)
	{
		m_blitter.stretch = 0;
		m_blitter.tilt    = 0;
		m_blitter.tilt_accumulator = 0;

		m_blitter.scb = m_blitter.scb_next; // current scb
		m_blitter.scb_next = dram_word_r(m_blitter.scb + SCB_SCBNEXT); // next scb
		m_blitter.spr_ctl0 = dram_byte_r(m_blitter.scb + SCB_SPRCTL0);
		m_blitter.spr_ctl1 = dram_byte_r(m_blitter.scb + SCB_SPRCTL1);
		m_blitter.spr_coll = dram_byte_r(m_blitter.scb + SCB_SPRCOLL);
		m_blitter.memory_accesses += 5;

		if (!(m_blitter.SKIP_SPRITE())) // sprite will be processed (if sprite is skipped first 5 bytes are still copied to suzy)
		{
			m_blitter.bitmap = dram_word_r(m_blitter.scb + SCB_SPRDLINE);
			m_blitter.x_pos = dram_word_r(m_blitter.scb + SCB_HPOSSTRT);
			m_blitter.y_pos = dram_word_r(m_blitter.scb + SCB_VPOSSTRT);
			m_blitter.memory_accesses += 6;

			switch (m_blitter.RELOAD_SCALE()) // reload sprite scaling
			{
				case 0b11: // width, height, tilt, stretch
					m_blitter.tilt = dram_word_r(m_blitter.scb + SCB_TILT);
					m_blitter.memory_accesses += 2;
					[[fallthrough]];
				case 0b10: // width, height, stretch
					m_blitter.stretch = dram_word_r(m_blitter.scb + SCB_STRETCH);
					m_blitter.memory_accesses += 2;
					[[fallthrough]];
				case 0b01: // width, height
					m_blitter.width = dram_word_r(m_blitter.scb + SCB_SPRHSIZ);
					m_blitter.height = dram_word_r(m_blitter.scb + SCB_SPRVSIZ);
					m_blitter.memory_accesses += 4;
			}

			if (!(m_blitter.REUSE_PALETTE())) // reload palette
			{
				if (m_blitter.RELOAD_SCALE() != 0)
					palette_offset = 0x0b + 2 * (m_blitter.RELOAD_SCALE() + 1); // palette data offset depends on width, height, etc. reloading
				else
					palette_offset = 0x0b;

				u8 colors = lynx_colors[m_blitter.BPP()];

				for (int i = 0; i < colors / 2; i++)
				{
					m_blitter.color[i * 2]     = dram_byte_r(m_blitter.scb + palette_offset + i) >> 4;
					m_blitter.color[i * 2 + 1] = dram_byte_r(m_blitter.scb + palette_offset + i) & 0x0f;
					m_blitter.memory_accesses++;
				}
			}
		}


		if (!(m_blitter.SKIP_SPRITE()))        // if 0, we skip this sprite
		{
			m_blitter.colpos = m_blitter.scb + (m_suzy.data[COLLOFFL] | (m_suzy.data[COLLOFFH] << 8));
			m_blitter.mode = m_blitter.SPRITE_TYPE();
			m_blitter.use_rle = !(m_blitter.RLE());
			m_blitter.line_color = m_blitter.BPP();

			m_blitter.sprite_collide = !(m_blitter.SPRITE_COLLEN());
			m_blitter.spritenr = m_blitter.SPRITE_COLNUM();
			m_blitter.fred = 0;

			/* Draw Sprite */
			blit_lines();

			if (m_blitter.SPRITE_COLLIDE())
			{
				switch (m_blitter.mode)
				{
					case BOUNDARY_SHADOW:
					case BOUNDARY:
					case NORMAL_SPRITE:
					case XOR_SPRITE:
					case SHADOW:
						dram_byte_w(m_blitter.colpos, m_blitter.fred);
						break;
				}
			}

			if (m_suzy.EVER_ON()) // Everon enabled
				dram_byte_w(m_blitter.colpos, m_blitter.everon ? 0x00 : 0x80, 0x80);
		}
	}

	m_blitter_timer->adjust(m_maincpu->cycles_to_attotime(m_blitter.memory_accesses));
}


/****************************************

    Suzy Emulation

****************************************/


/* Math bugs of the original hardware:

- in signed multiply, the hardware thinks that 8000 is a positive number
- in signed multiply, the hardware thinks that 0 is a negative number. This is not an immediate
problem for a multiply by zero, since the answer will be re-negated to the correct polarity of
zero. However, since it will set the sign flag, you can not depend on the sign flag to be correct
if you just load the lower byte after a multiply by zero.
- in divide, the remainder will have 2 possible errors, depending on its actual value (no further
notes on these errors available) */

void lynx_state::suzy_divide()
{
	u32 left;
	u16 right;
	u32 res, mod;
	/*
	Hardware divide:
	            EFGH
	*             NP
	----------------
	            ABCD
	Remainder (JK)LM
	*/

	left = m_suzy.data[MATH_H] | (m_suzy.data[MATH_G] << 8) | (m_suzy.data[MATH_F] << 16) | (m_suzy.data[MATH_E] << 24);
	right = m_suzy.data[MATH_P] | (m_suzy.data[MATH_N] << 8);

	m_suzy.accumulate_overflow = false;
	if (right == 0)
	{
		m_suzy.accumulate_overflow = true;  /* during divisions, this bit is used to detect denominator = 0 */
		res = 0xffffffff;
		mod = 0; //?
	}
	else
	{
		res = left / right;
		mod = left % right;
	}
//  logerror("coprocessor %8x / %8x = %4x\n", left, right, res);
	m_suzy.data[MATH_D] = res & 0xff;
	m_suzy.data[MATH_C] = res >> 8;
	m_suzy.data[MATH_B] = res >> 16;
	m_suzy.data[MATH_A] = res >> 24;

	m_suzy.data[MATH_M] = mod & 0xff;
	m_suzy.data[MATH_L] = mod >> 8;
	m_suzy.data[MATH_K] = 0; // documentation states the hardware sets these to zero on divides
	m_suzy.data[MATH_J] = 0;
}

void lynx_state::suzy_multiply()
{
	u16 left, right;
	u32 res, accu;
	/*
	Hardware multiply:
	              AB
	*             CD
	----------------
	            EFGH
	Accumulate  JKLM
	*/
	m_suzy.accumulate_overflow = false;

	left = m_suzy.data[MATH_B] | (m_suzy.data[MATH_A] << 8);
	right = m_suzy.data[MATH_D] | (m_suzy.data[MATH_C] << 8);

	res = left * right;

	if (m_suzy.signed_math)
	{
		if (!(m_sign_AB + m_sign_CD))   /* different signs */
			res = (res ^ 0xffffffff) + 1;
	}

	m_suzy.data[MATH_H] = res & 0xff;
	m_suzy.data[MATH_G] = res >> 8;
	m_suzy.data[MATH_F] = res >> 16;
	m_suzy.data[MATH_E] = res >> 24;

	if (m_suzy.accumulate)
	{
		accu = m_suzy.data[MATH_M] | m_suzy.data[MATH_L] << 8 | m_suzy.data[MATH_K] << 16 | m_suzy.data[MATH_J] << 24;
		accu += res;

		if (accu < res)
			m_suzy.accumulate_overflow = true;

		m_suzy.data[MATH_M] = accu;
		m_suzy.data[MATH_L] = accu >> 8;
		m_suzy.data[MATH_K] = accu >> 16;
		m_suzy.data[MATH_J] = accu >> 24;
	}
}

u8 lynx_state::suzy_read(offs_t offset)
{
	u8 value = 0, input;

	switch (offset)
	{
		case TILTACUML:
			return m_blitter.tilt_accumulator & 0xff;
		case TILTACUMH:
			return m_blitter.tilt_accumulator >> 8;
		case HOFFL:
			return m_blitter.xoff & 0xff;
		case HOFFH:
			return m_blitter.xoff >> 8;
		case VOFFL:
			return m_blitter.yoff & 0xff;
		case VOFFH:
			return m_blitter.yoff >> 8;
		case VIDBASL:
			return m_blitter.screen & 0xff;
		case VIDBASH:
			return m_blitter.screen >> 8;
		case COLLBASL:
			return m_blitter.colbuf & 0xff;
		case COLLBASH:
			return m_blitter.colbuf >> 8;
		case SCBNEXTL:
			return m_blitter.scb_next & 0xff;
		case SCBNEXTH:
			return m_blitter.scb_next >> 8;
		case SPRDLINEL:
			return m_blitter.bitmap & 0xff;
		case SPRDLINEH:
			return m_blitter.bitmap >> 8;
		case HPOSSTRTL:
			return m_blitter.x_pos & 0xff;
		case HPOSSTRTH:
			return m_blitter.x_pos >> 8;
		case VPOSSTRTL:
			return m_blitter.y_pos & 0xff;
		case VPOSSTRTH:
			return m_blitter.y_pos >> 8;
		case SPRHSIZL:
			return m_blitter.width & 0xff;
		case SPRHSIZH:
			return m_blitter.width >> 8;
		case SPRVSIZL:
			return m_blitter.height & 0xff;
		case SPRVSIZH:
			return m_blitter.height >> 8;
		case STRETCHL:
			return m_blitter.stretch & 0xff;
		case STRETCHH:
			return m_blitter.stretch >> 8;
		case TILTL:
			return m_blitter.tilt & 0xff;
		case TILTH:
			return m_blitter.tilt >> 8;
		// case SPRDOFFL:
		// case SPRVPOSL:
		// case COLLOFFL:
		case VSIZACUML:
			return m_blitter.height_accumulator & 0xff;
		case VSIZACUMH:
			return m_blitter.height_accumulator >> 8;
		case HSIZOFFL:
			return m_blitter.width_offset & 0xff;
		case HSIZOFFH:
			return m_blitter.width_offset >> 8;
		case VSIZOFFL:
			return m_blitter.height_offset & 0xff;
		case VSIZOFFH:
			return m_blitter.height_offset >> 8;
		case SCBADRL:
			return m_blitter.scb & 0xff;
		case SCBADRH:
			return m_blitter.scb >> 8;
		//case PROCADRL:
		case SUZYHREV: // Suzy hardware revision
			return 0x01; // must not be 0 for correct power up
		//case 0x89: // SUZYSREV Suzy software revision
		case SPRSYS:
			// math busy, last carry, unsafe access, and stop on current sprite bits not implemented.
			if (m_suzy.accumulate_overflow)
				value |= 0x40;
			if (m_blitter.vstretch)
				value |= 0x10;
			if (m_blitter.lefthanded)
				value |= 0x08;
			if (m_blitter.busy)
				value |= 0x01;
			break;
		case JOYSTICK:
			input = m_joy_io->read();
			switch (m_rotate)
			{
				case 1:
					value = input;
					input &= 0x0f;
					if (value & PAD_UP) input |= PAD_LEFT;
					if (value & PAD_LEFT) input |= PAD_DOWN;
					if (value & PAD_DOWN) input |= PAD_RIGHT;
					if (value & PAD_RIGHT) input |= PAD_UP;
					break;
				case 2:
					value = input;
					input &= 0x0f;
					if (value & PAD_UP) input |= PAD_RIGHT;
					if (value & PAD_RIGHT) input |= PAD_DOWN;
					if (value & PAD_DOWN) input |= PAD_LEFT;
					if (value & PAD_LEFT) input |= PAD_UP;
					break;
			}
			if (m_blitter.lefthanded)
			{
				value = input & 0x0f;
				if (input & PAD_UP) value |= PAD_DOWN;
				if (input & PAD_DOWN) value |= PAD_UP;
				if (input & PAD_LEFT) value |= PAD_RIGHT;
				if (input & PAD_RIGHT) value |= PAD_LEFT;
			}
			else
				value = input;
			break;
		case SWITCHES:
			value = m_pause_io->read();
			break;
		case RCART: // connected to CE0 in cartridge slot
			if (m_cart->exists())
				value = m_cart->read_rom(get_cart_addr());
			else
				value = 0;
			if (!machine().side_effects_disabled())
				m_cart_addr_counter = (m_cart_addr_counter + 1) & (m_granularity - 1); // 4040 clock
			break;
		//case RCART_BANK1:  // connected to CE1 in cartridge slot /* we need bank 1 emulation!!! */
		case SPRCTL0:
		case SPRCTL1:
		case SPRCOLL:
		case SPRINIT:
		case SUZYBUSEN:
		case SPRGO:
			logerror("read from write only register %x\n", offset);
			value = 0;
			break;
		default:
			value = m_suzy.data[offset];
	}
	//logerror("suzy read %.2x %.2x\n",offset,value);
	return value;
}

void lynx_state::suzy_write(offs_t offset, u8 data)
{
	m_suzy.data[offset] = data;
	//logerror("suzy write %.2x %.2x\n",offset,data);
	/* Additional effects of a write */
	/* Even addresses are the LSB. Any CPU write to an LSB in 0x00-0x7f will set the MSB to 0. */
	/* This in particular holds for math quantities:  Writing to B (0x54), D (0x52),
	F (0x62), H (0x60), K (0x6e) or M (0x6c) will force a '0' to be written to A (0x55),
	C (0x53), E (0x63), G (0x61), J (0x6f) or L (0x6d) respectively */
	if ((offset < 0x80) && !(offset & 0x01))
	m_suzy.data[offset + 1] = 0;

	switch (offset)
	{
		//case TMPADRL:
		//case TMPADRH:
		case TILTACUML:
			m_blitter.tilt_accumulator = data; // upper byte forced to zero see above.
			break;
		case TILTACUMH:
			m_blitter.tilt_accumulator &= 0xff;
			m_blitter.tilt_accumulator |= data << 8;
			break;
		case HOFFL:
			m_blitter.xoff = data;
			break;
		case HOFFH:
			m_blitter.xoff &= 0xff;
			m_blitter.xoff |= data << 8;
			break;
		case VOFFL:
			m_blitter.yoff = data;
			break;
		case VOFFH:
			m_blitter.yoff &= 0xff;
			m_blitter.yoff |= data << 8;
			break;
		case VIDBASL:
			m_blitter.screen = data;
			break;
		case VIDBASH:
			m_blitter.screen &= 0xff;
			m_blitter.screen |= data << 8;
			break;
		case COLLBASL:
			m_blitter.colbuf = data;
			break;
		case COLLBASH:
			m_blitter.colbuf &= 0xff;
			m_blitter.colbuf |= data << 8;
			break;
		case SCBNEXTL:
			m_blitter.scb_next = data;
			break;
		case SCBNEXTH:
			m_blitter.scb_next &= 0xff;
			m_blitter.scb_next |= data << 8;
			break;
		case SPRDLINEL:
			m_blitter.bitmap = data;
			break;
		case SPRDLINEH:
			m_blitter.bitmap &= 0xff;
			m_blitter.bitmap |= data << 8;
			break;
		case HPOSSTRTL:
			m_blitter.x_pos = data;
			[[fallthrough]]; // FIXME: really?
		case HPOSSTRTH:
			m_blitter.x_pos &= 0xff;
			m_blitter.x_pos |= data << 8;
			[[fallthrough]]; // FIXME: really?
		case VPOSSTRTL:
			m_blitter.y_pos = data;
			[[fallthrough]]; // FIXME: really?
		case VPOSSTRTH:
			m_blitter.y_pos &= 0xff;
			m_blitter.y_pos |= data << 8;
			[[fallthrough]]; // FIXME: really?
		case SPRHSIZL:
			m_blitter.width = data;
			break;
		case SPRHSIZH:
			m_blitter.width &= 0xff;
			m_blitter.width |= data << 8;
			break;
		case SPRVSIZL:
			m_blitter.height = data;
			break;
		case SPRVSIZH:
			m_blitter.height &= 0xff;
			m_blitter.height |= data << 8;
			break;
		case STRETCHL:
			m_blitter.stretch = data;
			break;
		case STRETCHH:
			m_blitter.stretch &= 0xff;
			m_blitter.stretch |= data << 8;
			break;
		case TILTL:
			m_blitter.tilt = data;
			break;
		case TILTH:
			m_blitter.tilt &= 0xff;
			m_blitter.tilt |= data << 8;
			break;
		// case SPRDOFFL:
		// case SPRVPOSL:
		// case COLLOFFL:
		case VSIZACUML:
			m_blitter.height_accumulator = data;
			break;
		case VSIZACUMH:
			m_blitter.height_accumulator &= 0xff;
			m_blitter.height_accumulator |= data << 8;
			break;
		case HSIZOFFL:
			m_blitter.width_offset = data;
			break;
		case HSIZOFFH:
			m_blitter.width_offset &= 0xff;
			m_blitter.width_offset |= data << 8;
			break;
		case VSIZOFFL:
			m_blitter.height_offset = data;
			break;
		case VSIZOFFH:
			m_blitter.height_offset &= 0xff;
			m_blitter.height_offset |= data << 8;
			break;
		case SCBADRL:
			m_blitter.scb = data;
			break;
		case SCBADRH:
			m_blitter.scb &= 0xff;
			m_blitter.scb |= data << 8;
			break;
		//case PROCADRL:

		/* Writing to M (0x6c) will also clear the accumulator overflow bit */
		case MATH_M:
			m_suzy.accumulate_overflow = false;
			break;
		case MATH_C:
			/* If we are going to perform a signed multiplication, we store the sign and convert the number
			to an unsigned one */
			if (m_suzy.signed_math)
			{
				u16 factor, temp;
				factor = m_suzy.data[MATH_D] | (m_suzy.data[MATH_C] << 8);
				if ((factor - 1) & 0x8000)      /* here we use -1 to cover the math bugs on the sign of 0 and 0x8000 */
				{
					temp = (factor ^ 0xffff) + 1;
					m_sign_CD = -1;
					m_suzy.data[MATH_D] = temp & 0xff;
					m_suzy.data[MATH_C] = temp >> 8;
				}
				else
					m_sign_CD = 1;
			}
			break;
		case MATH_D:
		/* Documentation states that writing to the MATH_D will set MATH_C to zero but not update the sign flag.
		Implementing the sign detection as described in the documentation causes Stun Runners to not work.
		Either the sign error in the docs is not as described or writing to the lower byte does update the sign flag.
		Here I assume the sign flag gets updated. */
			if (data)
				m_sign_CD = 1;
			break;
		/* Writing to A will start a 16 bit multiply */
		/* If we are going to perform a signed multiplication, we also store the sign and convert the
		number to an unsigned one */
		case MATH_A:
			if (m_suzy.signed_math)
			{
				u16 factor, temp;
				factor = m_suzy.data[MATH_B] | (m_suzy.data[MATH_A] << 8);
				if ((factor - 1) & 0x8000)      /* here we use -1 to cover the math bugs on the sign of 0 and 0x8000 */
				{
					temp = (factor ^ 0xffff) + 1;
					m_sign_AB = -1;
					m_suzy.data[MATH_B] = temp & 0xff;
					m_suzy.data[MATH_A] = temp >> 8;
				}
				else
					m_sign_AB = 1;
			}
			suzy_multiply();
			break;
		/* Writing to E will start a 16 bit divide */
		case MATH_E:
			suzy_divide();
			break;
		case SPRCTL0:
			m_blitter.spr_ctl0 = data;
			break;
		case SPRCTL1:
			m_blitter.spr_ctl1 = data;
			break;
		case SPRCOLL:
			m_blitter.spr_coll = data;
			break;
		case SUZYBUSEN:
			logerror("write to SUSYBUSEN %x \n", data);
			break;
		case SPRSYS:
			m_suzy.signed_math = BIT(data, 7);
			m_suzy.accumulate = BIT(data, 6);
			m_blitter.no_collide = BIT(data, 5);
			m_blitter.vstretch = BIT(data, 4);
			m_blitter.lefthanded = BIT(data, 3);
			// unsafe access clear and sprite engine stop request are not enabled
			if (BIT(data, 1)) logerror("sprite engine stop request\n");
			break;
		case SPRGO:
			if (m_suzy.SPRITE_GO() && m_suzy.data[SUZYBUSEN])
			{
				//m_blitter.time = machine().time();
				blitter();
			}
			break;
		case JOYSTICK:
		case SWITCHES:
			logerror("warning: write to read-only button registers\n");
			break;
	}
}


/****************************************

    Mikey emulation

****************************************/

/*
 0xfd0a r sync signal?
 0xfd81 r interrupt source bit 2 vertical refresh
 0xfd80 w interrupt quit
 0xfd87 w bit 1 !clr bit 0 blocknumber clk
 0xfd8b w bit 1 blocknumber hi B
 0xfd94 w 0
 0xfd95 w 4
 0xfda0-f rw farben 0..15
 0xfdb0-f rw bit0..3 farben 0..15
*/


void lynx_state::interrupt_set(u8 line)
{
	m_mikey.interrupt |= (1 << line);
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	interrupt_update();
}


void lynx_state::interrupt_update()
{
	m_maincpu->set_input_line(G65SC02_IRQ_LINE, (m_mikey.interrupt == 0) ? CLEAR_LINE : ASSERT_LINE);
}


/*
DISPCTL EQU $FD92       ; set to $D by INITMIKEY

; B7..B4        0
; B3    1 EQU color
; B2    1 EQU 4 bit mode
; B1    1 EQU flip screen
; B0    1 EQU video DMA enabled
*/

void lynx_state::draw_line()
{
	pen_t const *const pen = m_palette->pens();

	// calculate y: first three lines are vblank,
	int const y = 101 - m_timer[2].counter;
	// Documentation states lower two bits of buffer address are ignored (thus 0xfffc mask)
	u16 j = (m_mikey.disp_addr & 0xfffc) + y * 160 / 2; // clipping needed!

	if (m_mikey.data[0x92] & 0x02)
	{
		j -= 160 * 102 / 2 - 1;
		u32 *const line = &m_bitmap_temp.pix(102 - 1 - y);
		for (int x = 160 - 2; x >= 0; j++, x -= 2)
		{
			u8 const byte = dram_byte_r(j);
			line[x + 1] = pen[(byte >> 4) & 0x0f];
			line[x + 0] = pen[(byte >> 0) & 0x0f];
		}
	}
	else
	{
		u32 *const line = &m_bitmap_temp.pix(y);
		for (int x = 0; x < 160; j++, x += 2)
		{
			u8 const byte = dram_byte_r(j);
			line[x + 0] = pen[(byte >> 4) & 0x0f];
			line[x + 1] = pen[(byte >> 0) & 0x0f];
		}
	}
}

/****************************************

    Timers

****************************************/

/*
HCOUNTER        EQU TIMER0
VCOUNTER        EQU TIMER2
SERIALRATE      EQU TIMER4

TIM_BAKUP       EQU 0   ; backup-value (count+1)
TIM_CNTRL1      EQU 1   ; timer-control register
TIM_CNT EQU 2   ; current counter
TIM_CNTRL2      EQU 3   ; dynamic control

; TIM_CNTRL1
TIM_IRQ EQU %10000000   ; enable interrupt (not TIMER4 !)
TIM_RESETDONE   EQU %01000000   ; reset timer done
TIM_MAGMODE     EQU %00100000   ; nonsense in Lynx !!
TIM_RELOAD      EQU %00010000   ; enable reload
TIM_COUNT       EQU %00001000   ; enable counter
TIM_LINK        EQU %00000111
; link timers (0->2->4 / 1->3->5->7->Aud0->Aud1->Aud2->Aud3->1
TIM_64us        EQU %00000110
TIM_32us        EQU %00000101
TIM_16us        EQU %00000100
TIM_8us EQU %00000011
TIM_4us EQU %00000010
TIM_2us EQU %00000001
TIM_1us EQU %00000000

;TIM_CNTRL2 (read-only)
; B7..B4 unused
TIM_DONE        EQU %00001000   ; set if timer's done; reset with TIM_RESETDONE
TIM_LAST        EQU %00000100   ; last clock (??)
TIM_BORROWIN    EQU %00000010
TIM_BORROWOUT   EQU %00000001
*/

#define NR_LYNX_TIMERS  8




void lynx_state::timer_init(int which)
{
	m_timer[which] = LYNX_TIMER();
	m_timer[which].timer = timer_alloc(FUNC(lynx_state::timer_shot), this);

	save_item(NAME(m_timer[which].bakup), which);
	save_item(NAME(m_timer[which].cntrl1), which);
	save_item(NAME(m_timer[which].cntrl2), which);
	save_item(NAME(m_timer[which].counter), which);
	save_item(NAME(m_timer[which].timer_active), which);
}

void lynx_state::timer_signal_irq(int which)
{
	if ((m_timer[which].int_en()) && (which != 4)) // if interrupts are enabled and timer != 4
	{
		interrupt_set(which); // set interrupt poll register
	}
	switch (which) // count down linked timers
	{
	case 0:
		switch (m_timer[2].counter)
		{
			case 104:
				break;
			case 103:
				m_mikey.vb_rest = true;
				break;
			case 102:
				m_mikey.disp_addr = m_mikey.data[0x94] | (m_mikey.data[0x95] << 8);
				break;
			case 101:
				m_mikey.vb_rest = false;
				draw_line();
				break;
			default:
				draw_line();
				break;
		}
		timer_count_down(2);
		break;
	case 2:
		copybitmap(m_bitmap, m_bitmap_temp, 0, 0, 0, 0, m_screen->cliprect());
		update_screen_timing();
		timer_count_down(4);
		break;
	case 1:
		timer_count_down(3);
		break;
	case 3:
		timer_count_down(5);
		break;
	case 5:
		timer_count_down(7);
		break;
	case 7:
		m_sound->count_down(0);
		break;
	}
}

void lynx_state::timer_count_down(int which)
{
	if (m_timer[which].count_en() && m_timer[which].linked()) // count and linking enabled
	{
		if (m_timer[which].counter > 0)
		{
			m_timer[which].counter--;
			//m_timer[which].borrow_in = 1;
			return;
		}
		if (m_timer[which].counter == 0)
		{
			if (m_timer[which].borrow_out()) // borrow out
			{
				timer_signal_irq(which);
				if (m_timer[which].reload_en()) // if reload enabled
				{
					m_timer[which].counter = m_timer[which].bakup;
				}
				else
				{
					m_timer[which].set_timer_done(true); // set timer done
				}
				m_timer[which].set_borrow_out(false); // clear borrow out
			}
			else
				m_timer[which].set_borrow_out(true); // set borrow out
			return;
		}
	}
	else
	{
		//m_timer[which].borrow_in = 0;
		m_timer[which].set_borrow_out(false);
	}
}

u32 lynx_state::time_factor(int val)
{
	switch (val)
	{
		case 0: return 1000000;
		case 1: return 500000;
		case 2: return 250000;
		case 3: return 125000;
		case 4: return 62500;
		case 5: return 31250;
		case 6: return 15625;
		default: fatalerror("invalid value\n");
	}
}

TIMER_CALLBACK_MEMBER(lynx_state::timer_shot)
{
	timer_signal_irq(param);
	if (!(m_timer[param].reload_en())) // if reload not enabled
	{
		m_timer[param].timer_active = false;
		m_timer[param].set_timer_done(true); // set timer done
	}
	else
	{
		attotime t = (attotime::from_hz(time_factor(m_timer[param].timer_clock())) * (m_timer[param].bakup + 1));
		m_timer[param].timer->adjust(t, param);
	}
}

u8 lynx_state::timer_read(int which, int offset)
{
	u8 value = 0;

	switch (offset)
	{
		case 0:
			value = m_timer[which].bakup;
			break;
		case 1:
			value = m_timer[which].cntrl1;
			break;
		case 2:
			if (m_timer[which].linked()) // linked timer
			{
				value = m_timer[which].counter;
			}
			else
			{
				if (m_timer[which].timer_active)
				{
					value = u8(m_timer[which].timer->remaining().as_ticks(1000000>>(m_timer[which].timer_clock())));
					value -= value ? 1 : 0;
				}
			}
			break;

		case 3:
			value = m_timer[which].cntrl2;
			break;
	}
	// logerror("timer %d read %x %.2x\n", which, offset, value);
	return value;
}

void lynx_state::timer_write(int which, int offset, u8 data)
{
	//logerror("timer %d write %x %.2x\n", which, offset, data);
	attotime t;

	if (m_timer[which].timer_active && (!(m_timer[which].linked())))
	{
		m_timer[which].counter = u8(m_timer[which].timer->remaining().as_ticks(1000000>>(m_timer[which].timer_clock())));
		m_timer[which].counter -= (m_timer[which].counter) ? 1 : 0;
	}

	switch (offset)
	{
		case 0:
			m_timer[which].bakup = data;
			break;
		case 1:
			m_timer[which].cntrl1 = data;
			if (data & 0x40) // reset timer done
				m_timer[which].set_timer_done(false);
			break;
		case 2:
			m_timer[which].counter = data;
			break;
		case 3:
			m_timer[which].cntrl2 = (m_timer[which].cntrl2 & ~0x08) | (data & 0x08);
			break;
	}

	/* Update timers */
	//if (offset < 3)
	//{
		m_timer[which].timer->reset();
		m_timer[which].timer_active = false;
		if ((m_timer[which].count_en()) && !(m_timer[which].timer_done()))      // if enable count
		{
			if (!(m_timer[which].linked()))  // if not set to link mode
			{
				t = (attotime::from_hz(time_factor(m_timer[which].timer_clock())) * (m_timer[which].counter + 1));
				m_timer[which].timer->adjust(t, which);
				m_timer[which].timer_active = true;
			}
		}
	//}
}

void lynx_state::update_screen_timing()
{
	// variable framerate handling, but needs to verification for screen size handling
	if ((!(m_timer[0].linked()))
		&& ((m_timer[0].cntrl1 & 0x18) == 0x18)
		&& (m_timer[0].bakup != 0)
		&& ((m_timer[2].cntrl1 & 0x1f) == 0x1f)
		&& (m_timer[2].bakup != 0))
	{
		if ((m_pixclock != (m_timer[0].timer_clock())) || (m_hcount != m_timer[0].bakup) || (m_vcount != m_timer[2].bakup))
		{
			m_pixclock = time_factor(m_timer[0].timer_clock());
			m_hcount = m_timer[0].bakup; // TODO: multiplied internally?
			m_vcount = m_timer[2].bakup;
			attotime framerate = attotime::from_hz(m_pixclock) * (m_hcount + 1) * (m_vcount + 1);
			m_screen->configure(m_screen->width(), m_screen->height(), m_screen->visible_area(), framerate.attoseconds());
		}
	}
}


/****************************************

    UART Emulation

****************************************/


void lynx_state::uart_reset()
{
	m_uart = UART();
}

TIMER_CALLBACK_MEMBER(lynx_state::uart_loopback_timer)
{
	m_uart.received = false;
}

TIMER_CALLBACK_MEMBER(lynx_state::uart_timer)
{
	if (m_uart.buffer_loaded)
	{
		m_uart.data_to_send = m_uart.buffer;
		m_uart.buffer_loaded = false;
		m_uart_timer->adjust(attotime::from_usec(11*16));
	}
	else
	{
		m_uart.sending = false;
		m_uart.received = true;
		m_uart.data_received = m_uart.data_to_send;
		m_loopback_timer->adjust(attotime::from_usec(11*16));
		if (m_uart.RXINTEN())
		{
			interrupt_set(4);
		}
	}

	if (m_uart.TXINTEN())
	{
		interrupt_set(4);
	}
}

u8 lynx_state::uart_r(offs_t offset)
{
	u8 value = 0x00;
	switch (offset)
	{
		case 0x8c:
			if (!m_uart.buffer_loaded) // TXRDY Transmit buffer ready
				value |= 0x80;
			if (m_uart.received) // RXRDY Recevie character ready
				value |= 0x40;
			if (!m_uart.sending) // TXEMPTY Transmit done
				value |= 0x20;
			// value |= 0x10; PARERR Recevied parity error (not implemented)
			// value |= 0x08; OVERRUN Recevied overrun error (not implemented)
			// value |= 0x04; FRAMERR Recevied framing error (not implemented)
			// value |= 0x02; RXBRK Break recevied (not implemented)
			// value |= 0x01; PARBIT 9th bit (not implemented)
			break;

		case 0x8d:
			value = m_uart.data_received;
			break;
	}
	logerror("uart read %.2x %.2x\n", offset, value);
	return value;
}

void lynx_state::uart_w(offs_t offset, u8 data)
{
	logerror("uart write %.2x %.2x\n", offset, data);
	switch (offset)
	{
		case 0x8c:
			m_uart.serctl = data;
			break;

		case 0x8d:
			if (m_uart.sending)
			{
				m_uart.buffer = data;
				m_uart.buffer_loaded = true;
			}
			else
			{
				m_uart.sending = true;
				m_uart.data_to_send = data;
				// timing not accurate, baude rate should be calculated from timer 4 backup value and clock rate
				m_uart_timer->adjust(attotime::from_usec(11*16));
			}
			break;
	}
}


/****************************************

    Mikey memory handlers

****************************************/


u8 lynx_state::mikey_read(offs_t offset)
{
	u8 direction, value = 0x00;

	switch (offset)
	{
	case 0x00: case 0x01: case 0x02: case 0x03:
	case 0x04: case 0x05: case 0x06: case 0x07:
	case 0x08: case 0x09: case 0x0a: case 0x0b:
	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
	case 0x10: case 0x11: case 0x12: case 0x13:
	case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b:
	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		value = timer_read(offset >> 2, offset & 0x03);
		break;

	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
	case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x50:
		value = m_sound->read(offset);
		break;

	case 0x80:
	case 0x81:
		value = m_mikey.interrupt; // both registers access the same interrupt status byte
		// logerror("mikey read %.2x %.2x\n", offset, value);
		break;

	case 0x84: // MAGRDY0 Mag. tape channel 0 ready bit
	case 0x85: // MAGRDY1 Mag. tape channel 1 ready bit
		value = 0x00;
		break;

	case 0x86: // AUDIN Audio in
		value = 0x80;
		break;

	case 0x88: // MIKEYHREV Mikey hardware revision
		value = 0x01;
		break;

	//case 0x89: // MIKEYSREV Mikey software revision
	case 0x8b:
		direction = m_mikey.IODIR();
		value |= BIT(direction, 0) ? (m_mikey.data[offset] & 0x01) : 0x01; // External Power input
		value |= BIT(direction, 1) ? (m_mikey.data[offset] & 0x02) : 0x00; // Cart Address Data output (0 turns cart power on)
		value |= BIT(direction, 2) ? (m_mikey.data[offset] & 0x04) : 0x04; // noexp input
		// REST read returns actual rest state anded with rest output bit
		value |= BIT(direction, 3) ? (((m_mikey.data[offset] & 0x08) && (m_mikey.vb_rest)) ? 0x00 : 0x08) : 0x00;  // rest output
		value |= BIT(direction, 4) ? (m_mikey.data[offset] & 0x10) : 0x10; // audin input
		/* Hack: we disable COMLynx  */
		value |= 0x04;
		/* B5, B6 & B7 are not used */
		break;

	case 0x8c:
	case 0x8d:
		value = uart_r(offset);
		break;

	default:
		value = m_mikey.data[offset];
		//logerror("mikey read %.2x %.2x\n", offset, value);
	}
	return value;
}

void lynx_state::mikey_write(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0x00: case 0x01: case 0x02: case 0x03:
	case 0x04: case 0x05: case 0x06: case 0x07:
	case 0x08: case 0x09: case 0x0a: case 0x0b:
	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
	case 0x10: case 0x11: case 0x12: case 0x13:
	case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b:
	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		timer_write(offset >> 2, offset & 3, data);
		return;

	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
	case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x50:
		m_sound->write(offset, data);
		return;

	case 0x80:
		m_mikey.interrupt &= ~data; // clear interrupt source
		// logerror("mikey write %.2x %.2x\n", offset, data);
		if (!m_mikey.interrupt)
			interrupt_update();
		break;

	/* Is this correct? */ // Notes say writing to register will result in interrupt being triggered.
	case 0x81:
		m_mikey.interrupt |= data;
		if (data)
		{
			m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			interrupt_update();
			logerror("direct write to interrupt register\n");
		}
		break;

	case 0x87:
		m_mikey.data[offset] = data;
		if (BIT(data, 1))        // Power (1 = on)
		{
			if (BIT(data, 0))    // Cart Address Strobe, positive edge
			{
				m_cart_addr_block = ((m_cart_addr_block << 1) & 0xfe) | BIT(m_mikey.IODAT(), 1); // 74HC164 clock
				m_cart_addr_counter = 0; // 4040 reset
			}
		}
		else
		{
			m_cart_addr_block = 0;
			m_cart_addr_counter = 0;
		}
		break;

	/* TODO: properly implement these writes */
	case 0x8b:
		m_mikey.data[offset] = data;
		if ((m_audin_offset == 0) && (m_mikey.IODIR() & 0x10))
			logerror("Trying to enable bank 1 write. %d\n", m_mikey.data[offset] & 0x10);
		break;

	case 0x8c: case 0x8d:
		uart_w(offset, data);
		break;

	//case 0x90: // SDONEACK - Suzy Done Acknowledge
	case 0x91: // CPUSLEEP - CPU Bus Request Disable
		m_mikey.data[offset] = data;
		if (!data && m_blitter.busy)
		{
			m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			/* A write of '0' to this address will reset the CPU bus request flip flop */
		}
		break;
	//case 0x93: // PBKUP - Magic 'P' count value
	case 0x94: case 0x95:
		m_mikey.data[offset] = data;
		break;
	case 0x9c: case 0x9d: case 0x9e:
		m_mikey.data[offset] = data;
		logerror("Mtest%d write: %x\n", offset & 0x3, data);
		break;

	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
		m_mikey.data[offset] = data;

		/* RED = 0xb- & 0x0f, GREEN = 0xa- & 0x0f, BLUE = (0xb- & 0xf0) >> 4 */
		m_palette->set_pen_green_level(offset & 0x0f, pal4bit(data & 0x0f));
		break;

	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		m_mikey.data[offset] = data;

		/* RED = 0xb- & 0x0f, GREEN = 0xa- & 0x0f, BLUE = (0xb- & 0xf0) >> 4 */
		m_palette->set_pen_red_level(offset & 0x0f, pal4bit(data & 0x0f));
		m_palette->set_pen_blue_level(offset & 0x0f, pal4bit((data & 0xf0) >> 4));
		break;

	default:
		m_mikey.data[offset] = data;
		//logerror("mikey write %.2x %.2x\n",offset,data);
		break;
	}
}

/****************************************

    Init / Config

****************************************/

u8 lynx_state::memory_config_r()
{
	return m_memory_config;
}

void lynx_state::memory_config_w(u8 data)
{
	/* bit 7: hispeed, uses page mode accesses (4 instead of 5 cycles)
	 * when these are safe in the cpu */
	m_memory_config = data;

	if (BIT(data, 0))
		m_suzy_view.disable();
	else
		m_suzy_view.select(0);

	if (BIT(data, 1))
		m_mikey_view.disable();
	else
		m_mikey_view.select(0);

	if (BIT(data, 2))
		m_rom_view.disable();
	else
		m_rom_view.select(0);

	if (BIT(data, 3))
		m_vector_view.disable();
	else
		m_vector_view.select(0);
}

void lynx_state::machine_reset()
{
	memory_config_w(0);

	m_mikey.interrupt = 0;
	m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	m_maincpu->set_input_line(G65SC02_IRQ_LINE, CLEAR_LINE);

	m_suzy = SUZY();
	m_mikey = MIKEY();

	m_suzy.data[0x88]  = 0x01;
	m_suzy.data[0x90]  = 0x00;
	m_suzy.data[0x91]  = 0x00;
	m_mikey.data[0x80] = 0x00;
	m_mikey.data[0x81] = 0x00;
	m_mikey.data[0x88] = 0x01;
	m_mikey.data[0x8a] = 0x00;
	m_mikey.data[0x8c] = 0x00;
	m_mikey.data[0x90] = 0x00;
	m_mikey.data[0x92] = 0x00;

	uart_reset();

	// hack to allow current object loading to work
#if 0
	timer_write(0, 0, 158); // set backup value (hpos) = 159
	timer_write(0, 1, 0x10 | 0x8 | 0); // enable count, enable reload, 1us period
	timer_write(2, 0, 104); // set backup value (vpos) = 105
	timer_write(2, 1, 0x10 | 0x8 | 7); // enable count, enable reload, link
#endif

	render_target *target = machine().render().first_target();
	target->set_view(m_rotate);
}

void lynx_state::device_post_load()
{
	memory_config_w(m_memory_config);
}

void lynx_state::machine_start()
{
	m_bitmap_temp.allocate(160,105,0,0);

	// save driver variables
	save_item(NAME(m_cart_addr_block));
	save_item(NAME(m_cart_addr_counter));
	save_item(NAME(m_memory_config));
	save_item(NAME(m_sign_AB));
	save_item(NAME(m_sign_CD));
	save_item(NAME(m_rotate));
	save_item(NAME(m_pixclock));
	save_item(NAME(m_hcount));
	save_item(NAME(m_vcount));
	// save blitter variables
	save_item(NAME(m_blitter.screen));
	save_item(NAME(m_blitter.colbuf));
	save_item(NAME(m_blitter.colpos));
	save_item(NAME(m_blitter.xoff));
	save_item(NAME(m_blitter.yoff));
	save_item(NAME(m_blitter.mode));
	save_item(NAME(m_blitter.spr_coll));
	save_item(NAME(m_blitter.spritenr));
	save_item(NAME(m_blitter.x_pos));
	save_item(NAME(m_blitter.y_pos));
	save_item(NAME(m_blitter.width));
	save_item(NAME(m_blitter.height));
	save_item(NAME(m_blitter.tilt_accumulator));
	save_item(NAME(m_blitter.width_accumulator));
	save_item(NAME(m_blitter.height_accumulator));
	save_item(NAME(m_blitter.width_offset));
	save_item(NAME(m_blitter.height_offset));
	save_item(NAME(m_blitter.stretch));
	save_item(NAME(m_blitter.tilt));
	save_item(NAME(m_blitter.color));
	save_item(NAME(m_blitter.bitmap));
	save_item(NAME(m_blitter.use_rle));
	save_item(NAME(m_blitter.line_color));
	save_item(NAME(m_blitter.spr_ctl0));
	save_item(NAME(m_blitter.spr_ctl1));
	save_item(NAME(m_blitter.scb));
	save_item(NAME(m_blitter.scb_next));
	save_item(NAME(m_blitter.sprite_collide));
	save_item(NAME(m_blitter.everon));
	save_item(NAME(m_blitter.fred));
	save_item(NAME(m_blitter.memory_accesses));
	save_item(NAME(m_blitter.no_collide));
	save_item(NAME(m_blitter.vstretch));
	save_item(NAME(m_blitter.lefthanded));
	save_item(NAME(m_blitter.busy));
	// save suzy variables
	save_item(NAME(m_suzy.data));
	save_item(NAME(m_suzy.signed_math));
	save_item(NAME(m_suzy.accumulate));
	save_item(NAME(m_suzy.accumulate_overflow));
	// save mikey variables
	save_item(NAME(m_mikey.data));
	save_item(NAME(m_mikey.disp_addr));
	save_item(NAME(m_mikey.vb_rest));
	save_item(NAME(m_mikey.interrupt));
	// save uart variables
	save_item(NAME(m_uart.serctl));
	save_item(NAME(m_uart.data_received));
	save_item(NAME(m_uart.data_to_send));
	save_item(NAME(m_uart.buffer));
	save_item(NAME(m_uart.received));
	save_item(NAME(m_uart.sending));
	save_item(NAME(m_uart.buffer_loaded));

	m_suzy_view.select(0);
	m_mikey_view.select(0);
	m_rom_view.select(0);
	m_vector_view.select(0);

	for (int i = 0; i < NR_LYNX_TIMERS; i++)
		timer_init(i);

	m_blitter_timer = timer_alloc(FUNC(lynx_state::blitter_timer), this);
	m_loopback_timer = timer_alloc(FUNC(lynx_state::uart_loopback_timer), this);
	m_uart_timer = timer_alloc(FUNC(lynx_state::uart_timer), this);
}


/****************************************

    Image handling

****************************************/

std::pair<std::error_condition, std::string> lynx_state::verify_cart(const char *header, int kind)
{
	if (kind)
	{
		if (strncmp("BS93", &header[6], 4))
			return std::make_pair(image_error::INVALIDIMAGE, "This is not a valid Lynx image");
	}
	else
	{
		if (strncmp("LYNX", &header[0], 4))
		{
			if (!strncmp("BS93", &header[6], 4))
			{
				// FIXME: multi-line message may not be displayed correctly
				return std::make_pair(
						image_error::INVALIDIMAGE,
						"This image is probably a Quickload image with .lnx extension\n"
						"Try to load it with -quickload");
			}
			else
				return std::make_pair(image_error::INVALIDIMAGE, "This is not a valid Lynx image");
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}

DEVICE_IMAGE_LOAD_MEMBER(lynx_state::cart_load)
{
	/* Lynx carts have 19 address lines, the upper 8 used for bank select. The lower
	11 bits are used to address data within the selected bank. Valid bank sizes are 256,
	512, 1024 or 2048 bytes. Commercial roms use all 256 banks.*/
	u32 size = m_cart->common_get_size("rom");
	u16 gran = 0;

	if (!image.loaded_through_softlist())
	{
		// check for lnx header
		if (image.is_filetype("lnx"))
		{
			// 64 byte header
			// LYNX
			// intelword lower counter size
			// 0 0 1 0
			// 32 chars name
			// 22 chars manufacturer
			u8 header[0x40];
			image.fread(header, 0x40);

			// Check the image
			auto err = verify_cart((const char*)header, LYNX_CART);
			if (err.first)
				return err;

			/* 2008-10 FP: According to Handy source these should be page_size_bank0. Are we using
			 it correctly in MAME? Moreover, the next two values should be page_size_bank1. We should
			 implement this as well */
			gran = header[4] | (header[5] << 8);

			logerror ("%s %dkb cartridge with %dbyte granularity from %s\n", header + 10, size / 1024, gran, header + 42);
			size -= 0x40;
		}
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	// set-up granularity
	if (!image.loaded_through_softlist())
	{
		if (image.is_filetype("lnx"))     // from header
			m_granularity = gran;
		else if (image.is_filetype("lyx"))
		{
			/* 2008-10 FP: FIXME: .lyx file don't have an header, hence they miss "lynx_granularity"
			(see above). What if bank 0 has to be loaded elsewhere? And what about bank 1?
			These should work with most .lyx files, but we need additional info on raw cart images */
			if (size == 0x20000)
				m_granularity = 0x0200;
			else if (size == 0x80000)
				m_granularity = 0x0800;
			else
				m_granularity = 0x0400;
		}
	}
	else
	{
		// Some cartridge uses AUDIN pin for bankswitch
		if (image.get_feature("audin_offset") != nullptr)
			m_audin_offset = atol(image.get_feature("audin_offset"));

		if (image.get_feature("granularity") != nullptr)
			m_granularity = atol(image.get_feature("granularity"));
		else
		{
			if (size > 0xffff) // 64,128,256,512k cartridges
				m_granularity = size >> 8;
			else
				m_granularity = 0x400; // Homebrew roms not using all 256 banks (T-Tris) (none currently in softlist)
		}
	}

	// set-up rotation from softlist
	if (image.loaded_through_softlist())
	{
		const char *rotate = image.get_feature("rotation");
		m_rotate = 0;
		if (rotate)
		{
			if (!core_stricmp(rotate, "RIGHT"))
				m_rotate = 1;
			else if (!core_stricmp(rotate, "LEFT"))
				m_rotate = 2;
		}

	}

	return std::make_pair(std::error_condition(), std::string());
}
