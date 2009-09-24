/***************************************************************************

  RIOT 6532 emulation

The timer seems to follow these rules:
- When the timer flag changes from 0 to 1 the timer continues to count
  down at a 1 cycle rate.
- When the timer is being read or written the timer flag is reset.
- When the timer flag is set and the timer contents are 0, the counting
  stops.

***************************************************************************/

#include "driver.h"
#include "6532riot.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	TIMER_IDLE,
	TIMER_COUNTING,
	TIMER_FINISHING
};

#define TIMER_FLAG		0x80
#define PA7_FLAG		0x40



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _riot6532_port riot6532_port;
struct _riot6532_port
{
	UINT8					in;
	UINT8					out;
	UINT8			 		ddr;
	devcb_resolved_read8	in_func;
	devcb_resolved_write8	out_func;
};


typedef struct _riot6532_state riot6532_state;
struct _riot6532_state
{
	const device_config *device;
	const riot6532_interface *intf;
	int				index;

	riot6532_port	port[2];

	devcb_resolved_write_line	irq_func;

	UINT8			irqstate;
	UINT8			irqenable;

	UINT8 			pa7dir;		/* 0x80 = high-to-low, 0x00 = low-to-high */
	UINT8 			pa7prev;

	UINT8			timershift;
	UINT8			timerstate;
	emu_timer *		timer;
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - convert a device's token
    into a riot6532_state
-------------------------------------------------*/

INLINE riot6532_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == RIOT6532);
	return (riot6532_state *)device->token;
}


/*-------------------------------------------------
    update_irqstate - update the IRQ state
    based on interrupt enables
-------------------------------------------------*/

INLINE void update_irqstate(const device_config *device)
{
	riot6532_state *riot = get_safe_token(device);
	int state = (riot->irqstate & riot->irqenable);

	if (riot->irq_func.write != NULL)
		devcb_call_write_line(&riot->irq_func, (state != 0) ? ASSERT_LINE : CLEAR_LINE);
	else
		logerror("%s:6532RIOT chip #%d: no irq callback function\n", cpuexec_describe_context(device->machine), riot->index);
}


/*-------------------------------------------------
    apply_ddr - combine inputs and outputs
    according to the DDR
-------------------------------------------------*/

INLINE UINT8 apply_ddr(const riot6532_port *port)
{
	return (port->out & port->ddr) | (port->in & ~port->ddr);
}


/*-------------------------------------------------
    update_pa7_state - see if PA7 has changed
    and signal appropriately
-------------------------------------------------*/

INLINE void update_pa7_state(const device_config *device)
{
	riot6532_state *riot = get_safe_token(device);
	UINT8 data = apply_ddr(&riot->port[0]) & 0x80;

	/* if the state changed in the correct direction, set the PA7 flag and update IRQs */
	if ((riot->pa7prev ^ data) && (riot->pa7dir ^ data) == 0)
	{
		riot->irqstate |= PA7_FLAG;
		update_irqstate(device);
	}
	riot->pa7prev = data;
}


/*-------------------------------------------------
    get_timer - return the current timer value
-------------------------------------------------*/

INLINE UINT8 get_timer(riot6532_state *riot)
{
	/* if idle, return 0 */
	if (riot->timerstate == TIMER_IDLE)
		return 0;

	/* if counting, return the number of ticks remaining */
	else if (riot->timerstate == TIMER_COUNTING)
		return attotime_to_ticks(timer_timeleft(riot->timer), riot->device->clock) >> riot->timershift;

	/* if finishing, return the number of ticks without the shift */
	else
		return attotime_to_ticks(timer_timeleft(riot->timer), riot->device->clock);
}



/***************************************************************************
    INTERNAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    timer_end_callback - callback to process the
    timer
-------------------------------------------------*/

static TIMER_CALLBACK( timer_end_callback )
{
	const device_config *device = (const device_config *)ptr;
	riot6532_state *riot = get_safe_token(device);

	assert(riot->timerstate != TIMER_IDLE);

	/* if we finished counting, switch to the finishing state */
	if (riot->timerstate == TIMER_COUNTING)
	{
		riot->timerstate = TIMER_FINISHING;
		timer_adjust_oneshot(riot->timer, ticks_to_attotime(256, device->clock), 0);

		/* signal timer IRQ as well */
		riot->irqstate |= TIMER_FLAG;
		update_irqstate(device);
	}

	/* if we finished finishing, keep spinning */
	else if (riot->timerstate == TIMER_FINISHING)
	{
		timer_adjust_oneshot(riot->timer, ticks_to_attotime(256, device->clock), 0);
	}
}



/***************************************************************************
    I/O ACCESS
***************************************************************************/

