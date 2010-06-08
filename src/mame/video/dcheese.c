/***************************************************************************

    HAR MadMax hardware

**************************************************************************/


#include "emu.h"
#include "includes/dcheese.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define DSTBITMAP_WIDTH		512
#define DSTBITMAP_HEIGHT	512


/*************************************
 *
 *  Palette translation
 *
 *************************************/

PALETTE_INIT( dcheese )
{
	const UINT16 *src = (UINT16 *)memory_region(machine, "user1");
	int i;

	/* really 65536 colors, but they don't use the later ones so we can stay */
	/* within MAME's limits */
	for (i = 0; i < 65534; i++)
	{
		int data = *src++;
		palette_set_color_rgb(machine, i, pal6bit(data >> 0), pal5bit(data >> 6), pal5bit(data >> 11));
	}
}



/*************************************
 *
 *  Scanline interrupt
 *
 *************************************/

static void update_scanline_irq( running_machine *machine )
{
	dcheese_state *state = (dcheese_state *)machine->driver_data;

	/* if not in range, don't bother */
	if (state->blitter_vidparam[0x22/2] <= state->blitter_vidparam[0x1e/2])
	{
		int effscan;
		attotime time;

		/* compute the effective scanline of the interrupt */
		effscan = state->blitter_vidparam[0x22/2] - state->blitter_vidparam[0x1a/2];
		if (effscan < 0)
			effscan += state->blitter_vidparam[0x1e/2];

		/* determine the time; if it's in this scanline, bump to the next frame */
		time = machine->primary_screen->time_until_pos(effscan);
		if (attotime_compare(time, machine->primary_screen->scan_period()) < 0)
			time = attotime_add(time, machine->primary_screen->frame_period());
		timer_adjust_oneshot(state->blitter_timer, time, 0);
	}
}


static TIMER_CALLBACK( blitter_scanline_callback )
{
	dcheese_signal_irq(machine, 3);
	update_scanline_irq(machine);
}


static TIMER_CALLBACK( dcheese_signal_irq_callback )
{
	dcheese_signal_irq(machine, param);
}


/*************************************
 *
 *  Video start
 *
 *************************************/

VIDEO_START( dcheese )
{
	dcheese_state *state = (dcheese_state *)machine->driver_data;

	/* the destination bitmap is not directly accessible to the CPU */
	state->dstbitmap = auto_bitmap_alloc(machine, DSTBITMAP_WIDTH, DSTBITMAP_HEIGHT, machine->primary_screen->format());

	/* create a timer */
	state->blitter_timer = timer_alloc(machine, blitter_scanline_callback, NULL);

	/* register for saving */
	state_save_register_global_array(machine, state->blitter_color);
	state_save_register_global_array(machine, state->blitter_xparam);
	state_save_register_global_array(machine, state->blitter_yparam);
	state_save_register_global_array(machine, state->blitter_vidparam);
	state_save_register_global_bitmap(machine, state->dstbitmap);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( dcheese )
{
	dcheese_state *state = (dcheese_state *)screen->machine->driver_data;
	int x, y;

	/* update the pixels */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT16 *dest = BITMAP_ADDR16(bitmap, y, 0);
		UINT16 *src = BITMAP_ADDR16(state->dstbitmap, (y + state->blitter_vidparam[0x28/2]) % DSTBITMAP_HEIGHT, 0);

		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			dest[x] = src[x];
	}
	return 0;
}



/*************************************
 *
 *  Blitter implementation
 *
 *************************************/

static void do_clear( running_machine *machine )
{
	dcheese_state *state = (dcheese_state *)machine->driver_data;
	int y;

	/* clear the requested scanlines */
	for (y = state->blitter_vidparam[0x2c/2]; y < state->blitter_vidparam[0x2a/2]; y++)
		memset(BITMAP_ADDR16(state->dstbitmap, y % DSTBITMAP_HEIGHT, 0), 0, DSTBITMAP_WIDTH * 2);

	/* signal an IRQ when done (timing is just a guess) */
	timer_set(machine, machine->primary_screen->scan_period(), NULL, 1, dcheese_signal_irq_callback);
}


