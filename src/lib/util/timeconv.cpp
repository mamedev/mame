// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Nathan Woods
/*********************************************************************

    timeconv.cpp

    Time conversion utility code

***************************************************************************/

#include "timeconv.h"

#include <ctime>


namespace util {

namespace {

std::chrono::system_clock::duration calculate_system_clock_adjustment()
{
	constexpr auto days_in_year(365);
	constexpr auto days_in_four_years((days_in_year * 4) + 1);
	constexpr auto days_in_century((days_in_four_years * 25) - 1);
	constexpr auto days_in_four_centuries((days_in_century * 4) + 1);

	// can't use std::chrono::system_clock::duration here, out of fear of integer overflow
	typedef std::chrono::duration<std::int64_t, std::ratio<1, 1> > int64_second_duration;
	constexpr int64_second_duration day(std::chrono::hours(24));
	constexpr int64_second_duration year(day * days_in_year);
	constexpr int64_second_duration four_years(day * days_in_four_years);
	constexpr int64_second_duration century(day * days_in_century);
	constexpr int64_second_duration four_centuries(day * days_in_four_centuries);

	std::time_t const zero(0);
	std::tm const epoch(*std::gmtime(&zero));

	std::chrono::system_clock::duration result(day * epoch.tm_yday);
	result += std::chrono::hours(epoch.tm_hour);
	result += std::chrono::minutes(epoch.tm_min);
	result += std::chrono::seconds(epoch.tm_sec);

	int years(1900 - 1970 + epoch.tm_year);
	result += four_centuries * (years / 400);
	years %= 400;
	result += century * (years / 100);
	years %= 100;
	result += four_years * (years / 4);
	years %= 4;
	result += year * years;

	return result - std::chrono::system_clock::from_time_t(0).time_since_epoch();
}

} // anonymous namespace



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

std::chrono::system_clock::duration system_clock_adjustment(calculate_system_clock_adjustment());



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

arbitrary_datetime arbitrary_datetime::now()
{
	time_t sec;
	time(&sec);
	auto t = *localtime(&sec);

	arbitrary_datetime dt;
	dt.year         = t.tm_year + 1900;
	dt.month        = t.tm_mon + 1;
	dt.day_of_month = t.tm_mday;
	dt.hour         = t.tm_hour;
	dt.minute       = t.tm_min;
	dt.second       = t.tm_sec;

	return dt;
}



// -------------------------------------------------
// system_clock_time_point_from_ntfs_duration
// -------------------------------------------------

std::chrono::system_clock::time_point system_clock_time_point_from_ntfs_duration(ntfs_duration d)
{
	typedef arbitrary_clock<std::uint64_t, 1601, 1, 1, 0, 0, 0, std::ratio<1, 10000000 > > ntfs_clock;
	const std::chrono::time_point<ntfs_clock> tp(d);
	return ntfs_clock::to_system_clock(tp);
}

} // namespace util
