// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    EM Microelectronic-Marin (µem) M 3002 Real Time Clock

    TODO:
    - Pulse output (256 Hz, second, minute, hour)
    - Test mode
    - Emulate M 3000 differences (if any meaningful ones exist)

**********************************************************************/

#include "emu.h"
#include "m3002.h"

#define VERBOSE 0
#include "logmacro.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(M3002, m3002_device, "m3002", "EM M 3002 Real Time Clock")
DEFINE_DEVICE_TYPE(M3000, m3000_device, "m3000", "EM M 3000 Real Time Clock")

//**************************************************************************
//  DEVICE CONSTRUCTION AND INITIALIZATION
//**************************************************************************

ALLOW_SAVE_TYPE(m3002_device::mux_state);

//-------------------------------------------------
//  m3002_device - constructor
//-------------------------------------------------

m3002_device::m3002_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, device_rtc_interface(mconfig, *this)
	, m_irq_callback(*this)
	, m_address(0)
	, m_mux_state(mux_state::INIT)
	, m_irq_active(false)
	, m_update_deferred(false)
	, m_second_timer(nullptr)
{
	std::fill_n(&m_ram[0], 0x10, 0);
}

m3002_device::m3002_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m3002_device(mconfig, M3002, tag, owner, clock)
{
}


//-------------------------------------------------
//  m3000_device - constructor
//-------------------------------------------------

m3000_device::m3000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m3002_device(mconfig, M3000, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void m3002_device::device_resolve_objects()
{
	m_irq_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m3002_device::device_start()
{
	// Setup timer
	m_second_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(m3002_device::second_timer), this));

	// Save internal state
	save_item(NAME(m_ram));
	save_item(NAME(m_address));
	save_item(NAME(m_mux_state));
	save_item(NAME(m_irq_active));
	save_item(NAME(m_update_deferred));
}


//-------------------------------------------------
//  device_clock_changed - called when the
//  device clock is altered in any way
//-------------------------------------------------

void m3002_device::device_clock_changed()
{
	attotime second = clocks_to_attotime(32768);
	m_second_timer->adjust(second, 0, second);
}


//-------------------------------------------------
//  nvram_read - read NVRAM from the file
//-------------------------------------------------

bool m3002_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(&m_ram[0], 0x10, actual) && actual == 0x10;
}


//-------------------------------------------------
//  nvram_write - write NVRAM to the file
//-------------------------------------------------

bool m3002_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(&m_ram[0], 0x10, actual) && actual == 0x10;
}


//-------------------------------------------------
//  nvram_default - initialize NVRAM to its
//  default state
//-------------------------------------------------

void m3002_device::nvram_default()
{
	// Set watch, alarm and timer registers to minimum values
	m_ram[0x0] = m_ram[0x8] = m_ram[0xc] = 0x00;
	m_ram[0x1] = m_ram[0x9] = m_ram[0xd] = 0x00;
	m_ram[0x2] = m_ram[0xa] = m_ram[0xe] = 0x00;
	m_ram[0x3] = m_ram[0xb] = 0x01;
	m_ram[0x4] = 0x01;
	m_ram[0x5] = 0x00;
	m_ram[0x6] = 0x01;
	m_ram[0x7] = 0x01;

	// Allow watch to operate
	m_ram[0xf] = 0x01;
}


//-------------------------------------------------
//  rtc_clock_updated - set emulated time
//-------------------------------------------------

void m3002_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	if (!BIT(m_ram[0xf], 0))
		return;

	m_ram[0x0] = convert_to_bcd(second);
	m_ram[0x1] = convert_to_bcd(minute);
	m_ram[0x2] = convert_to_bcd(hour);
	m_ram[0x3] = convert_to_bcd(day);
	m_ram[0x4] = convert_to_bcd(month);
	m_ram[0x5] = convert_to_bcd(year);
	m_ram[0x6] = convert_to_bcd(day_of_week);

	unsigned day_of_year = day - 1;
	for (int n = 1; n < month; n++)
	{
		if (n == 2)
			day_of_year += (year % 4) == 0 ? 29 : 28;
		else
			day_of_year += BIT(0xa50, n) ? 30 : 31;
	}
	m_ram[0x7] = convert_to_bcd((day_of_year + 7 - day_of_week) / 7 + 1);
}


