// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    mm58167.c - National Semiconductor MM58167 real-time clock emulation

    TODO: standby interrupt

**********************************************************************/

#include "mm58167.h"

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type MM58167 = &device_creator<mm58167_device>;

// registers (0-7 are the live data, 8-f are the setting for the compare IRQ)
typedef enum
{
	R_CNT_MILLISECONDS = 0, // 0 = milliseconds
	R_CNT_HUNDTENTHS,       // 1 = hundreds and tenths of seconds
	R_CNT_SECONDS,          // 2 = seconds
	R_CNT_MINUTES,          // 3 = minutes
	R_CNT_HOURS,            // 4 = hours
	R_CNT_DAYOFWEEK,        // 5 = day of the week
	R_CNT_DAYOFMONTH,       // 6 = day of the month
	R_CNT_MONTH,            // 7 = month
	R_RAM_MILLISECONDS,     // 8 = milliseconds
	R_RAM_HUNDTENTHS,       // 9 = hundreds and tenths of seconds
	R_RAM_SECONDS,          // a = seconds
	R_RAM_MINUTES,          // b = minutes
	R_RAM_HOURS,            // c = hours
	R_RAM_DAYOFWEEK,        // d = day of the week
	R_RAM_DAYOFMONTH,       // e = day of the month
	R_RAM_MONTH,            // f = month
	R_CTL_IRQSTATUS,        // 10 = IRQ status (b7 = compare, b6 = 10th sec, b5 = sec, b4 = min, b3 = hour, b2 = day, b1 = week, b0 = month)
	R_CTL_IRQCONTROL,       // 11 = IRQ control (same bit layout as status, but write here to enable/disable/clear)
	R_CTL_RESETCOUNTERS,    // 12 = reset counters
	R_CTL_RESETRAM,         // 13 = reset RAM
	R_CTL_STATUS,           // 14 = status bit
	R_CTL_GOCMD,            // 15 = GO Command
	R_CTL_STANDBYIRQ,       // 16 = standby IRQ
	R_CTL_TESTMODE          // 17 = test mode
} mm58167_regs_t;

//-------------------------------------------------
//  mm58167_device - constructor
//-------------------------------------------------

