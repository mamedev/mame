/***************************************************************************

    Taito Field Goal video emulation

***************************************************************************/

#include "driver.h"
#include "includes/fgoal.h"

static bitmap_t *fgbitmap;
static bitmap_t *bgbitmap;

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
	fgbitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);
	bgbitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);
}


VIDEO_UPDATE( fgoal )
{
	const UINT8* VRAM = fgoal_video_ram;

	int x;
	int y;
	int n;

	/* draw color overlay foreground and background */

	if (fgoal_player == 1 && (input_port_read(screen->machine, "IN1") & 0x40))
	{
		drawgfxzoom_opaque(fgbitmap, cliprect, screen->machine->gfx[0],
			0, (fgoal_player << 2) | current_color,
			1, 1,
			0, 16,
			0x40000,
			0x40000);

		drawgfxzoom_opaque(bgbitmap, cliprect, screen->machine->gfx[1],
			0, 0,
			1, 1,
			0, 16,
			0x40000,
			0x40000);
	}
	else
	{
		drawgfxzoom_opaque(fgbitmap, cliprect, screen->machine->gfx[0],
			0, (fgoal_player << 2) | current_color,
			0, 0,
			0, 0,
			0x40000,
			0x40000);

		drawgfxzoom_opaque(bgbitmap, cliprect, screen->machine->gfx[1],
			0, 0,
			0, 0,
			0, 0,
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
				*BITMAP_ADDR16(fgbitmap, y, x) = 128 + 16;
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
