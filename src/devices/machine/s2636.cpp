// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, Peter Trauner
/**********************************************************************

    Signetics 2636 video chip

    PVI REGISTER DESCRIPTION
    ------------------------

          |              bit              |R/W| description
    byte  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |   |
          |                               |   |
    FC0   | size 4| size 3| size 2| size 1| W | size of the 4 objects(=sprites)
          |                               |   |
    FC1   |       |C1 |C2 |C3 |C1 |C2 |C3 | W | colors of the 4 objects
          |       |  color 1  |  color 2  |   |
    FC2   |       |C1 |C2 |C3 |C1 |C2 |C3 | W |
          |       |  color 3  |  color 4  |   |
          |                               |   |
    FC3   |                       |sh |pos| W | 1=shape 0=position
          |                               |   | display format and position
    FC4   |            (free)             |   |
    FC5   |            (free)             |   |
          |                               |   |
    FC6   |   |C1 |C2 |C3 |BG |scrn colr  | W | background lock and color
          |   |backg colr |enb|C1 |C2 |C3 |   | 3="enable"
          |                               |   |
    FC7   |            sound              | W | squarewave output
          |                               |   |
    FC8   |       N1      |      N2       | W | range of the 4 display digits
    FC9   |       N3      |      N4       | W |
          |                               |   |
          |obj/backgrnd   |complete object| R |
    FCA   | 1 | 2 | 3 | 4 | 1 | 2 | 3 | 4 |   |
          |                               |   |
    FCB   |   |VR-|   object collisions   | R | Composition of object and back-
          |   |LE |1/2|1/3|1/3|1/4|2/4|3/4|   | ground,collision detection and
          |                               |   | object display as a state display
          |                               |   | for the status register.Set VRLE.
          |                               |   | wait for VRST.Read out or transmit
          |                               |   | [copy?] all bits until reset by
          |                               |   | VRST.
          |                               |   |
    FCC   |            PORT1              | R | PORT1 and PORT2 for the range of
    FCD   |            PORT2              |   | the A/D conversion.Cleared by VRST
    FCE   |            (free)             |   |
    FCF   |            (free)             |   |


    Size control by byte FC0

     bit  matrix
    |0|0|  8x10
    |0|1| 16x20
    |1|0| 32x40
    |1|1| 64x80

    CE1 and not-CE2 are outputs from the PVI.$E80..$EFF also controls the
    analog multiplexer.


    SPRITES
    -------

    each object field: (=sprite data structure)

    0 \ 10 bytes of bitmap (Each object is 8 pixels wide.)
    9 /
    A   HC  horizontal object coordinate
    B   HCB horizontal duplicate coordinate
    C   VC  vertical object coordinate
    D   VCB vertical duplicate coordinate

*************************************************************/

#include "emu.h"
#include "machine/s2636.h"


/*************************************
 *
 *  Device interface
 *
 *************************************/

const device_type S2636 = &device_creator<s2636_device>;

s2636_device::s2636_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, S2636, "Signetics 2636", tag, owner, clock, "s2636", __FILE__),
		device_video_interface(mconfig, *this),
		device_sound_interface(mconfig, *this),
		m_channel(NULL),
		m_size(0),
		m_pos(0),
		m_level(0),
		m_work_ram_size(0),
		m_y_offset(0),
		m_x_offset(0)
{
	for (int i = 0; i < 1; i++)
		m_reg[i] = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s2636_device::device_start()
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_work_ram.resize(m_work_ram_size);
	memset(&m_work_ram[0], 0, m_work_ram_size);
	m_bitmap.resize(width, height);
	m_collision_bitmap.resize(width, height);

	save_item(NAME(m_work_ram));
	save_item(NAME(m_bitmap));
	save_item(NAME(m_collision_bitmap));

	m_channel = machine().sound().stream_alloc(*this, 0, 1, machine().sample_rate());
	save_item(NAME(m_size));
	save_item(NAME(m_pos));
	save_item(NAME(m_level));
	save_item(NAME(m_reg));
}

/*************************************
 *
 *  Constants
 *
 *************************************/

#define SPRITE_WIDTH    (8)
#define SPRITE_HEIGHT   (10)

static const int sprite_offsets[4] = { 0x00, 0x10, 0x20, 0x40 };


/*************************************
 *
 *  Draw a sprite
 *
 *************************************/

static void draw_sprite( UINT8 *gfx, int color, int y, int x, int expand, int or_mode, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/* for each row */
	for (int sy = 0; sy < SPRITE_HEIGHT; sy++)
	{
		/* for each pixel on the row */
		for (int sx = 0; sx < SPRITE_WIDTH; sx++)
		{
			/* each pixel can be expanded */
			for (int ey = 0; ey <= expand; ey++)
			{
				for (int ex = 0; ex <= expand; ex++)
				{
					/* compute effective destination pixel */
					int ty = y + sy * (expand + 1) + ey;
					int tx = x + sx * (expand + 1) + ex;

					/* get out if outside the drawing region */
					if (!cliprect.contains(tx, ty))
						continue;

					/* get out if current image bit is transparent */
					if (((gfx[sy] << sx) & 0x80) == 0x00)
						continue;

					if (or_mode)
						bitmap.pix16(ty, tx) = 0x08 | bitmap.pix16(ty, tx) | color;
					else
						bitmap.pix16(ty, tx) = 0x08 | color;
				}
			}
		}
	}
}


