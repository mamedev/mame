/***************************************************************************

  MIOT 6530 emulation

The timer seems to follow these rules:
- When the timer flag changes from 0 to 1 the timer continues to count
  down at a 1 cycle rate.
- When the timer is being read or written the timer flag is reset.
- When the timer flag is set and the timer contents are 0, the counting
  stops.

From the operation of the KIM1 it expects the irqflag to be set whenever
the unit is reset. This is something that is not clear from the datasheet
and should be verified against real hardware.

***************************************************************************/

#include "emu.h"
#include "mos6530.h"


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



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct mos6530_port
{
	devcb_resolved_read8		in_port_func;
	devcb_resolved_write8		out_port_func;

	UINT8				in;
	UINT8				out;
	UINT8				ddr;
};


struct mos6530_state
{
	devcb_resolved_write_line	out_irq_func;

	mos6530_port	port[2];

	UINT8			irqstate;
	UINT8			irqenable;

	UINT8			timershift;
	UINT8			timerstate;
	emu_timer *		timer;

	UINT32			clock;
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - convert a device's token
    into a mos6530_state
-------------------------------------------------*/

INLINE mos6530_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == MOS6530);
	return (mos6530_state *)downcast<mos6530_device *>(device)->token();
}


/*-------------------------------------------------
    update_irqstate - update the IRQ state
    based on interrupt enables
-------------------------------------------------*/

INLINE void update_irqstate(device_t *device)
{
	mos6530_state *miot = get_safe_token(device);
	UINT8 out = miot->port[1].out;

	if ( miot->irqenable )
		out = ( ( miot->irqstate & TIMER_FLAG ) ? 0x00 : 0x80 ) | ( out & 0x7F );

	if (!miot->port[1].out_port_func.isnull())
		miot->port[1].out_port_func(0, out);
	else
		logerror("6530MIOT chip %s: Port B is being written to but has no handler.\n", device->tag());
}


/*-------------------------------------------------
    get_timer - return the current timer value
-------------------------------------------------*/

INLINE UINT8 get_timer(mos6530_state *miot)
{
	/* if idle, return 0 */
	if (miot->timerstate == TIMER_IDLE)
		return 0;

	/* if counting, return the number of ticks remaining */
	else if (miot->timerstate == TIMER_COUNTING)
		return miot->timer->remaining().as_ticks(miot->clock) >> miot->timershift;

	/* if finishing, return the number of ticks without the shift */
	else
		return miot->timer->remaining().as_ticks(miot->clock);
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
	device_t *device = (device_t *)ptr;
	mos6530_state *miot = get_safe_token(device);

	assert(miot->timerstate != TIMER_IDLE);

	/* if we finished counting, switch to the finishing state */
	if (miot->timerstate == TIMER_COUNTING)
	{
		miot->timerstate = TIMER_FINISHING;
		miot->timer->adjust(attotime::from_ticks(256, miot->clock));

		/* signal timer IRQ as well */
		miot->irqstate |= TIMER_FLAG;
		update_irqstate(device);
	}

	/* if we finished finishing, switch to the idle state */
	else if (miot->timerstate == TIMER_FINISHING)
	{
		miot->timerstate = TIMER_IDLE;
		miot->timer->adjust(attotime::never);
	}
}



/***************************************************************************
    I/O ACCESS
***************************************************************************/

