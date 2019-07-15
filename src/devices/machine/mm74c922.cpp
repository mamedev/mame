// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MM74C922/MM74C923 16/20-Key Encoder emulation

**********************************************************************/

#include "emu.h"
#include "mm74c922.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MM74C922, mm74c922_device, "mm74c922", "MM74C923 16/20-Key Encoder")
decltype(MM74C922) MM74C923 = MM74C922;




//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mm74c922_device - constructor
//-------------------------------------------------

mm74c922_device::mm74c922_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MM74C922, tag, owner, clock),
	m_write_da(*this),
	m_read_x1(*this),
	m_read_x2(*this),
	m_read_x3(*this),
	m_read_x4(*this),
	m_read_x5(*this), m_cap_osc(0), m_cap_debounce(0),
	m_max_y(5), // TODO 4 for 74C922, 5 for 74C923
	m_inhibit(0),
	m_x(0),
	m_y(0), m_data(0),
	m_da(0),
	m_next_da(0), m_scan_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm74c922_device::device_start()
{
	// resolve callbacks
	m_write_da.resolve_safe();
	m_read_x1.resolve_safe(0);
	m_read_x2.resolve_safe(0);
	m_read_x3.resolve_safe(0);
	m_read_x4.resolve_safe(0);
	m_read_x5.resolve_safe(0);

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
//  read -
//-------------------------------------------------

uint8_t mm74c922_device::read()
{
	LOG("MM74C922 Data Read: %02x\n", m_data);

	return m_data;
}


//-------------------------------------------------
//  change_output_lines -
//-------------------------------------------------

void mm74c922_device::change_output_lines()
{
	if (m_next_da != m_da)
	{
		m_da = m_next_da;

		LOG("MM74C922 Data Available: %u\n", m_da);

		m_write_da(m_da);
	}
}


//-------------------------------------------------
//  clock_scan_counters -
//-------------------------------------------------

void mm74c922_device::clock_scan_counters()
{
	if (!m_inhibit)
	{
		m_x++;
		m_x &= 0x03;
	}
}


//-------------------------------------------------
//  detect_keypress -
//-------------------------------------------------

void mm74c922_device::detect_keypress()
{
	uint8_t data = 0xff;

	switch (m_x)
	{
	case 0: data = m_read_x1(0); break;
	case 1: data = m_read_x2(0); break;
	case 2: data = m_read_x3(0); break;
	case 3: data = m_read_x4(0); break;
	case 4: data = m_read_x5(0); break;
	}

	if (m_inhibit)
	{
		if (BIT(data, m_y))
		{
			// key released
			m_inhibit = 0;
			m_next_da = 0;
			m_data = 0xff; // high-Z

			LOG("MM74C922 Key Released\n");
		}
	}
	else
	{
		for (int y = 0; y < m_max_y; y++)
		{
			if (!BIT(data, y))
			{
				// key depressed
				m_inhibit = 1;
				m_next_da = 1;
				m_y = y;

				m_data = (y << 2) | m_x;

				LOG("MM74C922 Key Depressed: X %u Y %u = %02x\n", m_x, y, m_data);
				return;
			}
		}
	}
}
