// license:BSD-3-Clause
// copyright-holders:Curt Coder
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

#define TIMER_FLAG      0x80

/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

const device_type MOS6530 = &device_creator<mos6530_device>;

mos6530_device::mos6530_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MOS6530, "MOS6530 RRIOT", tag, owner, clock, "mos6530", __FILE__),
		m_in_pa_cb(*this),
		m_out_pa_cb(*this),
		m_in_pb_cb(*this),
		m_out_pb_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6530_device::device_start()
{
	/* set static values */
	m_clock = clock();

	/* resolve callbacks */
	m_in_pa_cb.resolve_safe(0);
	m_out_pa_cb.resolve_safe();
	m_in_pb_cb.resolve_safe(0);
	m_out_pb_cb.resolve_safe();

	/* allocate timers */
	m_timer = timer_alloc(TIMER_END_CALLBACK);

	/* register for save states */
	save_item(NAME(m_port[0].m_in));
	save_item(NAME(m_port[0].m_out));
	save_item(NAME(m_port[0].m_ddr));
	save_item(NAME(m_port[1].m_in));
	save_item(NAME(m_port[1].m_out));
	save_item(NAME(m_port[1].m_ddr));

	save_item(NAME(m_irqstate));
	save_item(NAME(m_irqenable));

	save_item(NAME(m_timershift));
	save_item(NAME(m_timerstate));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6530_device::device_reset()
{
	/* reset I/O states */
	m_port[0].m_out = 0;
	m_port[0].m_ddr = 0;
	m_port[1].m_out = 0;
	m_port[1].m_ddr = 0;

	/* reset IRQ states */
	m_irqenable = 0;
	m_irqstate = TIMER_FLAG;
	update_irqstate();

	/* reset timer states */
	m_timershift = 0;
	m_timerstate = TIMER_IDLE;
	m_timer->adjust(attotime::never);
}


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


/*-------------------------------------------------
    update_irqstate - update the IRQ state
    based on interrupt enables
-------------------------------------------------*/

void mos6530_device::update_irqstate()
{
	UINT8 out = m_port[1].m_out;

	if (m_irqenable)
		out = ((m_irqstate & TIMER_FLAG) ? 0x00 : 0x80) | (out & 0x7F);

	m_out_pb_cb((offs_t)0, out);
}


/*-------------------------------------------------
    get_timer - return the current timer value
-------------------------------------------------*/

UINT8 mos6530_device::get_timer()
{
	/* if idle, return 0 */
	if (m_timerstate == TIMER_IDLE)
		return 0;

	/* if counting, return the number of ticks remaining */
	else if (m_timerstate == TIMER_COUNTING)
		return m_timer->remaining().as_ticks(m_clock) >> m_timershift;

	/* if finishing, return the number of ticks without the shift */
	else
		return m_timer->remaining().as_ticks(m_clock);
}


/***************************************************************************
    INTERNAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    timer_end_callback - callback to process the
    timer
-------------------------------------------------*/

void mos6530_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		// deferred reset
		case TIMER_END_CALLBACK:
			assert(m_timerstate != TIMER_IDLE);

			/* if we finished counting, switch to the finishing state */
			if (m_timerstate == TIMER_COUNTING)
			{
				m_timerstate = TIMER_FINISHING;
				m_timer->adjust(attotime::from_ticks(256, m_clock));

				/* signal timer IRQ as well */
				m_irqstate |= TIMER_FLAG;
				update_irqstate();
			}

			/* if we finished finishing, switch to the idle state */
			else if (m_timerstate == TIMER_FINISHING)
			{
				m_timerstate = TIMER_IDLE;
				m_timer->adjust(attotime::never);
			}
			break;
	}
}

/***************************************************************************
    I/O ACCESS
***************************************************************************/

/*-------------------------------------------------
    mos6530_w - master I/O write access
-------------------------------------------------*/