/*-------------------------------------------------
    mos6530_w - master I/O write access
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( mos6530_w )
{
	mos6530_state *miot = get_safe_token(device);

	/* if A2 == 1, we are writing to the timer */
	if (offset & 0x04)
	{
		static const UINT8 timershift[4] = { 0, 3, 6, 10 };
		attotime curtime = device->machine().time();
		INT64 target;

		/* A0-A1 contain the timer divisor */
		miot->timershift = timershift[offset & 3];

		/* A3 contains the timer IRQ enable */
		if (offset & 8)
			miot->irqenable |= TIMER_FLAG;
		else
			miot->irqenable &= ~TIMER_FLAG;

		/* writes here clear the timer flag */
		if (miot->timerstate != TIMER_FINISHING || get_timer(miot) != 0xff)
			miot->irqstate &= ~TIMER_FLAG;
		update_irqstate(device);

		/* update the timer */
		miot->timerstate = TIMER_COUNTING;
		target = curtime.as_ticks(miot->clock) + 1 + (data << miot->timershift);
		miot->timer->adjust(attotime::from_ticks(target, miot->clock) - curtime);
	}

	/* if A2 == 0, we are writing to the I/O section */
	else
	{
		/* A1 selects the port */
		mos6530_port *port = &miot->port[(offset >> 1) & 1];

		/* if A0 == 1, we are writing to the port's DDR */
		if (offset & 1)
			port->ddr = data;

		/* if A0 == 0, we are writing to the port's output */
		else
		{
			UINT8 olddata = port->out;
			port->out = data;

			if ( ( offset & 2 ) && miot->irqenable )
			{
				olddata = ( ( miot->irqstate & TIMER_FLAG ) ? 0x00 : 0x80 ) | ( olddata & 0x7F );
				data = ( ( miot->irqstate & TIMER_FLAG ) ? 0x00 : 0x80 ) | ( data & 0x7F );
			}

			if (!port->out_port_func.isnull())
				port->out_port_func(0, data);
			else
				logerror("6530MIOT chip %s: Port %c is being written to but has no handler.  PC: %08X - %02X\n", device->tag(), 'A' + (offset & 1), device->machine().firstcpu->pc(), data);
		}
	}
}


/*-------------------------------------------------
    mos6530_r - master I/O read access
-------------------------------------------------*/

READ8_DEVICE_HANDLER( mos6530_r )
{
	mos6530_state *miot = get_safe_token(device);
	UINT8 val = 0;

	/* if A2 == 1 and A0 == 1, we are reading interrupt flags */
	if ((offset & 0x05) == 0x05)
	{
		val = miot->irqstate;
	}

	/* if A2 == 1 and A0 == 0, we are reading the timer */
	else if ((offset & 0x05) == 0x04)
	{
		val = get_timer(miot);

		/* A3 contains the timer IRQ enable */
		if (offset & 8)
			miot->irqenable |= TIMER_FLAG;
		else
			miot->irqenable &= ~TIMER_FLAG;

		/* implicitly clears the timer flag */
		if (miot->timerstate != TIMER_FINISHING || val != 0xff)
			miot->irqstate &= ~TIMER_FLAG;
		update_irqstate(device);
	}

	/* if A2 == 0 and A0 == anything, we are reading from ports */
	else
	{
		/* A1 selects the port */
		mos6530_port *port = &miot->port[(offset >> 1) & 1];

		/* if A0 == 1, we are reading the port's DDR */
		if (offset & 1)
			val = port->ddr;

		/* if A0 == 0, we are reading the port as an input */
		else
		{
			UINT8	out = port->out;

			if ( ( offset & 2 ) && miot->irqenable )
				out = ( ( miot->irqstate & TIMER_FLAG ) ? 0x00 : 0x80 ) | ( out & 0x7F );

			/* call the input callback if it exists */
			if (!port->in_port_func.isnull())
			{
				port->in = port->in_port_func(0);
			}
			else
				logerror("6530MIOT chip %s: Port %c is being read but has no handler.  PC: %08X\n", device->tag(), 'A' + (offset & 1), device->machine().firstcpu->pc());

			/* apply the DDR to the result */
			val = (out & port->ddr) | (port->in & ~port->ddr);
		}
	}
	return val;
}


/*-------------------------------------------------
    mos6530_porta_in_set - set port A input
    value
-------------------------------------------------*/

