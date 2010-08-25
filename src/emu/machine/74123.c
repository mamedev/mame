/*****************************************************************************

    74123 monoflop emulator - see 74123.h for pin out and truth table

    Formulas came from the TI datasheet revised on March 1998

 *****************************************************************************/

#include "emu.h"
#include "machine/74123.h"
#include "machine/rescap.h"


#define	LOG		(0)


//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  ttl74123_device_config - constructor
//-------------------------------------------------

ttl74123_device_config::ttl74123_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "TTL74123", tag, owner, clock)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *ttl74123_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(ttl74123_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *ttl74123_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, ttl74123_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ttl74123_device_config::device_config_complete()
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
    	memset(&m_output_changed_cb, 0, sizeof(m_output_changed_cb));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ttl74123_device - constructor
//-------------------------------------------------

ttl74123_device::ttl74123_device(running_machine &_machine, const ttl74123_device_config &config)
	: device_t(_machine, config),
	  m_config(config)
{

}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ttl74123_device::device_start()
{
	m_timer = timer_alloc(&m_machine, clear_callback, (void *)this);

	/* start with the defaults */
	m_a = m_config.m_a;
	m_b = m_config.m_b;
	m_clear = m_config.m_clear;

	/* register for state saving */
	state_save_register_device_item(this, 0, m_a);
	state_save_register_device_item(this, 0, m_b);
	state_save_register_device_item(this, 0, m_clear);
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

	switch (m_config.m_connection_type)
	{
	case TTL74123_NOT_GROUNDED_NO_DIODE:
		duration = 0.28 * m_config.m_res * m_config.m_cap * (1.0 + (700.0 / m_config.m_res));
		break;

	case TTL74123_NOT_GROUNDED_DIODE:
		duration = 0.25 * m_config.m_res * m_config.m_cap * (1.0 + (700.0 / m_config.m_res));
		break;

	case TTL74123_GROUNDED:
	default:
		if (m_config.m_cap < CAP_U(0.1))
		{
			/* this is really a curve - a very flat one in the 0.1uF-.01uF range */
			duration = 0.32 * m_config.m_res * m_config.m_cap;
		}
		else
		{
			duration = 0.33 * m_config.m_res * m_config.m_cap;
		}
		break;
	}

	return double_to_attotime(duration);
}


//-------------------------------------------------
//  timer_running - is the timer running?
//-------------------------------------------------

int ttl74123_device::timer_running()
{
	return (attotime_compare(timer_timeleft(m_timer), attotime_zero) > 0) &&
		   (attotime_compare(timer_timeleft(m_timer), attotime_never) != 0);
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
	m_config.m_output_changed_cb(this, 0, param);
}


//-------------------------------------------------
//  set_output - set the output line state
//-------------------------------------------------

void ttl74123_device::set_output()
{
	int output = timer_running();

	timer_set( &m_machine, attotime_zero, (void *)this, output, output_callback );

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

	m_config.m_output_changed_cb(this, 0, output);
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
		attotime delay_time = attotime_make(0, ATTOSECONDS_PER_SECOND * m_config.m_cap * 220);

		if(attotime_compare(timer_timeelapsed(m_timer), delay_time) >= 0)
		{
			timer_adjust_oneshot(m_timer, duration, 0);

			if (LOG) logerror("74123 %s:  Retriggering pulse.  Duration: %f\n", tag(), attotime_to_double(duration));
		}
		else
		{
			if (LOG) logerror("74123 %s:  Retriggering failed.\n", tag());
		}
	}
	else
	{
		/* starting */
		timer_adjust_oneshot(m_timer, duration, 0);

		set_output();

		if (LOG) logerror("74123 %s:  Starting pulse.  Duration: %f\n", tag(), attotime_to_double(duration));
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
	else if (!data)	 /* clear the output  */
	{
		timer_adjust_oneshot(m_timer, attotime_zero, 0);

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

const device_type TTL74123 = ttl74123_device_config::static_alloc_device_config;