WRITE8_MEMBER( mos6530_device::write )
{
	/* if A2 == 1, we are writing to the timer */
	if (offset & 0x04)
	{
		static const UINT8 timershift[4] = { 0, 3, 6, 10 };
		attotime curtime = space.machine().time();
		INT64 target;

		/* A0-A1 contain the timer divisor */
		m_timershift = timershift[offset & 3];

		/* A3 contains the timer IRQ enable */
		if (offset & 8)
			m_irqenable |= TIMER_FLAG;
		else
			m_irqenable &= ~TIMER_FLAG;

		/* writes here clear the timer flag */
		if (m_timerstate != TIMER_FINISHING || get_timer() != 0xff)
			m_irqstate &= ~TIMER_FLAG;
		update_irqstate();

		/* update the timer */
		m_timerstate = TIMER_COUNTING;
		target = curtime.as_ticks(m_clock) + 1 + (data << m_timershift);
		m_timer->adjust(attotime::from_ticks(target, m_clock) - curtime);
	}

	/* if A2 == 0, we are writing to the I/O section */
	else
	{
		/* A1 selects the port */
		mos6530_port *port = &m_port[BIT(offset, 1)];

		/* if A0 == 1, we are writing to the port's DDR */
		if (offset & 1)
			port->m_ddr = data;

		/* if A0 == 0, we are writing to the port's output */
		else
		{
			UINT8 olddata = port->m_out;
			port->m_out = data;

			if ((offset & 2) && m_irqenable)
			{
				olddata = ((m_irqstate & TIMER_FLAG) ? 0x00 : 0x80) | (olddata & 0x7F);
				data = ((m_irqstate & TIMER_FLAG) ? 0x00 : 0x80) | (data & 0x7F);
			}

			if (!BIT(offset, 1))
				m_out_pa_cb((offs_t)0, data);
			else
				m_out_pb_cb((offs_t)0, data);
		}
	}
}


/*-------------------------------------------------
    mos6530_r - master I/O read access
-------------------------------------------------*/

READ8_MEMBER( mos6530_device::read )
{
	UINT8 val;

	/* if A2 == 1 and A0 == 1, we are reading interrupt flags */
	if ((offset & 0x05) == 0x05)
	{
		val = m_irqstate;
	}

	/* if A2 == 1 and A0 == 0, we are reading the timer */
	else if ((offset & 0x05) == 0x04)
	{
		val = get_timer();

		/* A3 contains the timer IRQ enable */
		if (offset & 8)
			m_irqenable |= TIMER_FLAG;
		else
			m_irqenable &= ~TIMER_FLAG;

		/* implicitly clears the timer flag */
		if (m_timerstate != TIMER_FINISHING || val != 0xff)
			m_irqstate &= ~TIMER_FLAG;
		update_irqstate();
	}

	/* if A2 == 0 and A0 == anything, we are reading from ports */
	else
	{
		/* A1 selects the port */
		mos6530_port *port = &m_port[BIT(offset, 1)];

		/* if A0 == 1, we are reading the port's DDR */
		if (offset & 1)
			val = port->m_ddr;

		/* if A0 == 0, we are reading the port as an input */
		else
		{
			UINT8 out = port->m_out;

			if ((offset & 2) && m_irqenable)
				out = ((m_irqstate & TIMER_FLAG) ? 0x00 : 0x80) | (out & 0x7F);

			/* call the input callback if it exists */
			if (!BIT(offset, 1))
				port->m_in = m_in_pa_cb(0);
			else
				port->m_in = m_in_pb_cb(0);

			/* apply the DDR to the result */
			val = (out & port->m_ddr) | (port->m_in & ~port->m_ddr);
		}
	}
	return val;
}


/*-------------------------------------------------
    mos6530_porta_in_set - set port A input
    value
-------------------------------------------------*/

void mos6530_device::porta_in_set(UINT8 data, UINT8 mask)
{
	m_port[0].m_in = (m_port[0].m_in & ~mask) | (data & mask);
}


/*-------------------------------------------------
    mos6530_portb_in_set - set port B input
    value
-------------------------------------------------*/

void mos6530_device::portb_in_set(UINT8 data, UINT8 mask)
{
	m_port[1].m_in = (m_port[1].m_in & ~mask) | (data & mask);
}


/*-------------------------------------------------
    mos6530_porta_in_get - return port A input
    value
-------------------------------------------------*/

UINT8 mos6530_device::porta_in_get()
{
	return m_port[0].m_in;
}


/*-------------------------------------------------
    mos6530_portb_in_get - return port B input
    value
-------------------------------------------------*/

UINT8 mos6530_device::portb_in_get()
{
	return m_port[1].m_in;
}


/*-------------------------------------------------
    mos6530_porta_in_get - return port A output
    value
-------------------------------------------------*/

UINT8 mos6530_device::porta_out_get()
{
	return m_port[0].m_out;
}


/*-------------------------------------------------
    mos6530_portb_in_get - return port B output
    value
-------------------------------------------------*/

UINT8 mos6530_device::portb_out_get()
{
	return m_port[1].m_out;
}