/*-------------------------------------------------
    riot6532_w - master I/O write access
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( riot6532_w )
{
	riot6532_state *riot = get_safe_token(device);

	/* if A4 == 1 and A2 == 1, we are writing to the timer */
	if ((offset & 0x14) == 0x14)
	{
		static const UINT8 timershift[4] = { 0, 3, 6, 10 };
		attotime curtime = timer_get_time(device->machine);
		INT64 target;

		/* A0-A1 contain the timer divisor */
		riot->timershift = timershift[offset & 3];

		/* A3 contains the timer IRQ enable */
		if (offset & 8)
			riot->irqenable |= TIMER_FLAG;
		else
			riot->irqenable &= ~TIMER_FLAG;

		/* writes here clear the timer flag */
		if (riot->timerstate != TIMER_FINISHING || get_timer(riot) != 0xff)
			riot->irqstate &= ~TIMER_FLAG;
		update_irqstate(device);

		/* update the timer */
		riot->timerstate = TIMER_COUNTING;
		target = attotime_to_ticks(curtime, device->clock) + 1 + (data << riot->timershift);
		timer_adjust_oneshot(riot->timer, attotime_sub(ticks_to_attotime(target, device->clock), curtime), 0);
	}

	/* if A4 == 0 and A2 == 1, we are writing to the edge detect control */
	else if ((offset & 0x14) == 0x04)
	{
		/* A1 contains the A7 IRQ enable */
		if (offset & 2)
			riot->irqenable |= PA7_FLAG;
		else
			riot->irqenable &= ~PA7_FLAG;

		/* A0 specifies the edge detect direction: 0=negative, 1=positive */
		riot->pa7dir = (offset & 1) << 7;
	}

	/* if A4 == anything and A2 == 0, we are writing to the I/O section */
	else
	{
		/* A1 selects the port */
		riot6532_port *port = &riot->port[(offset >> 1) & 1];

		/* if A0 == 1, we are writing to the port's DDR */
		if (offset & 1)
			port->ddr = data;

		/* if A0 == 0, we are writing to the port's output */
		else
		{
			port->out = data;
			if (port->out_func.write != NULL)
				devcb_call_write8(&port->out_func, 0, data);
			else
				logerror("%s:6532RIOT chip %s: Port %c is being written to but has no handler. %02X\n", cpuexec_describe_context(device->machine), device->tag, 'A' + (offset & 1), data);
		}

		/* writes to port A need to update the PA7 state */
		if (port == &riot->port[0])
			update_pa7_state(device);
	}
}


/*-------------------------------------------------
    riot6532_r - master I/O read access
-------------------------------------------------*/

READ8_DEVICE_HANDLER( riot6532_r )
{
	riot6532_state *riot = get_safe_token(device);
	UINT8 val = 0;

	/* if A2 == 1 and A0 == 1, we are reading interrupt flags */
	if ((offset & 0x05) == 0x05)
	{
		val = riot->irqstate;

		/* implicitly clears the PA7 flag */
		riot->irqstate &= ~PA7_FLAG;
		update_irqstate(device);
	}

	/* if A2 == 1 and A0 == 0, we are reading the timer */
	else if ((offset & 0x05) == 0x04)
	{
		val = get_timer(riot);

		/* A3 contains the timer IRQ enable */
		if (offset & 8)
			riot->irqenable |= TIMER_FLAG;
		else
			riot->irqenable &= ~TIMER_FLAG;

		/* implicitly clears the timer flag */
		if (riot->timerstate != TIMER_FINISHING || val != 0xff)
			riot->irqstate &= ~TIMER_FLAG;
		update_irqstate(device);
	}

	/* if A2 == 0 and A0 == anything, we are reading from ports */
	else
	{
		/* A1 selects the port */
		riot6532_port *port = &riot->port[(offset >> 1) & 1];

		/* if A0 == 1, we are reading the port's DDR */
		if (offset & 1)
			val = port->ddr;

		/* if A0 == 0, we are reading the port as an input */
		else
		{
			/* call the input callback if it exists */
			if (port->in_func.read != NULL)
			{
				port->in = devcb_call_read8(&port->in_func, 0);

				/* changes to port A need to update the PA7 state */
				if (port == &riot->port[0])
					update_pa7_state(device);
			}
			else
				logerror("%s:6532RIOT chip %s: Port %c is being read but has no handler\n", cpuexec_describe_context(device->machine), device->tag, 'A' + (offset & 1));

			/* apply the DDR to the result */
			val = apply_ddr(port);
		}
	}
	return val;
}


/*-------------------------------------------------
    riot6532_porta_in_set - set port A input
    value
-------------------------------------------------*/

