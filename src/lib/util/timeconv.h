// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Nathan Woods
/***************************************************************************

    timeconv.h

    Time conversion utility code

***************************************************************************/

#ifndef MAME_LIB_UTIL_TIMECONV_H
#define MAME_LIB_UTIL_TIMECONV_H

#pragma once

#include "coreutil.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <stdexcept>


namespace util {
/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern std::chrono::system_clock::duration system_clock_adjustment;


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef std::chrono::duration<std::uint64_t, std::ratio<1, 10000000> > ntfs_duration;


//---------------------------------------------------------
//  arbitrary_datetime
//---------------------------------------------------------

struct arbitrary_datetime
{
	int year;           // absolute year (1900 AD = 1900)
	int month;          // month (1-12)
	int day_of_month;   // day of month (1-31)
	int hour;           // hour (0-23)
	int minute;         // minute (0-59)
	int second;         // second (0-59)
};


//---------------------------------------------------------
//  arbitrary_clock - an std::chrono clock that "knows" the
//  date of the epoch's begining
//---------------------------------------------------------

template<typename Rep, int Y, int M, int D, int H, int N, int S, typename Ratio>
class arbitrary_clock
{
public:
	typedef Rep                                         rep;
	typedef Ratio                                       period;
	typedef std::chrono::duration<rep, period>          duration;
	typedef std::chrono::time_point<arbitrary_clock>    time_point;
	static constexpr int base_year = Y;
	static constexpr int base_month = M;
	static constexpr int base_day = D;
	static constexpr int base_hour = H;
	static constexpr int base_minute = N;
	static constexpr int base_second = S;

	//---------------------------------------------------------
	//  from_arbitrary_datetime - converts an
	//  from_arbitrary_datetime to this arbitrary_clock's scale
	//---------------------------------------------------------

	static time_point from_arbitrary_datetime(const arbitrary_datetime &dt, bool clamp)
	{
		return time_point(duration_from_arbitrary_datetime(dt, clamp));
	}


	//---------------------------------------------------------
	//  from_arbitrary_time_point - converts an arbitrary_clock
	//  with a different scale to this arbitrary_clock's scale
	//---------------------------------------------------------

	template<typename Rep2, int Y2, int M2, int D2, int H2, int N2, int S2, typename Ratio2>
	static time_point from_arbitrary_time_point(const std::chrono::time_point<arbitrary_clock<Rep2, Y2, M2, D2, H2, N2, S2, Ratio2> > &tp)
	{
		arbitrary_datetime dt;
		dt.year = Y2;
		dt.month = M2;
		dt.day_of_month = D2;
		dt.hour = H2;
		dt.minute = N2;
		dt.second = S2;

		const duration adjustment = duration_from_arbitrary_datetime(dt, false);
		const duration result_duration = std::chrono::duration_cast<duration>(tp.time_since_epoch() + adjustment);
		return time_point(result_duration);
	}


	//---------------------------------------------------------
	//  to_arbitrary_time_point - converts an arbitrary_clock
	//  of this scale to one of different scale
	//---------------------------------------------------------

	template<typename Rep2, int Y2, int M2, int D2, int H2, int N2, int S2, typename Ratio2>
	static std::chrono::time_point<arbitrary_clock<Rep2, Y2, M2, D2, H2, N2, S2, Ratio2> > to_arbitrary_time_point(const time_point &tp)
	{
		return arbitrary_clock<Rep2, Y2, M2, D2, H2, N2, S2, Ratio2>::from_arbitrary_time_point(tp);
	}


	//---------------------------------------------------------
	//  to_tm - formats a structure of type 'struct tm'
	//---------------------------------------------------------

