/***************************************************************************

Taito Super Speed Race video emulation

***************************************************************************/

#include "driver.h"

static int toggle;

static unsigned driver_horz;
static unsigned driver_vert;
static unsigned driver_pic;

static unsigned drones_horz;
static unsigned drones_vert[3];
static unsigned drones_mask;

static unsigned track_horz;
static unsigned track_vert[2];
static unsigned track_ice;


WRITE8_HANDLER( sspeedr_driver_horz_w )
{
	driver_horz = (driver_horz & 0x100) | data;
}


WRITE8_HANDLER( sspeedr_driver_horz_2_w )
{
	driver_horz = (driver_horz & 0xff) | ((data & 1) << 8);
}


WRITE8_HANDLER( sspeedr_driver_vert_w )
{
	driver_vert = data;
}


WRITE8_HANDLER( sspeedr_driver_pic_w )
{
	driver_pic = data & 0x1f;
}


WRITE8_HANDLER( sspeedr_drones_horz_w )
{
	drones_horz = (drones_horz & 0x100) | data;
}


WRITE8_HANDLER( sspeedr_drones_horz_2_w )
{
	drones_horz = (drones_horz & 0xff) | ((data & 1) << 8);
}


WRITE8_HANDLER( sspeedr_drones_mask_w )
{
	drones_mask = data & 0x3f;
}


WRITE8_HANDLER( sspeedr_drones_vert_w )
{
	drones_vert[offset] = data;
}


WRITE8_HANDLER( sspeedr_track_horz_w )
{
	track_horz = (track_horz & 0x100) | data;
}


WRITE8_HANDLER( sspeedr_track_horz_2_w )
{
	track_horz = (track_horz & 0xff) | ((data & 1) << 8);
}


WRITE8_HANDLER( sspeedr_track_vert_w )
{
	track_vert[offset] = data & 0x7f;
}


WRITE8_HANDLER( sspeedr_track_ice_w )
{
	track_ice = data & 0x07;
}


static void draw_track(mame_bitmap* bitmap)
{
	const UINT8* p = memory_region(REGION_GFX3);

	int x;
	int y;

	for (x = 0; x < 376; x++)
	{
		unsigned counter_x = x + track_horz + 0x50;

		int flag = 0;

		if (track_ice & 2)
		{
			flag = 1;
		}
		else if (track_ice & 4)
		{
			if (track_ice & 1)
			{
				flag = (counter_x <= 0x1ff);
			}
			else
			{
				flag = (counter_x >= 0x200);
			}
		}

		if (counter_x >= 0x200)
		{
			counter_x -= 0x1c8;
		}

		y = 0;

		/* upper landscape */

		for (; y < track_vert[0]; y++)
		{
			unsigned counter_y = y - track_vert[0];

			int offset =
				((counter_y & 0x1f) << 3) |
				((counter_x & 0x1c) >> 2) |
				((counter_x & 0xe0) << 3);

			if (counter_x & 2)
			{
				*BITMAP_ADDR16(bitmap, y, x) = p[offset] / 16;
			}
			else
			{
				*BITMAP_ADDR16(bitmap, y, x) = p[offset] % 16;
			}
		}

		/* street */

		for (; y < 128 + track_vert[1]; y++)
		{
			*BITMAP_ADDR16(bitmap, y, x) = flag ? 15 : 0;
		}

		/* lower landscape */

		for (; y < 248; y++)
		{
			unsigned counter_y = y - track_vert[1];

			int offset =
				((counter_y & 0x1f) << 3) |
				((counter_x & 0x1c) >> 2) |
				((counter_x & 0xe0) << 3);

			if (counter_x & 2)
			{
				*BITMAP_ADDR16(bitmap, y, x) = p[offset] / 16;
			}
			else
			{
				*BITMAP_ADDR16(bitmap, y, x) = p[offset] % 16;
			}
		}
	}
}


static void draw_drones(running_machine *machine, mame_bitmap* bitmap, const rectangle* cliprect)
{
	static const UINT8 code[6] =
	{
		0xf, 0x4, 0x3, 0x9, 0x7, 0xc
	};

	int i;

	for (i = 0; i < 6; i++)
	{
		int x;
		int y;

		if ((drones_mask >> i) & 1)
		{
			continue;
		}

		x = (code[i] << 5) - drones_horz - 0x50;

		if (x <= -32)
		{
			x += 0x1c8;
		}

		y = 0xf0 - drones_vert[i >> 1];

		drawgfx(bitmap, machine->gfx[1],
			code[i] ^ toggle,
			0,
			0, 0,
			x,
			y,
			cliprect,
			TRANSPARENCY_PEN, 0);
	}
}


static void draw_driver(running_machine *machine, mame_bitmap* bitmap, const rectangle* cliprect)
{
	int x;
	int y;

	if (!(driver_pic & 0x10))
	{
		return;
	}

	x = 0x1e0 - driver_horz - 0x50;

	if (x <= -32)
	{
		x += 0x1c8;
	}

	y = 0xf0 - driver_vert;

	drawgfx(bitmap, machine->gfx[0],
		driver_pic,
		0,
		0, 0,
		x,
		y,
		cliprect,
		TRANSPARENCY_PEN, 0);
}


VIDEO_START( sspeedr )
{
	toggle = 0;
}


VIDEO_UPDATE( sspeedr )
{
	draw_track(bitmap);

	draw_drones(machine, bitmap, cliprect);

	draw_driver(machine, bitmap, cliprect);
	return 0;
}


VIDEO_EOF( sspeedr )
{
	toggle ^= 1;
}
