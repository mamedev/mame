// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emutime.cpp

    System time utilities for MAME.

***************************************************************************/

#include "emutime.h"
#include "osdcore.h"


//**************************************************************************
//  SYSTEM TIME
//**************************************************************************

//-------------------------------------------------
//  system_time - constructor
//-------------------------------------------------

system_time::system_time()
{
	set(0);
}

system_time::system_time(time_t t)
{
	set(t);
}


//-------------------------------------------------
//  set - fills out a system_time structure
//-------------------------------------------------

void system_time::set(time_t t)
{
	// FIXME: this crashes if localtime or gmtime returns nullptr
	time = t;
	local_time = util::arbitrary_datetime(*localtime(&t));
	utc_time = util::arbitrary_datetime(*gmtime(&t));
}


//-------------------------------------------------
//  customize - check whether string conforms to
//  YYYY-MM-DD hh:mm:ss format and modify date
//  and/or time if it does
//-------------------------------------------------

bool system_time::customize(std::string_view str)
{
	int yr = -1, mo = -1, dy = -1, hh = -1, mm = -1, ss = -1;
	if (str.length() >= 4 && str.find_first_not_of("0123456789") >= 4)
	{
		yr = (str[0] - '0') * 1000 + (str[1] - '0') * 100 + (str[2] - '0') * 10 + (str[3] - '0');
		if (str.length() == 4)
			str = std::string_view();
		else
		{
			if (str.length() < 10 || str[4] != str[7] || str[4] != '-')
				return false;
			if (str[5] < '0' || str[5] > '1' || str[6] < (str[5] == '0' ? '1' : '0') || str[6] > (str[5] == '1' ? '2' : '9'))
				return false;
			if (str[8] < '0' || str[8] > '3' || str[9] < (str[8] == '0' ? '1' : '0') || str[9] > (str[8] == '3' ? '1' : '9'))
				return false;
			mo = (str[5] - '0') * 10 + (str[6] - '0');
			dy = (str[8] - '0') * 10 + (str[9] - '0');
			if (str.length() == 10)
				str = std::string_view();
			else
			{
				auto pos = str.find_first_not_of(' ', 10);
				if (pos > 10 && pos != std::string_view::npos)
					str.remove_prefix(pos);
				else
					return false;
			}
		}
	}
	if (!str.empty())
	{
		if (str.length() < 5 || str[2] != ':')
			return false;
		if (str[0] < '0' || str[0] > '2' || str[1] < '0' || (str[1] > (str[0] == '2' ? '3' : '9')))
			return false;
		if (str[3] < '0' || str[3] > '5' || str[4] < '0' || str[4] > '9')
			return false;
		hh = (str[0] - '0') * 10 + (str[1] - '0');
		mm = (str[3] - '0') * 10 + (str[4] - '0');
		ss = 0;
		if (str.length() != 5)
		{
			if (str.length() != 8 || str[5] != ':')
				return false;
			if (str[6] < '0' || str[6] > '5' || str[7] < '0' || str[7] > '9')
				return false;
			ss = (str[6] - '0') * 10 + (str[7] - '0');
		}
	}

	if (yr != -1)
	{
		local_time.year = utc_time.year = yr;
		if (mo != -1)
		{
			local_time.month = utc_time.month = mo;
			local_time.day_of_month = utc_time.day_of_month = dy;
		}
		else if (!gregorian_is_leap_year(yr))
		{
			// Mitigate potential problems of having Leap Day in a non-leap year
			if (local_time.month == 2 && local_time.day_of_month == 29)
			{
				local_time.month = 3;
				local_time.day_of_month = 1;
			}
			if (utc_time.month == 2 && utc_time.day_of_month == 29)
			{
				utc_time.month = 3;
				utc_time.day_of_month = 1;
			}
		}
	}
	if (hh != -1)
	{
		local_time.hour = utc_time.hour = hh;
		local_time.minute = utc_time.minute = mm;
		local_time.second = utc_time.second = ss;
	}
	return true;
}
