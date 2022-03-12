// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    HAR MadMax hardware

**************************************************************************/

#include "emu.h"
#include "includes/dcheese.h"

#include <algorithm>


/*************************************
 *
 *  Constants
 *
 *************************************/

static constexpr u32 DSTBITMAP_WIDTH  = 512;
static constexpr u32 DSTBITMAP_HEIGHT = 512;


/*************************************
 *
 *  Palette translation
 *
 *************************************/

void dcheese_state::dcheese_palette(palette_device &palette) const
{
	for (int i = 0; i < 65536; i++)
	{
		u16 const data = m_palrom[i];
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
		/* compute the effective scanline of the interrupt */
		int effscan = m_blitter_vidparam[0x22/2] - m_blitter_vidparam[0x1a/2];
		if (effscan < 0)
			effscan += m_blitter_vidparam[0x1e/2];

		/* determine the time; if it's in this scanline, bump to the next frame */
		attotime time = m_screen->time_until_pos(effscan);
		if (time < m_screen->scan_period())
			time += m_screen->frame_period();
		m_blitter_timer->adjust(time);
	}
}


void dcheese_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_BLITTER_SCANLINE:
		signal_irq(3);
		update_scanline_irq();
		break;
	case TIMER_SIGNAL_IRQ:
		signal_irq(param);
		break;
	default:
		throw emu_fatalerror("Unknown id in dcheese_state::device_timer");
	}
}


/*************************************
 *
 *  Video start
 *
 *************************************/

void dcheese_state::video_start()
{
	// assumes it can make an address mask from m_gfxrom.length() - 1
	assert(!(m_gfxrom.length() & (m_gfxrom.length() - 1)));

	// the destination bitmap is not directly accessible to the CPU
	m_dstbitmap = std::make_unique<bitmap_ind16>(DSTBITMAP_WIDTH, DSTBITMAP_HEIGHT);

	// create timers
	m_blitter_timer = timer_alloc(TIMER_BLITTER_SCANLINE);
	m_signal_irq_timer = timer_alloc(TIMER_SIGNAL_IRQ);

	// register for saving
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

u32 dcheese_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* update the pixels */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 *const dest = &bitmap.pix(y);
		u16 const *const src = &m_dstbitmap->pix((y + m_blitter_vidparam[0x28/2]) & 0x1ff);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			dest[x] = src[x];
	}
	return 0;
}



/*************************************
 *
 *  Blitter implementation
 *
 *************************************/

void dcheese_state::do_clear()
{
	/* clear the requested scanlines */
	for (int y = m_blitter_vidparam[0x2c/2]; y < m_blitter_vidparam[0x2a/2]; y++)
		std::fill_n(&m_dstbitmap->pix(y & 0x1ff), DSTBITMAP_WIDTH, 0);

	/* signal an IRQ when done (timing is just a guess) */
	m_signal_irq_timer->adjust(m_screen->scan_period(), 1);
}


void dcheese_state::do_blit()
{
	s32 const srcminx = m_blitter_xparam[0] << 12;
	s32 const srcmaxx = m_blitter_xparam[1] << 12;
	s32 const srcminy = m_blitter_yparam[0] << 12;
	s32 const srcmaxy = m_blitter_yparam[1] << 12;
	s32 const srcx = ((m_blitter_xparam[2] & 0x0fff) | ((m_blitter_xparam[3] & 0x0fff) << 12)) << 7;
	s32 const srcy = ((m_blitter_yparam[2] & 0x0fff) | ((m_blitter_yparam[3] & 0x0fff) << 12)) << 7;
	s32 const dxdx = s32(((m_blitter_xparam[4] & 0x0fff) | ((m_blitter_xparam[5] & 0x0fff) << 12)) << 12) >> 12;
	s32 const dxdy = s32(((m_blitter_xparam[6] & 0x0fff) | ((m_blitter_xparam[7] & 0x0fff) << 12)) << 12) >> 12;
	s32 const dydx = s32(((m_blitter_yparam[4] & 0x0fff) | ((m_blitter_yparam[5] & 0x0fff) << 12)) << 12) >> 12;
	s32 const dydy = s32(((m_blitter_yparam[6] & 0x0fff) | ((m_blitter_yparam[7] & 0x0fff) << 12)) << 12) >> 12;
	u32 const pagemask = (m_gfxrom.length() - 1) >> 18;
	int const xstart = m_blitter_xparam[14];
	int const xend = m_blitter_xparam[15] + 1;
	int const ystart = m_blitter_yparam[14];
	int const yend = m_blitter_yparam[15];
	u32 const color = (m_blitter_color[0] << 8) & 0xff00;
	u8 const mask = (m_blitter_color[0] >> 8) & 0x00ff;
	bool const opaque = (dxdx | dxdy | dydx | dydy) == 0;  /* bit of a hack for fredmem */

	/* loop over target rows */
	for (int y = ystart; y <= yend; y++)
	{
		u16 *const dst = &m_dstbitmap->pix(y & 0x1ff);

		/* loop over target columns */
		for (int x = xstart; x <= xend; x++)
		{
			/* compute current X/Y positions */
			int const sx = (srcx + dxdx * (x - xstart) + dxdy * (y - ystart)) & 0xffffff;
			int const sy = (srcy + dydx * (x - xstart) + dydy * (y - ystart)) & 0xffffff;

			/* clip to source cliprect */
			if (sx >= srcminx && sx <= srcmaxx && sy >= srcminy && sy <= srcmaxy)
			{
				/* page comes from bit 22 of Y and bit 21 of X */
				u32 const page = (((sy >> 21) & 2) | ((sx >> 21) & 1) | ((sx >> 20) & 4)) & pagemask;
				u8 const pix = m_gfxrom[(page << 18) | (((sy >> 12) & 0x1ff) << 9) | ((sx >> 12) & 0x1ff)];

				/* only non-zero pixels get written */
				if (pix | opaque)
					dst[x & 0x1ff] = (pix & mask) | color;
			}
		}
	}

	/* signal an IRQ when done (timing is just a guess) */
	m_signal_irq_timer->adjust(m_screen->scan_period() / 2, 2);

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

void dcheese_state::blitter_color_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter_color[offset]);
}


void dcheese_state::blitter_xparam_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter_xparam[offset]);
}


void dcheese_state::blitter_yparam_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter_yparam[offset]);
}


void dcheese_state::blitter_vidparam_w(offs_t offset, u16 data, u16 mem_mask)
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
			logerror("%06X:write to %06X = %04X & %04x\n", m_maincpu->pc(), 0x2a0000 + 2 * offset, data, mem_mask);
			break;
	}
}


void dcheese_state::blitter_unknown_w(offs_t offset, u16 data, u16 mem_mask)
{
	/* written to just before the blitter command register is written */
	logerror("%06X:write to %06X = %04X & %04X\n", m_maincpu->pc(), 0x300000 + 2 * offset, data, mem_mask);
}


u16 dcheese_state::blitter_vidparam_r(offs_t offset)
{
	/* analog inputs seem to be hooked up here -- might not actually map to blitter */
	if (offset == 0x02/2)
		return m_2a0002_io->read();
	if (offset == 0x0e/2)
		return m_2a000e_io->read();

	/* early code polls on this bit, wants it to be 0 */
	if (offset == 0x36/2)
		return 0xffff ^ (1 << 5);

	/* log everything else */
	logerror("%06X:read from %06X\n", m_maincpu->pc(), 0x2a0000 + 2 * offset);
	return 0xffff;
}
