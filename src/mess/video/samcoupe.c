/***************************************************************************

    SAM Coupe

    Functions to emulate the video hardware

***************************************************************************/

#include "emu.h"
#include "includes/samcoupe.h"


/***************************************************************************
    MACROS
***************************************************************************/

/* border color from border register */
#define BORDER_COLOR(x)	((x & 0x20) >> 2 | (x & 0x07))

/* foreground and background color from attribute byte in mode 1 and 2 */
#define ATTR_BG(x)		((x >> 3) & 0x07)
#define ATTR_FG(x)		(((x >> 3) & 0x08) | (x & 0x07))


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

void samcoupe_state::video_start()
{
	machine().primary_screen->register_screen_bitmap(m_bitmap);
}

UINT32 samcoupe_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

static void draw_mode4_line(running_machine &machine, int y, int hpos)
{
	samcoupe_state *state = machine.driver_data<samcoupe_state>();
	UINT8 *videoram = state->m_videoram;

	/* get start address */
	UINT8 *vram = videoram + ((y - SAM_BORDER_TOP) * 128) + ((hpos - SAM_BORDER_LEFT) / 4);

	for (int i = 0; i < (SAM_BLOCK*2)/4; i++)
	{
		/* draw 2 pixels (doublewidth) */
		state->m_bitmap.pix16(y, hpos + i * 4 + 0) = state->m_clut[(*vram >> 4) & 0x0f];
		state->m_bitmap.pix16(y, hpos + i * 4 + 1) = state->m_clut[(*vram >> 4) & 0x0f];
		state->m_bitmap.pix16(y, hpos + i * 4 + 2) = state->m_clut[(*vram >> 0) & 0x0f];
		state->m_bitmap.pix16(y, hpos + i * 4 + 3) = state->m_clut[(*vram >> 0) & 0x0f];

		/* move to next address */
		vram++;

		/* attribute register contains the third displayed byte */
		if (i == 2)
			state->m_attribute = *vram;
	}
}

static void draw_mode3_line(running_machine &machine, int y, int hpos)
{
	samcoupe_state *state = machine.driver_data<samcoupe_state>();
	UINT8 *videoram = state->m_videoram;

	/* get start address */
	UINT8 *vram = videoram + ((y - SAM_BORDER_TOP) * 128) + ((hpos - SAM_BORDER_LEFT) / 4);

	for (int i = 0; i < (SAM_BLOCK*2)/4; i++)
	{
		/* draw 4 pixels */
		state->m_bitmap.pix16(y, hpos + i * 4 + 0) = state->m_clut[(*vram >> 6) & 0x03];
		state->m_bitmap.pix16(y, hpos + i * 4 + 1) = state->m_clut[(*vram >> 4) & 0x03];
		state->m_bitmap.pix16(y, hpos + i * 4 + 2) = state->m_clut[(*vram >> 2) & 0x03];
		state->m_bitmap.pix16(y, hpos + i * 4 + 3) = state->m_clut[(*vram >> 0) & 0x03];

		/* move to next address */
		vram++;

		/* attribute register contains the third displayed byte */
		if (i == 2)
			state->m_attribute = *vram;
	}
}

static void draw_mode12_block(samcoupe_state *state, bitmap_ind16 &bitmap, int vpos, int hpos, UINT8 mask)
{
	/* extract colors from attribute */
	UINT8 ink = state->m_clut[ATTR_FG(state->m_attribute)];
	UINT8 pap = state->m_clut[ATTR_BG(state->m_attribute)];

	/* draw block of 8 pixels (doubled to 16) */
	for (int i = 0; i < SAM_BLOCK; i++)
	{
		bitmap.pix16(vpos, hpos + i*2 + 0) = BIT(mask, 7 - i) ? ink : pap;
		bitmap.pix16(vpos, hpos + i*2 + 1) = BIT(mask, 7 - i) ? ink : pap;
	}
}

static void draw_mode2_line(running_machine &machine, int y, int hpos)
{
	samcoupe_state *state = machine.driver_data<samcoupe_state>();
	UINT8 *videoram = state->m_videoram;

	int cell = (y - SAM_BORDER_TOP) * 32 + (hpos - SAM_BORDER_LEFT) / SAM_BLOCK / 2;

	UINT8 mask = videoram[cell];
	state->m_attribute = videoram[cell + 0x2000];

	draw_mode12_block(state, state->m_bitmap, y, hpos, mask);
}

static void draw_mode1_line(running_machine &machine, int y, int hpos)
{
	samcoupe_state *state = machine.driver_data<samcoupe_state>();
	UINT8 *videoram = state->m_videoram;

	UINT8 mask = videoram[((((y - SAM_BORDER_TOP) & 0xc0) << 5) | (((y - SAM_BORDER_TOP) & 0x07) << 8) | (((y - SAM_BORDER_TOP) & 0x38) << 2)) + (hpos - SAM_BORDER_LEFT) / SAM_BLOCK / 2];
	state->m_attribute = videoram[32*192 + (((y - SAM_BORDER_TOP) & 0xf8) << 2) + (hpos - SAM_BORDER_LEFT) / SAM_BLOCK / 2];

	draw_mode12_block(state, state->m_bitmap, y, hpos, mask);
}

TIMER_CALLBACK( sam_video_update_callback )
{
	samcoupe_state *state = machine.driver_data<samcoupe_state>();
	int vpos = machine.primary_screen->vpos();
	int hpos = machine.primary_screen->hpos();

	int next_vpos = vpos;
	int next_hpos = hpos + SAM_BLOCK*2;

	/* next scanline? */
	if (next_hpos >= SAM_BORDER_LEFT + SAM_SCREEN_WIDTH + SAM_BORDER_RIGHT)
	{
		next_vpos = (vpos + 1) % (SAM_BORDER_TOP + SAM_SCREEN_HEIGHT + SAM_BORDER_BOTTOM);
		next_hpos = 0;
	}

	/* display disabled? (only in mode 3 or 4) */
	if (BIT(state->m_vmpr, 6) && BIT(state->m_border, 7))
	{
		state->m_bitmap.plot_box(hpos, vpos, SAM_BLOCK*2, 1, 0);
	}
	else
	{
		/* border area? */
		if (vpos < SAM_BORDER_TOP || vpos >= SAM_BORDER_TOP + SAM_SCREEN_HEIGHT || hpos < SAM_BORDER_LEFT || hpos >= SAM_BORDER_LEFT + SAM_SCREEN_WIDTH)
		{
			state->m_attribute = 0xff;
			state->m_bitmap.plot_box(hpos, vpos, SAM_BLOCK*2, 1, state->m_clut[BORDER_COLOR(state->m_border)]);
		}
		else
		{
			/* main screen area */
			switch ((state->m_vmpr & 0x60) >> 5)
			{
			case 0:	draw_mode1_line(machine, vpos, hpos); break;
			case 1:	draw_mode2_line(machine, vpos, hpos); break;
			case 2:	draw_mode3_line(machine, vpos, hpos); break;
			case 3:	draw_mode4_line(machine, vpos, hpos); break;
			}
		}
	}

	/* do we need to trigger the scanline interrupt (interrupt happens at the start of the right border before the specified line)? */
	if (state->m_line_int < SAM_SCREEN_HEIGHT && hpos == SAM_BORDER_LEFT + SAM_SCREEN_WIDTH && vpos == (state->m_line_int + SAM_BORDER_TOP - 1))
		samcoupe_irq(machine.firstcpu, SAM_LINE_INT);

	/* schedule next update */
	state->m_video_update_timer->adjust(machine.primary_screen->time_until_pos(next_vpos, next_hpos));
}
