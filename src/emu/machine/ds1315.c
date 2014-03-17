/*********************************************************************

    ds1315.c

    Dallas Semiconductor's Phantom Time Chip DS1315.

    by tim lindner, November 2001.

*********************************************************************/

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
	memset(m_raw_data, 0, sizeof(m_raw_data));
	m_count = 0;
	m_mode = DS_SEEK_MATCHING;
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
 read_0
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
    read_1
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
    write_data
-------------------------------------------------*/

WRITE8_MEMBER( ds1315_device::write_data )
{
	if (m_mode == DS_CALENDAR_IO)
	{
		m_raw_data[m_count++] = data & 0x01;

		if (m_count == 64)
		{
			m_mode = DS_SEEK_MATCHING;
			m_count = 0;
			input_raw_data();
		}
		return;
	}

	m_count = 0;
}


/*-------------------------------------------------
    fill_raw_data
-------------------------------------------------*/

void ds1315_device::fill_raw_data()
{
	/* This routine will (hopefully) call a standard 'C' library routine to get the current
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
    ds1315_input_raw_data
-------------------------------------------------*/

void ds1315_device::input_raw_data()
{
	/* This routine is called when new date and time has been written to the
	   clock chip. Currently we ignore setting the date and time in the clock
	   chip.

	   We always return the host's time when asked.
	*/
}
