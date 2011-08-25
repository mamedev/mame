/***************************************************************************

  machine.c

  Functions to emulate a prototypical ticket dispenser hardware.

  Right now, this is an *extremely* basic ticket dispenser.
  TODO: Active Bit may not be Bit 7 in all applications.
        Add a ticket dispenser interface instead of passing a bunch
        of arguments to ticket_dispenser_init.
***************************************************************************/

#include "emu.h"
#include "machine/ticket.h"

#define DEBUG_TICKET 0
#define LOG(x) do { if (DEBUG_TICKET) logerror x; } while (0)


typedef struct _ticket_state ticket_state;
struct _ticket_state
{
	int m_active_bit;
	int m_time_msec;
	int m_motoron;
	int m_ticketdispensed;
	int m_ticketnotdispensed;

	UINT8 m_status;
	UINT8 m_power;
	emu_timer *m_timer;
};

INLINE ticket_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TICKET_DISPENSER);

	return (ticket_state *)downcast<legacy_device_base *>(device)->token();
}


static TIMER_CALLBACK( ticket_dispenser_toggle )
{
	ticket_state *state = get_safe_token((device_t *)ptr);

	/* If we still have power, keep toggling ticket states. */
	if (state->m_power)
	{
		state->m_status ^= state->m_active_bit;
		LOG(("Ticket Status Changed to %02X\n", state->m_status));
		state->m_timer->adjust(attotime::from_msec(state->m_time_msec));
	}

	if (state->m_status == state->m_ticketdispensed)
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
	LOG(("%s: Ticket Status Read = %02X\n", device->machine().describe_context(), state->m_status));
	return state->m_status;
}


READ_LINE_DEVICE_HANDLER( ticket_dispenser_line_r )
{
	ticket_state *state = get_safe_token(device);
	return state->m_status ? 1 : 0;
}


WRITE8_DEVICE_HANDLER( ticket_dispenser_w )
{
	ticket_state *state = get_safe_token(device);

	/* On an activate signal, start dispensing! */
	if ((data & state->m_active_bit) == state->m_motoron)
	{
		if (!state->m_power)
		{
			LOG(("%s: Ticket Power On\n", device->machine().describe_context()));
			state->m_timer->adjust(attotime::from_msec(state->m_time_msec));
			state->m_power = 1;

			state->m_status = state->m_ticketnotdispensed;
		}
	}
	else
	{
		if (state->m_power)
		{
			LOG(("%s: Ticket Power Off\n", device->machine().describe_context()));
			state->m_timer->adjust(attotime::never);
			set_led_status(device->machine(), 2,0);
			state->m_power = 0;
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
	const ticket_config *config = (const ticket_config *)downcast<const legacy_device_base *>(device)->inline_config();
	ticket_state *state = get_safe_token(device);

	assert(config != NULL);

	/* initialize the state */
	state->m_active_bit			= 0x80;
	state->m_time_msec			= device->clock();
	state->m_motoron				= config->motorhigh  ? state->m_active_bit : 0;
	state->m_ticketdispensed		= config->statushigh ? state->m_active_bit : 0;
	state->m_ticketnotdispensed	= state->m_ticketdispensed ^ state->m_active_bit;

	state->m_timer				= device->machine().scheduler().timer_alloc(FUNC(ticket_dispenser_toggle), (void *)device);

	device->save_item(NAME(state->m_status));
	device->save_item(NAME(state->m_power));
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( ticket )
{
	ticket_state *state = get_safe_token(device);
	state->m_status				= state->m_ticketnotdispensed;
	state->m_power				= 0x00;
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


DEFINE_LEGACY_DEVICE(TICKET_DISPENSER, ticket);
