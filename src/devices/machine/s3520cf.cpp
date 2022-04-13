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
DEFINE_DEVICE_TYPE(S3520CF, s3520cf_device, "s3520cf", "Seiko Epson S-3520CF RTC")
DEFINE_DEVICE_TYPE(RTC4553, rtc4553_device, "rtc4553", "Epson RTC-4553 RTC/SRAM") // functionally same as above but integrated oscillator


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s3520cf_device - constructor
//-------------------------------------------------

s3520cf_device::s3520cf_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: s3520cf_device(mconfig, S3520CF, tag, owner, clock)
{
}

s3520cf_device::s3520cf_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_region(*this, DEVICE_SELF)
	, m_dir(0), m_latch(0), m_reset_line(0), m_read_latch(0), m_bitstream(0), m_stream_pos(0), m_mode(0), m_sysr(0), m_cntrl1(0), m_cntrl2(0)
{
}

void s3520cf_device::check_overflow()
{
	static constexpr u8 dpm[12] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };
	int dpm_count;

	if ((m_rtc.sec & 0x0f) >= 0x0a) { m_rtc.sec += 0x10; m_rtc.sec &= 0xf0; }
	if ((m_rtc.sec & 0xf0) >= 0x60) { m_rtc.min++; m_rtc.sec = 0; }
	if ((m_rtc.min & 0x0f) >= 0x0a) { m_rtc.min += 0x10; m_rtc.min &= 0xf0; }
	if ((m_rtc.min & 0xf0) >= 0x60) { m_rtc.hour++; m_rtc.min = 0; }
	if ((m_rtc.hour & 0x0f) >= 0x0a) { m_rtc.hour += 0x10; m_rtc.hour &= 0xf0; }
	if ((m_rtc.hour & 0xff) >= 0x24) { m_rtc.day++; m_rtc.wday++; m_rtc.hour = 0; }
	if (m_rtc.wday >= 7) { m_rtc.wday = 0; }
	if ((m_rtc.day & 0x0f) >= 0x0a) { m_rtc.day += 0x10; m_rtc.day &= 0xf0; }

	/* TODO: crude leap year support */
	dpm_count = (m_rtc.month & 0xf) + (((m_rtc.month & 0x10) >> 4) * 10) - 1;

	if (((m_rtc.year % 4) == 0) && m_rtc.month == 2)
	{
		if ((m_rtc.day & 0xff) >= dpm[dpm_count] + 1 + 1)
		{
			m_rtc.month++; m_rtc.day = 0x01;
		}
	}
	else if ((m_rtc.day & 0xff) >= dpm[dpm_count] + 1) { m_rtc.month++; m_rtc.day = 0x01; }
	if ((m_rtc.month & 0x0f) >= 0x0a) { m_rtc.month = 0x10; }
	if (m_rtc.month >= 0x13) { m_rtc.year++; m_rtc.month = 1; }
	if ((m_rtc.year & 0x0f) >= 0x0a) { m_rtc.year += 0x10; m_rtc.year &= 0xf0; }
	if ((m_rtc.year & 0xf0) >= 0xa0) { m_rtc.year = 0; } //1901-2000 possible timeframe
}


TIMER_CALLBACK_MEMBER(s3520cf_device::timer_callback)
{
	m_rtc.sec++;
	check_overflow();
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
	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(s3520cf_device::timer_callback), this));
	m_timer->adjust(attotime::from_hz(clock() / XTAL(32'768)), 0, attotime::from_hz(clock() / XTAL(32'768)));

	system_time systime;
	machine().base_datetime(systime);

	m_rtc.day = ((systime.local_time.mday / 10)<<4) | ((systime.local_time.mday % 10) & 0xf);
	m_rtc.month = (((systime.local_time.month+1) / 10) << 4) | (((systime.local_time.month+1) % 10) & 0xf);
	m_rtc.wday = systime.local_time.weekday;
	m_rtc.year = (((systime.local_time.year % 100)/10)<<4) | ((systime.local_time.year % 10) & 0xf);
	m_rtc.hour = ((systime.local_time.hour / 10)<<4) | ((systime.local_time.hour % 10) & 0xf);
	m_rtc.min = ((systime.local_time.minute / 10)<<4) | ((systime.local_time.minute % 10) & 0xf);
	m_rtc.sec = ((systime.local_time.second / 10)<<4) | ((systime.local_time.second % 10) & 0xf);

	save_item(NAME(m_dir));
	save_item(NAME(m_latch));
	save_item(NAME(m_reset_line));
	save_item(NAME(m_read_latch));
	save_item(NAME(m_bitstream));
	save_item(NAME(m_stream_pos));
	save_item(NAME(m_mode));
	save_item(NAME(m_sysr));
	save_item(NAME(m_cntrl1));
	save_item(NAME(m_cntrl2));
	save_item(NAME(m_rtc.sec));
	save_item(NAME(m_rtc.min));
	save_item(NAME(m_rtc.hour));
	save_item(NAME(m_rtc.day));
	save_item(NAME(m_rtc.wday));
	save_item(NAME(m_rtc.month));
	save_item(NAME(m_rtc.year));
	save_item(NAME(m_nvdata));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s3520cf_device::device_reset()
{
	m_mode = 0;
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void s3520cf_device::nvram_default()
{
	for (auto & elem : m_nvdata)
		elem = 0x00;

	if (!m_region.found())
		logerror("s3520cf(%s) region not found\n", tag());
	else if (m_region->bytes() != 15)
		logerror("s3520cf(%s) region length 0x%x expected 0x%x\n", tag(), m_region->bytes(), 15);
	else
		memcpy(m_nvdata, m_region->base(), 15);
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool s3520cf_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(m_nvdata, 15, actual) && actual == 15;
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool s3520cf_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(m_nvdata, 15, actual) && actual == 15;
}

//-------------------------------------------------
//  rtc_read - used to route RTC reading registers
//-------------------------------------------------

inline u8 s3520cf_device::rtc_read(u8 offset)
{
	u8 res = 0;

	if (offset == 0xf)
		res = (m_sysr << 3) | m_mode;
	else
		if (m_mode > 1)
		{
			if (m_mode > 2)
				offset += 15;
			res = (m_nvdata[offset / 2] >> ((offset & 1) * 4)) & 0xf;
		}
		else
		{
			switch (offset)
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
			case 0xd: res = m_cntrl1; break;
			case 0xe: res = m_cntrl2; break;
			}
		}

	return res;
}

