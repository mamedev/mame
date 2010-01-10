/***************************************************************************

    Fujitsu MB3773

    Power Supply Monitor with Watch Dog Timer (i.e. Reset IC)


    Todo:
        Calculate the timeout from parameters.


    2009-06 Converted to be a device

***************************************************************************/

#include "emu.h"
#include "mb3773.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _mb3773_state mb3773_state;
struct _mb3773_state
{
	emu_timer *watchdog_timer;
	UINT8 ck;
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE mb3773_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert((device->type == MB3773));
	return (mb3773_state *)device->token;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    TIMER_CALLBACK( watchdog_timeout )
-------------------------------------------------*/

static TIMER_CALLBACK( watchdog_timeout )
{
	mame_schedule_soft_reset(machine);
}

/*-------------------------------------------------
    reset_timer
-------------------------------------------------*/

static void reset_timer( const device_config *device )
{
	mb3773_state *mb3773 = get_safe_token(device);
	timer_adjust_oneshot(mb3773->watchdog_timer, ATTOTIME_IN_SEC( 5 ), 0);
}

/*-------------------------------------------------
    mb3773_set_ck
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( mb3773_set_ck )
{
	mb3773_state *mb3773 = get_safe_token(device);
	if( data == 0 && mb3773->ck != 0 )
	{
		reset_timer(device);
	}
	mb3773->ck = data;
}

/*-------------------------------------------------
    DEVICE_START( mb3773 )
-------------------------------------------------*/

static DEVICE_START( mb3773 )
{
	mb3773_state *mb3773 = get_safe_token(device);

	/* create the timer */
	mb3773->watchdog_timer = timer_alloc(device->machine, watchdog_timeout, NULL);
	reset_timer(device);

	/* register for state saving */
	state_save_register_device_item(device, 0, mb3773->ck);
}

/*-------------------------------------------------
    DEVICE_RESET( mb3773 )
-------------------------------------------------*/

static DEVICE_RESET( mb3773 )
{
	mb3773_state *mb3773 = get_safe_token(device);
	mb3773->ck = 0;
}

/*-------------------------------------------------
    DEVICE_GET_INFO( mb3773 )
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##mb3773##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME		"Fujistu MB3773"
#define DEVTEMPLATE_FAMILY		"Fujistu Power Supply Monitor"
#define DEVTEMPLATE_CLASS		DEVICE_CLASS_PERIPHERAL
#include "devtempl.h"
