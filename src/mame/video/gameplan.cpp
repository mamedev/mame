// license:BSD-3-Clause
// copyright-holders:Chris Moore
/***************************************************************************

GAME PLAN driver

driver by Chris Moore

****************************************************************************/

#include "emu.h"
#include "includes/gameplan.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define HTOTAL              (0x160)
#define HBEND               (0x000)
#define HBSTART             (0x100)
#define VTOTAL              (0x118)
#define VBEND               (0x000)
#define VBSTART             (0x100)

#define GAMEPLAN_NUM_PENS   (0x08)
#define LEPRECHN_NUM_PENS   (0x10)



/*************************************
 *
 *  Palette handling
 *
 *************************************/

void gameplan_state::gameplan_get_pens( pen_t *pens )
{
	offs_t i;

	for (i = 0; i < GAMEPLAN_NUM_PENS; i++)
		pens[i] = rgb_t(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
}


/* RGBI palette. Is it correct, or does it use the standard RGB? */
void gameplan_state::leprechn_get_pens( pen_t *pens )
{
	offs_t i;

	for (i = 0; i < LEPRECHN_NUM_PENS; i++)
	{
		uint8_t bk = (i & 8) ? 0x40 : 0x00;
		uint8_t r = (i & 1) ? 0xff : bk;
		uint8_t g = (i & 2) ? 0xff : bk;
		uint8_t b = (i & 4) ? 0xff : bk;

		pens[i] = rgb_t(r, g, b);
	}
}



/*************************************
 *
 *  Update
 *
 *************************************/

uint32_t gameplan_state::screen_update_gameplan(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[GAMEPLAN_NUM_PENS];

	gameplan_get_pens(pens);

	for (offs_t offs = 0; offs < m_videoram_size; offs++)
	{
		uint8_t y = offs >> 8;
		uint8_t x = offs & 0xff;

		bitmap.pix(y, x) = pens[m_videoram[offs] & 0x07];
	}

	return 0;
}


uint32_t gameplan_state::screen_update_leprechn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[LEPRECHN_NUM_PENS];

	leprechn_get_pens(pens);

	for (offs_t offs = 0; offs < m_videoram_size; offs++)
	{
		uint8_t y = offs >> 8;
		uint8_t x = offs & 0xff;

		bitmap.pix(y, x) = pens[m_videoram[offs] & (LEPRECHN_NUM_PENS-1)];
	}

	return 0;
}



/*************************************
 *
 *  VIA
 *
 *************************************/

void gameplan_state::video_data_w(uint8_t data)
{
	m_video_data = data;
}


void gameplan_state::gameplan_video_command_w(uint8_t data)
{
	m_video_command = data & 0x07;
}


void gameplan_state::leprechn_video_command_w(uint8_t data)
{
	m_video_command = (data >> 3) & 0x07;
}


uint8_t gameplan_state::leprechn_videoram_r()
{
	return m_video_previous;
}


TIMER_CALLBACK_MEMBER(gameplan_state::clear_screen_done_callback)
{
	/* indicate that the we are done clearing the screen */
	m_via_0->write_ca1(0);
}


WRITE_LINE_MEMBER(gameplan_state::video_command_trigger_w)
{
	if (state == 0)
	{
		switch (m_video_command)
		{
		/* draw pixel */
		case 0:
			/* auto-adjust X? */
			if (m_video_data & 0x10)
			{
				if (m_video_data & 0x40)
					m_video_x = m_video_x - 1;
				else
					m_video_x = m_video_x + 1;
			}

			/* auto-adjust Y? */
			if (m_video_data & 0x20)
			{
				if (m_video_data & 0x80)
					m_video_y = m_video_y - 1;
				else
					m_video_y = m_video_y + 1;
			}

			m_video_previous = m_videoram[m_video_y * (HBSTART - HBEND) + m_video_x];
			m_videoram[m_video_y * (HBSTART - HBEND) + m_video_x] = m_video_data & 0x0f;

			break;

		/* load X register */
		case 1:
			m_video_x = m_video_data;
			break;

		/* load Y register */
		case 2:
			m_video_y = m_video_data;
			break;

		/* clear screen */
		case 3:
			/* indicate that the we are busy */
			{
				m_via_0->write_ca1(1);
			}

			memset(m_videoram.get(), m_video_data & 0x0f, m_videoram_size);

			/* set a timer for an arbitrarily short period.
			   The real time it takes to clear to screen is not
			   important to the software */
			m_clear_done_timer->adjust(attotime::zero);

			break;
		}
	}
}


/*************************************
 *
 *  Start
 *
 *************************************/

void gameplan_state::video_start()
{
	m_videoram_size = (HBSTART - HBEND) * (VBSTART - VBEND);
	m_videoram = std::make_unique<uint8_t[]>(m_videoram_size);

	m_clear_done_timer = timer_alloc(FUNC(gameplan_state::clear_screen_done_callback), this);

	/* register for save states */
	save_pointer(NAME(m_videoram), m_videoram_size);
}



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void gameplan_state::gameplan_video(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(GAMEPLAN_PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(gameplan_state::screen_update_gameplan));
	m_screen->screen_vblank().set(m_via_0, FUNC(via6522_device::write_ca1)).invert(); // !VBLANK is connected to CA1
}

void gameplan_state::leprechn_video(machine_config &config)
{
	m_screen->set_screen_update(FUNC(gameplan_state::screen_update_leprechn));
}

void trvquest_state::trvquest_video(machine_config &config)
{
	gameplan_video(config);
	m_screen->set_screen_update(FUNC(trvquest_state::screen_update_gameplan));
	m_screen->screen_vblank().set(m_via_2, FUNC(via6522_device::write_ca1));
}
