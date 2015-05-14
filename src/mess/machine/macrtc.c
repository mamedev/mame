// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  macrtc.c - the real-time clock & NVRAM chip used in early 680x0 Macs,
  Apple part numbers 343-0040 (original) and 343-0042 (with extended PRAM)

  The IIgs has this chip also, but the VGC contains a relatively
  sophisticated logic block that offloads the low-level serial comms
  from the CPU, which makes it look quite different to software.

***************************************************************************/

#include "emu.h"
#include "macrtc.h"

#ifdef MAME_DEBUG
#define LOG_RTC         0
#else
#define LOG_RTC         0
#endif

enum
{
	RTC_STATE_NORMAL,
	RTC_STATE_WRITE,
	RTC_STATE_XPCOMMAND,
	RTC_STATE_XPWRITE
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type RTC3430042 = &device_creator<rtc3430042_device>;


//-------------------------------------------------
//  rtc4543_device - constructor
//-------------------------------------------------

rtc3430042_device::rtc3430042_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RTC3430042, "Apple 343-0042 clock/PRAM", tag, owner, clock, "rtc3430042", __FILE__),
		device_rtc_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rtc3430042_device::device_start()
{
	// allocate timers
	m_clock_timer = timer_alloc();
	m_clock_timer->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	// state saving
}

void rtc3430042_device::device_reset()
{
	set_current_time(machine());

	m_rtc_rTCEnb = 0;
	m_rtc_rTCClk = 0;
	m_rtc_bit_count = 0;
	m_rtc_data_dir = 0;
	m_rtc_data_out = 0;
	m_rtc_cmd = 0;
	m_rtc_write_protect = 0;
	m_rtc_state = 0;

	ce_w(1);
	m_rtc_state = RTC_STATE_NORMAL;
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void rtc3430042_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	advance_seconds();
}

//-------------------------------------------------
//  rtc_clock_updated - called by the RTC base class when the time changes
//-------------------------------------------------

void rtc3430042_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	struct tm cur_time, mac_reference;
	UINT32 seconds;

	cur_time.tm_sec = second;
	cur_time.tm_min = minute;
	cur_time.tm_hour = hour;
	cur_time.tm_mday = day;
	cur_time.tm_mon = month-1;
	cur_time.tm_year = year+100;    // assumes post-2000 current system time
	cur_time.tm_isdst = 0;

	/* The count starts on 1st January 1904 */
	mac_reference.tm_sec = 0;
	mac_reference.tm_min = 0;
	mac_reference.tm_hour = 0;
	mac_reference.tm_mday = 1;
	mac_reference.tm_mon = 0;
	mac_reference.tm_year = 4;
	mac_reference.tm_isdst = 0;

	seconds = difftime(mktime(&cur_time), mktime(&mac_reference));

	if (LOG_RTC)
		logerror("second count 0x%lX\n", (unsigned long) seconds);

	m_rtc_seconds[0] = seconds & 0xff;
	m_rtc_seconds[1] = (seconds >> 8) & 0xff;
	m_rtc_seconds[2] = (seconds >> 16) & 0xff;
	m_rtc_seconds[3] = (seconds >> 24) & 0xff;
}

/* write the rTCEnb state */
WRITE_LINE_MEMBER( rtc3430042_device::ce_w )
{
	if (state && (! m_rtc_rTCEnb))
	{
		/* rTCEnb goes high (inactive) */
		m_rtc_rTCEnb = 1;
		/* abort current transmission */
		m_rtc_data_byte = m_rtc_bit_count = m_rtc_data_dir = m_rtc_data_out = 0;
		m_rtc_state = RTC_STATE_NORMAL;
	}
	else if ((!state) && m_rtc_rTCEnb)
	{
		/* rTCEnb goes low (active) */
		m_rtc_rTCEnb = 0;
		/* abort current transmission */
		m_rtc_data_byte = m_rtc_bit_count = m_rtc_data_dir = m_rtc_data_out = 0;
		m_rtc_state = RTC_STATE_NORMAL;
	}

	m_rtc_rTCEnb = state;
}

WRITE_LINE_MEMBER( rtc3430042_device::clk_w )
{
	if ((!state) && (m_rtc_rTCClk))
	{
		rtc_shift_data(m_data_latch & 0x01);
	}

	m_rtc_rTCClk = state;
}

READ_LINE_MEMBER( rtc3430042_device::data_r )
{
	return m_rtc_data_out;
}

WRITE_LINE_MEMBER( rtc3430042_device::data_w )
{
	m_data_latch = state;
}

/* shift data (called on rTCClk high-to-low transition (?)) */
void rtc3430042_device::rtc_shift_data(int data)
{
	if (m_rtc_rTCEnb)
		/* if enable line inactive (high), do nothing */
		return;

	if (m_rtc_data_dir)
	{   /* RTC -> VIA transmission */
		m_rtc_data_out = (m_rtc_data_byte >> --m_rtc_bit_count) & 0x01;
		if (LOG_RTC)
			logerror("RTC shifted new data %d\n", m_rtc_data_out);
	}
	else
	{   /* VIA -> RTC transmission */
		m_rtc_data_byte = (m_rtc_data_byte << 1) | (data ? 1 : 0);

		if (++m_rtc_bit_count == 8)
		{   /* if one byte received, send to command interpreter */
			rtc_execute_cmd(m_rtc_data_byte);
		}
	}
}

