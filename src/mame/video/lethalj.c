/***************************************************************************

    The Game Room Lethal Justice hardware

***************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "includes/lethalj.h"


#define BLITTER_SOURCE_WIDTH		1024
#define BLITTER_DEST_WIDTH			512
#define BLITTER_DEST_HEIGHT			512


/*************************************
 *
 *  Compute X/Y coordinates
 *
 *************************************/

INLINE void get_crosshair_xy(running_machine &machine, int player, int *x, int *y)
{
	static const char *const gunnames[] = { "LIGHT0_X", "LIGHT0_Y", "LIGHT1_X", "LIGHT1_Y" };
	const rectangle &visarea = machine.primary_screen->visible_area();
	int width = visarea.width();
	int height = visarea.height();

	*x = ((input_port_read_safe(machine, gunnames[player * 2], 0x00) & 0xff) * width) / 255;
	*y = ((input_port_read_safe(machine, gunnames[1 + player * 2], 0x00) & 0xff) * height) / 255;
}



/*************************************
 *
 *  Gun input handling
 *
 *************************************/

READ16_MEMBER(lethalj_state::lethalj_gun_r)
{
	UINT16 result = 0;
	int beamx, beamy;

	switch (offset)
	{
		case 4:
		case 5:
			/* latch the crosshair position */
			get_crosshair_xy(machine(), offset - 4, &beamx, &beamy);
			m_gunx = beamx;
			m_guny = beamy;
			m_blank_palette = 1;
			break;

		case 6:
			result = m_gunx / 2;
			break;

		case 7:
			result = m_guny + 4;
			break;
	}
/*  logerror("%08X:lethalj_gun_r(%d) = %04X\n", cpu_get_pc(&space.device()), offset, result); */
	return result;
}



/*************************************
 *
 *  video startup
 *
 *************************************/

VIDEO_START( lethalj )
{
	lethalj_state *state = machine.driver_data<lethalj_state>();
	/* allocate video RAM for screen */
	state->m_screenram = auto_alloc_array(machine, UINT16, BLITTER_DEST_WIDTH * BLITTER_DEST_HEIGHT);

	/* predetermine blitter info */
	state->m_blitter_base = (UINT16 *)machine.region("gfx1")->base();
	state->m_blitter_rows = machine.region("gfx1")->bytes() / (2*BLITTER_SOURCE_WIDTH);
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static TIMER_CALLBACK( gen_ext1_int )
{
	cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);
}



void lethalj_state::do_blit()
{
	int dsty = (INT16)m_blitter_data[1];
	int srcx = (UINT16)m_blitter_data[2];
	int srcy = (UINT16)(m_blitter_data[3] + 1);
	int width = (UINT16)m_blitter_data[5];
	int dstx = (INT16)m_blitter_data[6];
	int height = (UINT16)m_blitter_data[7];
	int y;
/*
    logerror("blitter data = %04X %04X %04X %04X %04X %04X %04X %04X\n",
            m_blitter_data[0], m_blitter_data[1], m_blitter_data[2], m_blitter_data[3],
            m_blitter_data[4], m_blitter_data[5], m_blitter_data[6], m_blitter_data[7]);
*/
	/* loop over Y coordinates */
	for (y = 0; y <= height; y++, srcy++, dsty++)
	{
		/* clip in Y */
		if (dsty >= 0 && dsty < BLITTER_DEST_HEIGHT/2)
		{
			UINT16 *source = m_blitter_base + (srcy % m_blitter_rows) * BLITTER_SOURCE_WIDTH;
			UINT16 *dest = m_screenram + (dsty + (m_vispage ^ 1) * 256) * BLITTER_DEST_WIDTH;
			int sx = srcx;
			int dx = dstx;
			int x;

			/* loop over X coordinates */
			for (x = 0; x <= width; x++, sx++, dx++)
				if (dx >= 0 && dx < BLITTER_DEST_WIDTH)
				{
					int pix = source[sx % BLITTER_SOURCE_WIDTH];
					if (pix)
						dest[dx] = pix;
				}
		}
	}
}


WRITE16_MEMBER(lethalj_state::lethalj_blitter_w)
{
	/* combine the data */
	COMBINE_DATA(&m_blitter_data[offset]);

	/* blit on a write to offset 7, and signal an IRQ */
	if (offset == 7)
	{
		if (m_blitter_data[6] == 2 && m_blitter_data[7] == 2)
			m_vispage ^= 1;
		else
			do_blit();

		machine().scheduler().timer_set(attotime::from_hz(XTAL_32MHz) * ((m_blitter_data[5] + 1) * (m_blitter_data[7] + 1)), FUNC(gen_ext1_int));
	}

	/* clear the IRQ on offset 0 */
	else if (offset == 0)
		cputag_set_input_line(machine(), "maincpu", 0, CLEAR_LINE);
}



/*************************************
 *
 *  video update
 *
 *************************************/

void lethalj_scanline_update(screen_device &screen, bitmap_ind16 &bitmap, int scanline, const tms34010_display_params *params)
{
	lethalj_state *state = screen.machine().driver_data<lethalj_state>();
	UINT16 *src = &state->m_screenram[(state->m_vispage << 17) | ((params->rowaddr << 9) & 0x3fe00)];
	UINT16 *dest = &bitmap.pix16(scanline);
	int coladdr = params->coladdr << 1;
	int x;

	/* blank palette: fill with white */
	if (state->m_blank_palette)
	{
		for (x = params->heblnk; x < params->hsblnk; x++)
			dest[x] = 0x7fff;
		if (scanline == screen.visible_area().max_y)
			state->m_blank_palette = 0;
		return;
	}

	/* copy the non-blanked portions of this scanline */
	for (x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = src[coladdr++ & 0x1ff] & 0x7fff;
}
