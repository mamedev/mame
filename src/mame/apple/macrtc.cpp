// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    macrtc.cpp
    Real-time clock & NVRAM chips used in early 680x0 Macs and the Apple IIgs.
    Apple part numbers 343-0040 (original, 20 bytes of PRAM) and 343-0042-B (256 bytes of PRAM).
    By R. Belmont, based on previous work by Nathan Woods and Raphael Nabet

    Commands and data are sent and received serially, bit 7 first.
    For reading the chip, the data is valid after the falling edge of the clock.
    For writing, the data must be valid before the falling edge of the clock.

    The time is the number of seconds since midnight on January 1, 1904.

    Commands:
    R/W 00x0001 - Seconds (least significant byte)
    R/W 00x0101 - Seconds (2nd byte)
    R/W 00x1001 - Seconds (3rd byte)
    R/W 00x1101 - Seconds (most significant byte)
    0   0110001 - Test register
    0   0110101 - Write protect bit (343-0040) (When set, only the WP bit itself can be changed)
    0   01101xx - Write protect bit (343-0042-B)
    R/W 010aa01 - 4 PRAM addresses (aa is the address)
    R/W 1aaaa01 - 16 PRAM addresses (aaaa is the address)
    R/W 0111aaa - Extended PRAM address (aaa is the sector number. Sectors are 32 bytes)

***************************************************************************/

#include "emu.h"
#include "macrtc.h"

#define LOG_COMMANDS (1U << 1)

//#define VERBOSE (LOG_COMMANDS)
#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

enum
{
	RTC_STATE_NORMAL = 0,
	RTC_STATE_WRITE,
	RTC_STATE_XPCOMMAND,
	RTC_STATE_XPWRITE
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(RTC3430040, rtc3430040_device, "rtc3430040", "Apple 343-0040 clock/PRAM")
DEFINE_DEVICE_TYPE(RTC3430042, rtc3430042_device, "rtc3430042", "Apple 343-0042-B clock/PRAM")


//-------------------------------------------------
//  rtc4543_device - constructor
//-------------------------------------------------

rtc3430042_device::rtc3430042_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool hasBigPRAM) :
	device_t(mconfig, type, tag, owner, clock),
	device_rtc_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_is_big_PRAM(hasBigPRAM),
	m_time_was_set(false),
	m_cko_cb(*this)
{
}

rtc3430042_device::rtc3430042_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	rtc3430042_device(mconfig, RTC3430042, tag, owner, clock, true)
{
}

rtc3430040_device::rtc3430040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	rtc3430042_device(mconfig, RTC3430040, tag, owner, clock, false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rtc3430042_device::device_start()
{
	// allocate timers
	attotime period = clocks_to_attotime(32768 / 2);
	m_clock_timer = timer_alloc(FUNC(rtc3430042_device::half_seconds_tick), this);
	m_clock_timer->adjust(period, 0, period);
	m_cko = true;

	// state saving
	save_item(NAME(m_rTCEnb));
	save_item(NAME(m_rTCClk));
	save_item(NAME(m_data_byte));
	save_item(NAME(m_bit_count));
	save_item(NAME(m_data_dir));
	save_item(NAME(m_data_out));
	save_item(NAME(m_cmd));
	save_item(NAME(m_write_protect));
	save_item(NAME(m_test_mode));
	save_item(NAME(m_seconds));
	save_item(NAME(m_pram));
	save_item(NAME(m_xpaddr));
	save_item(NAME(m_state));
	save_item(NAME(m_data_latch));
	save_item(NAME(m_cko));
	save_item(NAME(m_time_was_set));
}

void rtc3430042_device::device_reset()
{
	m_rTCEnb = 0;
	m_rTCClk = 0;
	m_bit_count = 0;
	m_data_dir = 0;
	m_data_out = 0;
	m_cmd = 0;
	m_write_protect = 0;
	m_state = 0;

	ce_w(1);
	m_state = RTC_STATE_NORMAL;
}

//-------------------------------------------------
//  half_second_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(rtc3430042_device::half_seconds_tick)
{
	m_cko = !m_cko;
	m_cko_cb(m_cko);

	// seconds register increments following rising edge of CKO
	if (m_cko)
	{
		advance_seconds();
	}
}

//-------------------------------------------------
//  rtc_clock_updated - called by the RTC base class when the time changes
//-------------------------------------------------

void rtc3430042_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	struct tm cur_time, macref;
	uint32_t seconds;

	if (m_time_was_set)
	{
		seconds = m_seconds[0] | (m_seconds[1] << 8) | (m_seconds[2] << 16) | (m_seconds[3] << 24);
		seconds++;
	}
	else
	{
		cur_time.tm_sec = second;
		cur_time.tm_min = minute;
		cur_time.tm_hour = hour;
		cur_time.tm_mday = day;
		cur_time.tm_mon = month-1;
		cur_time.tm_year = year+100;    // assumes post-2000 current system time
		cur_time.tm_isdst = 0;

		macref.tm_sec = 0;
		macref.tm_min = 0;
		macref.tm_hour = 0;
		macref.tm_mday = 1;
		macref.tm_mon = 0;
		macref.tm_year = 4;
		macref.tm_isdst = 0;
		uint32_t ref = (uint32_t)mktime(&macref);

		seconds = (uint32_t)((uint32_t)mktime(&cur_time) - ref);
	}

	LOG("second count 0x%lX\n", (unsigned long) seconds);

	m_seconds[0] = seconds & 0xff;
	m_seconds[1] = (seconds >> 8) & 0xff;
	m_seconds[2] = (seconds >> 16) & 0xff;
	m_seconds[3] = (seconds >> 24) & 0xff;
}

