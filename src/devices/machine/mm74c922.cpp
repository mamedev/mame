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

DEFINE_DEVICE_TYPE(MM74C922, mm74c922_device, "mm74c922", "MM74C922 16-Key Encoder")
DEFINE_DEVICE_TYPE(MM74C923, mm74c923_device, "mm74c923", "MM74C923 20-Key Encoder")




//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mm74c922_device - constructor
//-------------------------------------------------

mm74c922_device::mm74c922_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int max_y) :
	device_t(mconfig, type, tag, owner, clock),
	m_write_da(*this), m_read_x(*this), m_tristate_data(*this),
	m_cap_osc(0), m_cap_debounce(0),
	m_max_y(max_y),
	m_inhibit(false),
	m_x(0),
	m_y(0),
	m_data(0), m_next_data(0),
	m_da(false), m_next_da(false),
	m_scan_timer(nullptr)
{
}

mm74c922_device::mm74c922_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mm74c922_device(mconfig, MM74C922, tag, owner, clock, 4)
{
}

mm74c923_device::mm74c923_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mm74c922_device(mconfig, MM74C923, tag, owner, clock, 5)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm74c922_device::device_start()
{
	// resolve callbacks
	m_write_da.resolve_safe();
	m_read_x.resolve_all_safe((1 << m_max_y) - 1);
	m_tristate_data.resolve_safe((1 << m_max_y) - 1);

	// set initial values
	change_output_lines();

	// allocate timers
	m_scan_timer = timer_alloc();
	m_scan_timer->adjust(attotime::zero, 0, attotime::from_hz(500)); // approximate rate from a 100n capacitor

	// register for state saving
	save_item(NAME(m_inhibit));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_data));
	save_item(NAME(m_next_data));
	save_item(NAME(m_da));
	save_item(NAME(m_next_da));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void mm74c922_device::device_timer(emu_timer &timer, device_timer_id id, int param)
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

		// active high output
		m_write_da(m_da ? 1 : 0);
	}

	// clock data latches
	if (m_next_data != m_data)
	{
		m_data = m_next_data;
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
	assert(m_x >= 0 && m_x < 4);
	uint8_t data = m_read_x[m_x]();

	if (m_inhibit)
	{
		if (BIT(data, m_y))
		{
			// key released
			m_inhibit = false;
			m_next_da = false;
			m_next_data = m_tristate_data(); // high-Z

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
				m_inhibit = true;
				m_next_da = true;
				m_y = y;

				m_next_data = (y << 2) | m_x;

				LOG("MM74C922 Key Depressed: X %u Y %u = %02x\n", m_x, y, m_next_data);
			}
		}
	}
}
