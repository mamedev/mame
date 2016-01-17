// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1871 Keyboard Encoder emulation

**********************************************************************/

#include "cdp1871.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CDP1871 = &device_creator<cdp1871_device>;



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

const UINT8 cdp1871_device::key_codes[4][11][8] =
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

cdp1871_device::cdp1871_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CDP1871, "RCA CDP1871", tag, owner, clock, "cdp1871", __FILE__),
	m_read_d1(*this),
	m_read_d2(*this),
	m_read_d3(*this),
	m_read_d4(*this),
	m_read_d5(*this),
	m_read_d6(*this),
	m_read_d7(*this),
	m_read_d8(*this),
	m_read_d9(*this),
	m_read_d10(*this),
	m_read_d11(*this),
	m_write_da(*this),
	m_write_rpt(*this),
	m_inhibit(false),
	m_sense(0),
	m_drive(0),
	m_shift(0),
	m_shift_latch(0),
	m_control(0),
	m_control_latch(0),
	m_alpha(0),
	m_da(0),
	m_next_da(CLEAR_LINE),
	m_rpt(0),
	m_next_rpt(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1871_device::device_start()
{
	// resolve callbacks
	m_read_d1.resolve_safe(0xff);
	m_read_d2.resolve_safe(0xff);
	m_read_d3.resolve_safe(0xff);
	m_read_d4.resolve_safe(0xff);
	m_read_d5.resolve_safe(0xff);
	m_read_d6.resolve_safe(0xff);
	m_read_d7.resolve_safe(0xff);
	m_read_d8.resolve_safe(0xff);
	m_read_d9.resolve_safe(0xff);
	m_read_d10.resolve_safe(0xff);
	m_read_d11.resolve_safe(0xff);
	m_write_da.resolve_safe();
	m_write_rpt.resolve_safe();

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
	save_item(NAME(m_shift_latch));
	save_item(NAME(m_control));
	save_item(NAME(m_control_latch));
	save_item(NAME(m_alpha));
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

		m_write_da(m_da);
	}

	if (m_next_rpt != m_rpt)
	{
		m_rpt = m_next_rpt;

		m_write_rpt(m_rpt);
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

	switch (m_drive) {
	case 0: data = m_read_d1(0); break;
	case 1: data = m_read_d2(0); break;
	case 2: data = m_read_d3(0); break;
	case 3: data = m_read_d4(0); break;
	case 4: data = m_read_d5(0); break;
	case 5: data = m_read_d6(0); break;
	case 6: data = m_read_d7(0); break;
	case 7: data = m_read_d8(0); break;
	case 8: data = m_read_d9(0); break;
	case 9: data = m_read_d10(0); break;
	case 10: data = m_read_d11(0); break;
	}

	if (data == (1 << m_sense))
	{
		if (!m_inhibit)
		{
			m_shift_latch = m_shift;
			m_control_latch = m_control;
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
//  read - keyboard data read
//-------------------------------------------------

READ8_MEMBER( cdp1871_device::read )
{
	int table = 0;

	if (m_control_latch) table = 3; else if (m_shift_latch) table = 2; else if (m_alpha) table = 1;

	// reset DA on next TPB
	m_next_da = CLEAR_LINE;

	return key_codes[table][m_drive][m_sense];
}