//**************************************************************************
//  INTERNAL UPDATE CYCLES
//**************************************************************************

//-------------------------------------------------
//  internal_busy - return true if in the middle
//  of a once-per-second update
//-------------------------------------------------

bool m3002_device::internal_busy() const
{
	// Each internal update lasts between 0.73 ms and 6 ms
	return attotime_to_clocks(m_second_timer->remaining()) < 239;
}


//-------------------------------------------------
//  bcd_increment - increment the value of one RAM
//  location using BCD sequence
//-------------------------------------------------

void m3002_device::bcd_increment(u8 location)
{
	if ((m_ram[location] & 0x09) != 0x09)
		m_ram[location]++;
	else
	{
		if ((m_ram[location] & 0x90) != 0x90)
			m_ram[location] = (m_ram[location] & 0xf0) + 0x10;
		else
			m_ram[location] = 0x00;
	}
}


//-------------------------------------------------
//  max_date - return the maximum date (BCD) in
//  the current month
//-------------------------------------------------

u8 m3002_device::max_date() const
{
	if (m_ram[4] == 0x02)
		return (m_ram[5] & 0x03) == 0x00 ? 0x29 : 0x28;
	else
		return BIT(0x20250, m_ram[4]) ? 0x30 : 0x31;
}


//-------------------------------------------------
//  watch_update - update watch registers
//-------------------------------------------------

void m3002_device::watch_update()
{
	// Count seconds
	if (m_ram[0x0] != 0x59)
	{
		bcd_increment(0x0);
		return;
	}
	m_ram[0x0] = 0x00;

	// Count minutes
	if (m_ram[0x1] != 0x59)
	{
		bcd_increment(0x1);
		return;
	}
	m_ram[0x1] = 0x00;

	// Count hours
	if (m_ram[0x2] != 0x23)
	{
		bcd_increment(0x2);
		return;
	}
	m_ram[0x2] = 0x00;

	// Count weekdays
	if (m_ram[0x6] != 0x07)
		bcd_increment(0x6);
	else
	{
		m_ram[0x6] = 0x01;

		// Count weeks
		bcd_increment(0x7);
	}

	// Count date
	if (m_ram[0x3] != max_date())
	{
		bcd_increment(0x3);
		return;
	}
	m_ram[0x3] = 0x01;

	// Count months
	if (m_ram[0x4] != 0x12)
	{
		bcd_increment(0x4);
		return;
	}
	m_ram[0x4] = 0x01;
	m_ram[0x7] = 0x01;

	// Count years
	bcd_increment(5);
}


//-------------------------------------------------
//  alarm_update - compare alarm registers against
//  watch registers
//-------------------------------------------------

void m3002_device::alarm_update()
{
	// Compare seconds, minutes, hours, date
	for (int r = 0x0; r <= 0x3; r++)
		if (m_ram[0x8 + r] != 0xff && m_ram[0x8 + r] != m_ram[r])
			return;

	// Alarm IRQ
	m_ram[0xf] |= 0x04;
}


//-------------------------------------------------
//  timer_update - update timer registers
//-------------------------------------------------

void m3002_device::timer_update()
{
	// Count seconds
	if (m_ram[0xc] != 0x59)
	{
		bcd_increment(0xc);
		return;
	}
	m_ram[0xc] = 0x00;

	// Count minutes
	if (m_ram[0xd] != 0x59)
	{
		bcd_increment(0xd);
		return;
	}
	m_ram[0xd] = 0x00;

	// Count hours
	if (m_ram[0xe] != 0x23)
	{
		bcd_increment(0xe);
		return;
	}
	m_ram[0xe] = 0x00;

	// Timer IRQ (23:59:59 -> 00:00:00)
	m_ram[0xf] |= 0x08;
}


