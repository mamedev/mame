/***************************************************************************

GAME PLAN driver

driver by Chris Moore

****************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "machine/6522via.h"
#include "gameplan.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define HTOTAL				(0x160)
#define HBEND				(0x000)
#define HBSTART				(0x100)
#define VTOTAL				(0x118)
#define VBEND				(0x000)
#define VBSTART				(0x100)

#define GAMEPLAN_NUM_PENS	(0x08)
#define LEPRECHN_NUM_PENS	(0x10)



/*************************************
 *
 *  Palette handling
 *
 *************************************/

static void gameplan_get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i < GAMEPLAN_NUM_PENS; i++)
		pens[i] = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
}


/* RGBI palette. Is it correct, or does it use the standard RGB? */
static void leprechn_get_pens(pen_t *pens)
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

static VIDEO_UPDATE( gameplan )
{
	gameplan_state *state = machine->driver_data;
	pen_t pens[GAMEPLAN_NUM_PENS];
	offs_t offs;

	gameplan_get_pens(pens);

	for (offs = 0; offs < state->videoram_size; offs++)
	{
		UINT8 y = offs >> 8;
		UINT8 x = offs & 0xff;

		*BITMAP_ADDR32(bitmap, y, x) = pens[state->videoram[offs] & 0x07];
	}

	return 0;
}


static VIDEO_UPDATE( leprechn )
{
	gameplan_state *state = machine->driver_data;
	pen_t pens[LEPRECHN_NUM_PENS];
	offs_t offs;

	leprechn_get_pens(pens);

	for (offs = 0; offs < state->videoram_size; offs++)
	{
		UINT8 y = offs >> 8;
		UINT8 x = offs & 0xff;

		*BITMAP_ADDR32(bitmap, y, x) = pens[state->videoram[offs]];
	}

	return 0;
}



/*************************************
 *
 *  VIA
 *
 *************************************/

static WRITE8_HANDLER( video_data_w )
{
	gameplan_state *state = Machine->driver_data;

	state->video_data = data;
}


static WRITE8_HANDLER( gameplan_video_command_w )
{
	gameplan_state *state = Machine->driver_data;

	state->video_command = data & 0x07;
}


static WRITE8_HANDLER( leprechn_video_command_w )
{
	gameplan_state *state = Machine->driver_data;

	state->video_command = (data >> 3) & 0x07;
}


static TIMER_CALLBACK( clear_screen_done_callback )
{
	/* indicate that the we are done clearing the screen */
	via_0_ca1_w(machine, 0, 0);
}


static WRITE8_HANDLER( video_command_trigger_w )
{
	gameplan_state *state = machine->driver_data;

	if (data == 0)
	{
		switch (state->video_command)
		{
		/* draw pixel */
		case 0:
			/* auto-adjust X? */
			if (state->video_data & 0x10)
			{
				if (state->video_data & 0x40)
					state->video_x = state->video_x - 1;
				else
					state->video_x = state->video_x + 1;
			}

			/* auto-adjust Y? */
			if (state->video_data & 0x20)
			{
				if (state->video_data & 0x80)
					state->video_y = state->video_y - 1;
				else
					state->video_y = state->video_y + 1;
			}

			state->videoram[state->video_y * (HBSTART - HBEND) + state->video_x] = state->video_data & 0x0f;

			break;

		/* load X register */
		case 1:
			state->video_x = state->video_data;
			break;

		/* load Y register */
		case 2:
			state->video_y = state->video_data;
			break;

		/* clear screen */
		case 3:
			/* indicate that the we are busy */
			via_0_ca1_w(machine, 0, 1);

			memset(state->videoram, state->video_data & 0x0f, state->videoram_size);

			/* set a timer for an arbitrarily short period.
               The real time it takes to clear to screen is not
               important to the software */
			timer_call_after_resynch(NULL, 0, clear_screen_done_callback);

			break;
		}
	}
}


static TIMER_CALLBACK( via_irq_delayed )
{
	cpunum_set_input_line(machine, 0, 0, param);
}


static void via_irq(int state)
{
	/* Kaos sits in a tight loop polling the VIA irq flags register, but that register is
       cleared by the irq handler. Therefore, I wait a bit before triggering the irq to
       leave time for the program to see the flag change. */
	timer_set(ATTOTIME_IN_USEC(50), NULL, state, via_irq_delayed);
}