static void do_blit( running_machine *machine )
{
	dcheese_state *state = (dcheese_state *)machine->driver_data;
	INT32 srcminx = state->blitter_xparam[0] << 12;
	INT32 srcmaxx = state->blitter_xparam[1] << 12;
	INT32 srcminy = state->blitter_yparam[0] << 12;
	INT32 srcmaxy = state->blitter_yparam[1] << 12;
	INT32 srcx = ((state->blitter_xparam[2] & 0x0fff) | ((state->blitter_xparam[3] & 0x0fff) << 12)) << 7;
	INT32 srcy = ((state->blitter_yparam[2] & 0x0fff) | ((state->blitter_yparam[3] & 0x0fff) << 12)) << 7;
	INT32 dxdx = (INT32)(((state->blitter_xparam[4] & 0x0fff) | ((state->blitter_xparam[5] & 0x0fff) << 12)) << 12) >> 12;
	INT32 dxdy = (INT32)(((state->blitter_xparam[6] & 0x0fff) | ((state->blitter_xparam[7] & 0x0fff) << 12)) << 12) >> 12;
	INT32 dydx = (INT32)(((state->blitter_yparam[4] & 0x0fff) | ((state->blitter_yparam[5] & 0x0fff) << 12)) << 12) >> 12;
	INT32 dydy = (INT32)(((state->blitter_yparam[6] & 0x0fff) | ((state->blitter_yparam[7] & 0x0fff) << 12)) << 12) >> 12;
	UINT8 *src = memory_region(machine, "gfx1");
	UINT32 pagemask = (memory_region_length(machine, "gfx1") - 1) / 0x40000;
	int xstart = state->blitter_xparam[14];
	int xend = state->blitter_xparam[15] + 1;
	int ystart = state->blitter_yparam[14];
	int yend = state->blitter_yparam[15];
	int color = (state->blitter_color[0] << 8) & 0xff00;
	int mask = (state->blitter_color[0] >> 8) & 0x00ff;
	int opaque = (dxdx | dxdy | dydx | dydy) == 0;	/* bit of a hack for fredmem */
	int x, y;

	/* loop over target rows */
	for (y = ystart; y <= yend; y++)
	{
		UINT16 *dst = BITMAP_ADDR16(state->dstbitmap, y % DSTBITMAP_HEIGHT, 0);

		/* loop over target columns */
		for (x = xstart; x <= xend; x++)
		{
			/* compute current X/Y positions */
			int sx = (srcx + dxdx * (x - xstart) + dxdy * (y - ystart)) & 0xffffff;
			int sy = (srcy + dydx * (x - xstart) + dydy * (y - ystart)) & 0xffffff;

			/* clip to source cliprect */
			if (sx >= srcminx && sx <= srcmaxx && sy >= srcminy && sy <= srcmaxy)
			{
				/* page comes from bit 22 of Y and bit 21 of X */
				int page = (((sy >> 21) & 2) | ((sx >> 21) & 1) | ((sx >> 20) & 4)) & pagemask;
				int pix = src[0x40000 * page + ((sy >> 12) & 0x1ff) * 512 + ((sx >> 12) & 0x1ff)];

				/* only non-zero pixels get written */
				if (pix | opaque)
					dst[x % DSTBITMAP_WIDTH] = (pix & mask) | color;
			}
		}
	}

	/* signal an IRQ when done (timing is just a guess) */
	timer_set(machine, attotime_make(0, attotime_to_attoseconds(machine->primary_screen->scan_period()) / 2), NULL, 2, dcheese_signal_irq_callback);

	/* these extra parameters are written but they are always zero, so I don't know what they do */
	if (state->blitter_xparam[8] != 0 || state->blitter_xparam[9] != 0 || state->blitter_xparam[10] != 0 || state->blitter_xparam[11] != 0 ||
		state->blitter_yparam[8] != 0 || state->blitter_yparam[9] != 0 || state->blitter_yparam[10] != 0 || state->blitter_yparam[11] != 0)
	{
		logerror("%s:blit! (%04X)\n", cpuexec_describe_context(machine), state->blitter_color[0]);
		logerror("   %04X %04X %04X %04X - %04X %04X %04X %04X - %04X %04X %04X %04X - %04X %04X %04X %04X\n",
				state->blitter_xparam[0], state->blitter_xparam[1], state->blitter_xparam[2], state->blitter_xparam[3],
				state->blitter_xparam[4], state->blitter_xparam[5], state->blitter_xparam[6], state->blitter_xparam[7],
				state->blitter_xparam[8], state->blitter_xparam[9], state->blitter_xparam[10], state->blitter_xparam[11],
				state->blitter_xparam[12], state->blitter_xparam[13], state->blitter_xparam[14], state->blitter_xparam[15]);
		logerror("   %04X %04X %04X %04X - %04X %04X %04X %04X - %04X %04X %04X %04X - %04X %04X %04X %04X\n",
				state->blitter_yparam[0], state->blitter_yparam[1], state->blitter_yparam[2], state->blitter_yparam[3],
				state->blitter_yparam[4], state->blitter_yparam[5], state->blitter_yparam[6], state->blitter_yparam[7],
				state->blitter_yparam[8], state->blitter_yparam[9], state->blitter_yparam[10], state->blitter_yparam[11],
				state->blitter_yparam[12], state->blitter_yparam[13], state->blitter_yparam[14], state->blitter_yparam[15]);
	}
}