//-------------------------------------------------
//  irq_update - update IRQ output
//-------------------------------------------------

void m3002_device::irq_update()
{
	if (!m_irq_active && (m_ram[0xf] & 0x0c) != 0)
	{
		LOG("IRQ occurred\n");
		m_irq_active = true;
		m_irq_callback(0);
	}
	else if (m_irq_active && (m_ram[0xf] & 0x0c) == 0)
	{
		LOG("IRQ cleared\n");
		m_irq_active = false;
		m_irq_callback(1);
	}
}


//-------------------------------------------------
//  update - perform one full internal update
//-------------------------------------------------

void m3002_device::update()
{
	if (BIT(m_ram[0xf], 0))
	{
		watch_update();
		if (BIT(m_ram[0xf], 1))
			alarm_update();
	}

	if (BIT(m_ram[0xf], 4))
		timer_update();

	irq_update();
}


//-------------------------------------------------
//  second_timer - called once each second
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(m3002_device::second_timer)
{
	if (m_update_deferred)
	{
		m_update_deferred = false;
		update();
	}
	else if (m_mux_state != mux_state::INIT)
		m_update_deferred = true;

	if (!m_update_deferred)
		update();
}

//**************************************************************************
//  MICROPROCESSOR ACCESS
//**************************************************************************

const char *const m3002_device::s_register_names[0x10] =
{
	"watch seconds",
	"watch minutes",
	"watch hours",
	"watch date",
	"watch month",
	"watch year",
	"watch weekday",
	"watch week",
	"alarm seconds",
	"alarm minutes",
	"alarm hours",
	"alarm date",
	"timer seconds",
	"timer minutes",
	"timer hours",
	"status/control"
};

//-------------------------------------------------
//  set_init_state - return the multiplexer to the
//  initial state
//-------------------------------------------------

void m3002_device::set_init_state()
{
	m_mux_state = mux_state::INIT;
	if (m_update_deferred)
	{
		m_update_deferred = false;
		update();
	}
	irq_update();
}


//-------------------------------------------------
//  read - read data or busy status using 3-step
//  access sequence
//-------------------------------------------------

u8 m3002_device::read()
{
	u8 data = 0x0;

	switch (m_mux_state)
	{
	case mux_state::INIT:
		if (internal_busy())
			data = 0xf;
		break;

	case mux_state::TENS:
		data = m_ram[m_address] >> 4;
		if (!machine().side_effects_disabled())
			m_mux_state = mux_state::UNITS;
		break;

	case mux_state::UNITS:
		data = m_ram[m_address] & 0x0f;
		if (!machine().side_effects_disabled())
		{
			LOG("%s: Read %02X from %s register\n", machine().describe_context(), m_ram[m_address], s_register_names[m_address]);
			set_init_state();
		}
		break;
	}

	return data;
}


//-------------------------------------------------
//  write - write address and/or data using 3-step
//  access sequence
//-------------------------------------------------

void m3002_device::write(u8 data)
{
	data &= 0xf;

	switch (m_mux_state)
	{
	case mux_state::INIT:
		m_address = data;
		m_mux_state = mux_state::TENS;
		break;

	case mux_state::TENS:
		m_ram[m_address] = (data << 4) | (m_ram[m_address] & 0x0f);
		m_mux_state = mux_state::UNITS;
		break;

	case mux_state::UNITS:
		m_ram[m_address] = (m_ram[m_address] & 0xf0) | data;
		LOG("%s: Writing %02X to %s register\n", machine().describe_context(), m_ram[m_address], s_register_names[m_address]);
		set_init_state();
		break;
	}
}
