/*
 * mb3773 - Power Supply Monitor with Watch Dog Timer
 *
 * Todo:
 *  Calculate the timeout from parameters.
 *
 */

#include "driver.h"
#include "machine/mb3773.h"

static emu_timer *watchdog_timer;
static UINT8 ck = 0;

static TIMER_CALLBACK( watchdog_timeout )
{
	mame_schedule_soft_reset(machine);
}

static void reset_timer( void )
{
	timer_adjust_oneshot( watchdog_timer, ATTOTIME_IN_SEC( 5 ), 0 );
}

void mb3773_set_ck( UINT8 new_ck )
{
	if( new_ck == 0 && ck != 0 )
	{
		reset_timer();
	}
	ck = new_ck;
}

void mb3773_init( running_machine *machine )
{
	watchdog_timer = timer_alloc(machine, watchdog_timeout, NULL);
	reset_timer();
	state_save_register_global(machine,  ck );
}
