// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    Intersil/Renesas ICM7170 Real Time Clock

*********************************************************************/

#include "emu.h"
#include "icm7170.h"

#include "coreutil.h"

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ICM7170, icm7170_device, "icm7170", "Intersil/Renesas ICM7170 Real Time Clock")

// internal registers
enum
{
	REG_CNT_100TH_SEC = 0,
	REG_CNT_HOURS,
	REG_CNT_MINUTES,
	REG_CNT_SECONDS,
	REG_CNT_MONTH,
	REG_CNT_DAY,
	REG_CNT_YEAR,
	REG_CNT_DAY_OF_WEEK,
	REG_RAM_100TH_SEC,
	REG_RAM_HOURS,
	REG_RAM_MINUTES,
	REG_RAM_SECONDS,
	REG_RAM_MONTH,
	REG_RAM_DAY,
	REG_RAM_YEAR,
	REG_RAM_DAY_OF_WEEK,
	REG_INT_STATUS_AND_MASK,
	REG_COMMAND
};

enum
{
	CMD_REG_TEST_MODE = 0x20,
	CMD_REG_IRQ_ENABLE = 0x10,
	CMD_REG_RUN = 0x08,
	CMD_REG_24_HOUR = 0x04,
	CMD_REG_FREQ_MASK = 0x03
};

enum
{
	IRQ_BIT_GLOBAL = 0x80,
	IRQ_BIT_DAY = 0x40,
	IRQ_BIT_HOUR = 0x20,
	IRQ_BIT_MINUTE = 0x10,
	IRQ_BIT_SECOND = 0x08,
	IRQ_BIT_10TH_SECOND = 0x04,
	IRQ_BIT_100TH_SECOND = 0x02,
	IRQ_BIT_ALARM = 0x01
};

static constexpr int ICM7170_TIMER_ID = 0;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  icm7170_device - constructor
//-------------------------------------------------

icm7170_device::icm7170_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ICM7170, tag, owner, clock),
		device_rtc_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this),
		m_out_irq_cb(*this),
		m_out_irq_state(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void icm7170_device::device_start()
{
	// resolve callbacks
	m_out_irq_cb.resolve_safe();

	m_timer = timer_alloc(ICM7170_TIMER_ID);

	// TODO: frequency should be based on input clock and divisor
	m_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));
	m_timer->enable(false);

	save_item(NAME(m_regs));
	save_item(NAME(m_out_irq_state));
}

//-------------------------------------------------
//  device_reset - device-specific reset handling
//-------------------------------------------------

void icm7170_device::device_reset()
{
	m_regs[REG_COMMAND] &= ~CMD_REG_RUN;
	m_irq_status = 0;
	recalc_irqs();
}

//-------------------------------------------------
//  device_timer - handles timer events
//-------------------------------------------------