/* write the chip enable state */
void rtc3430042_device::ce_w(int state)
{
	if (state && (! m_rTCEnb))
	{
		m_rTCEnb = 1;
		/* abort current transmission */
		m_data_byte = m_bit_count = m_data_dir = m_data_out = 0;
		m_state = RTC_STATE_NORMAL;
	}
	else if ((!state) && m_rTCEnb)
	{
		m_rTCEnb = 0;
		/* abort current transmission */
		m_data_byte = m_bit_count = m_data_dir = m_data_out = 0;
		m_state = RTC_STATE_NORMAL;
	}

	m_rTCEnb = state;
}

void rtc3430042_device::clk_w(int state)
{
	if ((!state) && (m_rTCClk))
	{
		rtc_shift_data(m_data_latch & 0x01);
	}

	m_rTCClk = state;
}

int rtc3430042_device::data_r()
{
	return m_data_out;
}

void rtc3430042_device::data_w(int state)
{
	m_data_latch = state;
}

/* shift data (called on rTCClk high-to-low transition) */
void rtc3430042_device::rtc_shift_data(int data)
{
	// Chip enable must be asserted for the chip to listen
	if (m_rTCEnb)
	{
		return;
	}

	// sending data to the host
	if (m_data_dir)
	{
		m_data_out = (m_data_byte >> --m_bit_count) & 0x01;
		LOG("RTC shifted new data %d\n", m_data_out);
	}
	else
	{
		// receiving data from the host
		m_data_byte = (m_data_byte << 1) | (data ? 1 : 0);

		m_bit_count++;
		if (m_bit_count == 8)
		{
			// got a byte, send it to the state machine
			rtc_execute_cmd(m_data_byte);
		}
	}
}

/* Executes a command.  Called when the first byte after "enable" is received, and
   when the data byte after a write command is received. */