void riot6532_porta_in_set(const device_config *device, UINT8 data, UINT8 mask)
{
	riot6532_state *riot = get_safe_token(device);
	riot->port[0].in = (riot->port[0].in & ~mask) | (data & mask);
	update_pa7_state(device);
}


/*-------------------------------------------------
    riot6532_portb_in_set - set port B input
    value
-------------------------------------------------*/

void riot6532_portb_in_set(const device_config *device, UINT8 data, UINT8 mask)
{
	riot6532_state *riot = get_safe_token(device);
	riot->port[1].in = (riot->port[1].in & ~mask) | (data & mask);
}


/*-------------------------------------------------
    riot6532_porta_in_get - return port A input
    value
-------------------------------------------------*/

UINT8 riot6532_porta_in_get(const device_config *device)
{
	riot6532_state *riot = get_safe_token(device);
	return riot->port[0].in;
}


/*-------------------------------------------------
    riot6532_portb_in_get - return port B input
    value
-------------------------------------------------*/

UINT8 riot6532_portb_in_get(const device_config *device)
{
	riot6532_state *riot = get_safe_token(device);
	return riot->port[1].in;
}


/*-------------------------------------------------
    riot6532_porta_in_get - return port A output
    value
-------------------------------------------------*/

UINT8 riot6532_porta_out_get(const device_config *device)
{
	riot6532_state *riot = get_safe_token(device);
	return riot->port[0].out;
}


/*-------------------------------------------------
    riot6532_portb_in_get - return port B output
    value
-------------------------------------------------*/

UINT8 riot6532_portb_out_get(const device_config *device)
{
	riot6532_state *riot = get_safe_token(device);
	return riot->port[1].out;
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    riot6532_portb_r - return port B output
    value
-------------------------------------------------*/

static DEVICE_START( riot6532 )
{
	riot6532_state *riot = get_safe_token(device);

	/* validate arguments */
	assert(device != NULL);
	assert(device->tag != NULL);
	assert(strlen(device->tag) < 20);

	/* set static values */
	riot->device = device;
	riot->intf = (riot6532_interface *)device->static_config;
	riot->index = device_list_index(device->machine->config->devicelist, RIOT6532, device->tag);

	/* configure the ports */
	devcb_resolve_read8(&riot->port[0].in_func, &riot->intf->in_a_func, device);
	devcb_resolve_write8(&riot->port[0].out_func, &riot->intf->out_a_func, device);
	devcb_resolve_read8(&riot->port[1].in_func, &riot->intf->in_b_func, device);
	devcb_resolve_write8(&riot->port[1].out_func, &riot->intf->out_b_func, device);

	/* resolve irq func */
	devcb_resolve_write_line(&riot->irq_func, &riot->intf->irq_func, device);

	/* allocate timers */
	riot->timer = timer_alloc(device->machine, timer_end_callback, (void *)device);

	/* register for save states */
	state_save_register_device_item(device, 0, riot->port[0].in);
	state_save_register_device_item(device, 0, riot->port[0].out);
	state_save_register_device_item(device, 0, riot->port[0].ddr);
	state_save_register_device_item(device, 0, riot->port[1].in);
	state_save_register_device_item(device, 0, riot->port[1].out);
	state_save_register_device_item(device, 0, riot->port[1].ddr);

	state_save_register_device_item(device, 0, riot->irqstate);
	state_save_register_device_item(device, 0, riot->irqenable);

	state_save_register_device_item(device, 0, riot->pa7dir);
	state_save_register_device_item(device, 0, riot->pa7prev);

	state_save_register_device_item(device, 0, riot->timershift);
	state_save_register_device_item(device, 0, riot->timerstate);
}


static DEVICE_RESET( riot6532 )
{
	riot6532_state *riot = get_safe_token(device);

	/* reset I/O states */
	riot->port[0].out = 0;
	riot->port[0].ddr = 0;
	riot->port[1].out = 0;
	riot->port[1].ddr = 0;

	/* reset IRQ states */
	riot->irqenable = 0;
	riot->irqstate = 0;
	update_irqstate(device);

	/* reset PA7 states */
	riot->pa7dir = 0;
	riot->pa7prev = 0;

	/* reset timer states */
	riot->timershift = 0;
	riot->timerstate = TIMER_IDLE;
	timer_adjust_oneshot(riot->timer, attotime_never, 0);
}


DEVICE_GET_INFO( riot6532 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(riot6532_state);		break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;							break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(riot6532);break;
		case DEVINFO_FCT_STOP:							/* Nothing */							break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(riot6532);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "6532 (RIOT)");			break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "I/O devices");			break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