void icm7170_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	// advance hundredths
	m_irq_status |= IRQ_BIT_100TH_SECOND;
	if (m_regs[REG_CNT_100TH_SEC]++ == 99)
	{
		m_regs[REG_CNT_100TH_SEC] = 0;

		if (m_regs[REG_COMMAND] & CMD_REG_24_HOUR)
			LOG("device_timer %02d-%02d-%02d %02d:%02d:%02d\n",
				m_regs[REG_CNT_YEAR], m_regs[REG_CNT_MONTH], m_regs[REG_CNT_DAY],
				m_regs[REG_CNT_HOURS], m_regs[REG_CNT_MINUTES], m_regs[REG_CNT_SECONDS]);
		else
			LOG("device_timer %02d-%02d-%02d %02d:%02d:%02d %s\n",
				m_regs[REG_CNT_YEAR], m_regs[REG_CNT_MONTH], m_regs[REG_CNT_DAY],
				m_regs[REG_CNT_HOURS] & 0xf, m_regs[REG_CNT_MINUTES], m_regs[REG_CNT_SECONDS],
				(m_regs[REG_CNT_HOURS] & 0x80) ? "pm" : "am");

		// advance seconds
		m_irq_status |= IRQ_BIT_SECOND;
		if (m_regs[REG_CNT_SECONDS]++ == 59)
		{
			m_regs[REG_CNT_SECONDS] = 0;

			// advance minutes
			m_irq_status |= IRQ_BIT_MINUTE;
			if (m_regs[REG_CNT_MINUTES]++ == 59)
			{
				m_regs[REG_CNT_MINUTES] = 0;

				// invert am/pm bit at 12:00
				if (!(m_regs[REG_COMMAND] & CMD_REG_24_HOUR) && (m_regs[REG_CNT_HOURS] & 0xf) == 11)
					m_regs[REG_CNT_HOURS] ^= 0x80;

				// advance hours
				m_irq_status |= IRQ_BIT_HOUR;
				if (((m_regs[REG_COMMAND] & CMD_REG_24_HOUR) && (m_regs[REG_CNT_HOURS] == 23))
				|| (!(m_regs[REG_COMMAND] & CMD_REG_24_HOUR) && (m_regs[REG_CNT_HOURS] & 0xf) == 12))
				{
					if (m_regs[REG_COMMAND] & CMD_REG_24_HOUR)
						m_regs[REG_CNT_HOURS] = 0;
					else
						m_regs[REG_CNT_HOURS] = (m_regs[REG_CNT_HOURS] & 0x80) | 1;

					// advance days
					m_irq_status |= IRQ_BIT_DAY;
					if (m_regs[REG_CNT_DAY]++ == gregorian_days_in_month(m_regs[REG_CNT_MONTH] & 0xf, (m_regs[REG_CNT_YEAR] & 0x7f) + 2000))
					{
						m_regs[REG_CNT_DAY] = 1;

						// advance months
						if (m_regs[REG_CNT_MONTH]++ == 12)
						{
							m_regs[REG_CNT_MONTH] = 1;

							// advance years
							if (m_regs[REG_CNT_YEAR]++ == 99)
								m_regs[REG_CNT_YEAR] = 0;
						}
					}

					// advance day of week
					if (m_regs[REG_CNT_DAY_OF_WEEK]++ == 6)
						m_regs[REG_CNT_DAY_OF_WEEK] = 0;
				}
				else
					m_regs[REG_CNT_HOURS]++;
			}
		}
	}

	// check tenths interrupt
	if (!(m_regs[REG_CNT_100TH_SEC] % 10))
		m_irq_status |= IRQ_BIT_10TH_SECOND;

	// check alarm
	if (m_irq_mask & IRQ_BIT_ALARM)
		if (BIT(m_regs[REG_RAM_YEAR], 7) || !((m_regs[REG_RAM_YEAR] ^ m_regs[REG_CNT_YEAR]) & 0x7f))
			if (BIT(m_regs[REG_RAM_MONTH], 7) || !((m_regs[REG_RAM_MONTH] ^ m_regs[REG_CNT_MONTH]) & 0xf))
				if (BIT(m_regs[REG_RAM_DAY], 7) || !((m_regs[REG_RAM_DAY] ^ m_regs[REG_CNT_DAY]) & 0x1f))
					if (BIT(m_regs[REG_RAM_HOURS], 6)
					|| ((m_regs[REG_COMMAND] & CMD_REG_24_HOUR) && !((m_regs[REG_RAM_HOURS] ^ m_regs[REG_CNT_HOURS]) & 0x1f))
					|| (!(m_regs[REG_COMMAND] & CMD_REG_24_HOUR) && !((m_regs[REG_RAM_HOURS] ^ m_regs[REG_CNT_HOURS]) & 0x8f)))
						if (BIT(m_regs[REG_RAM_MINUTES], 7) || !((m_regs[REG_RAM_MINUTES] ^ m_regs[REG_CNT_MINUTES]) & 0x3f))
							if (BIT(m_regs[REG_RAM_SECONDS], 7) || !((m_regs[REG_RAM_SECONDS] ^ m_regs[REG_CNT_SECONDS]) & 0x3f))
								if (BIT(m_regs[REG_RAM_100TH_SEC], 7) || !((m_regs[REG_RAM_100TH_SEC] ^ m_regs[REG_CNT_100TH_SEC]) & 0x7f))
									m_irq_status |= IRQ_BIT_ALARM;

	recalc_irqs();
}


