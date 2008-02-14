/***************************************************************************

Atari Boxer (prototype) video emulation

***************************************************************************/

#include "driver.h"

UINT8* boxer_tile_ram;
UINT8* boxer_sprite_ram;


static void draw_boxer(running_machine *machine, mame_bitmap* bitmap, const rectangle* cliprect)
{
	int n;

	for (n = 0; n < 2; n++)
	{
		const UINT8* p = memory_region(n == 0 ? REGION_USER1 : REGION_USER2);

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

				drawgfx(bitmap, machine->gfx[n],
					code,
					0,
					code & 0x80, 0,
					x + 8 * j,
					y + 8 * i,
					cliprect,
					TRANSPARENCY_PEN, 1);

				code = p[32 * r + 4 * i - j + 3];

				drawgfx(bitmap, machine->gfx[n],
					code,
					0,
					!(code & 0x80), 0,
					x + 8 * j + 32,
					y + 8 * i,
					cliprect,
					TRANSPARENCY_PEN, 1);
			}
		}
	}
}


VIDEO_UPDATE( boxer )
{
	int i;
	int j;

	fillbitmap(bitmap, 1, cliprect);

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 32; j++)
		{
			UINT8 code = boxer_tile_ram[32 * i + j];

			drawgfx(bitmap, machine->gfx[2],
				code,
				0,
				code & 0x40, code & 0x40,
				8 * j + 4,
				8 * (i % 2) + 32 * (i / 2),
				cliprect,
				TRANSPARENCY_PEN, 0);
		}
	}

	draw_boxer(machine, bitmap, cliprect);
	return 0;
}
