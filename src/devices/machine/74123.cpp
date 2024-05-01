// license:BSD-3-Clause
// copyright-holders:Couriersud
/*****************************************************************************

    74123 monoflop emulator - see 74123.h for pin out and truth table

    Formulas came from the TI datasheet revised on March 1998

 *****************************************************************************/

#include "emu.h"
#include "74123.h"

#include "machine/rescap.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(TTL74123, ttl74123_device, "ttl74123", "74123 TTL")

//-------------------------------------------------
//  ttl74123_device - constructor
//-------------------------------------------------

ttl74123_device::ttl74123_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TTL74123, tag, owner, clock),
	m_clear_timer(nullptr),
	m_output_timer(nullptr),
	m_connection_type(TTL74123_NOT_GROUNDED_NO_DIODE),
	m_res(1.0),
	m_cap(1.0),
	m_a(0),
	m_b(0),
	m_clear(0),
	m_output_changed_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl74123_device::device_start()
{
	m_clear_timer = timer_alloc(FUNC(ttl74123_device::clear_callback), this);
	m_output_timer = timer_alloc(FUNC(ttl74123_device::output_callback), this);

	/* register for state saving */
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_clear));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ttl74123_device::device_reset()
{
	set_output();
}



//-------------------------------------------------
//  compute_duration - compute timer duration
//-------------------------------------------------

attotime ttl74123_device::compute_duration()
{
	double duration;

	switch (m_connection_type)
	{
	case TTL74123_NOT_GROUNDED_NO_DIODE:
		duration = 0.28 * m_res * m_cap * (1.0 + (700.0 / m_res));
		break;

	case TTL74123_NOT_GROUNDED_DIODE:
		duration = 0.25 * m_res * m_cap * (1.0 + (700.0 / m_res));
		break;

	case TTL74123_GROUNDED:
	default:
		if (m_cap < CAP_U(0.1))
		{
			/* this is really a curve - a very flat one in the 0.1uF-.01uF range */
			duration = 0.32 * m_res * m_cap;
		}
		else
		{
			duration = 0.33 * m_res * m_cap;
		}
		break;
	}

	return attotime::from_double(duration);
}


//-------------------------------------------------
//  timer_running - is the timer running?
//-------------------------------------------------

int ttl74123_device::timer_running()
{
	return m_clear_timer->remaining() > attotime::zero && !m_clear_timer->remaining().is_never();
}


/*-------------------------------------------------
    TIMER_CALLBACK_MEMBER( output_callback )
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER( ttl74123_device::output_callback )
{
	m_output_changed_cb(param);
}


//-------------------------------------------------
//  set_output - set the output line state
//-------------------------------------------------

void ttl74123_device::set_output()
{
	int output = timer_running();

	m_output_timer->adjust(attotime::zero, output);

	LOG("74123:  Output: %d\n", output);
}


/*-------------------------------------------------
    TIMER_CALLBACK_MEMBER( clear_callback )
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER( ttl74123_device::clear_callback )
{
	int output = timer_running();

	m_output_changed_cb(output);
}

//-------------------------------------------------
//  start_pulse - begin timing
//-------------------------------------------------

void ttl74123_device::start_pulse()
{
	attotime duration = compute_duration();

	if(timer_running())
	{
		/* retriggering, but not if we are called to quickly */
		attotime delay_time = attotime(0, ATTOSECONDS_PER_SECOND * m_cap * 220);

		if(m_clear_timer->elapsed() >= delay_time)
		{
			m_clear_timer->adjust(duration);

			LOG("74123:  Retriggering pulse.  Duration: %f\n", duration.as_double());
		}
		else
		{
			LOG("74123:  Retriggering failed.\n");
		}
	}
	else
	{
		/* starting */
		m_clear_timer->adjust(duration);

		set_output();

		LOG("74123:  Starting pulse.  Duration: %f\n", duration.as_double());
	}
}


//-------------------------------------------------
//  a_w - write register a data
//-------------------------------------------------

void ttl74123_device::a_w(int state)
{
	/* start/regtrigger pulse if B=HI and falling edge on A (while clear is HI) */
	if (!state && m_a && m_b && m_clear)
	{
		start_pulse();
	}

	m_a = state;
}


//-------------------------------------------------
//  b_w - write register b data
//-------------------------------------------------

void ttl74123_device::b_w(int state)
{
	/* start/regtrigger pulse if A=LO and rising edge on B (while clear is HI) */
	if (state && !m_b && !m_a && m_clear)
	{
		start_pulse();
	}

	m_b = state;
}


//-------------------------------------------------
//  clear_w - write register clear data
//-------------------------------------------------

void ttl74123_device::clear_w(int state)
{
	/* start/regtrigger pulse if B=HI and A=LO and rising edge on clear */
	if (state && !m_a && m_b && !m_clear)
	{
		start_pulse();
	}
	else if (!state)  /* clear the output  */
	{
		m_clear_timer->adjust(attotime::zero);

		LOG("74123:  Cleared\n");
	}
	m_clear = state;
}


//-------------------------------------------------
//  reset_w - reset device
//-------------------------------------------------

void ttl74123_device::reset_w(int state)
{
	set_output();
}
