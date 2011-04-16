/**********************************************************************

    RCA CDP1852 Byte-Wide Input/Output Port emulation
	
    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "cdp1852.h"
#include "machine/devhelpr.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	MODE_INPUT = 0,
	MODE_OUTPUT
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CDP1852 = cdp1852_device_config::static_alloc_device_config;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

GENERIC_DEVICE_CONFIG_SETUP(cdp1852, "CDP1852")


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cdp1852_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const cdp1852_interface *intf = reinterpret_cast<const cdp1852_interface *>(static_config());
	if (intf != NULL)
		*static_cast<cdp1852_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_mode_func, 0, sizeof(m_in_mode_func));
		memset(&m_out_sr_func, 0, sizeof(m_out_sr_func));
		memset(&m_in_data_func, 0, sizeof(m_in_data_func));
		memset(&m_out_data_func, 0, sizeof(m_out_data_func));
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  get_mode - 
//-------------------------------------------------

int cdp1852_device::get_mode()
{
	return devcb_call_read_line(&m_in_mode_func);
}


//-------------------------------------------------
//  set_sr_line - 
//-------------------------------------------------

void cdp1852_device::set_sr_line(int state)
{
	if (m_sr != state)
	{
		m_sr = state;

		devcb_call_write_line(&m_out_sr_func, m_sr);
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdp1852_device - constructor
//-------------------------------------------------

cdp1852_device::cdp1852_device(running_machine &_machine, const cdp1852_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1852_device::device_start()
{
	// resolve callbacks
	devcb_resolve_read_line(&m_in_mode_func, &m_config.m_in_mode_func, this);
	devcb_resolve_write_line(&m_out_sr_func, &m_config.m_out_sr_func, this);
	devcb_resolve_read8(&m_in_data_func, &m_config.m_in_data_func, this);
	devcb_resolve_write8(&m_out_data_func, &m_config.m_out_data_func, this);

	// allocate timers
	if (clock() > 0)
	{
		m_scan_timer = timer_alloc();
		m_scan_timer->adjust(attotime::zero, 0, attotime::from_hz(clock()));
	}

	// register for state saving
	save_item(NAME(m_new_data));
	save_item(NAME(m_data));
	save_item(NAME(m_next_data));
	save_item(NAME(m_sr));
	save_item(NAME(m_next_sr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cdp1852_device::device_reset()
{
	// reset data register
	m_data = 0;

	if (get_mode() == MODE_INPUT)
	{
		// reset service request flip-flop
		set_sr_line(1);
	}
	else
	{
		// output data
		devcb_call_write8(&m_out_data_func, 0, m_data);

		// reset service request flip-flop
		set_sr_line(0);
	}
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void cdp1852_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (get_mode())
	{
	case MODE_INPUT:
		// input data into register
		m_data = devcb_call_read8(&m_in_data_func, 0);

		// signal processor
		set_sr_line(0);
		break;

	case MODE_OUTPUT:
		if (m_new_data)
		{
			m_new_data = 0;

			// latch data into register
			m_data = m_next_data;

			// output data
			devcb_call_write8(&m_out_data_func, 0, m_data);

			// signal peripheral device
			set_sr_line(1);

			m_next_sr = 0;
		}
		else
		{
			set_sr_line(m_next_sr);
		}
		break;
	}
}


//-------------------------------------------------
//  read - data read
//-------------------------------------------------

READ8_MEMBER( cdp1852_device::read )
{
	if ((get_mode() == MODE_INPUT) && (clock() == 0))
	{
		// input data into register
		m_data = devcb_call_read8(&m_in_data_func, 0);
	}

	set_sr_line(1);

	return m_data;
}


//-------------------------------------------------
//  write - data write
//-------------------------------------------------

WRITE8_MEMBER( cdp1852_device::write )
{
	if (get_mode() == MODE_OUTPUT)
	{
		m_next_data = data;
		m_new_data = 1;
	}
}