static READ8_HANDLER( vblank_r )
{
	/* this is needed for trivia quest */
	return 0x20;
}


static const struct via6522_interface gameplan_via_0_interface =
{
	0, vblank_r,							/*inputs : A/B         */
	0, 0, 0, 0,								/*inputs : CA/B1,CA/B2 */
	video_data_w, gameplan_video_command_w,	/*outputs: A/B         */
	0, 0, video_command_trigger_w, 0,		/*outputs: CA/B1,CA/B2 */
	via_irq									/*irq                  */
};


static const struct via6522_interface leprechn_via_0_interface =
{
	0, vblank_r,							/*inputs : A/B         */
	0, 0, 0, 0,								/*inputs : CA/B1,CA/B2 */
	video_data_w, leprechn_video_command_w,	/*outputs: A/B         */
	0, 0, video_command_trigger_w, 0,		/*outputs: CA/B1,CA/B2 */
	via_irq									/*irq                  */
};


static const struct via6522_interface trvquest_via_0_interface =
{
	0, vblank_r,							/*inputs : A/B         */
	0, 0, 0, 0,								/*inputs : CA/B1,CA/B2 */
	video_data_w, gameplan_video_command_w,	/*outputs: A/B         */
	0, 0, video_command_trigger_w, 0,		/*outputs: CA/B1,CA/B2 */
	0										/*irq                  */
};


static TIMER_CALLBACK( via_0_ca1_timer_callback )
{
	gameplan_state *state = machine->driver_data;

	/* !VBLANK is connected to CA1 */
	via_0_ca1_w(machine, 0, (UINT8)param);

	if (param)
		timer_adjust_oneshot(state->via_0_ca1_timer, video_screen_get_time_until_pos(0, VBSTART, 0), 0);
	else
		timer_adjust_oneshot(state->via_0_ca1_timer, video_screen_get_time_until_pos(0, VBEND, 0), 1);
}


static void create_via_0_timer(gameplan_state *state)
{
	state->via_0_ca1_timer = timer_alloc(via_0_ca1_timer_callback, NULL);
}


static void start_via_0_timer(gameplan_state *state)
{
	timer_adjust_oneshot(state->via_0_ca1_timer, video_screen_get_time_until_pos(0, VBSTART, 0), 0);
}



/*************************************
 *
 *  Start
 *
 *************************************/

static VIDEO_START( common )
{
	gameplan_state *state = machine->driver_data;

	state->videoram_size = (HBSTART - HBEND) * (VBSTART - VBEND);
	state->videoram = auto_malloc(state->videoram_size);

	create_via_0_timer(state);

	/* register for save states */
	state_save_register_global_pointer(state->videoram, state->videoram_size);
	state_save_register_global(state->video_x);
	state_save_register_global(state->video_y);
	state_save_register_global(state->video_command);
	state_save_register_global(state->video_data);
}


static VIDEO_START( gameplan )
{
	via_config(0, &gameplan_via_0_interface);

	VIDEO_START_CALL(common);
}


static VIDEO_START( leprechn )
{
	via_config(0, &leprechn_via_0_interface);

	VIDEO_START_CALL(common);
}


static VIDEO_START( trvquest )
{
	via_config(0, &trvquest_via_0_interface);

	VIDEO_START_CALL(common);
}



/*************************************
 *
 *  Reset
 *
 *************************************/

static VIDEO_RESET( gameplan )
{
	via_reset();
	start_via_0_timer(machine->driver_data);
}



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_DRIVER_START( gameplan_video )
	MDRV_VIDEO_START(gameplan)
	MDRV_VIDEO_RESET(gameplan)

	MDRV_VIDEO_START(gameplan)
	MDRV_VIDEO_UPDATE(gameplan)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(GAMEPLAN_PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( leprechn_video )
	MDRV_VIDEO_START(leprechn)
	MDRV_VIDEO_UPDATE(leprechn)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( trvquest_video )
	MDRV_IMPORT_FROM(gameplan_video)
	MDRV_VIDEO_START(trvquest)
	MDRV_VIDEO_UPDATE(gameplan)
MACHINE_DRIVER_END
