/***************************************************************************

GAME PLAN driver

driver by Chris Moore

****************************************************************************/

#include "emu.h"
#include "machine/6522via.h"
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

static void gameplan_get_pens( pen_t *pens )
{
	offs_t i;

	for (i = 0; i < GAMEPLAN_NUM_PENS; i++)
		pens[i] = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
}


/* RGBI palette. Is it correct, or does it use the standard RGB? */
static void leprechn_get_pens( pen_t *pens )
{
	offs_t i;

	for (i = 0; i < LEPRECHN_NUM_PENS; i++)
	{
		UINT8 bk = (i & 8) ? 0x40 : 0x00;
		UINT8 r = (i & 1) ? 0xff : bk;
		UINT8 g = (i & 2) ? 0xff : bk;
		UINT8 b = (i & 4) ? 0xff : bk;

		pens[i] = MAKE_RGB(r, g, b);
	}
}



/*************************************
 *
 *  Update
 *
 *************************************/

UINT32 gameplan_state::screen_update_gameplan(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[GAMEPLAN_NUM_PENS];
	offs_t offs;

	gameplan_get_pens(pens);

	for (offs = 0; offs < m_videoram_size; offs++)
	{
		UINT8 y = offs >> 8;
		UINT8 x = offs & 0xff;

		bitmap.pix32(y, x) = pens[m_videoram[offs] & 0x07];
	}

	return 0;
}


UINT32 gameplan_state::screen_update_leprechn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[LEPRECHN_NUM_PENS];
	offs_t offs;

	leprechn_get_pens(pens);

	for (offs = 0; offs < m_videoram_size; offs++)
	{
		UINT8 y = offs >> 8;
		UINT8 x = offs & 0xff;

		bitmap.pix32(y, x) = pens[m_videoram[offs]];
	}

	return 0;
}



/*************************************
 *
 *  VIA
 *
 *************************************/

WRITE8_MEMBER(gameplan_state::video_data_w)
{

	m_video_data = data;
}


WRITE8_MEMBER(gameplan_state::gameplan_video_command_w)
{

	m_video_command = data & 0x07;
}


WRITE8_MEMBER(gameplan_state::leprechn_video_command_w)
{

	m_video_command = (data >> 3) & 0x07;
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

			memset(m_videoram, m_video_data & 0x0f, m_videoram_size);

			/* set a timer for an arbitrarily short period.
               The real time it takes to clear to screen is not
               important to the software */
			machine().scheduler().synchronize(timer_expired_delegate(FUNC(gameplan_state::clear_screen_done_callback),this));

			break;
		}
	}
}


TIMER_CALLBACK_MEMBER(gameplan_state::via_irq_delayed)
{
	m_maincpu->set_input_line(0, param);
}


static void via_irq(device_t *device, int state)
{
	gameplan_state *driver_state = device->machine().driver_data<gameplan_state>();
	/* Kaos sits in a tight loop polling the VIA irq flags register, but that register is
       cleared by the irq handler. Therefore, I wait a bit before triggering the irq to
       leave time for the program to see the flag change. */
	device->machine().scheduler().timer_set(attotime::from_usec(50), timer_expired_delegate(FUNC(gameplan_state::via_irq_delayed),driver_state), state);
}


READ8_MEMBER(gameplan_state::vblank_r)
{
	/* this is needed for trivia quest */
	return 0x20;
}


const via6522_interface gameplan_via_0_interface =
{
	DEVCB_NULL, DEVCB_DRIVER_MEMBER(gameplan_state,vblank_r),										/*inputs : A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,								/*inputs : CA/B1,CA/B2 */
	DEVCB_DRIVER_MEMBER(gameplan_state,video_data_w), DEVCB_DRIVER_MEMBER(gameplan_state,gameplan_video_command_w),		/*outputs: A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_DRIVER_LINE_MEMBER(gameplan_state,video_command_trigger_w), DEVCB_NULL,	/*outputs: CA/B1,CA/B2 */
	DEVCB_LINE(via_irq)															/*irq                  */
};


const via6522_interface leprechn_via_0_interface =
{
	DEVCB_NULL, DEVCB_DRIVER_MEMBER(gameplan_state,vblank_r),										/*inputs : A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,								/*inputs : CA/B1,CA/B2 */
	DEVCB_DRIVER_MEMBER(gameplan_state,video_data_w), DEVCB_DRIVER_MEMBER(gameplan_state,leprechn_video_command_w),		/*outputs: A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_DRIVER_LINE_MEMBER(gameplan_state,video_command_trigger_w), DEVCB_NULL,	/*outputs: CA/B1,CA/B2 */
	DEVCB_LINE(via_irq)															/*irq                  */
};


const via6522_interface trvquest_via_0_interface =
{
	DEVCB_NULL, DEVCB_DRIVER_MEMBER(gameplan_state,vblank_r),										/*inputs : A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,								/*inputs : CA/B1,CA/B2 */
	DEVCB_DRIVER_MEMBER(gameplan_state,video_data_w), DEVCB_DRIVER_MEMBER(gameplan_state,gameplan_video_command_w),		/*outputs: A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_DRIVER_LINE_MEMBER(gameplan_state,video_command_trigger_w), DEVCB_NULL,	/*outputs: CA/B1,CA/B2 */
	DEVCB_NULL																	/*irq                  */
};


TIMER_CALLBACK_MEMBER(gameplan_state::via_0_ca1_timer_callback)
{

	/* !VBLANK is connected to CA1 */
	m_via_0->write_ca1(param);

	if (param)
		m_via_0_ca1_timer->adjust(machine().primary_screen->time_until_pos(VBSTART));
	else
		m_via_0_ca1_timer->adjust(machine().primary_screen->time_until_pos(VBEND), 1);
}


/*************************************
 *
 *  Start
 *
 *************************************/

VIDEO_START_MEMBER(gameplan_state,common)
{

	m_videoram_size = (HBSTART - HBEND) * (VBSTART - VBEND);
	m_videoram = auto_alloc_array(machine(), UINT8, m_videoram_size);

	m_via_0_ca1_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gameplan_state::via_0_ca1_timer_callback),this));

	/* register for save states */
	save_pointer(NAME(m_videoram), m_videoram_size);
}


VIDEO_START_MEMBER(gameplan_state,gameplan)
{
	VIDEO_START_CALL_MEMBER(common);
}


VIDEO_START_MEMBER(gameplan_state,leprechn)
{
	VIDEO_START_CALL_MEMBER(common);
}


VIDEO_START_MEMBER(gameplan_state,trvquest)
{
	VIDEO_START_CALL_MEMBER(common);
}



/*************************************
 *
 *  Reset
 *
 *************************************/

VIDEO_RESET_MEMBER(gameplan_state,gameplan)
{
	m_via_0_ca1_timer->adjust(machine().primary_screen->time_until_pos(VBSTART));
}



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( gameplan_video )
	MCFG_VIDEO_START_OVERRIDE(gameplan_state,gameplan)
	MCFG_VIDEO_RESET_OVERRIDE(gameplan_state,gameplan)

	MCFG_VIDEO_START_OVERRIDE(gameplan_state,gameplan)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(GAMEPLAN_PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(gameplan_state, screen_update_gameplan)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( leprechn_video )
	MCFG_VIDEO_START_OVERRIDE(gameplan_state,leprechn)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(gameplan_state, screen_update_leprechn)
MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( trvquest_video, gameplan_video )
	MCFG_VIDEO_START_OVERRIDE(gameplan_state,trvquest)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(gameplan_state, screen_update_gameplan)
MACHINE_CONFIG_END
