// license:BSD-3-Clause
// copyright-holders:Aaron Giles,smf
/***************************************************************************

    timekpr.h

    Various ST Microelectronics timekeeper SRAM implementations:
        - M48T02
        - M48T35
        - M48T37
        - M48T58
        - MK48T08
        - MK48T12

***************************************************************************/

#include "emu.h"
#include "machine/timekpr.h"
#include "machine/timehelp.h"

#define LOG_GENERAL (1U << 0)
#define LOG_TICKS   (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(M48T02,  m48t02_device,  "m48t02",  "M48T02 Timekeeper")
DEFINE_DEVICE_TYPE(M48T35,  m48t35_device,  "m48t35",  "M48T35 Timekeeper")
DEFINE_DEVICE_TYPE(M48T37,  m48t37_device,  "m48t37",  "M48T37 Timekeeper")
DEFINE_DEVICE_TYPE(M48T58,  m48t58_device,  "m48t58",  "M48T58 Timekeeper")
DEFINE_DEVICE_TYPE(MK48T08, mk48t08_device, "mk48t08", "MK48T08 Timekeeper")
DEFINE_DEVICE_TYPE(MK48T12, mk48t12_device, "mk48t12", "MK48T12 Timekeeper")


/***************************************************************************
    MACROS
***************************************************************************/

#define MASK_SECONDS (0x7f)
#define MASK_MINUTES (0x7f)
#define MASK_HOURS (0x3f)
#define MASK_DAY (0x07)
#define MASK_DATE (0x3f)
#define MASK_MONTH (0x1f)
#define MASK_YEAR (0xff)
#define MASK_CENTURY (0xff)

#define CONTROL_W (0x80)
#define CONTROL_R (0x40)
#define CONTROL_S (0x20) /* not emulated */
#define CONTROL_CALIBRATION (0x1f) /* not emulated */

#define SECONDS_ST (0x80)

#define DAY_FT (0x40) /* M48T37 - not emulated */
#define DAY_CEB (0x20) /* M48T35/M48T58 */
#define DAY_CB (0x10) /* M48T35/M48T58 */

#define DATE_BLE (0x80) /* M48T58: not emulated */
#define DATE_BL (0x40) /* M48T58: not emulated */

#define FLAGS_BL (0x10) /* MK48T08/M48T37: not emulated */
#define FLAGS_AF (0x40) /* M48T37: not emulated */
#define FLAGS_WDF (0x80) /* M48T37 */


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

inline void counter_to_ram(u8 *data, s32 offset, u8 counter)
{
	if (offset >= 0)
		data[offset] = counter;
}

inline int counter_from_ram(u8 const *data, s32 offset, u8 unmap = 0)
{
	return (offset >= 0) ? data[offset] : unmap;
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  timekeeper_device_config - constructor
//-------------------------------------------------

timekeeper_device::timekeeper_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 size)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_reset_cb(*this)
	, m_irq_cb(*this)
	, m_default_data(*this, DEVICE_SELF)
	, m_size(size)
{
}

m48t02_device::m48t02_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: timekeeper_device(mconfig, M48T02, tag, owner, clock, 0x800)
{
	m_offset_watchdog = -1;
	m_offset_control = 0x7f8;
	m_offset_seconds = 0x7f9;
	m_offset_minutes = 0x7fa;
	m_offset_hours = 0x7fb;
	m_offset_day = 0x7fc;
	m_offset_date = 0x7fd;
	m_offset_month = 0x7fe;
	m_offset_year = 0x7ff;
	m_offset_century = -1;
	m_offset_flags = -1;
}

m48t35_device::m48t35_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: timekeeper_device(mconfig, M48T35, tag, owner, clock, 0x8000)
{
	m_offset_watchdog = -1;
	m_offset_control = 0x7ff8;
	m_offset_seconds = 0x7ff9;
	m_offset_minutes = 0x7ffa;
	m_offset_hours = 0x7ffb;
	m_offset_day = 0x7ffc;
	m_offset_date = 0x7ffd;
	m_offset_month = 0x7ffe;
	m_offset_year = 0x7fff;
	m_offset_century = -1;
	m_offset_flags = -1;
}

m48t37_device::m48t37_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: timekeeper_device(mconfig, M48T37, tag, owner, clock, 0x8000)
{
	m_offset_watchdog = 0x7ff7;
	m_offset_control = 0x7ff8;
	m_offset_seconds = 0x7ff9;
	m_offset_minutes = 0x7ffa;
	m_offset_hours = 0x7ffb;
	m_offset_day = 0x7ffc;
	m_offset_date = 0x7ffd;
	m_offset_month = 0x7ffe;
	m_offset_year = 0x7fff;
	m_offset_century = 0x7ff1;
	m_offset_flags = 0x7ff0;
}

