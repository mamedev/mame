/***************************************************************************

    NEC uPD4990A

    Serial I/O Calendar & Clock IC used in the
        NEO GEO and probably a couple of other machines.


    Completed by ElSemi.

    I haven't found any schematics for this device
    so I had to make some assumptions about how it works.

    The three input bits seem to be used for a serial protocol

    bit 0 - data
    bit 1 - clock
    bit 2 - command end (?)

    the commands I've found so far are:

    0x0 - ?? sent after 2
    0x1 - Reset the (probable) shift register used for output
    0x2 - Store the contents of the shift reg to the current date
    0x3 - Load Shift register with current date

    0x7 - Switch test bit every frame
    0x8 - Switch test bit every half-second


    2009-06 Converted to be a device

***************************************************************************/

#include "emu.h"
#include "machine/pd4990a.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define DATA_BIT    0x01
#define CLOCK_BIT   0x02
#define END_BIT     0x04



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE UINT8 convert_to_bcd(int val)
{
	return ((val / 10) << 4) | (val % 10);
}

const device_type UPD4990A_OLD = &device_creator<upd4990a_old_device>;

upd4990a_old_device::upd4990a_old_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD4990A_OLD, "NEC uPD4990A", tag, owner, clock, "upd4990a_old", __FILE__)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void upd4990a_old_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd4990a_old_device::device_start()
{
	system_time curtime, *systime = &curtime;
	machine().current_datetime(curtime);

#if 0
	m_seconds = 0x00;
	m_minutes = 0x00;
	m_hours = 0x00;
	m_days = 0x09;
	m_month = 9;
	m_year = 0x73;
	m_weekday = 1;
#endif
	/* HACK: load time counter from system time */
	m_seconds = convert_to_bcd(systime->local_time.second);
	m_minutes = convert_to_bcd(systime->local_time.minute);
	m_hours = convert_to_bcd(systime->local_time.hour);
	m_days = convert_to_bcd(systime->local_time.mday);
	m_month = systime->local_time.month + 1;
	m_year = ((((systime->local_time.year - 1900) % 100) / 10) << 4) | ((systime->local_time.year - 1900) % 10);
	m_weekday = systime->local_time.weekday;

	/* register for state saving */
	save_item(NAME(m_seconds));
	save_item(NAME(m_minutes));
	save_item(NAME(m_hours));
	save_item(NAME(m_days));
	save_item(NAME(m_month));
	save_item(NAME(m_year));
	save_item(NAME(m_weekday));

	save_item(NAME(m_shiftlo));
	save_item(NAME(m_shifthi));

	save_item(NAME(m_retraces));
	save_item(NAME(m_testwaits));
	save_item(NAME(m_maxwaits));
	save_item(NAME(m_testbit));

	save_item(NAME(m_outputbit));
	save_item(NAME(m_bitno));
	save_item(NAME(m_reading));
	save_item(NAME(m_writing));

	save_item(NAME(m_clock_line));
	save_item(NAME(m_command_line));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd4990a_old_device::device_reset()
{
	m_shiftlo = 0;
	m_shifthi = 0;

	m_retraces = 0;
	m_testwaits = 0;
	m_maxwaits = 1;
	m_testbit = 0;

	m_outputbit = 0;
	m_bitno = 0;
	m_reading = 0;
	m_writing = 0;

	m_clock_line = 0;
	m_command_line = 0;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    increment_month
-------------------------------------------------*/

void upd4990a_old_device::increment_month()
{
	m_month++;
	if (m_month == 13)
	{
		m_month = 1;
		m_year++;
		if ((m_year & 0x0f) >= 10)
		{
			m_year &= 0xf0;
			m_year += 0x10;
		}
		if (m_year == 0xA0)
			m_year = 0;
	}
}

/*-------------------------------------------------
    increment_day
-------------------------------------------------*/

void upd4990a_old_device::increment_day()
{
	int real_year;

	m_days++;
	if ((m_days & 0x0f) >= 10)
	{
		m_days &= 0xf0;
		m_days += 0x10;
	}

	m_weekday++;
	if (m_weekday == 7)
		m_weekday = 0;

	switch (m_month)
	{
		case 1: case 3: case 5: case 7: case 8: case 10: case 12:
			if (m_days == 0x32)
			{
				m_days = 1;
				increment_month();
			}
			break;
		case 2:
			real_year = (m_year >> 4) * 10 + (m_year & 0xf);
			if ((real_year % 4) && (!(real_year % 100) || (real_year % 400)))
			{
				if (m_days == 0x29)
				{
					m_days = 1;
					increment_month();
				}
			}
			else
			{
				if (m_days == 0x30)
				{
					m_days = 1;
					increment_month();
				}
			}
			break;
		case 4: case 6: case 9: case 11:
			if (m_days == 0x31)
			{
				m_days = 1;
				increment_month();
			}
			break;
	}
}

/*-------------------------------------------------
    addretrace
-------------------------------------------------*/

void upd4990a_old_device::addretrace()
{
	++m_testwaits;
	if(m_testwaits >= m_maxwaits)
	{
		m_testbit ^= 1;
		m_testwaits = 0;
	}

	m_retraces++;
	if (m_retraces < 60)
		return;

	m_retraces = 0;
	m_seconds++;
	if ((m_seconds & 0x0f) < 10)
		return;

	m_seconds &= 0xf0;
	m_seconds += 0x10;
	if (m_seconds < 0x60)
		return;

	m_seconds = 0;
	m_minutes++;
	if ((m_minutes & 0x0f) < 10)
		return;

	m_minutes &= 0xf0;
	m_minutes += 0x10;
	if (m_minutes < 0x60)
		return;

	m_minutes = 0;
	m_hours++;
	if ((m_hours & 0x0f) < 10)
		return;

	m_hours &= 0xf0;
	m_hours += 0x10;
	if (m_hours < 0x24)
		return;

	m_hours = 0;
	increment_day();
}

/*-------------------------------------------------
    testbit_r
-------------------------------------------------*/

READ8_MEMBER( upd4990a_old_device::testbit_r )
{
	return m_testbit;
}

/*-------------------------------------------------
    databit_r
-------------------------------------------------*/

READ8_MEMBER( upd4990a_old_device::databit_r )
{
	return m_outputbit;
}

/*-------------------------------------------------
    readbit
-------------------------------------------------*/

void upd4990a_old_device::readbit()
{
	switch (m_bitno)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			m_outputbit = (m_seconds >> m_bitno) & 0x01;
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			m_outputbit = (m_minutes >> (m_bitno - 0x08)) & 0x01;
			break;
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
			m_outputbit = (m_hours >> (m_bitno - 0x10)) & 0x01;
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			m_outputbit = (m_days >> (m_bitno - 0x18)) & 0x01;
			break;
		case 0x20: case 0x21: case 0x22: case 0x23:
			m_outputbit = (m_weekday >> (m_bitno - 0x20)) & 0x01;
			break;
		case 0x24: case 0x25: case 0x26: case 0x27:
			m_outputbit = (m_month >> (m_bitno - 0x24)) & 0x01;
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			m_outputbit = (m_year >> (m_bitno - 0x28)) & 0x01;
			break;
		case 0x30: case 0x31: case 0x32: case 0x33:
			//unknown
			break;
	}
}

/*-------------------------------------------------
    resetbitstream
-------------------------------------------------*/

void upd4990a_old_device::resetbitstream()
{
	m_shiftlo = 0;
	m_shifthi = 0;
	m_bitno = 0;
}

/*-------------------------------------------------
    writebit
-------------------------------------------------*/

void upd4990a_old_device::writebit( UINT8 bit )
{
	if (m_bitno <= 31)  //low part
		m_shiftlo |= bit << m_bitno;
	else    //high part
		m_shifthi |= bit << (m_bitno - 32);
}

/*-------------------------------------------------
    nextbit
-------------------------------------------------*/

void upd4990a_old_device::nextbit()
{
	++m_bitno;

	if (m_reading)
		readbit();

	if (m_reading && m_bitno == 0x34)
	{
		m_reading = 0;
		resetbitstream();
	}

}

/*-------------------------------------------------
    getcommand
-------------------------------------------------*/

UINT8 upd4990a_old_device::getcommand()
{
	//Warning: problems if the 4 bits are in different
	//parts, It's very strange that this case could happen.
	if(m_bitno <= 31)
		return m_shiftlo >> (m_bitno - 4);
	else
		return m_shifthi >> (m_bitno - 32 - 4);
}

/*-------------------------------------------------
    update_date
-------------------------------------------------*/

void upd4990a_old_device::update_date()
{
	m_seconds = (m_shiftlo >> 0 ) & 0xff;
	m_minutes = (m_shiftlo >> 8 ) & 0xff;
	m_hours   = (m_shiftlo >> 16) & 0xff;
	m_days    = (m_shiftlo >> 24) & 0xff;
	m_weekday = (m_shifthi >> 0 ) & 0x0f;
	m_month   = (m_shifthi >> 4 ) & 0x0f;
	m_year    = (m_shifthi >> 8 ) & 0xff;
}

/*-------------------------------------------------
    process_command
-------------------------------------------------*/

void upd4990a_old_device::process_command()
{
	switch(getcommand())
	{
		case 0x1:   //load output register
			m_bitno = 0;
			if (m_reading)
				readbit();   //prepare first bit
			m_shiftlo = 0;
			m_shifthi = 0;
			break;
		case 0x2:
			m_writing = 0;  //store register to current date
			update_date();
			break;
		case 0x3:   //start reading
			m_reading = 1;
			break;
		case 0x7:   //switch testbit every frame
			m_maxwaits = 1;
			break;
		case 0x8:   //switch testbit every half-second
			m_maxwaits = 30;
			break;
	}
	resetbitstream();
}

/*-------------------------------------------------
    serial_control
-------------------------------------------------*/

void upd4990a_old_device::serial_control( UINT8 data )
{
	//Check for command end
	if(m_command_line && !(data & END_BIT)) //end of command
	{
		process_command();
	}
	m_command_line = data & END_BIT;

	if(m_clock_line && !(data & CLOCK_BIT)) //clock lower edge
	{
		writebit(data & DATA_BIT);
		nextbit();
	}
	m_clock_line = data & CLOCK_BIT;
}

/*-------------------------------------------------
    control_16_w
-------------------------------------------------*/

WRITE16_MEMBER( upd4990a_old_device::control_16_w )
{
	serial_control(data & 0x7);
}
