/***************************************************************************

    watchdog.c

    Watchdog handling

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static UINT8 watchdog_enabled;
static INT32 watchdog_counter;
static emu_timer *watchdog_timer;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void watchdog_internal_reset(running_machine *machine);
static TIMER_CALLBACK( watchdog_callback );



/*-------------------------------------------------
    watchdog_init - one time initialization
-------------------------------------------------*/

void watchdog_init(running_machine *machine)
{
	/* allocate a timer for the watchdog */
	watchdog_timer = timer_alloc(machine, watchdog_callback, NULL);

	add_reset_callback(machine, watchdog_internal_reset);

	/* save some stuff in the default tag */
	state_save_register_item(machine, "watchdog", NULL, 0, watchdog_enabled);
	state_save_register_item(machine, "watchdog", NULL, 0, watchdog_counter);
}


/*-------------------------------------------------
    watchdog_internal_reset - reset the watchdog
    system
-------------------------------------------------*/

static void watchdog_internal_reset(running_machine *machine)
{
	/* set up the watchdog timer; only start off enabled if explicitly configured */
	watchdog_enabled = (machine->config->watchdog_vblank_count != 0 || attotime_compare(machine->config->watchdog_time, attotime_zero) != 0);
	watchdog_reset(machine);
	watchdog_enabled = TRUE;
}


/*-------------------------------------------------
    watchdog_callback - watchdog timer callback
-------------------------------------------------*/

static TIMER_CALLBACK( watchdog_callback )
{
	logerror("Reset caused by the watchdog!!!\n");

#ifdef MAME_DEBUG
	popmessage("Reset caused by the watchdog!!!\n");
#endif

	mame_schedule_soft_reset(machine);
}


/*-------------------------------------------------
    on_vblank - updates VBLANK based watchdog
    timers
-------------------------------------------------*/

static void on_vblank(running_device *screen, void *param, int vblank_state)
{
	/* VBLANK starting */
	if (vblank_state && watchdog_enabled)
	{
		/* check the watchdog */
		if (screen->machine->config->watchdog_vblank_count != 0)
		{
			watchdog_counter = watchdog_counter - 1;

			if (watchdog_counter == 0)
				watchdog_callback(screen->machine, NULL, 0);
		}
	}
}


/*-------------------------------------------------
    watchdog_reset - reset the watchdog timer
-------------------------------------------------*/

void watchdog_reset(running_machine *machine)
{
	/* if we're not enabled, skip it */
	if (!watchdog_enabled)
		timer_adjust_oneshot(watchdog_timer, attotime_never, 0);

	/* VBLANK-based watchdog? */
	else if (machine->config->watchdog_vblank_count != 0)
	{
		watchdog_counter = machine->config->watchdog_vblank_count;

		/* register a VBLANK callback for the primary screen */
		if (machine->primary_screen != NULL)
			video_screen_register_vblank_callback(machine->primary_screen, on_vblank, NULL);
	}

	/* timer-based watchdog? */
	else if (attotime_compare(machine->config->watchdog_time, attotime_zero) != 0)
		timer_adjust_oneshot(watchdog_timer, machine->config->watchdog_time, 0);

	/* default to an obscene amount of time (3 seconds) */
	else
		timer_adjust_oneshot(watchdog_timer, ATTOTIME_IN_SEC(3), 0);
}


/*-------------------------------------------------
    watchdog_enable - reset the watchdog timer
-------------------------------------------------*/

void watchdog_enable(running_machine *machine, int enable)
{
	/* when re-enabled, we reset our state */
	if (watchdog_enabled != enable)
	{
		watchdog_enabled = enable;
		watchdog_reset(machine);
	}
}