void mos6530_porta_in_set(device_t *device, UINT8 data, UINT8 mask)
{
	mos6530_state *miot = get_safe_token(device);
	miot->port[0].in = (miot->port[0].in & ~mask) | (data & mask);
}


/*-------------------------------------------------
    mos6530_portb_in_set - set port B input
    value
-------------------------------------------------*/

void mos6530_portb_in_set(device_t *device, UINT8 data, UINT8 mask)
{
	mos6530_state *miot = get_safe_token(device);
	miot->port[1].in = (miot->port[1].in & ~mask) | (data & mask);
}


/*-------------------------------------------------
    mos6530_porta_in_get - return port A input
    value
-------------------------------------------------*/

UINT8 mos6530_porta_in_get(device_t *device)
{
	mos6530_state *miot = get_safe_token(device);
	return miot->port[0].in;
}


/*-------------------------------------------------
    mos6530_portb_in_get - return port B input
    value
-------------------------------------------------*/

UINT8 mos6530_portb_in_get(device_t *device)
{
	mos6530_state *miot = get_safe_token(device);
	return miot->port[1].in;
}


/*-------------------------------------------------
    mos6530_porta_in_get - return port A output
    value
-------------------------------------------------*/

UINT8 mos6530_porta_out_get(device_t *device)
{
	mos6530_state *miot = get_safe_token(device);
	return miot->port[0].out;
}


/*-------------------------------------------------
    mos6530_portb_in_get - return port B output
    value
-------------------------------------------------*/

UINT8 mos6530_portb_out_get(device_t *device)
{
	mos6530_state *miot = get_safe_token(device);
	return miot->port[1].out;
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

static DEVICE_START( mos6530 )
{
	mos6530_state *miot = get_safe_token(device);
	const mos6530_interface *intf = (const mos6530_interface*)device->static_config();

	/* validate arguments */
	assert(device != NULL);
	assert(device->tag() != NULL);

	/* set static values */
	miot->clock = device->clock();

	/* resolve callbacks */
	miot->port[0].in_port_func.resolve(intf->in_pa_func, *device);
	miot->port[1].in_port_func.resolve(intf->in_pb_func, *device);
	miot->port[0].out_port_func.resolve(intf->out_pa_func, *device);
	miot->port[1].out_port_func.resolve(intf->out_pb_func, *device);

	/* allocate timers */
	miot->timer = device->machine().scheduler().timer_alloc(FUNC(timer_end_callback), (void *)device);

	/* register for save states */
	device->save_item(NAME(miot->port[0].in));
	device->save_item(NAME(miot->port[0].out));
	device->save_item(NAME(miot->port[0].ddr));
	device->save_item(NAME(miot->port[1].in));
	device->save_item(NAME(miot->port[1].out));
	device->save_item(NAME(miot->port[1].ddr));

	device->save_item(NAME(miot->irqstate));
	device->save_item(NAME(miot->irqenable));

	device->save_item(NAME(miot->timershift));
	device->save_item(NAME(miot->timerstate));
}


static DEVICE_RESET( mos6530 )
{
	mos6530_state *miot = get_safe_token(device);

	/* reset I/O states */
	miot->port[0].out = 0;
	miot->port[0].ddr = 0;
	miot->port[1].out = 0;
	miot->port[1].ddr = 0;

	/* reset IRQ states */
	miot->irqenable = 0;
	miot->irqstate = TIMER_FLAG;
	update_irqstate(device);

	/* reset timer states */
	miot->timershift = 0;
	miot->timerstate = TIMER_IDLE;
	miot->timer->adjust(attotime::never);
}


const device_type MOS6530 = &device_creator<mos6530_device>;

mos6530_device::mos6530_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MOS6530, "MOS6530", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(mos6530_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mos6530_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6530_device::device_start()
{
	DEVICE_START_NAME( mos6530 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6530_device::device_reset()
{
	DEVICE_RESET_NAME( mos6530 )(this);
}