/*************************************
 *
 *  Blitter read/write
 *
 *************************************/

WRITE16_HANDLER( madmax_blitter_color_w )
{
	dcheese_state *state = (dcheese_state *)space->machine->driver_data;
	COMBINE_DATA(&state->blitter_color[offset]);
}


WRITE16_HANDLER( madmax_blitter_xparam_w )
{
	dcheese_state *state = (dcheese_state *)space->machine->driver_data;
	COMBINE_DATA(&state->blitter_xparam[offset]);
}


WRITE16_HANDLER( madmax_blitter_yparam_w )
{
	dcheese_state *state = (dcheese_state *)space->machine->driver_data;
	COMBINE_DATA(&state->blitter_yparam[offset]);
}


WRITE16_HANDLER( madmax_blitter_vidparam_w )
{
	dcheese_state *state = (dcheese_state *)space->machine->driver_data;
	COMBINE_DATA(&state->blitter_vidparam[offset]);

	switch (offset)
	{
		case 0x10/2:		/* horiz front porch */
		case 0x12/2:		/* horiz display start */
		case 0x14/2:		/* horiz display end */
		case 0x16/2:		/* horiz back porch */

		case 0x18/2:		/* vert front porch */
		case 0x1a/2:		/* vert display start */
		case 0x1c/2:		/* vert display end */
		case 0x1e/2:		/* vert back porch */
			break;

		case 0x22/2:		/* scanline interrupt */
			update_scanline_irq(space->machine);
			break;

		case 0x24/2:		/* writes here after writing to 0x28 */
			break;

		case 0x28/2:		/* display starting y */
		case 0x2a/2:		/* clear end y */
		case 0x2c/2:		/* clear start y */
			break;

		case 0x38/2:		/* blit */
			do_blit(space->machine);
			break;

		case 0x3e/2:		/* clear */
			do_clear(space->machine);
			break;

		default:
			logerror("%06X:write to %06X = %04X & %04x\n", cpu_get_pc(space->cpu), 0x2a0000 + 2 * offset, data, mem_mask);
			break;
	}
}


WRITE16_HANDLER( madmax_blitter_unknown_w )
{
	/* written to just before the blitter command register is written */
	logerror("%06X:write to %06X = %04X & %04X\n", cpu_get_pc(space->cpu), 0x300000 + 2 * offset, data, mem_mask);
}


READ16_HANDLER( madmax_blitter_vidparam_r )
{
	/* analog inputs seem to be hooked up here -- might not actually map to blitter */
	if (offset == 0x02/2)
		return input_port_read(space->machine, "2a0002");
	if (offset == 0x0e/2)
		return input_port_read(space->machine, "2a000e");

	/* early code polls on this bit, wants it to be 0 */
	if (offset == 0x36/2)
		return 0xffff ^ (1 << 5);

	/* log everything else */
	logerror("%06X:read from %06X\n", cpu_get_pc(space->cpu), 0x2a0000 + 2 * offset);
	return 0xffff;
}