m48t58_device::m48t58_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: timekeeper_device(mconfig, M48T58, tag, owner, clock, 0x2000)
{
	m_offset_watchdog = -1;
	m_offset_control = 0x1ff8;
	m_offset_seconds = 0x1ff9;
	m_offset_minutes = 0x1ffa;
	m_offset_hours = 0x1ffb;
	m_offset_day = 0x1ffc;
	m_offset_date = 0x1ffd;
	m_offset_month = 0x1ffe;
	m_offset_year = 0x1fff;
	m_offset_century = -1;
	m_offset_flags = -1;
}

mk48t08_device::mk48t08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: timekeeper_device(mconfig, MK48T08, tag, owner, clock, 0x2000)
{
	m_offset_watchdog = -1;
	m_offset_control = 0x1ff8;
	m_offset_seconds = 0x1ff9;
	m_offset_minutes = 0x1ffa;
	m_offset_hours = 0x1ffb;
	m_offset_day = 0x1ffc;
	m_offset_date = 0x1ffd;
	m_offset_month = 0x1ffe;
	m_offset_year = 0x1fff;
	m_offset_century = 0x1ff1;
	m_offset_flags = 0x1ff0;
}

mk48t12_device::mk48t12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: timekeeper_device(mconfig, MK48T12, tag, owner, clock, 0x800)
{
	m_offset_watchdog = -1;
	m_offset_control = 0x7f8;
	m_offset_seconds = 0x7f9;
	m_offset_minutes = 0x7fa;
	m_offset_hours = 0x7fb;
	m_offset_day = 0x7fc;
	m_offset_date = 0x7fd;
	m_offset_month = 0x7fe;
	m_offset_year = 0x7ff;
	m_offset_century = -1;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void timekeeper_device::device_start()
{
	system_time systime;

	machine().base_datetime(systime);

	m_control = 0;
	m_seconds = time_helper::make_bcd(systime.local_time.second);
	m_minutes = time_helper::make_bcd(systime.local_time.minute);
	m_hours = time_helper::make_bcd(systime.local_time.hour);
	m_day = time_helper::make_bcd(systime.local_time.weekday + 1);
	m_date = time_helper::make_bcd(systime.local_time.mday);
	m_month = time_helper::make_bcd(systime.local_time.month + 1);
	m_year = time_helper::make_bcd(systime.local_time.year % 100);
	m_century = time_helper::make_bcd(systime.local_time.year / 100);
	m_data.resize(m_size);

	save_item(NAME(m_control));
	save_item(NAME(m_seconds));
	save_item(NAME(m_minutes));
	save_item(NAME(m_hours));
	save_item(NAME(m_day));
	save_item(NAME(m_date));
	save_item(NAME(m_month));
	save_item(NAME(m_year));
	save_item(NAME(m_century));
	save_item(NAME(m_data));
	save_item(NAME(m_watchdog_delay));

	emu_timer *timer = timer_alloc();
	timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	m_watchdog_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(timekeeper_device::watchdog_callback), this));
	m_watchdog_timer->adjust(attotime::never);
	m_reset_cb.resolve_safe();
	m_irq_cb.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void timekeeper_device::device_reset() { }

void timekeeper_device::counters_to_ram()
{
	counter_to_ram(&m_data[0], m_offset_control, m_control);
	counter_to_ram(&m_data[0], m_offset_seconds, m_seconds);
	counter_to_ram(&m_data[0], m_offset_minutes, m_minutes);
	counter_to_ram(&m_data[0], m_offset_hours, m_hours);
	counter_to_ram(&m_data[0], m_offset_day, m_day);
	counter_to_ram(&m_data[0], m_offset_date, m_date);
	counter_to_ram(&m_data[0], m_offset_month, m_month);
	counter_to_ram(&m_data[0], m_offset_year, m_year);
	counter_to_ram(&m_data[0], m_offset_century, m_century);
}

void timekeeper_device::counters_from_ram()
{
	m_control = counter_from_ram(&m_data[0], m_offset_control);
	m_seconds = counter_from_ram(&m_data[0], m_offset_seconds);
	m_minutes = counter_from_ram(&m_data[0], m_offset_minutes);
	m_hours = counter_from_ram(&m_data[0], m_offset_hours);
	m_day = counter_from_ram(&m_data[0], m_offset_day);
	m_date = counter_from_ram(&m_data[0], m_offset_date);
	m_month = counter_from_ram(&m_data[0], m_offset_month);
	m_year = counter_from_ram(&m_data[0], m_offset_year);
	m_century = counter_from_ram(&m_data[0], m_offset_century);
}

