#include "emu.h"
#include "includes/lsasquad.h"

static void draw_layer( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *scrollram )
{
	lsasquad_state *state = machine.driver_data<lsasquad_state>();
	int offs, scrollx, scrolly;

	scrollx = scrollram[3];
	scrolly = -scrollram[0];

	for (offs = 0; offs < 0x080; offs += 4)
	{
		int base, y, sx, sy, code, color;

		base = 64 * scrollram[offs + 1];
		sx = 8 * (offs / 4) + scrollx;
		if (state->flip_screen())
			sx = 248 - sx;

		sx &= 0xff;

		for (y = 0; y < 32; y++)
		{
			int attr;

			sy = 8 * y + scrolly;
			if (state->flip_screen())
				sy = 248 - sy;
			sy &= 0xff;

			attr = state->m_videoram[base + 2 * y + 1];
			code = state->m_videoram[base + 2 * y] + ((attr & 0x0f) << 8);
			color = attr >> 4;

			drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
					code,
					color,
					state->flip_screen(),state->flip_screen(),
					sx,sy,15);
			if (sx > 248)	/* wraparound */
				drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
						code,
						color,
						state->flip_screen(),state->flip_screen(),
						sx-256,sy,15);
		}
	}
}

static int draw_layer_daikaiju( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int  * previd, int type )
{
	lsasquad_state *state = machine.driver_data<lsasquad_state>();
	int id, scrollx, scrolly, initoffs, globalscrollx;
	int stepx = 0;

	initoffs = offs;
	globalscrollx = 0;

	id = state->m_scrollram[offs + 2];

	for( ; offs < 0x400; offs += 4)
	{
		int base, y, sx, sy, code, color;

		 //id change
		if (id != state->m_scrollram[offs + 2])
		{
			*previd = id;
			return offs;
		}
		else
		{
			id = state->m_scrollram[offs + 2];
		}

		//skip empty (??) column, potential probs with 1st column in scrollram (scroll 0, tile 0, id 0)
		if ((state->m_scrollram[offs + 0] | state->m_scrollram[offs + 1] | state->m_scrollram[offs + 2] | state->m_scrollram[offs + 3]) == 0)
			continue;

		//local scroll x/y
		scrolly = -state->m_scrollram[offs + 0];
		scrollx =  state->m_scrollram[offs + 3];

		//check for global x scroll used in bg layer in game (starts at offset 0 in scrollram
		// and game name/logo on title screen (starts in the middle of scrollram, but with different
		// (NOT unique )id than prev coulmn(s)

		if (*previd != 1)
		{
			if (offs != initoffs)
			{
				scrollx += globalscrollx;
			}
			else
			{
				//global scroll init
				globalscrollx = scrollx;
			}
		}

		base = 64 * state->m_scrollram[offs + 1];
		sx = scrollx + stepx;

		if (state->flip_screen())
			sx = 248 - sx;
		sx &= 0xff;

		for (y = 0; y < 32; y++)
		{
			int attr;

			sy = 8 * y + scrolly;
			if (state->flip_screen())
				sy = 248 - sy;
			sy &= 0xff;

			attr = state->m_videoram[base + 2 * y + 1];
			code = state->m_videoram[base + 2 * y] + ((attr & 0x0f) << 8);
			color = attr >> 4;

			if ((type == 0 && color != 0x0d) || (type != 0 && color == 0x0d))
			{
				drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
					code,
					color,
					state->flip_screen(),state->flip_screen(),
					sx,sy,15);
				if (sx > 248)	/* wraparound */
					drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
						code,
						color,
						state->flip_screen(),state->flip_screen(),
						sx-256,sy,15);
			}
		}
	}
	return offs;
}

static void drawbg( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int type )
{
	lsasquad_state *state = machine.driver_data<lsasquad_state>();
	int i = 0;
	int id = -1;

	while (i < 0x400)
	{
		if (!(state->m_scrollram[i + 2] & 1))
		{
			i = draw_layer_daikaiju(machine, bitmap, cliprect, i, &id, type);
		}
		else
		{
			id = state->m_scrollram[i + 2];
			i += 4;
		}
	}
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	lsasquad_state *state = machine.driver_data<lsasquad_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = state->m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int sx, sy, attr, code, color, flipx, flipy;

		sx = spriteram[offs + 3];
		sy = 240 - spriteram[offs];
		attr = spriteram[offs + 1];
		code = spriteram[offs + 2] + ((attr & 0x30) << 4);
		color = attr & 0x0f;
		flipx = attr & 0x40;
		flipy = attr & 0x80;

		if (state->flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				color,
				flipx,flipy,
				sx,sy,15);
		/* wraparound */
		drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
				code,
				color,
				flipx,flipy,
				sx-256,sy,15);
	}
}

SCREEN_UPDATE_IND16( lsasquad )
{
	lsasquad_state *state = screen.machine().driver_data<lsasquad_state>();
	bitmap.fill(511, cliprect);

	draw_layer(screen.machine(), bitmap, cliprect, state->m_scrollram + 0x000);
	draw_layer(screen.machine(), bitmap, cliprect, state->m_scrollram + 0x080);
	draw_sprites(screen.machine(), bitmap, cliprect);
	draw_layer(screen.machine(), bitmap, cliprect, state->m_scrollram + 0x100);
	return 0;
}


SCREEN_UPDATE_IND16( daikaiju )
{
	bitmap.fill(511, cliprect);
	drawbg(screen.machine(), bitmap, cliprect, 0); // bottom
	draw_sprites(screen.machine(), bitmap, cliprect);
	drawbg(screen.machine(), bitmap, cliprect, 1);	// top = palette $d ?
	return 0;
}
