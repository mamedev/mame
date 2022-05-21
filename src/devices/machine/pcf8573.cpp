// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Philips PCF8573 Clock and Calendar with Power Fail Detector

    TODO:
    - status bits 3/4 seconds/minutes are not set.

*********************************************************************/

#include "emu.h"
#include "pcf8573.h"

#define LOG_DATA (1 << 1)
#define LOG_LINE (1 << 2)

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(PCF8573, pcf8573_device, "pcf8573", "PCF8573 RTC with Power Fail Detector")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pcf8573_device - constructor
//-------------------------------------------------

pcf8573_device::pcf8573_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PCF8573, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, m_comp_cb(*this)
	, m_min_cb(*this)
	, m_sec_cb(*this)
	, m_clock_timer(nullptr)
	, m_slave_address(PCF8573_SLAVE_ADDRESS)
	, m_scl(0)
	, m_sdaw(0)
	, m_sdar(1)
	, m_state(STATE_IDLE)
	, m_bits(0)
	, m_shift(0)
	, m_devsel(0)
	, m_mode_pointer(0)
	, m_address(0)
	, m_status(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pcf8573_device::device_start()
{
	m_comp_cb.resolve_safe();
	m_min_cb.resolve_safe();
	m_sec_cb.resolve_safe();

	// allocate timers
	m_clock_timer = timer_alloc();
	m_clock_timer->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	// state saving
	save_item(NAME(m_scl));
	save_item(NAME(m_sdaw));
	save_item(NAME(m_sdar));
	save_item(NAME(m_state));
	save_item(NAME(m_bits));
	save_item(NAME(m_shift));
	save_item(NAME(m_devsel));
	save_item(NAME(m_mode_pointer));
	save_item(NAME(m_address));
	save_item(NAME(m_status));
	save_item(NAME(m_data));
	save_item(NAME(m_slave_address));
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void pcf8573_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	advance_seconds();

	// one pulse per second output
	m_sec_cb(1);
	m_sec_cb(0);
}

void pcf8573_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	if (m_data[REG_MINUTES] == convert_to_bcd(minute))
		return;

	// update registers
	set_time_minute(minute);
	set_time_hour(hour);
	set_date_day(day);
	set_date_month(month);

	// one pulse per minute output
	m_min_cb(1);
	m_min_cb(0);

	// comparator output
	if (m_data[REG_HOURS] == m_data[REG_ALARM_HOURS] && m_data[REG_MINUTES] == m_data[REG_ALARM_MINUTES])
	{
		if (BIT(m_status, 2) || (m_data[REG_DAYS] == m_data[REG_ALARM_DAYS] && m_data[REG_MONTHS] == m_data[REG_ALARM_MONTHS]))
		{
			// set COMP flag
			m_status |= 1 << 1;
			m_comp_cb(1);
		}
	}
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_MEMBER(pcf8573_device::a0_w)
{
	state &= 1;
	if (BIT(m_slave_address, 1) != state)
	{
		LOGMASKED(LOG_LINE, "set a0 %d\n", state );
		m_slave_address = (m_slave_address & 0xfd) | (state << 1);
	}
}

WRITE_LINE_MEMBER(pcf8573_device::a1_w)
{
	state &= 1;
	if (BIT(m_slave_address, 2) != state)
	{
		LOGMASKED(LOG_LINE, "set a1 %d\n", state );
		m_slave_address = (m_slave_address & 0xfb) | (state << 2);
	}
}

WRITE_LINE_MEMBER(pcf8573_device::scl_w)
{
	if (m_scl != state)
	{
		m_scl = state;
		LOGMASKED(LOG_LINE, "set_scl_line %d\n", m_scl);

		switch (m_state)
		{
		case STATE_ADDRESS:
		case STATE_MODE:
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
					m_bits++;
				}
				else
				{
					if (m_bits == 8)
					{
						switch (m_state)
						{
						case STATE_ADDRESS:
							m_devsel = m_shift;

							if ((m_devsel & 0xfe) != m_slave_address)
							{
								LOGMASKED(LOG_DATA, "address %02x: not this device\n", m_devsel);
								m_state = STATE_IDLE;
							}
							else if ((m_devsel & 1) == 0)
							{
								LOGMASKED(LOG_DATA, "address %02x: write\n", m_devsel);
								m_state = STATE_MODE;
							}
							else
							{
								LOGMASKED(LOG_DATA, "address %02x: read\n", m_devsel);
								m_state = STATE_READSELACK;
							}
							break;

						case STATE_MODE:
							m_mode_pointer = m_shift;

							LOGMASKED(LOG_DATA, "mode pointer %02x\n", m_shift);

							switch (m_mode_pointer & 0x70)
							{
							case 0x00: // execute address
								m_address = m_mode_pointer & 0x07;
								break;
							case 0x10: // read control/status flags
								break;
							case 0x20: // reset prescaler
								set_clock_register(RTC_SECOND, 0);
								adjust_seconds();
								break;
							case 0x30: // time adjust
								adjust_seconds();
								break;
							case 0x40: // reset NODA flag
								m_status &= ~(1 << 2);
								break;
							case 0x50: // set NODA flag
								m_status |= 1 << 2;
								break;
							case 0x60: // reset COMP flag
								m_status &= ~(1 << 1);
								m_comp_cb(0);
								break;
							}

							m_state = STATE_DATAIN;
							break;

						case STATE_DATAIN:
							switch (m_mode_pointer & 0x70)
							{
							case 0x00: // execute address
								m_data[m_address] = m_shift;

								LOGMASKED(LOG_DATA, "data[ %02x ] <- %02x\n", m_address, m_shift);

								switch (m_address)
								{
								case REG_HOURS: set_clock_register(RTC_HOUR, bcd_to_integer(m_data[REG_HOURS])); break;
								case REG_MINUTES: set_clock_register(RTC_MINUTE, bcd_to_integer(m_data[REG_MINUTES])); break;
								case REG_DAYS: set_clock_register(RTC_DAY, bcd_to_integer(m_data[REG_DAYS])); break;
								case REG_MONTHS: set_clock_register(RTC_MONTH, bcd_to_integer(m_data[REG_MONTHS])); break;
								}

								m_address = (m_address & 0x04) | ((m_address + 1) & 0x03);
								break;
							}
							break;
						}

						if( m_state != STATE_IDLE )
						{
							m_sdar = 0 ;
						}
					}
					else
					{
						m_bits = 0;
						m_sdar = 1;
					}
				}
			}
			break;

		case STATE_READSELACK:
			m_bits = 0;
			m_state = STATE_DATAOUT;
			break;

		case STATE_DATAOUT:
			if (m_bits < 8)
			{
				if (m_scl)
				{
					m_bits++;
				}
				else
				{
					if (m_bits == 0)
					{
						switch (m_mode_pointer & 0x70)
						{
						case 0x00: // execute address
							m_shift = m_data[m_address];
							LOGMASKED(LOG_DATA, "data[ %02x ] -> %02x\n", m_address, m_shift);
							m_address = (m_address & 0x04) | ((m_address + 1) & 0x03);
							break;

						case 0x10: // read control/status flags
							m_shift = m_status;
							LOGMASKED(LOG_DATA, "status -> %02x\n", m_address, m_shift);
							break;
						}
					}

					m_sdar = (m_shift >> 7) & 1;

					m_shift = (m_shift << 1) & 0xff;
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

					m_bits = 0;
				}
				else
				{
					m_sdar = 1;
				}
			}
			break;
		}
	}
}

WRITE_LINE_MEMBER(pcf8573_device::sda_w)
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
				m_state = STATE_ADDRESS;
				m_bits = 0;
			}

			m_sdar = 1;
		}
	}
}

READ_LINE_MEMBER(pcf8573_device::sda_r)
{
	int res = m_sdar & 1;

	LOGMASKED(LOG_LINE, "read sda %d\n", res);

	return res;
}
