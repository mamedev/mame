// license:BSD-3-Clause
// copyright-holders:Angelo Salese,cam900
/***************************************************************************

    v3021.cpp

    EM Microelectronic-Marin SA Ultra Low Power 32kHz CMOS RTC (DIP8/SO8)

    Serial Real Time Clock

    - Reference: https://www.emmicroelectronic.com/product/real-time-clocks-ic/v3021

    Pin assignment (SO8)

          |-------------------------|
         _|o                        |_
     XI |_|                         |_| Vdd
          |                         |
         _|                         |_
     XO |_|                         |_| WR
          |          V3021          |
         _|                         |_
     CS |_|                         |_| RD
          |                         |
         _|                         |_
    Vss |_|                         |_| I/O
          |-------------------------|

    Pin description

    |-----|------|-----------------------------------------|
    | Pin | Name | Description                             |
    |-----|------|-----------------------------------------|
    1  1  |  XI  | 32 kHz Crystal input                    |
    |-----|------|-----------------------------------------|
    1  2  |  XO  | 32 kHz Crystal output                   |
    |-----|------|-----------------------------------------|
    1  3  |  CS  | Chip select input                       |
    |-----|------|-----------------------------------------|
    1  4  | Vss  | Ground supply                           |
    |-----|------|-----------------------------------------|
    1  5  | I/O  | Data input and output                   |
    |-----|------|-----------------------------------------|
    1  6  |  RD  | Intel RD, Motorola DS (or tie to CS)    |
    |-----|------|-----------------------------------------|
    1  7  |  WR  | Intel WR, Motorola R/W                  |
    |-----|------|-----------------------------------------|
    1  8  | Vdd  | Positive supply                         |
    |-----|------|-----------------------------------------|

    Register map (Unused bits are reserved)

    Address Bits     Description
            76543210
    Data space
    0       ---x---- Time set lock
            ---0---- Enable copy RAM to clock
            ---1---- Disable copy RAM to clock
            ----xx-- Test mode
            ----00-- Normal operation
            ----01-- All time keeping accelerated by 32
            ----10-- Parallel increment of all time data
                     at 1 Hz with no carry over
            ----11-- Parallel increment of all time data
                     at 32 Hz with no carry over
            -------x Frequency measurement mode

    1       (Read only)
            x------- Week number is changed
            -x------ Weekday is changed
            --x----- Year is changed
            ---x---- Month is changed
            ----x--- Day of month is changed
            -----x-- Hours is changed
            ------x- Minutes is changed
            -------x Seconds is changed

    2       -xxxxxxx Seconds (BCD 00-59)
    3       -xxxxxxx Minutes (BCD 00-59)
    4       --xxxxxx Hours (BCD 00-23)
    5       --xxxxxx Day of month (BCD 1-31)
    6       ---xxxxx Month (BCD 01-12)
    7       xxxxxxxx Year (BCD 00-99)
    8       ----xxxx Week day (BCD 01-07)
    9       -xxxxxxx Week number (BCD 00-52)

    Address command space
    e       Copy RAM to clock
    f       Copy clock to RAM

    TODO:
    - verify status bit (RAM 0x00)
    - Support Week number correctly

***************************************************************************/

#include "emu.h"
#include "v3021.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(V3021, v3021_device, "v3021", "EM Microelectronic-Marin SA V3021 RTC")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  v3021_device - constructor
//-------------------------------------------------

v3021_device::v3021_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, V3021, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
{
}

void v3021_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_rtc.year = convert_to_bcd(year);
	m_rtc.month = convert_to_bcd(month);
	m_rtc.day = convert_to_bcd(day);
	m_rtc.wday_bcd = convert_to_bcd(day_of_week);
	m_rtc.wnum = convert_to_bcd((day % 7) + 1); // TODO: placeholder
	m_rtc.hour = convert_to_bcd(hour);
	m_rtc.min = convert_to_bcd(minute);
	m_rtc.sec = convert_to_bcd(second);
}

