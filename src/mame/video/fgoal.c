/***************************************************************************

    Taito Field Goal video emulation

***************************************************************************/

#include "driver.h"
#include "includes/fgoal.h"

static mame_bitmap *fgbitmap;
static mame_bitmap *bgbitmap;

static UINT8 xpos;
static UINT8 ypos;

static int current_color;


WRITE8_HANDLER( fgoal_color_w )
{
	current_color = data & 3;
}


WRITE8_HANDLER( fgoal_ypos_w )
{
	ypos = data;
}


WRITE8_HANDLER( fgoal_xpos_w )
{
	xpos = data;
}


VIDEO_START( fgoal )
{
	fgbitmap = auto_bitmap_alloc(256, 256, machine->screen[0].format);
	bgbitmap = auto_bitmap_alloc(256, 256, machine->screen[0].format);
}


VIDEO_UPDATE( fgoal )
{
	const UINT8* VRAM = fgoal_video_ram;

	int x;
	int y;
	int n;

	/* draw color overlay foreground and background */

	if (fgoal_player == 1 && (readinputport(1) & 0x40))
	{
		drawgfxzoom(fgbitmap, machine->gfx[0],
			0, (fgoal_player << 2) | current_color,
			1, 1,
			0, 16,
			cliprect, TRANSPARENCY_NONE, 0,
			0x40000,
			0x40000);

		drawgfxzoom(bgbitmap, machine->gfx[1],
			0, 0,
			1, 1,
			0, 16,
			cliprect, TRANSPARENCY_NONE, 0,
			0x40000,
			0x40000);
	}
	else
	{
		drawgfxzoom(fgbitmap, machine->gfx[0],
			0, (fgoal_player << 2) | current_color,
			0, 0,
			0, 0,
			cliprect, TRANSPARENCY_NONE, 0,
			0x40000,
			0x40000);

		drawgfxzoom(bgbitmap, machine->gfx[1],
			0, 0,
			0, 0,
			0, 0,
			cliprect, TRANSPARENCY_NONE, 0,
			0x40000,
			0x40000);
	}

	/* the ball has a fixed color */

	for (y = ypos; y < ypos + 8; y++)
	{
		for (x = xpos; x < xpos + 8; x++)
		{
			if (y < 256 && x < 256)
			{
				*BITMAP_ADDR16(fgbitmap, y, x) = 0x38;
			}
		}
	}

	/* draw bitmap layer */

	for (y = 0; y < 256; y++)
	{
		UINT16* p = BITMAP_ADDR16(bitmap, y, 0);

		const UINT16* FG = BITMAP_ADDR16(fgbitmap, y, 0);
		const UINT16* BG = BITMAP_ADDR16(bgbitmap, y, 0);

		for (x = 0; x < 256; x += 8)
		{
			UINT8 v = *VRAM++;

			for (n = 0; n < 8; n++)
			{
				if (v & (1 << n))
				{
					p[x + n] = FG[x + n];
				}
				else
				{
					p[x + n] = BG[x + n];
				}
			}
		}
	}
	return 0;
}
