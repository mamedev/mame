// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Seiko/Epson S-3520CF

preliminary device by Angelo Salese

TODO:
- kludge on address?
- SRAM hook-ups;
- SRAM load/save;
- system bits;

***************************************************************************/

#include "emu.h"
#include "machine/s3520cf.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type S3520CF = &device_creator<s3520cf_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s3520cf_device - constructor
//-------------------------------------------------

s3520cf_device::s3520cf_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, S3520CF, "S-3520CF RTC", tag, owner, clock, "s3520cf", __FILE__), m_dir(0), m_latch(0), m_reset_line(0), m_read_latch(0), m_current_cmd(0), m_cmd_stream_pos(0), m_rtc_addr(0), m_mode(0), m_sysr(0), m_rtc_state()
{
}

void s3520cf_device::timer_callback()
{
	static const UINT8 dpm[12] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };
	int dpm_count;

	m_rtc.sec++;

	if((m_rtc.sec & 0x0f) >= 0x0a)              { m_rtc.sec+=0x10; m_rtc.sec&=0xf0; }
	if((m_rtc.sec & 0xf0) >= 0x60)              { m_rtc.min++; m_rtc.sec = 0; }
	if((m_rtc.min & 0x0f) >= 0x0a)              { m_rtc.min+=0x10; m_rtc.min&=0xf0; }
	if((m_rtc.min & 0xf0) >= 0x60)              { m_rtc.hour++; m_rtc.min = 0; }
	if((m_rtc.hour & 0x0f) >= 0x0a)             { m_rtc.hour+=0x10; m_rtc.hour&=0xf0; }
	if((m_rtc.hour & 0xff) >= 0x24)             { m_rtc.day++; m_rtc.wday++; m_rtc.hour = 0; }
	if(m_rtc.wday >= 7)                         { m_rtc.wday = 0; }
	if((m_rtc.day & 0x0f) >= 0x0a)              { m_rtc.day+=0x10; m_rtc.day&=0xf0; }

	/* TODO: crude leap year support */
	dpm_count = (m_rtc.month & 0xf) + (((m_rtc.month & 0x10) >> 4)*10)-1;

	if(((m_rtc.year % 4) == 0) && m_rtc.month == 2)
	{
		if((m_rtc.day & 0xff) >= dpm[dpm_count]+1+1)
			{ m_rtc.month++; m_rtc.day = 0x01; }
	}
	else if((m_rtc.day & 0xff) >= dpm[dpm_count]+1){ m_rtc.month++; m_rtc.day = 0x01; }
	if((m_rtc.month & 0x0f) >= 0x0a)            { m_rtc.month = 0x10; }
	if(m_rtc.month >= 0x13)                     { m_rtc.year++; m_rtc.month = 1; }
	if((m_rtc.year & 0x0f) >= 0x0a)             { m_rtc.year+=0x10; m_rtc.year&=0xf0; }
	if((m_rtc.year & 0xf0) >= 0xa0)             { m_rtc.year = 0; } //1901-2000 possible timeframe
}

