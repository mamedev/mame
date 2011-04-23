/**********************************************************************

    MM74C922/MM74C923 16/20-Key Encoder emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "mm74c922.h"
#include "machine/devhelpr.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 1



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MM74C922 = mm74c922_device_config::static_alloc_device_config;
const device_type MM74C923 = mm74c922_device_config::static_alloc_device_config;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

GENERIC_DEVICE_CONFIG_SETUP(mm74c922, "MM74C922")


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mm74c922_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const mm74c922_interface *intf = reinterpret_cast<const mm74c922_interface *>(static_config());
	if (intf != NULL)
		*static_cast<mm74c922_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_da_func, 0, sizeof(m_out_da_func));
		memset(&m_in_x1_func, 0, sizeof(m_in_x1_func));
		memset(&m_in_x2_func, 0, sizeof(m_in_x2_func));
		memset(&m_in_x3_func, 0, sizeof(m_in_x3_func));
		memset(&m_in_x4_func, 0, sizeof(m_in_x4_func));
		memset(&m_in_x5_func, 0, sizeof(m_in_x5_func));
	}
}


//-------------------------------------------------
//  static_set_config - configuration helper
//-------------------------------------------------

void mm74c922_device_config::static_set_config(device_config *device, int max_y)
{
	mm74c922_device_config *mm74c922 = downcast<mm74c922_device_config *>(device);

	mm74c922->m_max_y = max_y;
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  change_output_lines - 
//-------------------------------------------------

inline void mm74c922_device::change_output_lines()
{
	if (m_next_da != m_da)
	{
		m_da = m_next_da;

		if (LOG) logerror("MM74C922 '%s' Data Available: %u\n", tag(), m_da);

		devcb_call_write_line(&m_out_da_func, m_da);
	}
}


//-------------------------------------------------
//  clock_scan_counters - 
//-------------------------------------------------

inline void mm74c922_device::clock_scan_counters()
{
	if (!m_inhibit)
	{
		m_x++;
		m_x &= 0x03;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

inline void mm74c922_device::detect_keypress()
{
	if (m_inhibit)
	{
		UINT8 data = devcb_call_read8(&m_in_x_func[m_x], 0);

		if (BIT(data, m_y))
		{
			// key released
			m_inhibit = 0;
			m_next_da = 0;
			m_data = 0xff; // high-Z

			if (LOG) logerror("MM74C922 '%s' Key Released\n", tag());
		}
	}
	else
	{
		UINT8 data = devcb_call_read8(&m_in_x_func[m_x], 0);

		for (int y = 0; y < m_config.m_max_y; y++)
		{
			if (!BIT(data, y))
			{
				// key depressed
				m_inhibit = 1;
				m_next_da = 1;
				m_y = y;

				m_data = (y << 2) | m_x;

				if (LOG) logerror("MM74C922 '%s' Key Depressed: X %u Y %u = %02x\n", tag(), m_x, y, m_data);
				return;
			}
		}
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mm74c922_device - constructor
//-------------------------------------------------

mm74c922_device::mm74c922_device(running_machine &_machine, const mm74c922_device_config &config)
    : device_t(_machine, config),
	  m_x(0),
	  m_y(0),
	  m_next_da(0),
      m_config(config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm74c922_device::device_start()
{
	// resolve callbacks
	devcb_resolve_write_line(&m_out_da_func, &m_config.m_out_da_func, this);
	devcb_resolve_read8(&m_in_x_func[0], &m_config.m_in_x1_func, this);
	devcb_resolve_read8(&m_in_x_func[1], &m_config.m_in_x2_func, this);
	devcb_resolve_read8(&m_in_x_func[2], &m_config.m_in_x3_func, this);
	devcb_resolve_read8(&m_in_x_func[3], &m_config.m_in_x4_func, this);
	devcb_resolve_read8(&m_in_x_func[4], &m_config.m_in_x5_func, this);

	// set initial values
	change_output_lines();

	// allocate timers
	m_scan_timer = timer_alloc();
	m_scan_timer->adjust(attotime::zero, 0, attotime::from_hz(50));

	// register for state saving
	save_item(NAME(m_inhibit));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_data));
	save_item(NAME(m_da));
	save_item(NAME(m_next_da));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void mm74c922_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	change_output_lines();
	clock_scan_counters();
	detect_keypress();
}


//-------------------------------------------------
//  data_out_r - 
//-------------------------------------------------

UINT8 mm74c922_device::data_out_r()
{
	if (LOG) logerror("MM74C922 '%s' Data Read: %02x\n", tag(), m_data);

	return m_data;
}
