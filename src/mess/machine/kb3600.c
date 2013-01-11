/**********************************************************************

    General Instruments AY-5-3600 Keyboard Encoder emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

/*

    TODO:

    - scan timer clock frequency
    - more accurate emulation of real chip

*/

#include "kb3600.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type AY3600 = &device_creator<ay3600_device>;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ay3600_device::device_config_complete()
{
	// inherit a copy of the static data
	const ay3600_interface *intf = reinterpret_cast<const ay3600_interface *>(static_config());
	if (intf != NULL)
		*static_cast<ay3600_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_shift_cb, 0, sizeof(m_in_shift_cb));
		memset(&m_in_control_cb, 0, sizeof(m_in_control_cb));
		memset(&m_out_data_ready_cb, 0, sizeof(m_out_data_ready_cb));
		memset(&m_out_ako_cb, 0, sizeof(m_out_ako_cb));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ay3600_device - constructor
//-------------------------------------------------

ay3600_device::ay3600_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AY3600, "AY-5-3600", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ay3600_device::device_start()
{
	// resolve callbacks
	m_in_x_func[0].resolve(m_in_x0_cb, *this);
	m_in_x_func[1].resolve(m_in_x1_cb, *this);
	m_in_x_func[2].resolve(m_in_x2_cb, *this);
	m_in_x_func[3].resolve(m_in_x3_cb, *this);
	m_in_x_func[4].resolve(m_in_x4_cb, *this);
	m_in_x_func[5].resolve(m_in_x5_cb, *this);
	m_in_x_func[6].resolve(m_in_x6_cb, *this);
	m_in_x_func[7].resolve(m_in_x7_cb, *this);
	m_in_x_func[8].resolve(m_in_x8_cb, *this);
	m_in_shift_func.resolve(m_in_shift_cb, *this);
	m_in_control_func.resolve(m_in_control_cb, *this);
	m_out_data_ready_func.resolve(m_out_data_ready_cb, *this);
	m_out_ako_func.resolve(m_out_ako_cb, *this);

	// allocate timers
	m_scan_timer = timer_alloc();
	m_scan_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));

	// state saving
	save_item(NAME(m_b));
	save_item(NAME(m_ako));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void ay3600_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int ako = 0;

	for (int x = 0; x < 9; x++)
	{
		UINT16 data = m_in_x_func[x](0,0xffff);

		for (int y = 0; y < 10; y++)
		{
			if (BIT(data, y))
			{
				int b = (x * 10) + y;

				ako = 1;

				if (b > 63)
				{
					b -= 64;
					b = 0x100 | b;
				}

				b |= (m_in_shift_func() << 6);
				b |= (m_in_control_func() << 7);

				if (m_b != b)
				{
					m_b = b;

					m_out_data_ready_func(1);
					return;
				}
			}
		}
	}

	if (!ako)
	{
		m_b = -1;
	}

	if (ako != m_ako)
	{
		m_out_ako_func(ako);
		m_ako = ako;
	}
}


//-------------------------------------------------
//  b_r -
//-------------------------------------------------

UINT16 ay3600_device::b_r()
{
	UINT16 data = m_b;

	m_out_data_ready_func(0);

	return data;
}
