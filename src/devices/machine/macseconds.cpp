// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Classic Mac time converter mix-in
    by R. Belmont

    This is a mix-in for anything wanting to convert MAME time to the classic Mac time format
    (seconds since 1/1/1904 at midnight).
*/

#include "dirtc.h"

#include "emu.h"
#include "macseconds.h"

macseconds_interface::macseconds_interface()
{
	// Get the current time to get the DST flag and compute the offset from GMT
	const time_t cur_time_t = time(NULL);
	struct tm *local_tm = localtime(&cur_time_t);
	struct tm *gmt_tm = gmtime(&cur_time_t);

	// Sync DST
	m_is_dst = gmt_tm->tm_isdst = local_tm->tm_isdst;

	// MSYS2 struct tm doesn't have tm_gmtoff, so we calculate the offset the long way
	m_gmt_offset = mktime(local_tm) - mktime(gmt_tm);
}

macseconds_interface::~macseconds_interface()
{
}

u32 macseconds_interface::get_local_seconds(system_time &systime)
{
	const system_time::full_time &time = systime.local_time;

	return get_seconds(time.year - 2000, time.month + 1, time.mday, time.weekday + 1, time.hour,
							  time.minute, time.second);
}

u32 macseconds_interface::get_seconds(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	struct tm cur_time;

	cur_time.tm_sec = second;
	cur_time.tm_min = minute;
	cur_time.tm_hour = hour;
	cur_time.tm_mday = day;
	cur_time.tm_mon = month - 1; // tm_mon is 0-based
	cur_time.tm_year = year + 100; // tm_year is years since 1900
	cur_time.tm_isdst = m_is_dst;

	// Add the offset between the Unix epoch and the classic Mac OS epoch (hat tip to https://www.epochconverter.com/mac)
	return static_cast<u32>(mktime(&cur_time) + (2082844800 + m_gmt_offset));
}