void rtc3430042_device::rtc_execute_cmd(int data)
{
	int i;

	LOGMASKED(LOG_COMMANDS, "rtc_execute_cmd: data=%x, state=%x\n", data, m_state);

	if (m_state == RTC_STATE_XPCOMMAND)
	{
		m_xpaddr = ((m_cmd & 7)<<5) | ((data&0x7c)>>2);
		if ((m_cmd & 0x80) != 0)
		{
			// read command
			LOGMASKED(LOG_COMMANDS, "RTC: Reading extended address %x = %x\n", m_xpaddr, m_pram[m_xpaddr]);

			m_data_dir = 1;
			m_data_byte = m_pram[m_xpaddr];
			m_state = RTC_STATE_NORMAL;
		}
		else
		{
			// write command
			m_state = RTC_STATE_XPWRITE;
			m_data_byte = 0;
			m_bit_count = 0;
		}
	}
	else if (m_state == RTC_STATE_XPWRITE)
	{
		LOGMASKED(LOG_COMMANDS, "RTC: writing %x to extended address %x\n", data, m_xpaddr);
		m_pram[m_xpaddr] = data;
		m_state = RTC_STATE_NORMAL;
	}
	else if (m_state == RTC_STATE_WRITE)
	{
		m_state = RTC_STATE_NORMAL;

		// Register write
		i = (m_cmd >> 2) & 0x1f;
		if (m_write_protect && (i != 13))
		{
			return;
		}

		switch(i)
		{
		case 0: case 1: case 2: case 3: // seconds register
		case 4: case 5: case 6: case 7: // bit 4 is don't care
			LOGMASKED(LOG_COMMANDS, "RTC clock write, address = %X, data = %X\n", i, (int)m_data_byte);
			m_seconds[i & 3] = m_data_byte;
			m_time_was_set = true;
			break;

		case 8: case 9: case 10: case 11:   // PRAM addresses 0x10-0x13
			LOGMASKED(LOG_COMMANDS, "PRAM write, address = %X, data = %X\n", i, (int)m_data_byte);
			m_pram[i] = m_data_byte;
			break;

		case 12:
			// Test register - resets the seconds counter and increments it on the raw clock (32768 Hz) instead of once a second (not implemented)
			LOGMASKED(LOG_COMMANDS, "RTC write to test register, data = %X\n", (int)m_data_byte);
			m_test_mode = BIT(m_data_byte, 7);
			break;

		case 13:
			// Write protect - when set, all registers become read-only except this one
			if (!m_is_big_PRAM)
			{
				if (m_cmd == 0x35)  // b00110101 for 343-0040
				{
					LOGMASKED(LOG_COMMANDS, "RTC write to write-protect register, data = %X\n", (int)m_data_byte & 0x80);
					m_write_protect = BIT(m_data_byte, 7);
				}
				else
				{
					logerror("macrtc: 343-0040 illegal write protect command %02x\n", m_cmd);
				}
			}
			else
			{
				LOGMASKED(LOG_COMMANDS, "RTC write to write-protect register, data = %X\n", (int)m_data_byte & 0x80);
				m_write_protect = BIT(m_data_byte, 7);
			}
			break;

		case 16: case 17: case 18: case 19: // PRAM addresses 0x00-0x0f
		case 20: case 21: case 22: case 23:
		case 24: case 25: case 26: case 27:
		case 28: case 29: case 30: case 31:
			LOGMASKED(LOG_COMMANDS, "PRAM write, address = %X, data = %X\n", i, (int)m_data_byte);
			m_pram[i] = m_data_byte;
			break;

		default:
			LOGMASKED(LOG_COMMANDS, "Unknown RTC write command : %X, data = %d\n", (int)m_cmd, (int)m_data_byte);
			break;
		}
	}
	else
	{
		// always save this byte to m_cmd
		m_cmd = m_data_byte;

		if ((m_cmd & 0x78) == 0x38) // extended command
		{
			m_state = RTC_STATE_XPCOMMAND;
			m_data_byte = 0;
			m_bit_count = 0;
		}
		else
		{
			if (m_cmd & 0x80)
			{
				m_state = RTC_STATE_NORMAL;

				// RTC register read
				m_data_dir = 1;
				i = (m_cmd >> 2) & 0x1f;
				switch(i)
				{
					case 0: case 1: case 2: case 3:
					case 4: case 5: case 6: case 7:
						m_data_byte = m_seconds[i & 3];
						LOGMASKED(LOG_COMMANDS, "RTC clock read, address = %X -> data = %X\n", i, m_data_byte);
						break;

					case 8: case 9: case 10: case 11:
						LOGMASKED(LOG_COMMANDS, "PRAM read, address = %X data = %x\n", i, m_pram[i]);
						m_data_byte = m_pram[i];
						break;

					case 16: case 17: case 18: case 19:
					case 20: case 21: case 22: case 23:
					case 24: case 25: case 26: case 27:
					case 28: case 29: case 30: case 31:
						LOGMASKED(LOG_COMMANDS, "PRAM read, address = %X data = %x\n", i, m_pram[i]);
						m_data_byte = m_pram[i];
						break;

					default:
						LOGMASKED(LOG_COMMANDS, "Unknown RTC read command : %X\n", (int)m_cmd);
						m_data_byte = 0;
						break;
				}
			}
			else
			{
				// RTC register write - wait for data byte
				LOGMASKED(LOG_COMMANDS, "RTC write, waiting for data byte : %X\n", (int)m_cmd);
				m_state = RTC_STATE_WRITE;
				m_data_byte = 0;
				m_bit_count = 0;
			}
		}
	}
}

void rtc3430042_device::nvram_default()
{
	memset(m_pram, 0, 0x100);
}

bool rtc3430042_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_pram, 0x100);
	return !err && (actual == 0x100);
}

bool rtc3430042_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_pram, 0x100);
	return !err;
}

bool rtc3430040_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_pram, 20);
	return !err && (actual == 20);
}

bool rtc3430040_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_pram, 20);
	return !err;
}