/*************************************
 *
 *  Collision detection
 *
 *************************************/

int s2636_device::check_collision( int spriteno1, int spriteno2, const rectangle &cliprect )
{
	int checksum = 0;

	UINT8* attr1 = &m_work_ram[sprite_offsets[spriteno1]];
	UINT8* attr2 = &m_work_ram[sprite_offsets[spriteno2]];

	/* TODO: does not check shadow sprites yet */

	m_collision_bitmap.fill(0, cliprect);

	if ((attr1[0x0a] != 0xff) && (attr2[0x0a] != 0xff))
	{
		int x, y;

		int x1 = attr1[0x0a] + m_x_offset;
		int y1 = attr1[0x0c] + m_y_offset;
		int x2 = attr2[0x0a] + m_x_offset;
		int y2 = attr2[0x0c] + m_y_offset;

		int expand1 = (m_work_ram[0xc0] >> (spriteno1 << 1)) & 0x03;
		int expand2 = (m_work_ram[0xc0] >> (spriteno2 << 1)) & 0x03;

		/* draw first sprite */
		draw_sprite(attr1, 1, y1, x1, expand1, FALSE, m_collision_bitmap, cliprect);

		/* get fingerprint */
		for (x = x1; x < x1 + SPRITE_WIDTH; x++)
			for (y = y1; y < y1 + SPRITE_HEIGHT; y++)
			{
				if (!cliprect.contains(x, y))
					continue;

				checksum = checksum + m_collision_bitmap.pix16(y, x);
			}

		/* black out second sprite */
		draw_sprite(attr2, 0, y2, x2, expand2, FALSE, m_collision_bitmap, cliprect);

		/* remove fingerprint */
		for (x = x1; x < x1 + SPRITE_WIDTH; x++)
			for (y = y1; y < y1 + SPRITE_HEIGHT; y++)
			{
				if (!cliprect.contains(x, y))
					continue;

				checksum = checksum - m_collision_bitmap.pix16(y, x);
			}
	}

	return (checksum != 0);
}



/*************************************
 *
 *  Main drawing
 *
 *************************************/

bitmap_ind16 &s2636_device::update( const rectangle &cliprect )
{
	UINT8 collision = 0;
	int spriteno;

	m_bitmap.fill(0, cliprect);

	for (spriteno = 0; spriteno < 4; spriteno++)
	{
		int color, expand, x, y;
		UINT8* attr = &m_work_ram[sprite_offsets[spriteno]];

		/* get out if sprite is turned off */
		if (attr[0x0a] == 0xff)
			continue;

		x = attr[0x0a] + m_x_offset;
		y = attr[0x0c] + m_y_offset;

		color = (m_work_ram[0xc1 + (spriteno >> 1)] >> ((spriteno & 1) ? 0 : 3)) & 0x07;
		expand = (m_work_ram[0xc0] >> (spriteno << 1)) & 0x03;

		draw_sprite(attr, color, y, x, expand, TRUE, m_bitmap, cliprect);

		/* bail if no shadow sprites */
		if ((attr[0x0b] == 0xff) || (attr[0x0d] == 0xfe))
			continue;

		x = attr[0x0b] + m_x_offset;

		while (y < 0xff)
		{
			y = y + SPRITE_HEIGHT + attr[0x0d];

			draw_sprite(attr, color, y, x, expand, TRUE, m_bitmap, cliprect);
		}
	}

	/* collision detection */
	if (check_collision(0, 1, cliprect))  collision |= 0x20;
	if (check_collision(0, 2, cliprect))  collision |= 0x10;
	if (check_collision(0, 3, cliprect))  collision |= 0x08;
	if (check_collision(1, 2, cliprect))  collision |= 0x04;
	if (check_collision(1, 3, cliprect))  collision |= 0x02;
	if (check_collision(2, 3, cliprect))  collision |= 0x01;

	m_work_ram[0xcb] = collision;

	return m_bitmap;
}


/*************************************
 *
 *  Work RAM access handlers
 *
 *************************************/

WRITE8_MEMBER( s2636_device::work_ram_w )
{
	assert(offset < m_work_ram_size);

	if ( offset == 0xc7 )
	{
		soundport_w(0, data);
	}

	m_work_ram[offset] = data;
}


READ8_MEMBER( s2636_device::work_ram_r )
{
	assert(offset < m_work_ram_size);

	return m_work_ram[offset];
}

/* Sound */

void s2636_device::soundport_w (int offset, int data)
{
	m_channel->update();
	m_reg[offset] = data;
	switch (offset)
	{
		case 0:
			m_pos = 0;
			m_level = TRUE;
			// frequency 7874/(data+1)
			m_size = machine().sample_rate() * (data + 1) /7874;
			break;
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void s2636_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int i;
	stream_sample_t *buffer = outputs[0];

	for (i = 0; i < samples; i++, buffer++)
	{
		*buffer = 0;
		if (m_reg[0] && m_pos <= m_size / 2)
		{
			*buffer = 0x7fff;
		}
		if (m_pos <= m_size)
			m_pos++;
		if (m_pos > m_size)
			m_pos = 0;
	}
}