/* Executes a command.
Called when the first byte after "enable" is received, and when the data byte after a write command
is received. */
void rtc3430042_device::rtc_execute_cmd(int data)
{
	int i;

	if (LOG_RTC)
		printf("rtc_execute_cmd: data=%x, state=%x\n", data, m_rtc_state);

	if (m_rtc_state == RTC_STATE_XPCOMMAND)
	{
		m_rtc_xpaddr = ((m_rtc_cmd & 7)<<5) | ((data&0x7c)>>2);
		if ((m_rtc_cmd & 0x80) != 0)
		{
			// read command
			if (LOG_RTC)
				printf("RTC: Reading extended address %x = %x\n", m_rtc_xpaddr, m_pram[m_rtc_xpaddr]);

			m_rtc_data_dir = 1;
			m_rtc_data_byte = m_pram[m_rtc_xpaddr];
			m_rtc_state = RTC_STATE_NORMAL;
		}
		else
		{
			// write command
			m_rtc_state = RTC_STATE_XPWRITE;
			m_rtc_data_byte = 0;
			m_rtc_bit_count = 0;
		}
	}
	else if (m_rtc_state == RTC_STATE_XPWRITE)
	{
		if (LOG_RTC)
			printf("RTC: writing %x to extended address %x\n", data, m_rtc_xpaddr);
		m_pram[m_rtc_xpaddr] = data;
		m_rtc_state = RTC_STATE_NORMAL;
	}
	else if (m_rtc_state == RTC_STATE_WRITE)
	{
		m_rtc_state = RTC_STATE_NORMAL;

		/* Writing an RTC register */
		i = (m_rtc_cmd >> 2) & 0x1f;
		if (m_rtc_write_protect && (i != 13))
			/* write-protection : only write-protect can be written again */
			return;
		switch(i)
		{
		case 0: case 1: case 2: case 3: /* seconds register */
		case 4: case 5: case 6: case 7: /* ??? (not described in IM III) */
			{
				/* after various tries, I assumed m_rtc_seconds[4+i] is mapped to m_rtc_seconds[i] */
				if (LOG_RTC)
					logerror("RTC clock write, address = %X, data = %X\n", i, (int) m_rtc_data_byte);
				m_rtc_seconds[i & 3] = m_rtc_data_byte;

				// TODO: call the base class's time set here
			}
			break;

		case 8: case 9: case 10: case 11:   /* RAM address $10-$13 */
			if (LOG_RTC)
				printf("PRAM write, address = %X, data = %X\n", i, (int) m_rtc_data_byte);
			m_pram[i] = m_rtc_data_byte;
			break;

		case 12:
			/* Test register - do nothing */
			if (LOG_RTC)
				logerror("RTC write to test register, data = %X\n", (int) m_rtc_data_byte);
			break;

		case 13:
			/* Write-protect register  */
			if (LOG_RTC)
				printf("RTC write to write-protect register, data = %X\n", (int) m_rtc_data_byte&0x80);
			m_rtc_write_protect = (m_rtc_data_byte & 0x80) ? TRUE : FALSE;
			break;

		case 16: case 17: case 18: case 19: /* RAM address $00-$0f */
		case 20: case 21: case 22: case 23:
		case 24: case 25: case 26: case 27:
		case 28: case 29: case 30: case 31:
			if (LOG_RTC)
				printf("PRAM write, address = %X, data = %X\n", i, (int) m_rtc_data_byte);
			m_pram[i] = m_rtc_data_byte;
			break;

		default:
			printf("Unknown RTC write command : %X, data = %d\n", (int) m_rtc_cmd, (int) m_rtc_data_byte);
			break;
		}
	}
	else
	{
		// always save this byte to m_rtc_cmd
		m_rtc_cmd = m_rtc_data_byte;

		if ((m_rtc_cmd & 0x78) == 0x38) // extended command
		{
			m_rtc_state = RTC_STATE_XPCOMMAND;
			m_rtc_data_byte = 0;
			m_rtc_bit_count = 0;
		}
		else
		{
			if (m_rtc_cmd & 0x80)
			{
				m_rtc_state = RTC_STATE_NORMAL;

				/* Reading an RTC register */
				m_rtc_data_dir = 1;
				i = (m_rtc_cmd >> 2) & 0x1f;
				switch(i)
				{
					case 0: case 1: case 2: case 3:
					case 4: case 5: case 6: case 7:
						m_rtc_data_byte = m_rtc_seconds[i & 3];
						if (LOG_RTC)
							printf("RTC clock read, address = %X -> data = %X\n", i, m_rtc_data_byte);
						break;

					case 8: case 9: case 10: case 11:
						if (LOG_RTC)
							printf("PRAM read, address = %X data = %x\n", i, m_pram[i]);
						m_rtc_data_byte = m_pram[i];
						break;

					case 16: case 17: case 18: case 19:
					case 20: case 21: case 22: case 23:
					case 24: case 25: case 26: case 27:
					case 28: case 29: case 30: case 31:
						if (LOG_RTC)
							printf("PRAM read, address = %X data = %x\n", i, m_pram[i]);
						m_rtc_data_byte = m_pram[i];
						break;

					default:
						if (LOG_RTC)
							logerror("Unknown RTC read command : %X\n", (int) m_rtc_cmd);
						m_rtc_data_byte = 0;
						break;
				}
			}
			else
			{
				/* Writing an RTC register */
				/* wait for extra data byte */
				if (LOG_RTC)
					logerror("RTC write, waiting for data byte : %X\n", (int) m_rtc_cmd);
				m_rtc_state = RTC_STATE_WRITE;
				m_rtc_data_byte = 0;
				m_rtc_bit_count = 0;
			}
		}
	}
}

void rtc3430042_device::nvram_default()
{
	memset(m_pram, 0, 0x100);
}

void rtc3430042_device::nvram_read(emu_file &file)
{
	file.read(m_pram, 0x100);
}

void rtc3430042_device::nvram_write(emu_file &file)
{
	file.write(m_pram, 0x100);
}
