// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Philips PCF8583 Clock and Calendar with 240 x 8-bit RAM

    TODO:
    - Event-counter mode

*********************************************************************/

#include "emu.h"
#include "pcf8583.h"

#define LOG_DATA (1 << 1)
#define LOG_LINE (1 << 2)


#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(PCF8583, pcf8583_device, "pcf8583", "PCF8583 RTC with 240x8 RAM")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pcf8583_device - constructor
//-------------------------------------------------

pcf8583_device::pcf8583_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PCF8583, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, device_nvram_interface(mconfig, *this)
	, m_region(*this, DEVICE_SELF)
	, m_irq_cb(*this)
	, m_slave_address(PCF8583_SLAVE_ADDRESS)
	, m_scl(0)
	, m_sdaw(0)
	, m_sdar(1)
	, m_state(STATE_IDLE)
	, m_bits(0)
	, m_shift(0)
	, m_devsel(0)
	, m_register(0)
	, m_timer(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pcf8583_device::device_start()
{
	m_timer = timer_alloc(TIMER_TICK);
	m_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));

	save_item(NAME(m_scl));
	save_item(NAME(m_sdaw));
	save_item(NAME(m_sdar));
	save_item(NAME(m_state));
	save_item(NAME(m_bits));
	save_item(NAME(m_shift));
	save_item(NAME(m_devsel));
	save_item(NAME(m_register));
	save_item(NAME(m_irq));
	save_item(NAME(m_data));
	save_item(NAME(m_slave_address));

	m_irq_cb.resolve_safe();
}

void pcf8583_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_TICK:
		if (!BIT(m_data[REG_CONTROL], CONTROL_STOP_BIT))
			advance_hundredths();
		break;
	}
}

void pcf8583_device::advance_hundredths()
{
	uint8_t hundredths = bcd_to_integer(m_data[REG_HUNDREDTHS]);
	hundredths++;
	if (hundredths >= 100)
	{
		hundredths = 0;
		advance_seconds();
		m_irq = !m_irq;

		m_irq_cb(m_irq);
	}
	m_data[REG_HUNDREDTHS] = convert_to_bcd(hundredths);
}