mm58167_device::mm58167_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MM58167, "National Semiconductor MM58167", tag, owner, clock, "mm58167", __FILE__),
		device_rtc_interface(mconfig, *this),
		m_irq_w(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm58167_device::device_start()
{
	// allocate timers
	m_clock_timer = timer_alloc();
	m_clock_timer->adjust(attotime::from_hz(clock() / 32.768f), 0, attotime::from_hz(clock() / 32.768f));

	m_irq_w.resolve_safe();

	// state saving
	save_item(NAME(m_regs));
	save_item(NAME(m_milliseconds));
	save_item(NAME(m_comparator_state));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mm58167_device::device_reset()
{
	set_current_time(machine());

	m_regs[R_CTL_STATUS] = 0;   // not busy
	m_regs[R_CTL_IRQSTATUS] = 0;
	m_regs[R_CTL_IRQCONTROL] = 0;
	m_milliseconds = 0;
	m_comparator_state = false;
}


INLINE UINT8 make_bcd(UINT8 data)
{
	return ((data / 10) << 4) | (data % 10);
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void mm58167_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_milliseconds++;

	if (m_milliseconds >= 999)
	{
		int old_seconds = m_regs[R_CNT_SECONDS];
		int old_minutes = m_regs[R_CNT_MINUTES];
		int old_hours = m_regs[R_CNT_HOURS];
		int old_dayofmonth = m_regs[R_CNT_DAYOFMONTH];
		int old_dayofweek = m_regs[R_CNT_DAYOFWEEK];
		int old_month = m_regs[R_CNT_MONTH];

		advance_seconds();
		m_milliseconds = 0;

		if ((m_regs[R_CTL_IRQCONTROL] & 0x04) && m_regs[R_CNT_SECONDS]    != old_seconds)       set_irq(2); // every second
		if ((m_regs[R_CTL_IRQCONTROL] & 0x08) && m_regs[R_CNT_MINUTES]    != old_minutes)       set_irq(3); // every minute
		if ((m_regs[R_CTL_IRQCONTROL] & 0x10) && m_regs[R_CNT_HOURS]      != old_hours)         set_irq(4); // every hour
		if ((m_regs[R_CTL_IRQCONTROL] & 0x20) && m_regs[R_CNT_DAYOFMONTH] != old_dayofmonth)    set_irq(5); // every day
		if ((m_regs[R_CTL_IRQCONTROL] & 0x40) && m_regs[R_CNT_DAYOFWEEK]  <  old_dayofweek)     set_irq(6); // every week
		if ((m_regs[R_CTL_IRQCONTROL] & 0x80) && m_regs[R_CNT_MONTH]      != old_month)         set_irq(7); // every month
	}

	m_regs[R_CNT_MILLISECONDS] = make_bcd(m_milliseconds % 10);
	m_regs[R_CNT_HUNDTENTHS] = make_bcd(m_milliseconds / 10);

	// 10Hz IRQ
	if ((m_regs[R_CTL_IRQCONTROL] & 0x02) && (m_milliseconds % 100) == 0)
		set_irq(1);

	// comparator IRQ
	bool new_state = true;
	for (int i = R_CNT_MILLISECONDS; i <= R_CNT_MONTH; i++)
	{
		// nibbles that have the 2 MSB set always compares true
		// Milliseconds use only the high nibble and Day of Week only the low nibble
		if ((i != R_CNT_MILLISECONDS && (m_regs[i + 8] & 0x0c) != 0x0c && (m_regs[i + 8] & 0x0f) != (m_regs[i] & 0x0f)) ||
			(i != R_CNT_DAYOFWEEK    && (m_regs[i + 8] & 0xc0) != 0xc0 && (m_regs[i + 8] & 0xf0) != (m_regs[i] & 0xf0)))
		{
			new_state = false;
			break;
		}
	}

	if ((m_regs[R_CTL_IRQCONTROL] & 0x01) && !m_comparator_state && new_state)  // positive-edge-triggered
		set_irq(0);

	m_comparator_state = new_state;
}


//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void mm58167_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_regs[R_CNT_SECONDS] = make_bcd(second);           // seconds (BCD)
	m_regs[R_CNT_MINUTES] = make_bcd(minute);           // minutes (BCD)
	m_regs[R_CNT_HOURS] = make_bcd(hour);               // hour (BCD)
	m_regs[R_CNT_DAYOFWEEK] = make_bcd(day_of_week);    // day of the week (BCD)
	m_regs[R_CNT_DAYOFMONTH] = make_bcd(day);           // day of the month (BCD)
	m_regs[R_CNT_MONTH] = make_bcd(month);              // month (BCD)
}

void mm58167_device::set_irq(int bit)
{
	m_regs[R_CTL_IRQSTATUS] |= (1 << bit);
	m_irq_w(ASSERT_LINE);
}

void mm58167_device::update_rtc()
{
	set_clock_register(RTC_SECOND, bcd_to_integer(m_regs[R_CNT_SECONDS]));
	set_clock_register(RTC_MINUTE, bcd_to_integer(m_regs[R_CNT_MINUTES]));
	set_clock_register(RTC_HOUR, bcd_to_integer(m_regs[R_CNT_HOURS]));
	set_clock_register(RTC_DAY, bcd_to_integer(m_regs[R_CNT_DAYOFMONTH]));
	set_clock_register(RTC_DAY_OF_WEEK, bcd_to_integer(m_regs[R_CNT_DAYOFWEEK]));
	set_clock_register(RTC_MONTH, bcd_to_integer(m_regs[R_CNT_MONTH]));
	m_milliseconds = (bcd_to_integer(m_regs[R_CNT_HUNDTENTHS]) * 10) + (bcd_to_integer(m_regs[R_CNT_MILLISECONDS] >> 4) % 10);
}

READ8_MEMBER(mm58167_device::read)
{
//  printf("read reg %x = %02x\n", offset, m_regs[offset]);

	if (offset == R_CTL_IRQSTATUS && !space.debugger_access())
	{
		// reading the IRQ status clears IRQ line and IRQ status
		UINT8 data = m_regs[offset];
		m_regs[R_CTL_IRQSTATUS] = 0;
		m_irq_w(CLEAR_LINE);
		return data;
	}

	return m_regs[offset];
}

WRITE8_MEMBER(mm58167_device::write)
{
//  printf("%02x to reg %x\n", data, offset);

	if ((offset >= R_RAM_MILLISECONDS) && (offset != R_CTL_IRQSTATUS))
	{
		m_regs[offset] = data;
	}

	switch (offset)
	{
		case R_CNT_MILLISECONDS:
		case R_CNT_HUNDTENTHS:
		case R_CNT_SECONDS:
		case R_CNT_MINUTES:
		case R_CNT_HOURS:
		case R_CNT_DAYOFWEEK:
		case R_CNT_DAYOFMONTH:
		case R_CNT_MONTH:
			m_regs[offset] = data;
			update_rtc();
			break;

		// any write to this starts at the current time and zero milliseconds
		case R_CTL_GOCMD:
			m_milliseconds = 0;
			break;

		case R_CTL_RESETCOUNTERS:
			if (data == 0xff)
			{
				for (int i = R_CNT_MILLISECONDS; i <= R_CNT_MONTH; i++)
				{
					m_regs[i] = 0;
				}

				update_rtc();
			}
			break;

		case R_CTL_RESETRAM:
			if (data == 0xff)
			{
				for (int i = R_RAM_MILLISECONDS; i < R_CTL_IRQSTATUS; i++)
				{
					m_regs[i] = 0;
				}
			}
			break;

		case R_CTL_IRQCONTROL:
			if (data != 0)
			{
				logerror("MM58167: IRQs not implemented\n");
			}
			break;
	}
}
