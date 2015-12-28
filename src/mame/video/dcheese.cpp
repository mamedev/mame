// license:BSD-3-Clause
// copyright-holders:Aaron Giles
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

#define DSTBITMAP_WIDTH     512
#define DSTBITMAP_HEIGHT    512


/*************************************
 *
 *  Palette translation
 *
 *************************************/

PALETTE_INIT_MEMBER(dcheese_state, dcheese)
{
	const UINT16 *src = (UINT16 *)memregion("user1")->base();
	int i;

	/* really 65536 colors, but they don't use the later ones so we can stay */
	/* within MAME's limits */
	for (i = 0; i < 65534; i++)
	{
		int data = *src++;
		palette.set_pen_color(i, pal6bit(data >> 0), pal5bit(data >> 6), pal5bit(data >> 11));
	}
}



/*************************************
 *
 *  Scanline interrupt
 *
 *************************************/

void dcheese_state::update_scanline_irq()
{
	/* if not in range, don't bother */
	if (m_blitter_vidparam[0x22/2] <= m_blitter_vidparam[0x1e/2])
	{
		int effscan;
		attotime time;

		/* compute the effective scanline of the interrupt */
		effscan = m_blitter_vidparam[0x22/2] - m_blitter_vidparam[0x1a/2];
		if (effscan < 0)
			effscan += m_blitter_vidparam[0x1e/2];

		/* determine the time; if it's in this scanline, bump to the next frame */
		time = m_screen->time_until_pos(effscan);
		if (time < m_screen->scan_period())
			time += m_screen->frame_period();
		m_blitter_timer->adjust(time);
	}
}


void dcheese_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_BLITTER_SCANLINE:
		dcheese_signal_irq(3);
		update_scanline_irq();
		break;
	case TIMER_SIGNAL_IRQ:
		dcheese_signal_irq(param);
		break;
	default:
		assert_always(FALSE, "Unknown id in dcheese_state::device_timer");
	}
}


/*************************************
 *
 *  Video start
 *
 *************************************/

void dcheese_state::video_start()
{
	/* the destination bitmap is not directly accessible to the CPU */
	m_dstbitmap = std::make_unique<bitmap_ind16>(DSTBITMAP_WIDTH, DSTBITMAP_HEIGHT);

	/* create a timer */
	m_blitter_timer = timer_alloc(TIMER_BLITTER_SCANLINE);

	/* register for saving */
	save_item(NAME(m_blitter_color));
	save_item(NAME(m_blitter_xparam));
	save_item(NAME(m_blitter_yparam));
	save_item(NAME(m_blitter_vidparam));
	save_item(NAME(*m_dstbitmap));
}



/*************************************
 *
 *  Video update
 *
 *************************************/

UINT32 dcheese_state::screen_update_dcheese(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	/* update the pixels */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *dest = &bitmap.pix16(y);
		UINT16 *src = &m_dstbitmap->pix16((y + m_blitter_vidparam[0x28/2]) % DSTBITMAP_HEIGHT);

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			dest[x] = src[x];
	}
	return 0;
}



/*************************************
 *
 *  Blitter implementation
 *
 *************************************/

void dcheese_state::do_clear(  )
{
	int y;

	/* clear the requested scanlines */
	for (y = m_blitter_vidparam[0x2c/2]; y < m_blitter_vidparam[0x2a/2]; y++)
		memset(&m_dstbitmap->pix16(y % DSTBITMAP_HEIGHT), 0, DSTBITMAP_WIDTH * 2);

	/* signal an IRQ when done (timing is just a guess) */
	timer_set(m_screen->scan_period(), TIMER_SIGNAL_IRQ, 1);
}


