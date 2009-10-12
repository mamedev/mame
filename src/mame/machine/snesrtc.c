/***************************************************************************

  snessdd1.c

  File to handle emulation of the SNES "S-RTC" add-on chip.

  Based on C++ implementation by Byuu in BSNES.

  Byuu's code is released under GNU General Public License
  version 2 as published by the Free Software Foundation.
  The implementation below is released under the MAME license
  for use in MAME, MESS and derivatives by permission of the
  author

***************************************************************************/

enum
{
	RTCM_Ready,
	RTCM_Command,
	RTCM_Read,
	RTCM_Write
};

typedef struct
{
	UINT8 rtc_ram[13];
	INT32 rtc_mode;
	INT8 rtc_index;
} _snes_rtc_t;

static _snes_rtc_t snes_rtc;

static const UINT8 srtc_months[12] =
{
	31, 28, 31,
	30, 31, 30,
	31, 31, 30,
	31, 30, 31
};

static void srtc_update_time(running_machine *machine)
{
	mame_system_time curtime, *systime = &curtime;
	mame_get_current_datetime(machine, &curtime);
	snes_rtc.rtc_ram[0] = systime->local_time.second % 10;
	snes_rtc.rtc_ram[1] = systime->local_time.second / 10;
	snes_rtc.rtc_ram[2] = systime->local_time.minute % 10;
	snes_rtc.rtc_ram[3] = systime->local_time.minute / 10;
	snes_rtc.rtc_ram[4] = systime->local_time.hour % 10;
	snes_rtc.rtc_ram[5] = systime->local_time.hour / 10;
	snes_rtc.rtc_ram[6] = systime->local_time.mday % 10;
	snes_rtc.rtc_ram[7] = systime->local_time.mday / 10;
	snes_rtc.rtc_ram[8] = systime->local_time.month;
	snes_rtc.rtc_ram[9] = (systime->local_time.year - 1900) % 10;
	snes_rtc.rtc_ram[10] = ((systime->local_time.year - 1900) / 10) % 10;
	snes_rtc.rtc_ram[11] = (systime->local_time.year - 1900) / 100;
	snes_rtc.rtc_ram[12] = systime->local_time.weekday % 7;
}

// Returns day-of-week for specified date
// e.g. 0 = Sunday, 1 = Monday, ... 6 = Saturday
// Usage: weekday(2008, 1, 1) returns the weekday of January 1st, 2008
static UINT8 srtc_weekday(UINT32 year, UINT32 month, UINT32 day)
{
	UINT32 y = 1900, m = 1;	// Epoch is 1900-01-01
	UINT32 sum = 0;			// Number of days passed since epoch

	year = MAX(1900, year);
	month = MAX(1, MIN(12, month));
	day = MAX(1, MIN(31, day));

	while(y < year)
	{
		UINT8 leapyear = 0;
		if((y % 4) == 0)
		{
			leapyear = 1;
			if((y % 100) == 0 && (y % 400) != 0)
			{
				leapyear = 0;
			}
		}
		sum += leapyear ? 366 : 365;
		y++;
	}

	while(m < month)
	{
		UINT32 days = srtc_months[m - 1];
		if(days == 28)
		{
			UINT8 leapyear = 0;
			if((y % 4) == 0)
			{
				leapyear = 1;
				if((y % 100) == 0 && (y % 400) != 0)
				{
					leapyear = 0;
				}
			}
			days += leapyear ? 1 : 0;
		}
		sum += days;
		m++;
	}

	sum += day - 1;
	return (sum + 1) % 7; // 1900-01-01 was a Monday
}

static UINT8 srtc_mmio_read(running_machine *machine, UINT16 addr)
{
	addr &= 0xffff;

	if(addr == 0x2800)
	{
		if(snes_rtc.rtc_mode != RTCM_Read)
		{
			return 0x00;
		}

		if(snes_rtc.rtc_index < 0)
		{
			srtc_update_time(machine);
			snes_rtc.rtc_index++;
			return 0x0f;
		}
		else if(snes_rtc.rtc_index > 12)
		{
			snes_rtc.rtc_index = -1;
			return 0x0f;
		}
		else
		{
			return snes_rtc.rtc_ram[snes_rtc.rtc_index++];
		}
	}

	return 0xff;
}

static void srtc_mmio_write(running_machine *machine, UINT16 addr, UINT8 data)
{
	addr &= 0xffff;

	if(addr == 0x2801)
	{
		data &= 0x0f;	// Only the low four bits are used

		if(data == 0x0d)
		{
			snes_rtc.rtc_mode = RTCM_Read;
			snes_rtc.rtc_index = -1;
			return;
		}

		if(data == 0x0e)
		{
			snes_rtc.rtc_mode = RTCM_Command;
			return;
		}

		if(data == 0x0f)
		{
			return;	// Unknown behaviour
		}

		if(snes_rtc.rtc_mode == RTCM_Write)
		{
			if(snes_rtc.rtc_index >= 0 && snes_rtc.rtc_index < 12)
			{
				snes_rtc.rtc_ram[snes_rtc.rtc_index++] = data;

				if(snes_rtc.rtc_index == 12)
				{
					// Day of week is automatically calculated and written
					UINT32 day   = snes_rtc.rtc_ram[6] + snes_rtc.rtc_ram[7] * 10;
					UINT32 month = snes_rtc.rtc_ram[8];
					UINT32 year  = snes_rtc.rtc_ram[9] + snes_rtc.rtc_ram[10] * 10 + snes_rtc.rtc_ram[11] * 100;
					year += 1000;

					snes_rtc.rtc_ram[snes_rtc.rtc_index++] = srtc_weekday(year, month, day);
				}
			}
		}
		else if(snes_rtc.rtc_mode == RTCM_Command)
		{
			if(data == 0)
			{
				snes_rtc.rtc_mode = RTCM_Write;
				snes_rtc.rtc_index = 0;
			}
			else if(data == 4)
			{
				UINT8 i;
				snes_rtc.rtc_mode = RTCM_Ready;
				snes_rtc.rtc_index = -1;
				for(i = 0; i < 13; i++)
				{
					snes_rtc.rtc_ram[i] = 0;
				}
			}
			else
			{
				// Unknown behaviour
				snes_rtc.rtc_mode = RTCM_Ready;
			}
		}
	}
}

static void srtc_reset(running_machine *machine)
{
	snes_rtc.rtc_mode = RTCM_Read;
	snes_rtc.rtc_index = -1;
	srtc_update_time(machine);
}

