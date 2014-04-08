/**********************************************************************

    mm58167.c - National Semiconductor MM58167 real-time clock emulation

    TODO: alarms and IRQs not used by the Apple /// and aren't implemented.

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
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mm58167_device::device_reset()
{
	set_current_time(machine());

	m_regs[R_CTL_STATUS] = 0;   // not busy
	m_milliseconds = 0;
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
		advance_seconds();
		m_milliseconds = 0;
	}

	m_regs[R_CNT_MILLISECONDS] = make_bcd(m_milliseconds % 10);
	m_regs[R_CNT_HUNDTENTHS] = make_bcd(m_milliseconds / 10);
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

READ8_MEMBER(mm58167_device::read)
{
//  printf("read reg %x = %02x\n", offset, m_regs[offset]);
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
		// any write to this starts at the current time and zero milliseconds
		case R_CTL_GOCMD:
			m_milliseconds = 0;
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
