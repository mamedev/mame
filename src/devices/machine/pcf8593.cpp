// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*********************************************************************

    Philips PCF8593 CMOS clock/calendar circuit

    (c) 2001-2007 Tim Schuerewegen

*********************************************************************/

#include "emu.h"
#include "pcf8593.h"


/***************************************************************************
    PARAMETERS/CONSTANTS/MACROS
***************************************************************************/

#define LOG_LEVEL  1
#define _logerror(level,x)  do { if (LOG_LEVEL > level) logerror x; } while (0)

// get/set date
#define RTC_GET_DATE_YEAR       ((m_data[5] >> 6) & 3)
#define RTC_SET_DATE_YEAR(x)    m_data[5] = (m_data[5] & 0x3F) | (((x) % 4) << 6)
#define RTC_GET_DATE_MONTH      bcd_to_integer( m_data[6])
#define RTC_SET_DATE_MONTH(x)   m_data[6] = convert_to_bcd( x)
#define RTC_GET_DATE_DAY        (bcd_to_integer( m_data[5] & 0x3F))
#define RTC_SET_DATE_DAY(x)     m_data[5] = (m_data[5] & 0xC0) | convert_to_bcd( x)

// get/set time
#define RTC_GET_TIME_HOUR       bcd_to_integer( m_data[4])
#define RTC_SET_TIME_HOUR(x)    m_data[4] = convert_to_bcd( x)
#define RTC_GET_TIME_MINUTE     bcd_to_integer( m_data[3])
#define RTC_SET_TIME_MINUTE(x)  m_data[3] = convert_to_bcd( x)
#define RTC_GET_TIME_SECOND     bcd_to_integer( m_data[2])
#define RTC_SET_TIME_SECOND(x)  m_data[2] = convert_to_bcd( x)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(PCF8593, pcf8593_device, "pcf8593", "PCF8593 RTC")


//-------------------------------------------------
//  pcf8593_device - constructor
//-------------------------------------------------

