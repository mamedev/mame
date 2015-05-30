// license:BSD-3-Clause
// copyright-holders:Tim Lindner
/*****************************************************************************************

    ds1315.c

    Dallas Semiconductor's Phantom Time Chip DS1315.
    NOTE: writes are decoded, but the host's time will always be returned when asked.

    April 2015: chip enable / chip reset / phantom writes by Karl-Ludwig Deisenhofer

    November 2001: implementation by Tim Lindner

    HOW DOES IT WORK?

    READS: pattern recognition (64 bits in correct order). When RTC finally enables
    64 bits of data can be read. Chance of accidential pattern recognition is minimal.

    WRITES: two different locations (bits 0 and 1) are used to transfer data to the
    DS1315.   64 bit with time/date info are transmitted directly after recognition
    of the magic 64 bit pattern (see read above).
    **************************************************************************************/

#include "ds1315.h"
#include "coreutil.h"


const device_type DS1315 = &device_creator<ds1315_device>;

ds1315_device::ds1315_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, DS1315, "Dallas Semiconductor DS1315", tag, owner, clock, "ds1315", __FILE__)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ds1315_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ds1315_device::device_start()
{
	save_item(NAME(m_count));
	save_item(NAME(m_mode));
	save_item(NAME(m_raw_data));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ds1315_device::device_reset()
{
	chip_reset();
}



/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

static const UINT8 ds1315_pattern[] =
{
	1, 0, 1, 0, 0, 0, 1, 1,
	0, 1, 0, 1, 1, 1, 0, 0,
	1, 1, 0, 0, 0, 1, 0, 1,
	0, 0, 1, 1, 1, 0, 1, 0,
	1, 0, 1, 0, 0, 0, 1, 1,
	0, 1, 0, 1, 1, 1, 0, 0,
	1, 1, 0, 0, 0, 1, 0, 1,
	0, 0, 1, 1, 1, 0, 1, 0
};


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
 read_0 (actual data)
 -------------------------------------------------*/

READ8_MEMBER( ds1315_device::read_0 )
{
	if (ds1315_pattern[m_count++] == 0)
	{
		if (m_count == 64)
		{
			/* entire pattern matched */
			m_count = 0;
			m_mode = DS_CALENDAR_IO;
			fill_raw_data();
		}

		return 0;
	}

	m_count = 0;
	m_mode = DS_SEEK_MATCHING;
	return 0;
}


/*-------------------------------------------------
    read_1 (actual data)
-------------------------------------------------*/

READ8_MEMBER( ds1315_device::read_1 )
{
	if (ds1315_pattern[m_count++] == 1)
	{
		m_count %= 64;
		return 0;
	}

	m_count = 0;
	m_mode = DS_SEEK_MATCHING;
	return 0;
}


/*-------------------------------------------------
    read_data
-------------------------------------------------*/

READ8_MEMBER( ds1315_device::read_data )
{
	UINT8 result;

	if (m_mode == DS_CALENDAR_IO)
	{
		result = m_raw_data[m_count++];

		if (m_count == 64)
		{
			m_mode = DS_SEEK_MATCHING;
			m_count = 0;
		}

		return result;
	}

	m_count = 0;
	return 0;
}


/*-------------------------------------------------
    fill_raw_data
-------------------------------------------------*/

void ds1315_device::fill_raw_data()
{
	/* This routine calls a standard 'C' library routine to get the current
	   date and time and then fill in the raw data struct.
	*/

	system_time systime;
	int raw[8], i, j;

	/* get the current date/time from the core */
	machine().current_datetime(systime);

	raw[0] = 0; /* tenths and hundreths of seconds are always zero */
	raw[1] = dec_2_bcd(systime.local_time.second);
	raw[2] = dec_2_bcd(systime.local_time.minute);
	raw[3] = dec_2_bcd(systime.local_time.hour);

	raw[4] = dec_2_bcd((systime.local_time.weekday != 0) ? systime.local_time.weekday : 7);
	raw[5] = dec_2_bcd(systime.local_time.mday);
	raw[6] = dec_2_bcd(systime.local_time.month + 1);
	raw[7] = dec_2_bcd(systime.local_time.year - 1900); /* Epoch is 1900 */

	/* Ok now we have the raw bcd bytes. Now we need to push them into our bit array */

	for (i = 0; i < 64; i++)
	{
		j = i / 8;
		m_raw_data[i] = (raw[j] & 0x0001);
		raw[j] = raw[j] >> 1;
	}
}




/*-------------------------------------------------
write_data
-------------------------------------------------*/

READ8_MEMBER(ds1315_device::write_data)
{
	static int write_count;
	if (write_count >= 64)
		write_count = 0;

	if (m_mode == DS_CALENDAR_IO)
	{
		m_raw_data[write_count++] = offset & 0x01;

		if (write_count == 64)
		{
			write_count = 0;

			m_mode = DS_SEEK_MATCHING;
			m_count = 0;
			input_raw_data();
		}
	}
	return 0; // ignore
}

/*-------------------------------------------------
  ds1315_input_raw_data

  Routine is called when new date and time has
  been written to the clock chip. Currently we
  ignore setting the date and time in the clock
  chip.
-------------------------------------------------*/

void ds1315_device::input_raw_data()
{
	int raw[8], i, j=0;
	raw[0] = raw[1] = raw[2] = raw[3] = raw[4] = raw[5] = raw[6] = raw[7] = 0;
	UINT8 flag = 1;

	for (i = 0; i < 64; i++)
	{
		j = i / 8;
		if ((i % 8) == 0)
			flag = 1;

		if (m_raw_data[i] & 1)
				raw[j] |= flag;
		flag <<= 1;
	}
	raw[0] = bcd_2_dec(raw[0]); // hundreds of seconds
	raw[1] = bcd_2_dec(raw[1]); // seconds (often set to zero)
	raw[2] = bcd_2_dec(raw[2]); // minute
	raw[3] = bcd_2_dec(raw[3]); // hour

	raw[4] = bcd_2_dec(raw[4]); // weekday (10 for Friday ?!)
	raw[5] = bcd_2_dec(raw[5]); // mday
	raw[6] = bcd_2_dec(raw[6]); // month
	raw[7] = bcd_2_dec(raw[7]); // year (two digits)

	printf("\nDS1315 RTC INPUT (WILL BE IGNORED) mm/dd/yy  hh:mm:ss - %02d/%02d/%02d %02d/%02d/%02d",
				raw[6], raw[5], raw[7], raw[3], raw[2], raw[1]
			);
}

/*-------------------------------------------------
   query and reset chip status
   -------------------------------------------------*/
bool ds1315_device::chip_enable()
{
	return (m_mode == DS_CALENDAR_IO);
}

// Set a defined state (important for pattern detection)
void ds1315_device::chip_reset()
{
	memset(m_raw_data, 0, sizeof(m_raw_data));
	m_count = 0;
	m_mode = DS_SEEK_MATCHING;
}