TIMER_CALLBACK( s3520cf_device::rtc_inc_callback )
{
	reinterpret_cast<s3520cf_device *>(ptr)->timer_callback();
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void s3520cf_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s3520cf_device::device_start()
{
	/* let's call the timer callback every second for now */
	machine().scheduler().timer_pulse(attotime::from_hz(clock() / XTAL_32_768kHz), FUNC(rtc_inc_callback), 0, (void *)this);

	system_time systime;
	machine().base_datetime(systime);

	m_rtc.day = ((systime.local_time.mday / 10)<<4) | ((systime.local_time.mday % 10) & 0xf);
	m_rtc.month = (((systime.local_time.month+1) / 10) << 4) | (((systime.local_time.month+1) % 10) & 0xf);
	m_rtc.wday = systime.local_time.weekday;
	m_rtc.year = (((systime.local_time.year % 100)/10)<<4) | ((systime.local_time.year % 10) & 0xf);
	m_rtc.hour = ((systime.local_time.hour / 10)<<4) | ((systime.local_time.hour % 10) & 0xf);
	m_rtc.min = ((systime.local_time.minute / 10)<<4) | ((systime.local_time.minute % 10) & 0xf);
	m_rtc.sec = ((systime.local_time.second / 10)<<4) | ((systime.local_time.second % 10) & 0xf);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s3520cf_device::device_reset()
{
	m_mode = 0;
}

//-------------------------------------------------
//  rtc_read - used to route RTC reading registers
//-------------------------------------------------

inline UINT8 s3520cf_device::rtc_read(UINT8 offset)
{
	UINT8 res;

	res = 0;

	if(m_mode != 0)
	{
		if(offset == 0xf)
			res = (m_sysr << 3) | m_mode;
		else
		{
			res = 0;
			printf("Warning: S-3520CF RTC reads SRAM %02x %02x\n",offset,m_mode);
		}
	}
	else
	{
		switch(offset)
		{
			case 0x0: res = m_rtc.sec & 0xf; break;
			case 0x1: res = m_rtc.sec >> 4; break;
			case 0x2: res = m_rtc.min & 0xf; break;
			case 0x3: res = m_rtc.min >> 4; break;
			case 0x4: res = m_rtc.hour & 0xf; break;
			case 0x5: res = m_rtc.hour >> 4; break;
			case 0x6: res = m_rtc.wday & 0xf; break;
			case 0x7: res = m_rtc.day & 0xf; break;
			case 0x8: res = m_rtc.day >> 4; break;
			case 0x9: res = m_rtc.month & 0xf; break;
			case 0xa: res = m_rtc.month >> 4; break;
			case 0xb: res = m_rtc.year & 0xf; break;
			case 0xc: res = m_rtc.year >> 4; break;
		}
	}

	return res;
}

inline void s3520cf_device::rtc_write(UINT8 offset,UINT8 data)
{
	if(offset == 0xf)
	{
		m_mode = data & 3;
		m_sysr = (data & 8) >> 3;
		printf("%02x\n",data);
	}
	else
	{
		if(m_mode != 0)
			printf("Warning: S-3520CF RTC writes SRAM %02x %d\n",offset,m_mode);
	}
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ_LINE_MEMBER( s3520cf_device::read_bit )
{
	return m_read_latch;
}

WRITE_LINE_MEMBER( s3520cf_device::set_dir_line )
{
	//printf("%d DIR LINE\n",state);

	m_dir = state;
}

WRITE_LINE_MEMBER( s3520cf_device::set_cs_line )
{
	m_reset_line = state;

	//printf("%d CS LINE\n",state);

	if(m_reset_line != CLEAR_LINE)
	{
		//printf("Reset asserted\n");
		m_current_cmd = 0;
		m_cmd_stream_pos = 0;
		m_rtc_state = RTC_SET_ADDRESS;
		//m_latch = 0;
	}
}

WRITE_LINE_MEMBER( s3520cf_device::write_bit )
{
	m_latch = state;
//  printf("%d LATCH LINE\n",state);
}

WRITE_LINE_MEMBER( s3520cf_device::set_clock_line )
{
	if(state == 1 && m_reset_line == CLEAR_LINE)
	{
		//printf("%d %d\n",m_latch, m_dir);

		switch(m_rtc_state)
		{
			case RTC_SET_ADDRESS:
				m_current_cmd = (m_current_cmd >> 1) | ((m_latch<<3)&8);
				m_cmd_stream_pos++;

				if(m_cmd_stream_pos == 4)
				{
					m_rtc_addr = (m_current_cmd) & 0xf;
					m_rtc_state = RTC_SET_DATA;
					m_cmd_stream_pos = 0;
					m_current_cmd = 0;
				}
				break;
			case RTC_SET_DATA:
				if(m_dir == 1) // READ
				{
					//if(m_cmd_stream_pos == 0)
					{
						//printf("%02x %d\n",m_rtc_addr,m_cmd_stream_pos);
					}
					m_read_latch = (rtc_read((m_rtc_addr+1) & 0xf) >> (m_cmd_stream_pos)) & 1; /* TODO: +1??? */
				}

				m_current_cmd = (m_current_cmd >> 1) | ((m_latch<<3)&8);
				m_cmd_stream_pos++;
				if(m_cmd_stream_pos == 4)
				{
					if(m_dir == 0) // WRITE
					{
						//printf("%02x %02x\n",m_rtc_addr,m_current_cmd);
						rtc_write((m_rtc_addr - 1) & 0xf,m_current_cmd); /* TODO: -1??? */
					}

					m_rtc_addr = m_current_cmd;
					m_rtc_state = RTC_SET_ADDRESS;
					m_cmd_stream_pos = 0;
					m_current_cmd = 0;
				}
				break;
		}
	}
}