pcf8593_device::pcf8593_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PCF8593, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, device_nvram_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pcf8593_device::device_start()
{
	_logerror( 0, ("pcf8593_init\n"));
	memset(m_register, 0, sizeof(m_register));
	m_timer = timer_alloc(TIMER_UPDATE_COUNTER);
	m_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pcf8593_device::device_reset()
{
	_logerror( 0, ("pcf8593_reset\n"));
	m_pin_scl = 1;
	m_pin_sda = 1;
	m_active  = false;
	m_inp     = 0;
	m_mode    = RTC_MODE_RECV;
	m_bits    = 0;
	m_pos     = 0;
	clear_buffer_rx();
	set_time(true, RTC_GET_DATE_YEAR, RTC_GET_DATE_MONTH, RTC_GET_DATE_DAY, 0, RTC_GET_TIME_HOUR, RTC_GET_TIME_MINUTE, RTC_GET_TIME_SECOND);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void pcf8593_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch(id)
	{
		case TIMER_UPDATE_COUNTER:
			_logerror( 2, ("pcf8593_timer_callback (%d)\n", param));
			// check if counting is enabled
			if (!(m_data[0] & 0x80))
				advance_seconds();
			break;
	}
}


//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void pcf8593_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	RTC_SET_TIME_SECOND(second);
	RTC_SET_TIME_MINUTE(minute);
	RTC_SET_TIME_HOUR(hour);
	RTC_SET_DATE_DAY(day);
	RTC_SET_DATE_MONTH(month);
	RTC_SET_DATE_YEAR(year);
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void pcf8593_device::nvram_default()
{
	memset(m_data, 0, sizeof(m_data));
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool pcf8593_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(m_data, sizeof(m_data), actual) && actual == sizeof(m_data);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool pcf8593_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(m_data, sizeof(m_data), actual) && actual == sizeof(m_data);
}



/*-------------------------------------------------
    pcf8593_pin_scl
-------------------------------------------------*/

WRITE_LINE_MEMBER(pcf8593_device::scl_w)
{
	// send bit
	if ((m_active) && (!m_pin_scl) && (state))
	{
		switch (m_mode)
		{
			// HOST -> RTC
			case RTC_MODE_RECV :
			{
				// get bit
				if (m_pin_sda) m_data_recv[m_data_recv_index] = m_data_recv[m_data_recv_index] | (0x80 >> m_bits);
				m_bits++;
				// bit 9 = end
				if (m_bits > 8)
				{
					_logerror( 2, ("pcf8593_write_byte(%02X)\n", m_data_recv[m_data_recv_index]));
					// enter receive mode when 1st byte = 0xA3
					if ((m_data_recv[0] == 0xA3) && (m_data_recv_index == 0))
					{
						m_mode = RTC_MODE_SEND;
					}
					// A2 + xx = "read from pos xx" command
					if ((m_data_recv[0] == 0xA2) && (m_data_recv_index == 1))
					{
						m_pos = m_data_recv[1];
					}
					// A2 + xx + .. = write byte
					if ((m_data_recv[0] == 0xA2) && (m_data_recv_index >= 2))
					{
						uint8_t rtc_pos, rtc_val;
						rtc_pos = m_data_recv[1] + (m_data_recv_index - 2);
						rtc_val = m_data_recv[m_data_recv_index];
						//if (rtc_pos == 0) rtc_val = rtc_val & 3; // what is this doing here?
						m_data[rtc_pos] = rtc_val;
						set_time(false, RTC_GET_DATE_YEAR, RTC_GET_DATE_MONTH, RTC_GET_DATE_DAY, 0, RTC_GET_TIME_HOUR, RTC_GET_TIME_MINUTE, RTC_GET_TIME_SECOND);
					}
					// next byte
					m_bits = 0;
					m_data_recv_index++;
				}
			}
			break;
			// RTC -> HOST
			case RTC_MODE_SEND :
			{
				// set bit
				m_inp = (m_data[m_pos] >> (7 - m_bits)) & 1;
				m_bits++;
				// bit 9 = end
				if (m_bits > 8)
				{
					_logerror( 2, ("pcf8593_read_byte(%02X)\n", m_data[m_pos]));
					// end ?
					if (m_pin_sda)
					{
						_logerror( 2, ("pcf8593 end\n"));
						m_mode = RTC_MODE_RECV;
						clear_buffer_rx();
					}
					// next byte
					m_bits = 0;
					m_pos++;
				}
			}
			break;
		}
	}
	// save scl
	m_pin_scl = state;
}



/*-------------------------------------------------
    pcf8593_pin_sda_w
-------------------------------------------------*/

WRITE_LINE_MEMBER(pcf8593_device::sda_w)
{
	// clock is high
	if (m_pin_scl)
	{
		// log init I2C
		if (state) _logerror( 1, ("pcf8593 init i2c\n"));
		// start condition (high to low when clock is high)
		if ((!state) && (m_pin_sda))
		{
			_logerror( 1, ("pcf8593 start condition\n"));
			m_active          = true;
			m_bits            = 0;
			m_data_recv_index = 0;
			clear_buffer_rx();
			//m_pos = 0;
		}
		// stop condition (low to high when clock is high)
		if ((state) && (!m_pin_sda))
		{
			_logerror( 1, ("pcf8593 stop condition\n"));
			m_active = false;
		}
	}
	// save sda
	m_pin_sda = state;
}



/*-------------------------------------------------
    pcf8593_pin_sda_r
-------------------------------------------------*/

READ_LINE_MEMBER(pcf8593_device::sda_r)
{
	return m_inp;
}



/*-------------------------------------------------
    pcf8593_clear_buffer_rx
-------------------------------------------------*/

void pcf8593_device::clear_buffer_rx()
{
	memset(&m_data_recv[0], 0, sizeof( m_data_recv));
	m_data_recv_index = 0;
}
