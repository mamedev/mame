/**********************************************************************

    RCA CDP1871 Keyboard Encoder emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "cdp1871.h"


// device type definition
const device_type CDP1871 = &device_creator<cdp1871_device>;


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

static const UINT8 CDP1871_KEY_CODES[4][11][8] =
{
	// normal
	{
		{ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 },
		{ 0x38, 0x39, 0x3a, 0x3b, 0x2c, 0x2d, 0x2e, 0x2f },
		{ 0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67 },
		{ 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f },
		{ 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77 },
		{ 0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f },
		{ 0x20, 0xff, 0x0a, 0x1b, 0xff, 0x0d, 0xff, 0x7f },
		{ 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 },
		{ 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f },
		{ 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97 },
		{ 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f }
	},

	// alpha
	{
		{ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 },
		{ 0x38, 0x39, 0x3a, 0x3b, 0x2c, 0x2d, 0x2e, 0x2f },
		{ 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 },
		{ 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f },
		{ 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57 },
		{ 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f },
		{ 0x20, 0xff, 0x0a, 0x1b, 0xff, 0x0d, 0xff, 0x7f },
		{ 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 },
		{ 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f },
		{ 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97 },
		{ 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f }
	},

	// shift
	{
		{ 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 },
		{ 0x28, 0x29, 0x2a, 0x2b, 0x3c, 0x3d, 0x3e, 0x3f },
		{ 0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47 },
		{ 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f },
		{ 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57 },
		{ 0x58, 0x59, 0x5a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f },
		{ 0x20, 0xff, 0x0a, 0x1b, 0xff, 0x0d, 0xff, 0x7f },
		{ 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 },
		{ 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f },
		{ 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97 },
		{ 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f }
	},

	// control
	{
		{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
		{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
		{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 },
		{ 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f },
		{ 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 },
		{ 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f },
		{ 0x20, 0xff, 0x0a, 0x1b, 0xff, 0x0d, 0xff, 0x7f },
		{ 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 },
		{ 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f },
		{ 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97 },
		{ 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f }
	}
};



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdp1871_device - constructor
//-------------------------------------------------

cdp1871_device::cdp1871_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CDP1871, "RCA CDP1871", tag, owner, clock),
		m_inhibit(false),
		m_sense(0),
		m_drive(0),
		m_next_da(CLEAR_LINE),
		m_next_rpt(CLEAR_LINE)
{

}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cdp1871_device::device_config_complete()
{
	// inherit a copy of the static data
	const cdp1871_interface *intf = reinterpret_cast<const cdp1871_interface *>(static_config());
	if (intf != NULL)
		*static_cast<cdp1871_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&out_da_cb, 0, sizeof(out_da_cb));
		memset(&out_rpt_cb, 0, sizeof(out_rpt_cb));
		// m_in_d_cb[]
		memset(&in_shift_cb, 0, sizeof(in_shift_cb));
		memset(&in_control_cb, 0, sizeof(in_control_cb));
		memset(&in_alpha_cb, 0, sizeof(in_alpha_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1871_device::device_start()
{
	// resolve callbacks
	m_out_da_func.resolve(out_da_cb, *this);
	m_out_rpt_func.resolve(out_rpt_cb, *this);
	m_in_d_func[0].resolve(in_d1_cb, *this);
	m_in_d_func[1].resolve(in_d2_cb, *this);
	m_in_d_func[2].resolve(in_d3_cb, *this);
	m_in_d_func[3].resolve(in_d4_cb, *this);
	m_in_d_func[4].resolve(in_d5_cb, *this);
	m_in_d_func[5].resolve(in_d6_cb, *this);
	m_in_d_func[6].resolve(in_d7_cb, *this);
	m_in_d_func[7].resolve(in_d8_cb, *this);
	m_in_d_func[8].resolve(in_d9_cb, *this);
	m_in_d_func[9].resolve(in_d10_cb, *this);
	m_in_d_func[10].resolve(in_d11_cb, *this);
	m_in_shift_func.resolve(in_shift_cb, *this);
	m_in_control_func.resolve(in_control_cb, *this);
	m_in_alpha_func.resolve(in_alpha_cb, *this);

	// set initial values
	change_output_lines();

	// allocate timers
	m_scan_timer = timer_alloc();
	m_scan_timer->adjust(attotime::zero, 0, attotime::from_hz(clock()));

	// register for state saving
	save_item(NAME(m_inhibit));
	save_item(NAME(m_sense));
	save_item(NAME(m_drive));
	save_item(NAME(m_shift));
	save_item(NAME(m_control));
	save_item(NAME(m_da));
	save_item(NAME(m_next_da));
	save_item(NAME(m_rpt));
	save_item(NAME(m_next_rpt));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void cdp1871_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	change_output_lines();
	clock_scan_counters();
	detect_keypress();
}


//-------------------------------------------------
//  change_output_lines - change output lines
//-------------------------------------------------

void cdp1871_device::change_output_lines()
{
	if (m_next_da != m_da)
	{
		m_da = m_next_da;

		m_out_da_func(m_da);
	}

	if (m_next_rpt != m_rpt)
	{
		m_rpt = m_next_rpt;

		m_out_rpt_func(m_rpt);
	}
}


//-------------------------------------------------
//  clock_scan_counters - clock the keyboard
//  scan counters
//-------------------------------------------------

void cdp1871_device::clock_scan_counters()
{
	if (!m_inhibit)
	{
		m_sense++;

		if (m_sense == 8)
		{
			m_sense = 0;
			m_drive++;

			if (m_drive == 11)
			{
				m_drive = 0;
			}
		}
	}
}


//-------------------------------------------------
//  detect_keypress - detect key press
//-------------------------------------------------

void cdp1871_device::detect_keypress()
{
	UINT8 data = 0;

	data = m_in_d_func[m_drive](0);

	if (data == (1 << m_sense))
	{
		if (!m_inhibit)
		{
			m_shift = m_in_shift_func();
			m_control = m_in_control_func();
			m_inhibit = true;
			m_next_da = ASSERT_LINE;
		}
		else
		{
			m_next_rpt = ASSERT_LINE;
		}
	}
	else
	{
		m_inhibit = false;
		m_next_rpt = CLEAR_LINE;
	}
}


//-------------------------------------------------
//  data_r - keyboard data read
//-------------------------------------------------

READ8_MEMBER( cdp1871_device::data_r )
{
	int table = 0;
	int alpha = m_in_alpha_func();

	if (m_control) table = 3; else if (m_shift) table = 2; else if (alpha) table = 1;

	// reset DA on next TPB
	m_next_da = CLEAR_LINE;

	return CDP1871_KEY_CODES[table][m_drive][m_sense];
}


//-------------------------------------------------
//  da_r - data available
//-------------------------------------------------

READ_LINE_MEMBER( cdp1871_device::da_r )
{
	return m_da;
}


//-------------------------------------------------
//  rpt_r - keyboard repeat
//-------------------------------------------------

READ_LINE_MEMBER( cdp1871_device::rpt_r )
{
	return m_rpt;
}
