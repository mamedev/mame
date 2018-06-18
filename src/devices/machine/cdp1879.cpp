// license:BSD-3-Clause
// copyright-holders:R. Belmont,Carl
/**********************************************************************

    cdp1879.c - RCA CDP1879 real-time clock emulation

**********************************************************************/

#include "emu.h"
#include "cdp1879.h"
#include "machine/timehelp.h"

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(CDP1879, cdp1879_device, "cdp1879", "RCA CDP1879 RTC")

//-------------------------------------------------
//  cdp1879_device - constructor
//-------------------------------------------------

cdp1879_device::cdp1879_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CDP1879, tag, owner, clock),
		device_rtc_interface(mconfig, *this),
		m_irq_w(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1879_device::device_start()
{
	// allocate timers
	m_clock_timer = timer_alloc();
	m_clock_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	m_irq_w.resolve_safe();

	// state saving
	save_item(NAME(m_regs));
	save_item(NAME(m_comparator_state));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cdp1879_device::device_reset()
{
	m_regs[0] = m_regs[1] = 0;
	m_regs[R_CTL_IRQSTATUS] = 0;
	m_regs[R_CTL_CONTROL] = 0;
	m_comparator_state = false;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void cdp1879_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	advance_seconds();

	// comparator IRQ
	bool new_state = true;
	for (int i = R_CNT_SECONDS; i <= R_CNT_HOURS; i++)
	{
		if(m_regs[i] != m_regs[i + 6])
		{
			new_state = false;
			break;
		}
	}

	if (!m_comparator_state && new_state)  // positive-edge-triggered
		set_irq(7);

	m_comparator_state = new_state;
}


//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void cdp1879_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_regs[R_CNT_SECONDS] = time_helper::make_bcd(second);           // seconds (BCD)
	m_regs[R_CNT_MINUTES] = time_helper::make_bcd(minute);           // minutes (BCD)
	m_regs[R_CNT_HOURS] = time_helper::make_bcd(hour);               // hour (BCD)
	m_regs[R_CNT_DAYOFMONTH] = time_helper::make_bcd(day);           // day of the month (BCD)
	m_regs[R_CNT_MONTH] = time_helper::make_bcd(month);              // month (BCD)
}

void cdp1879_device::set_irq(int bit)
{
	m_regs[R_CTL_IRQSTATUS] |= (1 << bit);
	m_irq_w(ASSERT_LINE);
}

void cdp1879_device::update_rtc()
{
	set_clock_register(RTC_SECOND, bcd_to_integer(m_regs[R_CNT_SECONDS]));
	set_clock_register(RTC_MINUTE, bcd_to_integer(m_regs[R_CNT_MINUTES]));
	set_clock_register(RTC_HOUR, bcd_to_integer(m_regs[R_CNT_HOURS]));
	set_clock_register(RTC_DAY, bcd_to_integer(m_regs[R_CNT_DAYOFMONTH]));
	set_clock_register(RTC_MONTH, bcd_to_integer(m_regs[R_CNT_MONTH]));
}

READ8_MEMBER(cdp1879_device::read)
{
	if (offset == R_CTL_IRQSTATUS && !machine().side_effects_disabled())
	{
		// reading the IRQ status clears IRQ line and IRQ status
		uint8_t data = m_regs[offset];
		m_regs[R_CTL_IRQSTATUS] = 0;
		m_irq_w(CLEAR_LINE);
		return data;
	}

	return m_regs[offset];
}

WRITE8_MEMBER(cdp1879_device::write)
{
	switch (offset)
	{
		case R_CNT_SECONDS:
		case R_CNT_MINUTES:
		case R_CNT_HOURS:
			if(BIT(m_regs[R_CTL_CONTROL], 3))
			{
				m_regs[offset + 6] = data;
				break;
			}
		case R_CNT_DAYOFMONTH:
		case R_CNT_MONTH:
			m_regs[offset] = data;
			update_rtc();
			break;

		case R_CTL_CONTROL:
			m_regs[offset] = data;
			break;
	}
}