inline void s3520cf_device::rtc_write(u8 offset,u8 data)
{
	if(offset == 0xf)
	{
		m_mode = data & 3;
		m_sysr = (data & 8) >> 3;
		if (m_sysr)
		{
			m_rtc.wday = m_rtc.hour = m_rtc.min = m_rtc.sec = 0;
			m_rtc.year = m_rtc.month = m_rtc.day = 1;
		}
	}
	else
	{
		if (m_mode > 1)
		{
			if (m_mode > 2)
				offset += 15;
			if (offset & 1)
				m_nvdata[offset / 2] = (m_nvdata[offset / 2] & 0xf) | (data << 4);
			else
				m_nvdata[offset / 2] = (m_nvdata[offset / 2] & 0xf0) | (data & 0xf);
		}
		else
			switch (offset)
			{
			case 0x0: m_rtc.sec = (m_cntrl1 & 2) ? 0 : m_rtc.sec + 1; check_overflow(); break;
			case 0x1: m_rtc.sec = (m_cntrl1 & 2) ? 0 : m_rtc.sec + 0x10; check_overflow(); break;
			case 0x2: m_rtc.min = (m_cntrl1 & 2) ? 0 : m_rtc.min + 1; check_overflow(); break;
			case 0x3: m_rtc.min = (m_cntrl1 & 2) ? 0 : m_rtc.min + 0x10; check_overflow(); break;
			case 0x4: m_rtc.hour = (m_cntrl1 & 2) ? 0 : m_rtc.hour + 1; check_overflow(); break;
			case 0x5: m_rtc.hour = (m_cntrl1 & 2) ? 0 : m_rtc.hour; check_overflow(); break;
			case 0x6: m_rtc.wday = (m_cntrl1 & 2) ? 0 : m_rtc.wday + 1; check_overflow(); break;
			case 0x7: m_rtc.day = (m_cntrl1 & 2) ? 1 : m_rtc.day + 1; check_overflow(); break;
			case 0x8: m_rtc.day = (m_cntrl1 & 2) ? 1 : m_rtc.day + 0x10; check_overflow(); break;
			case 0x9: m_rtc.month = (m_cntrl1 & 2) ? 1: m_rtc.month + 1; check_overflow(); break;
			case 0xa: m_rtc.month = (m_cntrl1 & 2) ? 1 : m_rtc.month + 0x10; check_overflow(); break;
			case 0xb: m_rtc.year = (m_cntrl1 & 2) ? m_rtc.year & 0xf0 : m_rtc.year + 1; check_overflow(); break;
			case 0xc: m_rtc.year = (m_cntrl1 & 2) ? m_rtc.year & 0x0f : m_rtc.year + 0x10; check_overflow(); break;
			case 0xd: m_cntrl1 = data & 0xf; break;
			case 0xe: m_cntrl2 = data & 0xf; break;
			}
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
		m_stream_pos = 0;
		//m_latch = 0; // should be high impedance
	}
}

WRITE_LINE_MEMBER( s3520cf_device::write_bit )
{
	m_latch = state;
//  printf("%d LATCH LINE\n",state);
}

WRITE_LINE_MEMBER( s3520cf_device::set_clock_line )
{
	// NOTE: this device use 1-cycle (8 clocks) delayed data output
	if(state == 1 && m_reset_line == CLEAR_LINE)
	{
		//printf("%d %d\n",m_latch, m_dir);
		m_read_latch = m_bitstream & 1;
		m_bitstream = (m_bitstream >> 1) | ((m_latch & 1) << 7);
		m_stream_pos = (m_stream_pos + 1) & 7;

		if (m_stream_pos == 0)
		{
			u8 addr = m_bitstream & 0xf;
			if (m_dir == 0) // Write
				rtc_write(addr, m_bitstream >> 4);
			// Read/Verify
			m_bitstream = addr | (rtc_read(addr) << 4);
		}
	}
}

rtc4553_device::rtc4553_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: s3520cf_device(mconfig, RTC4553, tag, owner, clock)
{
}