void pcf8583_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	set_time_second(second);
	set_time_minute(minute);
	set_time_hour(hour);
	set_date_day(day);
	set_date_month(month);
	set_date_year(year);

	if (BIT(m_data[REG_HOURS], 7)) // 12h format
	{
		// update AM/PM flag
		m_data[REG_HOURS] = (m_data[REG_HOURS] & 0xbf) | (bcd_to_integer(m_data[REG_HOURS] & 0x3f) >= 12 ? 0x40 : 0x00);
		// convert from 24h to 12h
		m_data[REG_HOURS] = (m_data[REG_HOURS] & 0xc0) | convert_to_bcd(bcd_to_integer((m_data[REG_HOURS] & 0x3f) % 12));
	}

	if (BIT(m_data[REG_CONTROL], 2)) // alarm enabled
	{
		switch (m_data[REG_ALARM_CONTROL] & 0x30)
		{
		case 0x00: // no alarm
			break;

		case 0x10: // daily alarm
			if (m_data[REG_HUNDREDTHS] == m_data[REG_ALARM_HUNDREDTHS] && m_data[REG_SECONDS] == m_data[REG_ALARM_SECONDS] &&
				m_data[REG_MINUTES] == m_data[REG_ALARM_MINUTES] && m_data[REG_HOURS] == m_data[REG_ALARM_HOURS])
			{
				m_data[REG_ALARM_CONTROL] |= 0x80;
			}
			break;

		case 0x20: // weekday alarm
			if (BIT(m_data[REG_ALARM_MONTH], m_data[REG_MONTH_DAY] >> 5)) // weekday enabled
			{
				if (m_data[REG_HUNDREDTHS] == m_data[REG_ALARM_HUNDREDTHS] && m_data[REG_SECONDS] == m_data[REG_ALARM_SECONDS] &&
					m_data[REG_MINUTES] == m_data[REG_ALARM_MINUTES] && m_data[REG_HOURS] == m_data[REG_ALARM_HOURS])
				{
					m_data[REG_ALARM_CONTROL] |= 0x80;
				}
			}
			break;

		case 0x30: // dated alarm
			if (m_data[REG_HUNDREDTHS] == m_data[REG_ALARM_HUNDREDTHS] && m_data[REG_SECONDS] == m_data[REG_ALARM_SECONDS] &&
				m_data[REG_MINUTES] == m_data[REG_ALARM_MINUTES] && m_data[REG_HOURS] == m_data[REG_ALARM_HOURS] &&
				(m_data[REG_YEAR_DATE] & 0x3f) == m_data[REG_ALARM_DATE] && (m_data[REG_MONTH_DAY] & 0x1f) == m_data[REG_ALARM_MONTH])
			{
				m_data[REG_ALARM_CONTROL] |= 0x80;
			}
			break;
		}

		// alarm interrupt enable
		m_irq_cb(BIT(m_data[REG_ALARM_CONTROL], 7));
	}
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void pcf8583_device::nvram_default()
{
	// populate from a memory region if present
	if (m_region.found())
	{
		if (m_region->bytes() != 0x100)
		{
			fatalerror("pcf8583 region '%s' wrong size (expected size = 0x100)\n", tag());
		}

		std::copy_n(m_region->base(), m_region->bytes(), &m_data[0]);
	}
	else
	{
		std::fill(std::begin(m_data), std::end(m_data), 0);
	}
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool pcf8583_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(m_data, sizeof(m_data), actual) && actual == sizeof(m_data);
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool pcf8583_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(m_data, sizeof(m_data), actual) && actual == sizeof(m_data);
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_MEMBER(pcf8583_device::a0_w)
{
	state &= 1;
	if (BIT(m_slave_address, 1) != state)
	{
		LOGMASKED(LOG_LINE, "set a0 %d\n", state );
		m_slave_address = (m_slave_address & 0xfd) | (state << 1);
	}
}

WRITE_LINE_MEMBER(pcf8583_device::scl_w)
{
	if (m_scl != state)
	{
		m_scl = state;
		LOGMASKED(LOG_LINE, "set_scl_line %d\n", m_scl);

		switch (m_state)
		{
		case STATE_DEVSEL:
		case STATE_REGISTER:
		case STATE_DATAIN:
			if (m_bits < 8)
			{
				if (m_scl)
				{
					m_shift = ((m_shift << 1) | m_sdaw) & 0xff;
					m_bits++;
				}
			}
			else
			{
				if (m_scl)
				{
					switch (m_state)
					{
					case STATE_DEVSEL:
						m_devsel = m_shift;

						if ((m_devsel & 0xfe) != m_slave_address)
						{
							LOGMASKED(LOG_DATA, "devsel %02x: not this device\n", m_devsel);
							m_state = STATE_IDLE;
						}
						else if ((m_devsel & 1) == 0)
						{
							LOGMASKED(LOG_DATA, "devsel %02x: write\n", m_devsel);
							m_state = STATE_REGISTER;
						}
						else
						{
							LOGMASKED(LOG_DATA, "devsel %02x: read\n", m_devsel);
							m_state = STATE_DATAOUT;
						}
						break;

					case STATE_REGISTER:
						m_register = m_shift;

						LOGMASKED(LOG_DATA, "register %02x\n", m_register);

						m_state = STATE_DATAIN;
						break;

					case STATE_DATAIN:
						LOGMASKED(LOG_DATA, "data[ %02x ] <- %02x\n", m_register, m_shift);

						m_data[m_register] = m_shift;

						switch (m_register)
						{
						case REG_CONTROL:
							if ((m_shift & 0x24) == 0x04)
								logerror("Timer not implemented");
							break;
						case REG_SECONDS:
							set_clock_register(RTC_SECOND, bcd_to_integer(m_data[REG_SECONDS]));
							break;
						case REG_MINUTES:
							set_clock_register(RTC_MINUTE, bcd_to_integer(m_data[REG_MINUTES]));
							break;
						case REG_HOURS:
							set_clock_register(RTC_HOUR, bcd_to_integer(m_data[REG_HOURS]));
							break;
						case REG_YEAR_DATE:
							set_clock_register(RTC_DAY, bcd_to_integer(m_data[REG_YEAR_DATE] & 0x3f));
							set_clock_register(RTC_YEAR, bcd_to_integer(m_data[REG_YEAR_DATE] >> 6));
							break;
						case REG_MONTH_DAY:
							set_clock_register(RTC_MONTH, bcd_to_integer(m_data[REG_MONTH_DAY] & 0x1f));
							set_clock_register(RTC_DAY_OF_WEEK, bcd_to_integer((m_data[REG_MONTH_DAY] >> 5) + 1));
							break;
						case REG_ALARM_CONTROL:
							m_irq_cb(m_data[REG_ALARM_CONTROL] & 0x88 ? 1 : 0);
							break;
						}

						m_register++;
						break;
					}

					m_bits++;
				}
				else
				{
					if (m_bits == 8)
					{
						m_sdar = 0;
					}
					else
					{
						m_bits = 0;
						m_sdar = 1;
					}
				}
			}
			break;

		case STATE_DATAOUT:
			if (m_bits < 8)
			{
				if (m_scl)
				{
					if (m_bits == 0)
					{
						m_shift = m_data[m_register];

						switch (m_register)
						{
						case 0x05:
							if (BIT(m_data[0x00], 3)) // mask flag
								m_shift &= 0x3f;
							break;
						case 0x06:
							if (BIT(m_data[0x00], 3)) // mask flag
								m_shift &= 0x1f;
							break;
						}

						LOGMASKED(LOG_DATA, "data[ %02x ] -> %02x\n", m_register, m_shift);
						m_register++;
					}

					m_sdar = (m_shift >> 7) & 1;

					m_shift = (m_shift << 1) & 0xff;
					m_bits++;
				}
			}
			else
			{
				if (m_scl)
				{
					if (m_sdaw)
					{
						LOGMASKED(LOG_DATA, "nack\n");
						m_state = STATE_IDLE;
					}

					m_bits++;
				}
				else
				{
					if (m_bits == 8)
					{
						m_sdar = 1;
					}
					else
					{
						m_bits = 0;
					}
				}
			}
			break;
		}
	}
}

WRITE_LINE_MEMBER(pcf8583_device::sda_w)
{
	state &= 1;
	if (m_sdaw != state)
	{
		LOGMASKED(LOG_LINE, "set sda %d\n", state);
		m_sdaw = state;

		if (m_scl)
		{
			if (m_sdaw)
			{
				LOGMASKED(LOG_DATA, "stop\n");
				m_state = STATE_IDLE;
			}
			else
			{
				LOGMASKED(LOG_DATA, "start\n");
				m_state = STATE_DEVSEL;
				m_bits = 0;
			}

			m_sdar = 1;
		}
	}
}

READ_LINE_MEMBER(pcf8583_device::sda_r)
{
	int res = m_sdar & 1;

	LOGMASKED(LOG_LINE, "read sda %d\n", res);

	return res;
}
