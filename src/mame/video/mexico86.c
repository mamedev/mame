#include "emu.h"
#include "includes/mexico86.h"


WRITE8_HANDLER( mexico86_bankswitch_w )
{
	mexico86_state *state = (mexico86_state *)space->machine->driver_data;

	if ((data & 7) > 5)
		popmessage("Switching to invalid bank!");

	memory_set_bank(space->machine, "bank1", data & 0x07);

	state->charbank = BIT(data, 5);
}



VIDEO_UPDATE( mexico86 )
{
	mexico86_state *state = (mexico86_state *)screen->machine->driver_data;
	int offs;
	int sx, sy, xc, yc;
	int gfx_num, gfx_attr, gfx_offs;


	/* Bubble Bobble doesn't have a real video RAM. All graphics (characters */
	/* and sprites) are stored in the same memory region, and information on */
	/* the background character columns is stored inthe area dd00-dd3f */
	bitmap_fill(bitmap, cliprect, 255);

	sx = 0;

	/* the score display seems to be outside of the main objectram. */
	for (offs = 0; offs < state->objectram_size + 0x200; offs += 4)
	{
		int height;

		if (offs >= state->objectram_size && offs < state->objectram_size + 0x180)
			continue;

		if (offs >= state->objectram_size + 0x1c0)
			continue;

		/* skip empty sprites */
		/* this is dword aligned so the UINT32 * cast shouldn't give problems */
		/* on any architecture */
		if (*(UINT32 *)(&state->objectram[offs]) == 0)
			continue;

		gfx_num = state->objectram[offs + 1];
		gfx_attr = state->objectram[offs + 3];

		if (!BIT(gfx_num, 7))  /* 16x16 sprites */
		{
			gfx_offs = ((gfx_num & 0x1f) * 0x80) + ((gfx_num & 0x60) >> 1) + 12;
			height = 2;
		}
		else    /* tilemaps (each sprite is a 16x256 column) */
		{
			gfx_offs = ((gfx_num & 0x3f) * 0x80);
			height = 32;
		}

		if ((gfx_num & 0xc0) == 0xc0)   /* next column */
			sx += 16;
		else
		{
			sx = state->objectram[offs + 2];
			//if (gfx_attr & 0x40) sx -= 256;
		}
		sy = 256 - height * 8 - (state->objectram[offs + 0]);

		for (xc = 0; xc < 2; xc++)
		{
			for (yc = 0; yc < height; yc++)
			{
				int goffs, code, color, flipx, flipy, x, y;

				goffs = gfx_offs + xc * 0x40 + yc * 0x02;
				code = state->videoram[goffs] + ((state->videoram[goffs + 1] & 0x07) << 8)
						+ ((state->videoram[goffs + 1] & 0x80) << 4) + (state->charbank << 12);
				color = ((state->videoram[goffs + 1] & 0x38) >> 3) + ((gfx_attr & 0x02) << 2);
				flipx = state->videoram[goffs + 1] & 0x40;
				flipy = 0;

				//x = sx + xc * 8;
				x = (sx + xc * 8) & 0xff;
				y = (sy + yc * 8) & 0xff;

				drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],
						code,
						color,
						flipx,flipy,
						x,y,15);
			}
		}
	}
	return 0;
}

VIDEO_UPDATE( kikikai )
{
	mexico86_state *state = (mexico86_state *)screen->machine->driver_data;
	int offs;
	int sx, sy, yc;
	int gfx_num, /*gfx_attr,*/ gfx_offs;
	int height;
	int goffs, code, color, y;
	int tx, ty;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	sx = 0;
	for (offs = 0; offs < state->objectram_size; offs += 4)
	{
		if (*(UINT32*)(state->objectram + offs) == 0)
			continue;

		ty = state->objectram[offs];
		gfx_num = state->objectram[offs + 1];
		tx = state->objectram[offs + 2];
		//gfx_attr = state->objectram[offs + 3];

		if (gfx_num & 0x80)
		{
			gfx_offs = ((gfx_num & 0x3f) << 7);
			height = 32;
			if (gfx_num & 0x40) sx += 16;
			else sx = tx;
		}
		else
		{
			if (!(ty && tx)) continue;
			gfx_offs = ((gfx_num & 0x1f) << 7) + ((gfx_num & 0x60) >> 1) + 12;
			height = 2;
			sx = tx;
		}

		sy = 256 - (height << 3) - ty;

		height <<= 1;
		for (yc = 0; yc < height; yc += 2)
		{
			y = (sy + (yc << 2)) & 0xff;
			goffs = gfx_offs + yc;
			code = state->videoram[goffs] + ((state->videoram[goffs + 1] & 0x1f) << 8);
			color = (state->videoram[goffs + 1] & 0xe0) >> 5;
			goffs += 0x40;

			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],
					code,
					color,
					0,0,
					sx&0xff,y,15);

			code = state->videoram[goffs] + ((state->videoram[goffs + 1] & 0x1f) << 8);
			color = (state->videoram[goffs + 1] & 0xe0) >> 5;

			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],
					code,
					color,
					0,0,
					(sx+8)&0xff,y,15);
		}
	}
	return 0;
}
