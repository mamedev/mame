/***************************************************************************

First version of the Dynax blitter.

Can handle up to 8 256x256 bitmaps; in the games supported, every pair of
bitmaps is interleaved horizontally to form 4 higher res 512x256 layer.

The blitter reads compressed data from ROM and copies it to the bitmap RAM.

***************************************************************************/

#include "emu.h"
#include "includes/hnayayoi.h"

static void common_vh_start( running_machine *machine, int num_pixmaps )
{
	hnayayoi_state *state = machine->driver_data<hnayayoi_state>();
	int i;

	state->total_pixmaps = num_pixmaps;

	for (i = 0; i < 8; i++)
	{
		if (i < state->total_pixmaps)
		{
			state->pixmap[i] = auto_alloc_array(machine, UINT8, 256 * 256);
		}
		else
			state->pixmap[i] = NULL;
	}
}

VIDEO_START( hnayayoi )
{
	hnayayoi_state *state = machine->driver_data<hnayayoi_state>();
	common_vh_start(machine, 4);	/* 4 bitmaps -> 2 layers */

	state_save_register_global_pointer(machine, state->pixmap[0], 256 * 256);
	state_save_register_global_pointer(machine, state->pixmap[1], 256 * 256);
	state_save_register_global_pointer(machine, state->pixmap[2], 256 * 256);
	state_save_register_global_pointer(machine, state->pixmap[3], 256 * 256);
}

VIDEO_START( untoucha )
{
	hnayayoi_state *state = machine->driver_data<hnayayoi_state>();
	common_vh_start(machine, 8);	/* 8 bitmaps -> 4 layers */

	state_save_register_global_pointer(machine, state->pixmap[0], 256 * 256);
	state_save_register_global_pointer(machine, state->pixmap[1], 256 * 256);
	state_save_register_global_pointer(machine, state->pixmap[2], 256 * 256);
	state_save_register_global_pointer(machine, state->pixmap[3], 256 * 256);
	state_save_register_global_pointer(machine, state->pixmap[4], 256 * 256);
	state_save_register_global_pointer(machine, state->pixmap[5], 256 * 256);
	state_save_register_global_pointer(machine, state->pixmap[6], 256 * 256);
	state_save_register_global_pointer(machine, state->pixmap[7], 256 * 256);
}



/***************************************************************************

Blitter support

three parameters:
blit_layer: mask of the bitmaps to write to (can write to multiple bitmaps
            at the same time)
blit_dest:  position in the destination bitmap where to start blitting
blit_src:   address of source data in the gfx ROM

additional parameters specify the palette base, but this is handled while rendering
the screen, not during blitting (games change the palette base without redrawing
the screen).

It is not known whether the palette base control registers are part of the blitter
hardware or latched somewhere else. Since they are mapped in memory immediately
before the bitter parameters, they probably are part of the blitter, but I'm
handling them separately anyway.


The format of the blitter data stored in ROM is very simple:

7654 ----   Pen to draw with
---- 3210   Command

Commands:

0       Stop
1-b     Draw 1-b pixels along X.
c       Followed by 1 byte (N): draw N pixels along X.
d       Followed by 2 bytes (X,N): move on the line to pixel (start+X), draw N pixels
        along X.
e       Followed by 1 byte (N): set blit_layer = N. Used to draw interleaved graphics
        with a single blitter run.
f       Move to next line.

At the end of the blit, blit_src is left pointing to the next data in the gfx ROM.
This is used to draw interleaved graphics with two blitter runs without having to set
up blit_src for the second call.

***************************************************************************/

WRITE8_HANDLER( dynax_blitter_rev1_param_w )
{
	hnayayoi_state *state = space->machine->driver_data<hnayayoi_state>();
	switch (offset)
	{
		case 0: state->blit_dest = (state->blit_dest & 0xff00) | (data << 0); break;
		case 1: state->blit_dest = (state->blit_dest & 0x00ff) | (data << 8); break;
		case 2: state->blit_layer = data; break;
		case 3: state->blit_src = (state->blit_src & 0xffff00) | (data << 0); break;
		case 4: state->blit_src = (state->blit_src & 0xff00ff) | (data << 8); break;
		case 5: state->blit_src = (state->blit_src & 0x00ffff) | (data <<16); break;
	}
}

static void copy_pixel( running_machine *machine, int x, int y, int pen )
{
	hnayayoi_state *state = machine->driver_data<hnayayoi_state>();
	if (x >= 0 && x <= 255 && y >= 0 && y <= 255)
	{
		int i;

		for (i = 0; i < 8; i++)
		{
			if ((~state->blit_layer & (1 << i)) && (state->pixmap[i]))
				state->pixmap[i][256 * y + x] = pen;
		}
	}
}