//-------------------------------------------------
//  rtc_clock_updated - called when the host time changes
//-------------------------------------------------

void icm7170_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	if (m_regs[REG_COMMAND] & CMD_REG_RUN)
	{
		m_regs[REG_CNT_YEAR] = year % 99;
		m_regs[REG_CNT_MONTH] = month;
		m_regs[REG_CNT_DAY] = day;
		m_regs[REG_CNT_DAY_OF_WEEK] = day_of_week;
		if (!(m_regs[REG_COMMAND] & CMD_REG_24_HOUR))
		{
			// transform hours from 0..23 to 1..12
			m_regs[REG_CNT_HOURS] = (hour > 12) ? hour - 12 : hour ? hour : 1;

			// set am/pm bit
			if (hour > 11)
				m_regs[REG_CNT_HOURS] |= 0x80;
		}
		else
			m_regs[REG_CNT_HOURS] = hour;
		m_regs[REG_CNT_MINUTES] = minute;
		m_regs[REG_CNT_SECONDS] = second;

		if (m_regs[REG_COMMAND] & CMD_REG_24_HOUR)
			LOG("rtc_clock_updated %02d-%02d-%02d %02d:%02d:%02d\n",
				m_regs[REG_CNT_YEAR], m_regs[REG_CNT_MONTH], m_regs[REG_CNT_DAY],
				m_regs[REG_CNT_HOURS], m_regs[REG_CNT_MINUTES], m_regs[REG_CNT_SECONDS]);
		else
			LOG("rtc_clock_updated %02d-%02d-%02d %02d:%02d:%02d %s\n",
				m_regs[REG_CNT_YEAR], m_regs[REG_CNT_MONTH], m_regs[REG_CNT_DAY],
				m_regs[REG_CNT_HOURS] & 0xf, m_regs[REG_CNT_MINUTES], m_regs[REG_CNT_SECONDS],
				(m_regs[REG_CNT_HOURS] & 0x80) ? "pm" : "am");
	}
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void icm7170_device::nvram_default()
{
	memset(m_regs, 0, 0x20);
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool icm7170_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(m_regs, 0x20, actual) && actual == 0x20;
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool icm7170_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(m_regs, 0x20, actual) && actual == 0x20;
}

// non-inherited device functions
uint8_t icm7170_device::read(offs_t offset)
{
	uint8_t data = m_regs[offset & 0x1f];

	if ((offset & 0x1f) == REG_INT_STATUS_AND_MASK)
	{
		data = m_irq_status;
		m_irq_status = 0;
		recalc_irqs();
	}

	LOG("ICM7170 Register %d Read %02x\n", offset, data);

	return data;
}

void icm7170_device::write(offs_t offset, uint8_t data)
{
	// TODO: unused register bits should be masked
	switch (offset & 0x1f)
	{
		case REG_INT_STATUS_AND_MASK:
			m_irq_mask = data;
			LOG("ICM7170 IRQ Mask Write %02x\n", data);
			recalc_irqs();
			break;

		case REG_COMMAND:
			m_timer->enable(data & CMD_REG_RUN);
			[[fallthrough]];

		default:
			m_regs[offset & 0x1f] = data;
			LOG("ICM7170 Register %d Write %02x\n", offset & 0x1f, data);
			break;
	}
}

void icm7170_device::recalc_irqs()
{
	if (m_irq_status & m_irq_mask & ~IRQ_BIT_GLOBAL)
		m_irq_status |= IRQ_BIT_GLOBAL;
	else
		m_irq_status &= ~IRQ_BIT_GLOBAL;

	bool const irq_state = (m_regs[REG_COMMAND] & CMD_REG_IRQ_ENABLE) && (m_irq_status & IRQ_BIT_GLOBAL);
	if (m_out_irq_state != irq_state)
	{
		m_out_irq_state = irq_state;
		m_out_irq_cb(m_out_irq_state);
	}
}