TIMER_CALLBACK_MEMBER(v3021_device::timer_callback)
{
	advance_seconds();
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void v3021_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void v3021_device::device_start()
{
	/* let's call the timer callback every second */
	m_timer = timer_alloc(FUNC(v3021_device::timer_callback), this);
	m_timer->adjust(attotime::from_hz(clock() / XTAL(32'768)), 0, attotime::from_hz(clock() / XTAL(32'768)));

	copy_clock_to_ram();

	save_item(NAME(m_cs));
	save_item(NAME(m_io));
	save_item(NAME(m_addr));
	save_item(NAME(m_data));
	save_item(NAME(m_ram));
	save_item(NAME(m_cnt));
	save_item(NAME(m_mode));

	save_item(NAME(m_rtc.sec));
	save_item(NAME(m_rtc.min));
	save_item(NAME(m_rtc.hour));
	save_item(NAME(m_rtc.day));
	save_item(NAME(m_rtc.wday));
	save_item(NAME(m_rtc.wday_bcd));
	save_item(NAME(m_rtc.wnum));
	save_item(NAME(m_rtc.month));
	save_item(NAME(m_rtc.year));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void v3021_device::device_reset()
{
	m_cs = false;
	m_io = 0;
	m_addr = 0;
	m_data = 0;
	m_ram[0] = m_ram[1] = 0;
	m_cnt = 0;
	m_mode = 0;
}


//-------------------------------------------------
//  read/write - parallel interface, verified from
//  "typical applications" section from datasheet
//-------------------------------------------------

u8 v3021_device::read()
{
	u8 ret = io_r();
	if (!machine().side_effects_disabled())
	{
		cs_w(1);
		cs_w(0);
	}
	return ret & 1;
}

void v3021_device::write(u8 data)
{
	io_w(BIT(data, 0));
	if (!machine().side_effects_disabled())
	{
		cs_w(1);
		cs_w(0);
	}
}

//-------------------------------------------------
//  cs_w - CS pin handler
//-------------------------------------------------

void v3021_device::cs_w(int state)
{
	if (m_cs != state)
	{
		m_cs = state;
		if (m_cs)
		{
			if (!m_mode) // address
			{
				m_addr = ((m_addr >> 1) | (m_io << 3)) & 0xf;
				if (++m_cnt >= 4)
				{
					if (m_addr == 0xe) // copy RAM to clock
					{
						if (!BIT(m_ram[0x0], 4)) // time set lock
							copy_ram_to_clock();
					}
					else if (m_addr == 0xf) // copy clock to RAM
					{
						copy_clock_to_ram();
					}
					else // normal accessing
					{
						m_data = m_ram[m_addr & 0xf];
						m_mode = 1;
					}
					m_cnt = 0;
				}
			}
			else // data
			{
				m_data = ((m_data >> 1) | (m_io << 7)) & 0xff;
				if (++m_cnt >= 8)
				{
					if (m_addr != 1 && m_addr <= 9)
						m_ram[m_addr & 0xf] = m_data;
					else
						logerror("%s: Writing undocumented register %02X at %02X\n", this->tag(), m_data, m_addr);

					m_mode = 0;
					m_cnt = 0;
				}
			}
		}
	}
}

//-------------------------------------------------
//  io_w - I/O pin write handler
//-------------------------------------------------

void v3021_device::io_w(int state)
{
	m_io = state;
}

//-------------------------------------------------
//  io_r - I/O pin read handler
//-------------------------------------------------

int v3021_device::io_r()
{
	return m_data & 1;
}

//-------------------------------------------------
//  copy_clock_to_ram - copy current clock to RAM
//-------------------------------------------------

void v3021_device::copy_clock_to_ram()
{
	// set status1
	m_ram[0x1] = 0;
	if (m_ram[0x2] != m_rtc.sec)
		m_ram[0x1] |= (1 << 0);
	if (m_ram[0x3] != m_rtc.min)
		m_ram[0x1] |= (1 << 1);
	if (m_ram[0x4] != m_rtc.hour)
		m_ram[0x1] |= (1 << 2);
	if (m_ram[0x5] != m_rtc.day)
		m_ram[0x1] |= (1 << 3);
	if (m_ram[0x6] != m_rtc.month)
		m_ram[0x1] |= (1 << 4);
	if (m_ram[0x7] != m_rtc.year)
		m_ram[0x1] |= (1 << 5);
	if (m_ram[0x8] != m_rtc.wday_bcd)
		m_ram[0x1] |= (1 << 6);
	if (m_ram[0x9] != m_rtc.wnum)
		m_ram[0x1] |= (1 << 7);

	m_ram[0x2] = m_rtc.sec;
	m_ram[0x3] = m_rtc.min;
	m_ram[0x4] = m_rtc.hour;
	m_ram[0x5] = m_rtc.day;
	m_ram[0x6] = m_rtc.month;
	m_ram[0x7] = m_rtc.year;
	m_ram[0x8] = m_rtc.wday_bcd;
	m_ram[0x9] = m_rtc.wnum;
}

//-------------------------------------------------
//  copy_ram_to_clock - copy RAM data to clock
//-------------------------------------------------

void v3021_device::copy_ram_to_clock()
{
	// clear status1
	m_ram[0x1] = 0;
	m_rtc.sec = m_ram[0x2];
	m_rtc.min = m_ram[0x3];
	m_rtc.hour = m_ram[0x4];
	m_rtc.day = m_ram[0x5];
	m_rtc.month = m_ram[0x6];
	m_rtc.year = m_ram[0x7];
	m_rtc.wday_bcd = m_ram[0x8];
	m_rtc.wnum = m_ram[0x9];
	set_time(true, bcd_to_integer(m_rtc.year), bcd_to_integer(m_rtc.month), bcd_to_integer(m_rtc.day),
					bcd_to_integer(m_rtc.wday_bcd), bcd_to_integer(m_rtc.hour), bcd_to_integer(m_rtc.min), bcd_to_integer(m_rtc.sec));
}
