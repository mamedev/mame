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
/***************************************************************************
	PROTOTYPES
***************************************************************************/

util::ntfs_duration calculate_ntfs_offset();


/***************************************************************************
	GLOBAL VARIABLES
***************************************************************************/

util::ntfs_duration f_ntfs_offset(calculate_ntfs_offset());


/***************************************************************************
	IMPLEMENTATION
***************************************************************************/

util::ntfs_duration calculate_ntfs_offset()
{
	constexpr auto days_in_year(365);
	constexpr auto days_in_four_years((days_in_year * 4) + 1);
	constexpr auto days_in_century((days_in_four_years * 25) - 1);
	constexpr auto days_in_four_centuries((days_in_century * 4) + 1);

	constexpr ntfs_duration day(std::chrono::hours(24));
	constexpr ntfs_duration year(day * days_in_year);
	constexpr ntfs_duration four_years(day * days_in_four_years);
	constexpr ntfs_duration century(day * days_in_century);
	constexpr ntfs_duration four_centuries(day * days_in_four_centuries);

	std::time_t const zero(0);
	std::tm const epoch(*std::gmtime(&zero));

	ntfs_duration result(day * epoch.tm_yday);
	result += std::chrono::hours(epoch.tm_hour);
	result += std::chrono::minutes(epoch.tm_min);
	result += std::chrono::seconds(epoch.tm_sec);

	int years(1900 - 1601 + epoch.tm_year);
	result += four_centuries * (years / 400);
	years %= 400;
	result += century * (years / 100);
	years %= 100;
	result += four_years * (years / 4);
	years %= 4;
	result += year * years;

	return result;
}

} // anonymous namespace



// -------------------------------------------------
// system_clock_time_point_from_ntfs_duration
// -------------------------------------------------

std::chrono::system_clock::time_point system_clock_time_point_from_ntfs_duration(ntfs_duration d)
{
	return std::chrono::system_clock::from_time_t(0) + std::chrono::duration_cast<std::chrono::system_clock::duration>(d - f_ntfs_offset);
}

} // namespace util
