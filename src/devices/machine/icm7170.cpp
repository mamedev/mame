// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    Intersil/Renesas ICM7170 Real Time Clock

*********************************************************************/

#include "emu.h"
#include "icm7170.h"
#include "coreutil.h"

#define VERBOSE (1)
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
		m_out_irq_cb(*this)
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
	m_timer->adjust(attotime::never);

	save_item(NAME(m_regs));
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

void icm7170_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
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
		m_regs[REG_CNT_HOURS] = hour;
		m_regs[REG_CNT_MINUTES] = minute;
		m_regs[REG_CNT_SECONDS] = second;
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

void icm7170_device::nvram_read(emu_file &file)
{
	file.read(m_regs, 0x20);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void icm7170_device::nvram_write(emu_file &file)
{
	file.write(m_regs, 0x20);
}

// non-inherited device functions
uint8_t icm7170_device::read(offs_t offset)
{
	uint8_t data =  m_regs[offset & 0x1f];

	if ((offset & 0x1f) == REG_INT_STATUS_AND_MASK)
	{
		data = m_irq_status;
	}

	LOG("ICM7170 Register %d Read %02x\n", offset, data);

	return data;
}

void icm7170_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x1f)
	{
		case REG_INT_STATUS_AND_MASK:
			m_irq_mask = data;
			LOG("ICM7170 IRQ Mask Write %02x\n", data);
			recalc_irqs();
			break;

		default:
			m_regs[offset & 0x1f] = data;
			LOG("ICM7170 Register %d Write %02x\n", offset & 0x1f, data);
			break;
	}
}

void icm7170_device::recalc_irqs()
{
}