WRITE8_HANDLER( dynax_blitter_rev1_start_w )
{
	hnayayoi_state *state = space->machine->driver_data<hnayayoi_state>();
	UINT8 *rom = memory_region(space->machine, "gfx1");
	int romlen = memory_region_length(space->machine, "gfx1");
	int sx = state->blit_dest & 0xff;
	int sy = state->blit_dest >> 8;
	int x, y;

	x = sx;
	y = sy;
	while (state->blit_src < romlen)
	{
		int cmd = rom[state->blit_src] & 0x0f;
		int pen = rom[state->blit_src] >> 4;

		state->blit_src++;

		switch (cmd)
		{
			case 0xf:
				y++;
				x = sx;
				break;

			case 0xe:
				if (state->blit_src >= romlen)
				{
					popmessage("GFXROM OVER %06x", state->blit_src);
					return;
				}
				x = sx;
				state->blit_layer = rom[state->blit_src++];
				break;

			case 0xd:
				if (state->blit_src >= romlen)
				{
					popmessage("GFXROM OVER %06x", state->blit_src);
					return;
				}
				x = sx + rom[state->blit_src++];
				/* fall through into next case */

			case 0xc:
				if (state->blit_src >= romlen)
				{
					popmessage("GFXROM OVER %06x", state->blit_src);
					return;
				}
				cmd = rom[state->blit_src++];
				/* fall through into next case */

			case 0xb:
			case 0xa:
			case 0x9:
			case 0x8:
			case 0x7:
			case 0x6:
			case 0x5:
			case 0x4:
			case 0x3:
			case 0x2:
			case 0x1:
				while (cmd--)
					copy_pixel(space->machine, x++, y, pen);
				break;

			case 0x0:
				return;
		}
	}

	popmessage("GFXROM OVER %06x", state->blit_src);
}

WRITE8_HANDLER( dynax_blitter_rev1_clear_w )
{
	hnayayoi_state *state = space->machine->driver_data<hnayayoi_state>();
	int pen = data >> 4;
	int i;

	for (i = 0; i < 8; i++)
	{
		if ((~state->blit_layer & (1 << i)) && (state->pixmap[i]))
			memset(state->pixmap[i] + state->blit_dest, pen, 0x10000 - state->blit_dest);
	}
}


WRITE8_HANDLER( hnayayoi_palbank_w )
{
	hnayayoi_state *state = space->machine->driver_data<hnayayoi_state>();
	offset *= 8;
	state->palbank = (state->palbank & (0xff00 >> offset)) | (data << offset);
}


static void draw_layer_interleaved( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int left_pixmap, int right_pixmap, int palbase, int transp )
{
	hnayayoi_state *state = machine->driver_data<hnayayoi_state>();
	int county, countx, pen;
	UINT8 *src1 = state->pixmap[left_pixmap];
	UINT8 *src2 = state->pixmap[right_pixmap];
	UINT16 *dstbase = (UINT16 *)bitmap->base;

	palbase *= 16;

	for (county = 255; county >= 0; county--, dstbase += bitmap->rowpixels)
	{
		UINT16 *dst = dstbase;

		if (transp)
		{
			for (countx = 255; countx >= 0; countx--, dst += 2)
			{
				pen = *(src1++);
				if (pen) *dst = palbase + pen;
				pen = *(src2++);
				if (pen) *(dst + 1) = palbase + pen;
			}
		}
		else
		{
			for (countx = 255; countx >= 0; countx--, dst += 2)
			{
				*dst = palbase + *(src1++);
				*(dst + 1) = palbase + *(src2++);
			}
		}
	}
}


VIDEO_UPDATE( hnayayoi )
{
	hnayayoi_state *state = screen->machine->driver_data<hnayayoi_state>();
	int col0 = (state->palbank >>  0) & 0x0f;
	int col1 = (state->palbank >>  4) & 0x0f;
	int col2 = (state->palbank >>  8) & 0x0f;
	int col3 = (state->palbank >> 12) & 0x0f;

	if (state->total_pixmaps == 4)
	{
		draw_layer_interleaved(screen->machine, bitmap, cliprect, 3, 2, col1, 0);
		draw_layer_interleaved(screen->machine, bitmap, cliprect, 1, 0, col0, 1);
	}
	else	/* total_pixmaps == 8 */
	{
		draw_layer_interleaved(screen->machine, bitmap, cliprect, 7, 6, col3, 0);
		draw_layer_interleaved(screen->machine, bitmap, cliprect, 5, 4, col2, 1);
		draw_layer_interleaved(screen->machine, bitmap, cliprect, 3, 2, col1, 1);
		draw_layer_interleaved(screen->machine, bitmap, cliprect, 1, 0, col0, 1);
	}
	return 0;
}
