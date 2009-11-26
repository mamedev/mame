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

#define MAX_DISPENSERS	2

struct ticket_state
{
	UINT8 status;
	UINT8 power;
	emu_timer *timer;
};

static int active_bit = 0x80;
static int time_msec;
static int motoron;
static int ticketdispensed;
static int ticketnotdispensed;

static struct ticket_state dispenser[MAX_DISPENSERS];

static TIMER_CALLBACK( ticket_dispenser_toggle );


/***************************************************************************
  ticket_dispenser_init

***************************************************************************/
void ticket_dispenser_init(running_machine *machine, int msec, int motoronhigh, int statusactivehigh)
{
	int i;

	time_msec			= msec;
	motoron				= motoronhigh  ? active_bit : 0;
	ticketdispensed		= statusactivehigh ? active_bit : 0;
	ticketnotdispensed	= ticketdispensed ^ active_bit;

	for (i = 0; i < MAX_DISPENSERS; i++)
	{
		dispenser[i].status	= ticketnotdispensed;
		dispenser[i].power 	= 0x00;
		dispenser[i].timer 	= timer_alloc(machine, ticket_dispenser_toggle, &dispenser[i]);

		state_save_register_item(machine, "ticket", NULL, i, dispenser[i].status);
		state_save_register_item(machine, "ticket", NULL, i, dispenser[i].power);
	}
}

/***************************************************************************
  ticket_dispenser_r
***************************************************************************/
READ8_HANDLER( ticket_dispenser_r )
{
	return ticket_dispenser_0_r(space, offset);
}

READ8_HANDLER( ticket_dispenser_0_r )
{
	LOG(("PC: %04X  Ticket Status Read = %02X\n", cpu_get_pc(space->cpu), dispenser[0].status));
	return dispenser[0].status;
}

READ8_HANDLER( ticket_dispenser_1_r )
{
	LOG(("PC: %04X  Ticket Status Read = %02X\n", cpu_get_pc(space->cpu), dispenser[1].status));
	return dispenser[1].status;
}

CUSTOM_INPUT( ticket_dispenser_0_port_r )
{
	return dispenser[0].status ? 1 : 0;
}

CUSTOM_INPUT( ticket_dispenser_1_port_r )
{
	return dispenser[1].status ? 1 : 0;
}

/***************************************************************************
  ticket_dispenser_w
***************************************************************************/
WRITE8_HANDLER( ticket_dispenser_w )
{
	ticket_dispenser_0_w(space, offset, data);
}

WRITE8_HANDLER( ticket_dispenser_0_w )
{
	/* On an activate signal, start dispensing! */
	if ((data & active_bit) == motoron)
	{
		if (!dispenser[0].power)
		{
			LOG(("PC: %04X  Ticket Power On\n", cpu_get_pc(space->cpu)));
			timer_adjust_oneshot(dispenser[0].timer, ATTOTIME_IN_MSEC(time_msec), 0);
			dispenser[0].power = 1;

			dispenser[0].status = ticketnotdispensed;
		}
	}
	else
	{
		if (dispenser[0].power)
		{
			LOG(("PC: %04X  Ticket Power Off\n", cpu_get_pc(space->cpu)));
			timer_adjust_oneshot(dispenser[0].timer, attotime_never, 0);
			set_led_status(space->machine, 2,0);
			dispenser[0].power = 0;
		}
	}
}

WRITE8_HANDLER( ticket_dispenser_1_w )
{
	/* On an activate signal, start dispensing! */
	if ((data & active_bit) == motoron)
	{
		if (!dispenser[1].power)
		{
			LOG(("PC: %04X  Ticket Power On\n", cpu_get_pc(space->cpu)));
			timer_adjust_oneshot(dispenser[1].timer, ATTOTIME_IN_MSEC(time_msec), 0);
			dispenser[1].power = 1;

			dispenser[1].status = ticketnotdispensed;
		}
	}
	else
	{
		if (dispenser[1].power)
		{
			LOG(("PC: %04X  Ticket Power Off\n", cpu_get_pc(space->cpu)));
			timer_adjust_oneshot(dispenser[1].timer, attotime_never, 0);
			set_led_status(space->machine, 2,0);
			dispenser[1].power = 0;
		}
	}
}


/***************************************************************************
  ticket_dispenser_toggle

  How I think this works:
  When a ticket dispenses, there is N milliseconds of status = high,
  and N milliseconds of status = low (a wait cycle?).
***************************************************************************/
static TIMER_CALLBACK( ticket_dispenser_toggle )
{
	struct ticket_state *dispenser = (struct ticket_state *)ptr;

	/* If we still have power, keep toggling ticket states. */
	if (dispenser->power)
	{
		dispenser->status ^= active_bit;
		LOG(("Ticket Status Changed to %02X\n", dispenser->status));
		timer_adjust_oneshot(dispenser->timer, ATTOTIME_IN_MSEC(time_msec), 0);
	}

	if (dispenser->status == ticketdispensed)
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