	static struct tm to_tm(const time_point &tp)
	{
		std::chrono::time_point<tm_conversion_clock> normalized_tp = to_arbitrary_time_point<
			std::int64_t,
			tm_conversion_clock::base_year,
			tm_conversion_clock::base_month,
			tm_conversion_clock::base_day,
			tm_conversion_clock::base_hour,
			tm_conversion_clock::base_minute,
			tm_conversion_clock::base_second,
			tm_conversion_clock::period>(tp);
		return internal_to_tm(normalized_tp.time_since_epoch());
	}


	//---------------------------------------------------------
	//  to_system_clock - converts to a system_clock time_point
	//---------------------------------------------------------

	static std::chrono::time_point<std::chrono::system_clock> to_system_clock(const time_point &tp)
	{
		auto normalized_tp = to_arbitrary_time_point<
			std::int64_t,
			system_conversion_clock::base_year,
			system_conversion_clock::base_month,
			system_conversion_clock::base_day,
			system_conversion_clock::base_hour,
			system_conversion_clock::base_minute,
			system_conversion_clock::base_second,
			system_conversion_clock::period>(tp);
		return std::chrono::time_point<std::chrono::system_clock>(normalized_tp.time_since_epoch() + system_clock_adjustment);
	}

	//---------------------------------------------------------
	//  from_system_clock - converts from a system_clock time_point
	//---------------------------------------------------------

	static time_point from_system_clock(const std::chrono::time_point<std::chrono::system_clock> &tp)
	{
		std::chrono::time_point<system_conversion_clock> normalized_tp(tp.time_since_epoch() - system_clock_adjustment);
		return from_arbitrary_time_point(normalized_tp);
	}

private:
	// By positioning the base year at 1601, we can ensure that:
	//
	//   * years with leap years are at the end of every quadyear
	//   * quadyears without leap years are at the end of every century
	//   * centuries where the last quadyear has a leap year at the end are at the
	//     end of every quadcentury
	typedef arbitrary_clock<std::int64_t, 1601, 1, 1, 0, 0, 0, std::ratio<1, 1> > tm_conversion_clock;


	//---------------------------------------------------------
	//  clamp_or_throw
	//---------------------------------------------------------

	static int clamp_or_throw(int value, int minimum, int maximum, bool clamp, const char *out_of_range_message)
	{
		if (value < minimum || value > maximum)
		{
			if (clamp)
				value = std::min(std::max(value, minimum), maximum);
			else
				throw std::out_of_range(out_of_range_message);
		}
		return value;
	}

	//---------------------------------------------------------
	//  duration_from_arbitrary_datetime - converts an
	//  arbitrary_datetime to this arbitrary_clock's duration
	//---------------------------------------------------------

	static duration duration_from_arbitrary_datetime(const arbitrary_datetime &dt, bool clamp)
	{
		// range checking
		const int month         = clamp_or_throw(dt.month, 1, 12, clamp, "invalid dt.month");
		const int day_of_month  = clamp_or_throw(dt.day_of_month, 1, gregorian_days_in_month(month, dt.year), clamp, "invalid dt.day_of_month");
		const int hour          = clamp_or_throw(dt.hour, 0, 23, clamp, "invalid dt.hour");
		const int minute        = clamp_or_throw(dt.minute, 0, 59, clamp, "invalid dt.minute");
		const int second        = clamp_or_throw(dt.second, 0, 59, clamp, "invalid dt.second");

		const int64_t our_absolute_day = absolute_day(Y, M, D);
		const int64_t their_absolute_day = absolute_day(dt.year, month, day_of_month);

		const auto our_fract_day = std::chrono::hours(H) + std::chrono::minutes(N) + std::chrono::seconds(S);
		const auto their_fract_day = std::chrono::hours(hour) + std::chrono::minutes(minute) + std::chrono::seconds(second);

		return std::chrono::duration<Rep, Ratio>(std::chrono::hours(24) * (their_absolute_day - our_absolute_day) + (their_fract_day - our_fract_day));
	}

	//---------------------------------------------------------
	//  internal_to_tm - formats a structure of type 'struct tm'
	//  based on a normalized clock
	//---------------------------------------------------------

