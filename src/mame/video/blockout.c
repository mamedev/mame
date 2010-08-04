/***************************************************************************

    Block Out

***************************************************************************/

#include "emu.h"
#include "includes/blockout.h"


static void setcolor( running_machine *machine, int color, int rgb )
{
	int bit0, bit1, bit2, bit3;
	int r, g, b;


	/* red component */
	bit0 = (rgb >> 0) & 0x01;
	bit1 = (rgb >> 1) & 0x01;
	bit2 = (rgb >> 2) & 0x01;
	bit3 = (rgb >> 3) & 0x01;
	r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	/* green component */
	bit0 = (rgb >> 4) & 0x01;
	bit1 = (rgb >> 5) & 0x01;
	bit2 = (rgb >> 6) & 0x01;
	bit3 = (rgb >> 7) & 0x01;
	g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	/* blue component */
	bit0 = (rgb >> 8) & 0x01;
	bit1 = (rgb >> 9) & 0x01;
	bit2 = (rgb >> 10) & 0x01;
	bit3 = (rgb >> 11) & 0x01;
	b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	palette_set_color(machine, color, MAKE_RGB(r,g,b));
}

WRITE16_HANDLER( blockout_paletteram_w )
{
	blockout_state *state = space->machine->driver_data<blockout_state>();

	COMBINE_DATA(&state->paletteram[offset]);
	setcolor(space->machine, offset, state->paletteram[offset]);
}

WRITE16_HANDLER( blockout_frontcolor_w )
{
	blockout_state *state = space->machine->driver_data<blockout_state>();

	COMBINE_DATA(&state->color);
	setcolor(space->machine, 512, state->color);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
VIDEO_START( blockout )
{
	blockout_state *state = machine->driver_data<blockout_state>();

	/* Allocate temporary bitmaps */
	state->tmpbitmap = machine->primary_screen->alloc_compatible_bitmap();
	state_save_register_global_bitmap(machine, state->tmpbitmap);
}

static void update_pixels( running_machine *machine, int x, int y )
{
	blockout_state *state = machine->driver_data<blockout_state>();
	UINT16 front, back;
	int color;
	const rectangle &visarea = machine->primary_screen->visible_area();

	if (x < visarea.min_x || x > visarea.max_x || y < visarea.min_y || y > visarea.max_y)
		return;

	front = state->videoram[y * 256 + x / 2];
	back = state->videoram[0x10000 + y * 256 + x / 2];

	if (front >> 8)
		color = front >> 8;
	else
		color = (back >> 8) + 256;

	*BITMAP_ADDR16(state->tmpbitmap, y, x) = color;

	if (front & 0xff)
		color = front & 0xff;
	else
		color = (back & 0xff) + 256;

	*BITMAP_ADDR16(state->tmpbitmap, y, x + 1) = color;
}



WRITE16_HANDLER( blockout_videoram_w )
{
	blockout_state *state = space->machine->driver_data<blockout_state>();

	COMBINE_DATA(&state->videoram[offset]);
	update_pixels(space->machine, (offset % 256) * 2, (offset / 256) % 256);
}



VIDEO_UPDATE( blockout )
{
	blockout_state *state = screen->machine->driver_data<blockout_state>();
	int x, y;
	pen_t color = 512;

	copybitmap(bitmap, state->tmpbitmap, 0, 0, 0, 0, cliprect);

	for (y = 0; y < 256; y++)
	{
		for (x = 0; x < 320; x += 8)
		{
			int d = state->frontvideoram[y * 64 + (x / 8)];

			if (d)
			{
				if (d & 0x80) *BITMAP_ADDR16(bitmap, y, x + 0) = color;
				if (d & 0x40) *BITMAP_ADDR16(bitmap, y, x + 1) = color;
				if (d & 0x20) *BITMAP_ADDR16(bitmap, y, x + 2) = color;
				if (d & 0x10) *BITMAP_ADDR16(bitmap, y, x + 3) = color;
				if (d & 0x08) *BITMAP_ADDR16(bitmap, y, x + 4) = color;
				if (d & 0x04) *BITMAP_ADDR16(bitmap, y, x + 5) = color;
				if (d & 0x02) *BITMAP_ADDR16(bitmap, y, x + 6) = color;
				if (d & 0x01) *BITMAP_ADDR16(bitmap, y, x + 7) = color;
			}
		}
	}

	return 0;
}
