/*****************************************************************************

    74123 monoflop emulator - see 74123.h for pin out and truth table

    Formulas came from the TI datasheet revised on March 1998

 *****************************************************************************/

#include "emu.h"
#include "machine/74123.h"
#include "machine/rescap.h"


#define LOG     (0)



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type TTL74123 = &device_creator<ttl74123_device>;

//-------------------------------------------------
//  ttl74123_device - constructor
//-------------------------------------------------

ttl74123_device::ttl74123_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TTL74123, "TTL74123", tag, owner, clock)
{

}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ttl74123_device::device_config_complete()
{
	// inherit a copy of the static data
	const ttl74123_interface *intf = reinterpret_cast<const ttl74123_interface *>(static_config());
	if (intf != NULL)
		*static_cast<ttl74123_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		m_connection_type = TTL74123_NOT_GROUNDED_NO_DIODE;
		m_res = 1.0;
		m_cap = 1.0;
		m_a = 0;
		m_b = 0;
		m_clear = 0;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl74123_device::device_start()
{
	m_output_changed.resolve(m_output_changed_cb, *this);

	m_timer = machine().scheduler().timer_alloc(FUNC(clear_callback), (void *)this);

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
	return (m_timer->remaining() > attotime::zero) &&
			(m_timer->remaining() != attotime::never);
}


/*-------------------------------------------------
    TIMER_CALLBACK( output_callback )
-------------------------------------------------*/

TIMER_CALLBACK( ttl74123_device::output_callback )
{
	ttl74123_device *dev = reinterpret_cast<ttl74123_device*>(ptr);
	dev->output(param);
}

void ttl74123_device::output(INT32 param)
{
	m_output_changed(0, param);
}


//-------------------------------------------------
//  set_output - set the output line state
//-------------------------------------------------

void ttl74123_device::set_output()
{
	int output = timer_running();

	machine().scheduler().timer_set( attotime::zero, FUNC(output_callback ), output, (void *)this);

	if (LOG) logerror("74123 %s:  Output: %d\n", tag(), output);
}


/*-------------------------------------------------
    TIMER_CALLBACK( clear_callback )
-------------------------------------------------*/

TIMER_CALLBACK( ttl74123_device::clear_callback )
{
	ttl74123_device *dev = reinterpret_cast<ttl74123_device*>(ptr);
	dev->clear();
}

void ttl74123_device::clear()
{
	int output = timer_running();

	m_output_changed(0, output);
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

		if(m_timer->elapsed() >= delay_time)
		{
			m_timer->adjust(duration);

			if (LOG) logerror("74123 %s:  Retriggering pulse.  Duration: %f\n", tag(), duration.as_double());
		}
		else
		{
			if (LOG) logerror("74123 %s:  Retriggering failed.\n", tag());
		}
	}
	else
	{
		/* starting */
		m_timer->adjust(duration);

		set_output();

		if (LOG) logerror("74123 %s:  Starting pulse.  Duration: %f\n", tag(), duration.as_double());
	}
}


//-------------------------------------------------
//  a_w - write register a data
//-------------------------------------------------

WRITE8_DEVICE_HANDLER( ttl74123_a_w )
{
	ttl74123_device *dev = downcast<ttl74123_device *>(device);
	dev->a_w(data);
}

void ttl74123_device::a_w(UINT8 data)
{
	/* start/regtrigger pulse if B=HI and falling edge on A (while clear is HI) */
	if (!data && m_a && m_b && m_clear)
	{
		start_pulse();
	}

	m_a = data;
}


//-------------------------------------------------
//  b_w - write register b data
//-------------------------------------------------

WRITE8_DEVICE_HANDLER( ttl74123_b_w )
{
	ttl74123_device *dev = downcast<ttl74123_device *>(device);
	dev->b_w(data);
}

void ttl74123_device::b_w(UINT8 data)
{
	/* start/regtrigger pulse if A=LO and rising edge on B (while clear is HI) */
	if (data && !m_b && !m_a && m_clear)
	{
		start_pulse();
	}

	m_b = data;
}


//-------------------------------------------------
//  clear_w - write register clear data
//-------------------------------------------------

WRITE8_DEVICE_HANDLER( ttl74123_clear_w )
{
	ttl74123_device *dev = downcast<ttl74123_device *>(device);
	dev->clear_w(data);
}

void ttl74123_device::clear_w(UINT8 data)
{
	/* start/regtrigger pulse if B=HI and A=LO and rising edge on clear */
	if (data && !m_a && m_b && !m_clear)
	{
		start_pulse();
	}
	else if (!data)  /* clear the output  */
	{
		m_timer->adjust(attotime::zero);

		if (LOG) logerror("74123 #%s:  Cleared\n", tag() );
	}
	m_clear = data;
}


//-------------------------------------------------
//  reset_w - reset device
//-------------------------------------------------

WRITE8_DEVICE_HANDLER( ttl74123_reset_w )
{
	ttl74123_device *dev = downcast<ttl74123_device *>(device);
	dev->reset_w();
}

void ttl74123_device::reset_w()
{
	set_output();
}