	static struct tm internal_to_tm(std::chrono::duration<std::int64_t, std::ratio<1, 1> > duration)
	{
		constexpr int days_in_year(365);
		constexpr int days_in_four_years((days_in_year * 4) + 1);
		constexpr int days_in_century((days_in_four_years * 25) - 1);
		constexpr int days_in_four_centuries((days_in_century * 4) + 1);

		constexpr tm_conversion_clock::duration day(std::chrono::hours(24));
		constexpr tm_conversion_clock::duration year(day * days_in_year);
		constexpr tm_conversion_clock::duration four_years(day * days_in_four_years);
		constexpr tm_conversion_clock::duration century(day * days_in_century);
		constexpr tm_conversion_clock::duration four_centuries(day * days_in_four_centuries);

		// figure out the day of week (note that 0 is Sunday, but January 1st 1601 is
		// a Monday, so we have to adjust by one day)
		const int day_of_week = int((duration + std::chrono::hours(24)) / day % 7);

		// figure out the year
		const int four_centuries_count = int(duration / four_centuries);
		duration -= four_centuries_count * four_centuries;
		const int century_count = std::min(int(duration / century), 3);
		duration -= century_count * century;
		const int four_years_count = std::min(int(duration / four_years), 25);
		duration -= four_years_count * four_years;
		const int year_count = std::min(int(duration / year), 3);
		duration -= year_count * year;
		const int actual_year = tm_conversion_clock::base_year + four_centuries_count * 400 + century_count * 100 + four_years_count * 4 + year_count;

		// figure out the day in the year
		const int day_in_year = int(duration / day);
		duration -= day_in_year * day;

		// figure out the month
		int month, day_in_month = day_in_year;
		for (month = 0; month < 12; month++)
		{
			int days_in_this_month = gregorian_days_in_month(month + 1, actual_year);
			if (day_in_month < days_in_this_month)
				break;
			day_in_month -= days_in_this_month;
		}
		if (month >= 12)
			throw false;

		// figure out the time
		const int hour = int(duration / std::chrono::hours(1));
		duration -= std::chrono::hours(hour);
		const int minute = int(duration / std::chrono::minutes(1));
		duration -= std::chrono::minutes(minute);
		const int second = int(duration / std::chrono::seconds(1));
		duration -= std::chrono::seconds(second);

		// populate the result and return
		struct tm result;
		memset(&result, 0, sizeof(result));
		result.tm_year = actual_year - 1900;
		result.tm_mon = month;
		result.tm_mday = day_in_month + 1;
		result.tm_yday = day_in_year;
		result.tm_sec = second;
		result.tm_min = minute;
		result.tm_hour = hour;
		result.tm_wday = day_of_week;
		return result;
	}


	typedef arbitrary_clock<std::int64_t, 1970, 1, 1, 0, 0, 0, std::chrono::system_clock::period > system_conversion_clock;

	//-------------------------------------------------
	//  absolute_day - returns the absolute day count
	//  for the specified year/month/day
	//-------------------------------------------------

	static int64_t absolute_day(int year, int month, int day)
	{
		// first factor the year
		int64_t result = (year - 1) * 365;
		result += (year - 1) / 4;
		result -= (year - 1) / 100;
		result += (year - 1) / 400;

		// then the month
		for (int i = 1; i < month; i++)
			result += gregorian_days_in_month(i, year);

		// then the day
		result += day - 1;
		return result;
	}
};


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

// -------------------------------------------------
// ntfs_duration_from_filetime
// -------------------------------------------------

inline constexpr ntfs_duration ntfs_duration_from_filetime(std::uint32_t high, std::uint32_t low)
{
	return ntfs_duration((std::uint64_t(high) << 32) | std::uint64_t(low));
}


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

std::chrono::system_clock::time_point system_clock_time_point_from_ntfs_duration(ntfs_duration d);


} // namespace util

#endif  // MAME_LIB_UTIL_TIMECONV_H
