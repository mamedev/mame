/***************************************************************************

Atari Boxer (prototype) video emulation

***************************************************************************/

#include "driver.h"

UINT8* boxer_tile_ram;
UINT8* boxer_sprite_ram;


static void draw_boxer(running_machine *machine, bitmap_t* bitmap, const rectangle* cliprect)
{
	int n;

	for (n = 0; n < 2; n++)
	{
		const UINT8* p = memory_region(machine, n == 0 ? "user1" : "user2");

		int i;
		int j;

		int x = 196 - boxer_sprite_ram[0 + 2 * n];
		int y = 192 - boxer_sprite_ram[1 + 2 * n];

		int l = boxer_sprite_ram[4 + 2 * n] & 15;
		int r = boxer_sprite_ram[5 + 2 * n] & 15;

		for (i = 0; i < 8; i++)
		{
			for (j = 0; j < 4; j++)
			{
				UINT8 code;

				code = p[32 * l + 4 * i + j];

				drawgfx_transpen(bitmap, cliprect,
					machine->gfx[n],
					code,
					0,
					code & 0x80, 0,
					x + 8 * j,
					y + 8 * i, 1);

				code = p[32 * r + 4 * i - j + 3];

				drawgfx_transpen(bitmap, cliprect,
					machine->gfx[n],
					code,
					0,
					!(code & 0x80), 0,
					x + 8 * j + 32,
					y + 8 * i, 1);
			}
		}
	}
}


VIDEO_UPDATE( boxer )
{
	int i;
	int j;

	bitmap_fill(bitmap, cliprect, 1);

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 32; j++)
		{
			UINT8 code = boxer_tile_ram[32 * i + j];

			drawgfx_transpen(bitmap, cliprect,
				screen->machine->gfx[2],
				code,
				0,
				code & 0x40, code & 0x40,
				8 * j + 4,
				8 * (i % 2) + 32 * (i / 2), 0);
		}
	}

	draw_boxer(screen->machine, bitmap, cliprect);
	return 0;
}
