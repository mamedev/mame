/***************************************************************************

  machine.c

  Functions to emulate a prototypical ticket dispenser hardware.

  Right now, this is an *extremely* basic ticket dispenser.
  TODO: Active Bit may not be Bit 7 in all applications.
        Add a ticket dispenser interface instead of passing a bunch
        of arguments to ticket_dispenser_init.
***************************************************************************/

#include "driver.h"
#include "machine/ticket.h"

#define DEBUG_TICKET 0
#define LOG(x) do { if (DEBUG_TICKET) logerror x; } while (0)


typedef struct _ticket_state ticket_state;
struct _ticket_state
{
	int active_bit;
	int time_msec;
	int motoron;
	int ticketdispensed;
	int ticketnotdispensed;

	UINT8 status;
	UINT8 power;
	emu_timer *timer;
};

INLINE ticket_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == TICKET_DISPENSER);

	return (ticket_state *)device->token;
}


static TIMER_CALLBACK( ticket_dispenser_toggle )
{
	ticket_state *state = get_safe_token((const device_config *)ptr);

	/* If we still have power, keep toggling ticket states. */
	if (state->power)
	{
		state->status ^= state->active_bit;
		LOG(("Ticket Status Changed to %02X\n", state->status));
		timer_adjust_oneshot(state->timer, ATTOTIME_IN_MSEC(state->time_msec), 0);
	}

	if (state->status == state->ticketdispensed)
	{
		set_led_status(machine, 2,1);
		increment_dispensed_tickets(machine, 1);

		LOG(("Ticket Dispensed\n"));
	}
	else
	{
		set_led_status(machine, 2,0);
	}
}


READ8_DEVICE_HANDLER( ticket_dispenser_r )
{
	ticket_state *state = get_safe_token(device);
	LOG(("%s: Ticket Status Read = %02X\n", cpuexec_describe_context(device->machine), state->status));
	return state->status;
}


READ_LINE_DEVICE_HANDLER( ticket_dispenser_line_r )
{
	ticket_state *state = get_safe_token(device);
	return state->status ? 1 : 0;
}


WRITE8_DEVICE_HANDLER( ticket_dispenser_w )
{
	ticket_state *state = get_safe_token(device);

	/* On an activate signal, start dispensing! */
	if ((data & state->active_bit) == state->motoron)
	{
		if (!state->power)
		{
			LOG(("%s: Ticket Power On\n", cpuexec_describe_context(device->machine)));
			timer_adjust_oneshot(state->timer, ATTOTIME_IN_MSEC(state->time_msec), 0);
			state->power = 1;

			state->status = state->ticketnotdispensed;
		}
	}
	else
	{
		if (state->power)
		{
			LOG(("%s: Ticket Power Off\n", cpuexec_describe_context(device->machine)));
			timer_adjust_oneshot(state->timer, attotime_never, 0);
			set_led_status(device->machine, 2,0);
			state->power = 0;
		}
	}
}



/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( ticket )
{
	const ticket_config *config = (const ticket_config *)device->inline_config;
	ticket_state *state = get_safe_token(device);

	assert(config != NULL);

	/* initialize the state */
	state->active_bit			= 0x80;
	state->time_msec			= device->clock;
	state->motoron				= config->motorhigh  ? state->active_bit : 0;
	state->ticketdispensed		= config->statushigh ? state->active_bit : 0;
	state->ticketnotdispensed	= state->ticketdispensed ^ state->active_bit;

	state->timer				= timer_alloc(device->machine, ticket_dispenser_toggle, (void *)device);

	state_save_register_device_item(device, 0, state->status);
	state_save_register_device_item(device, 0, state->power);
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( ticket )
{
	ticket_state *state = get_safe_token(device);
	state->status				= state->ticketnotdispensed;
	state->power				= 0x00;
}


/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##ticket##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET | DT_HAS_INLINE_CONFIG
#define DEVTEMPLATE_NAME		"Ticket Dispenser"
#define DEVTEMPLATE_FAMILY		"Generic"
#include "devtempl.h"
