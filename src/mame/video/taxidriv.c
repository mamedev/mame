#include "emu.h"
#include "includes/taxidriv.h"


WRITE8_DEVICE_HANDLER( taxidriv_spritectrl_w )
{
	taxidriv_state *state = device->machine().driver_data<taxidriv_state>();
	state->m_spritectrl[offset] = data;
}



SCREEN_UPDATE( taxidriv )
{
	taxidriv_state *state = screen->machine().driver_data<taxidriv_state>();
	int offs;
	int sx,sy;


	if (state->m_bghide)
	{
		bitmap_fill(bitmap,cliprect,0);


		/* kludge to fix scroll after death */
		state->m_scroll[0] = state->m_scroll[1] = state->m_scroll[2] = state->m_scroll[3] = 0;
		state->m_spritectrl[2] = state->m_spritectrl[5] = state->m_spritectrl[8] = 0;
	}
	else
	{
		for (offs = 0;offs < 0x400;offs++)
		{
			sx = offs % 32;
			sy = offs / 32;

			drawgfx_opaque(bitmap,cliprect,screen->machine().gfx[3],
					state->m_vram3[offs],
					0,
					0,0,
					(sx*8-state->m_scroll[0])&0xff,(sy*8-state->m_scroll[1])&0xff);
		}

		for (offs = 0;offs < 0x400;offs++)
		{
			sx = offs % 32;
			sy = offs / 32;

			drawgfx_transpen(bitmap,cliprect,screen->machine().gfx[2],
					state->m_vram2[offs]+256*state->m_vram2[offs+0x400],
					0,
					0,0,
					(sx*8-state->m_scroll[2])&0xff,(sy*8-state->m_scroll[3])&0xff,0);
		}

		if (state->m_spritectrl[2] & 4)
		{
			for (offs = 0;offs < 0x1000;offs++)
			{
				int color;

				sx = ((offs/2) % 64-state->m_spritectrl[0]-256*(state->m_spritectrl[2]&1))&0x1ff;
				sy = ((offs/2) / 64-state->m_spritectrl[1]-128*(state->m_spritectrl[2]&2))&0x1ff;

				color = (state->m_vram5[offs/4]>>(2*(offs&3)))&0x03;
				if (color)
				{
					if (sx > 0 && sx < 256 && sy > 0 && sy < 256)
						*BITMAP_ADDR16(bitmap, sy, sx) = color;
				}
			}
		}

		if (state->m_spritectrl[5] & 4)
		{
			for (offs = 0;offs < 0x1000;offs++)
			{
				int color;

				sx = ((offs/2) % 64-state->m_spritectrl[3]-256*(state->m_spritectrl[5]&1))&0x1ff;
				sy = ((offs/2) / 64-state->m_spritectrl[4]-128*(state->m_spritectrl[5]&2))&0x1ff;

				color = (state->m_vram6[offs/4]>>(2*(offs&3)))&0x03;
				if (color)
				{
					if (sx > 0 && sx < 256 && sy > 0 && sy < 256)
						*BITMAP_ADDR16(bitmap, sy, sx) = color;
				}
			}
		}

		if (state->m_spritectrl[8] & 4)
		{
			for (offs = 0;offs < 0x1000;offs++)
			{
				int color;

				sx = ((offs/2) % 64-state->m_spritectrl[6]-256*(state->m_spritectrl[8]&1))&0x1ff;
				sy = ((offs/2) / 64-state->m_spritectrl[7]-128*(state->m_spritectrl[8]&2))&0x1ff;

				color = (state->m_vram7[offs/4]>>(2*(offs&3)))&0x03;
				if (color)
				{
					if (sx > 0 && sx < 256 && sy > 0 && sy < 256)
						*BITMAP_ADDR16(bitmap, sy, sx) = color;
				}
			}
		}

		for (offs = 0;offs < 0x400;offs++)
		{
			sx = offs % 32;
			sy = offs / 32;

			drawgfx_transpen(bitmap,cliprect,screen->machine().gfx[1],
					state->m_vram1[offs],
					0,
					0,0,
					sx*8,sy*8,0);
		}

		for (offs = 0;offs < 0x2000;offs++)
		{
			int color;

			sx = (offs/2) % 64;
			sy = (offs/2) / 64;

			color = (state->m_vram4[offs/4]>>(2*(offs&3)))&0x03;
			if (color)
			{
				*BITMAP_ADDR16(bitmap, sy, sx) = 2 * color;
			}
		}
	}

	for (offs = 0;offs < 0x400;offs++)
	{
		sx = offs % 32;
		sy = offs / 32;

		drawgfx_transpen(bitmap,cliprect,screen->machine().gfx[0],
				state->m_vram0[offs],
				0,
				0,0,
				sx*8,sy*8,0);
	}
	return 0;
}