void timekeeper_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	LOGMASKED(LOG_TICKS, "Tick\n");
	if ((m_seconds & SECONDS_ST) != 0 ||
		(m_control & CONTROL_W) != 0)
	{
		logerror("No Tick\n");
		return;
	}

	int carry = time_helper::inc_bcd(&m_seconds, MASK_SECONDS, 0x00, 0x59);
	if (carry)
	{
		carry = time_helper::inc_bcd(&m_minutes, MASK_MINUTES, 0x00, 0x59);
	}
	if (carry)
	{
		carry = time_helper::inc_bcd(&m_hours, MASK_HOURS, 0x00, 0x23);
	}

	if (carry)
	{
		u8 maxdays;
		static const u8 daysinmonth[] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };

		time_helper::inc_bcd(&m_day, MASK_DAY, 0x01, 0x07);

		u8 month = time_helper::from_bcd(m_month);
		u8 year = time_helper::from_bcd(m_year);

		if (month == 2 && (year % 4) == 0)
		{
			maxdays = 0x29;
		}
		else if (month >= 1 && month <= 12)
		{
			maxdays = daysinmonth[month - 1];
		}
		else
		{
			maxdays = 0x31;
		}

		carry = time_helper::inc_bcd(&m_date, MASK_DATE, 0x01, maxdays);
	}
	if (carry)
	{
		carry = time_helper::inc_bcd(&m_month, MASK_MONTH, 0x01, 0x12);
	}
	if (carry)
	{
		carry = time_helper::inc_bcd(&m_year, MASK_YEAR, 0x00, 0x99);
	}
	if (carry)
	{
		carry = time_helper::inc_bcd(&m_century, MASK_CENTURY, 0x00, 0x99);

		if (type() == M48T35 ||
			type() == M48T58)
		{
			if ((m_day & DAY_CEB) != 0)
			{
				m_day ^= DAY_CB;
			}
		}
	}

	if ((m_control & CONTROL_R) == 0)
	{
		counters_to_ram();
	}
}

TIMER_CALLBACK_MEMBER(timekeeper_device::watchdog_callback)
{
	// Set Flag
	m_data[m_offset_flags] |= FLAGS_WDF;
	// WDS (bit 7) selects callback
	if (m_data[m_offset_watchdog] & 0x80)
	{
		// Clear watchdog register
		m_data[m_offset_watchdog] = 0;
		m_reset_cb(ASSERT_LINE);
	}
	else
		m_irq_cb(ASSERT_LINE);

	logerror("watchdog_callback: WD Control: %02x WD Flags: %02x\n", m_data[m_offset_watchdog], m_data[m_offset_flags]);
}

void timekeeper_device::watchdog_write(u8 data)
{
	if ((m_data[m_offset_watchdog] & 0x7f) != 0)
		m_watchdog_timer->adjust(m_watchdog_delay);
}

void timekeeper_device::write(offs_t offset, u8 data)
{
	LOGMASKED(LOG_GENERAL, "timekeeper_device::write: %04x = %02x\n", offset, data);
	if (offset == m_offset_control)
	{
		if ((m_control & CONTROL_W) != 0 &&
			(data & CONTROL_W) == 0)
		{
			counters_from_ram();
		}

		if ((m_control & CONTROL_R) != 0 &&
			(data & CONTROL_W) == 0)
		{
			counters_to_ram();
		}

		m_control = data;
	}
	else if (offset == m_offset_day)
	{
		if (type() == M48T35 ||
			type() == M48T58)
		{
			m_day = (m_day & ~DAY_CEB) | (data & DAY_CEB);
		}
	}
	else if (offset == m_offset_watchdog && type() == M48T37)
	{
		if ((data & 0x7f) == 0)
		{
			m_watchdog_timer->adjust(attotime::never);
		}
		else
		{
			// Calculate the time unit
			m_watchdog_delay = attotime::from_usec(62500 << (2 * (data & 0x3)));
			// Adjust by multiplier
			m_watchdog_delay *= (data >> 2) & 0x1f;
			m_watchdog_timer->adjust(m_watchdog_delay);
			//logerror("write: setting watchdog to %s WatchdogReg = 0x%02x\n", m_watchdog_delay.as_string(), data);
		}
	}

	m_data[offset] = data;
}

u8 timekeeper_device::read(offs_t offset)
{
	u8 result = m_data[offset];
	if (offset == m_offset_date && type() == M48T58)
	{
		result &= ~DATE_BL;
	}
	else if (offset == m_offset_flags && type() == M48T37)
	{
		// Clear the watchdog flag
		m_data[m_offset_flags] &= ~FLAGS_WDF;
		// Clear callbacks
		m_reset_cb(CLEAR_LINE);
		m_irq_cb(CLEAR_LINE);
	}
	LOGMASKED(LOG_GENERAL, "timekeeper_device::read: %04x (%02x)\n", offset, result);
	return result;
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void timekeeper_device::nvram_default()
{
	if (m_default_data.found())
	{
		memcpy(&m_data[0], m_default_data, m_size);
	}
	else
	{
		memset(&m_data[0], 0xff, m_data.size());
	}

	if (m_offset_flags >= 0)
		m_data[m_offset_flags] = 0;
	counters_to_ram();
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool timekeeper_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	if (file.read(&m_data[0], m_size, actual) || actual != m_size)
		return false;

	counters_to_ram();
	return true;
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool timekeeper_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(&m_data[0], m_size, actual) && actual == m_size;
}