void dcheese_state::do_blit(  )
{
	INT32 srcminx = m_blitter_xparam[0] << 12;
	INT32 srcmaxx = m_blitter_xparam[1] << 12;
	INT32 srcminy = m_blitter_yparam[0] << 12;
	INT32 srcmaxy = m_blitter_yparam[1] << 12;
	INT32 srcx = ((m_blitter_xparam[2] & 0x0fff) | ((m_blitter_xparam[3] & 0x0fff) << 12)) << 7;
	INT32 srcy = ((m_blitter_yparam[2] & 0x0fff) | ((m_blitter_yparam[3] & 0x0fff) << 12)) << 7;
	INT32 dxdx = (INT32)(((m_blitter_xparam[4] & 0x0fff) | ((m_blitter_xparam[5] & 0x0fff) << 12)) << 12) >> 12;
	INT32 dxdy = (INT32)(((m_blitter_xparam[6] & 0x0fff) | ((m_blitter_xparam[7] & 0x0fff) << 12)) << 12) >> 12;
	INT32 dydx = (INT32)(((m_blitter_yparam[4] & 0x0fff) | ((m_blitter_yparam[5] & 0x0fff) << 12)) << 12) >> 12;
	INT32 dydy = (INT32)(((m_blitter_yparam[6] & 0x0fff) | ((m_blitter_yparam[7] & 0x0fff) << 12)) << 12) >> 12;
	UINT8 *src = memregion("gfx1")->base();
	UINT32 pagemask = (memregion("gfx1")->bytes() - 1) / 0x40000;
	int xstart = m_blitter_xparam[14];
	int xend = m_blitter_xparam[15] + 1;
	int ystart = m_blitter_yparam[14];
	int yend = m_blitter_yparam[15];
	int color = (m_blitter_color[0] << 8) & 0xff00;
	int mask = (m_blitter_color[0] >> 8) & 0x00ff;
	int opaque = (dxdx | dxdy | dydx | dydy) == 0;  /* bit of a hack for fredmem */
	int x, y;

	/* loop over target rows */
	for (y = ystart; y <= yend; y++)
	{
		UINT16 *dst = &m_dstbitmap->pix16(y % DSTBITMAP_HEIGHT);

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
	timer_set(m_screen->scan_period() / 2, TIMER_SIGNAL_IRQ, 2);

	/* these extra parameters are written but they are always zero, so I don't know what they do */
	if (m_blitter_xparam[8] != 0 || m_blitter_xparam[9] != 0 || m_blitter_xparam[10] != 0 || m_blitter_xparam[11] != 0 ||
		m_blitter_yparam[8] != 0 || m_blitter_yparam[9] != 0 || m_blitter_yparam[10] != 0 || m_blitter_yparam[11] != 0)
	{
		logerror("%s:blit! (%04X)\n", machine().describe_context(), m_blitter_color[0]);
		logerror("   %04X %04X %04X %04X - %04X %04X %04X %04X - %04X %04X %04X %04X - %04X %04X %04X %04X\n",
				m_blitter_xparam[0], m_blitter_xparam[1], m_blitter_xparam[2], m_blitter_xparam[3],
				m_blitter_xparam[4], m_blitter_xparam[5], m_blitter_xparam[6], m_blitter_xparam[7],
				m_blitter_xparam[8], m_blitter_xparam[9], m_blitter_xparam[10], m_blitter_xparam[11],
				m_blitter_xparam[12], m_blitter_xparam[13], m_blitter_xparam[14], m_blitter_xparam[15]);
		logerror("   %04X %04X %04X %04X - %04X %04X %04X %04X - %04X %04X %04X %04X - %04X %04X %04X %04X\n",
				m_blitter_yparam[0], m_blitter_yparam[1], m_blitter_yparam[2], m_blitter_yparam[3],
				m_blitter_yparam[4], m_blitter_yparam[5], m_blitter_yparam[6], m_blitter_yparam[7],
				m_blitter_yparam[8], m_blitter_yparam[9], m_blitter_yparam[10], m_blitter_yparam[11],
				m_blitter_yparam[12], m_blitter_yparam[13], m_blitter_yparam[14], m_blitter_yparam[15]);
	}
}



/*************************************
 *
 *  Blitter read/write
 *
 *************************************/

WRITE16_MEMBER(dcheese_state::madmax_blitter_color_w)
{
	COMBINE_DATA(&m_blitter_color[offset]);
}


WRITE16_MEMBER(dcheese_state::madmax_blitter_xparam_w)
{
	COMBINE_DATA(&m_blitter_xparam[offset]);
}


WRITE16_MEMBER(dcheese_state::madmax_blitter_yparam_w)
{
	COMBINE_DATA(&m_blitter_yparam[offset]);
}


WRITE16_MEMBER(dcheese_state::madmax_blitter_vidparam_w)
{
	COMBINE_DATA(&m_blitter_vidparam[offset]);

	switch (offset)
	{
		case 0x10/2:        /* horiz front porch */
		case 0x12/2:        /* horiz display start */
		case 0x14/2:        /* horiz display end */
		case 0x16/2:        /* horiz back porch */

		case 0x18/2:        /* vert front porch */
		case 0x1a/2:        /* vert display start */
		case 0x1c/2:        /* vert display end */
		case 0x1e/2:        /* vert back porch */
			break;

		case 0x22/2:        /* scanline interrupt */
			update_scanline_irq();
			break;

		case 0x24/2:        /* writes here after writing to 0x28 */
			break;

		case 0x28/2:        /* display starting y */
		case 0x2a/2:        /* clear end y */
		case 0x2c/2:        /* clear start y */
			break;

		case 0x38/2:        /* blit */
			do_blit();
			break;

		case 0x3e/2:        /* clear */
			do_clear();
			break;

		default:
			logerror("%06X:write to %06X = %04X & %04x\n", space.device().safe_pc(), 0x2a0000 + 2 * offset, data, mem_mask);
			break;
	}
}


WRITE16_MEMBER(dcheese_state::madmax_blitter_unknown_w)
{
	/* written to just before the blitter command register is written */
	logerror("%06X:write to %06X = %04X & %04X\n", space.device().safe_pc(), 0x300000 + 2 * offset, data, mem_mask);
}


READ16_MEMBER(dcheese_state::madmax_blitter_vidparam_r)
{
	/* analog inputs seem to be hooked up here -- might not actually map to blitter */
	if (offset == 0x02/2)
		return ioport("2a0002")->read();
	if (offset == 0x0e/2)
		return ioport("2a000e")->read();

	/* early code polls on this bit, wants it to be 0 */
	if (offset == 0x36/2)
		return 0xffff ^ (1 << 5);

	/* log everything else */
	logerror("%06X:read from %06X\n", space.device().safe_pc(), 0x2a0000 + 2 * offset);
	return 0xffff;
}
