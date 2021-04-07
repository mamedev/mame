// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    attotime.cpp

    Support functions for working with attotime data.

***************************************************************************/

#include "emucore.h"
#include "eminline.h"
#include "attotime.h"

namespace oldtime
{
//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const attotime oldtime::attotime::zero(0);
const attotime oldtime::attotime::never(MAX_SECONDS);

//**************************************************************************
//  CORE MATH FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  operator*= - multiply an attotime by a
//  constant
//-------------------------------------------------

template<typename T, std::enable_if_t<std::is_integral<T>::value, bool>>
attotime &attotime::operator*=(T factor)
{
	// if one of the items is attotime::never, return attotime::never
	if (m_seconds >= MAX_SECONDS)
		return *this = never;

	// 0 times anything is zero
	if (factor == 0)
		return *this = zero;

	// split subseconds into upper and lower halves which fit into 32 bits
	u32 attolo;
	u32 attohi = divu_64x32_rem(m_subseconds, subseconds::PER_SECOND_SQRT, attolo);

	// scale the lower half, then split into high/low parts
	u64 temp = mulu_32x32(attolo, factor);
	u32 reslo;
	temp = divu_64x32_rem(temp, subseconds::PER_SECOND_SQRT, reslo);

	// scale the upper half, then split into high/low parts
	temp += mulu_32x32(attohi, factor);
	u32 reshi;
	temp = divu_64x32_rem(temp, subseconds::PER_SECOND_SQRT, reshi);

	// scale the seconds
	temp += mulu_32x32(m_seconds, factor);
	if (temp >= MAX_SECONDS)
		return *this = never;

	// build the result
	m_seconds = temp;
	m_subseconds = s64(reslo) + mul_32x32(reshi, subseconds::PER_SECOND_SQRT);
	return *this;
}

template attotime &attotime::operator*=<s32>(s32);
template attotime &attotime::operator*=<u32>(u32);
template attotime &attotime::operator*=<s64>(s64);
template attotime &attotime::operator*=<u64>(u64);


//-------------------------------------------------
//  operator/= - divide an attotime by a constant
//-------------------------------------------------

template<typename T, std::enable_if_t<std::is_integral<T>::value && sizeof(T) < 8, bool>>
attotime &attotime::operator/=(T factor)
{
	// if one of the items is attotime::never, return attotime::never
	if (m_seconds >= MAX_SECONDS)
		return *this = never;

	// ignore divide by zero
	if (factor == 0)
		return *this;

	// special case the signed divide
	if (UNEXPECTED(m_seconds < 0))
		return *this = -(-*this / factor);
	if (std::is_signed<T>::value && UNEXPECTED(factor < 0))
		return *this = -(*this / -factor);

	// split subseconds into upper and lower halves which fit into 32 bits
	u32 attolo;
	u32 attohi = divu_64x32_rem(m_subseconds, subseconds::PER_SECOND_SQRT, attolo);

	// divide the seconds and get the remainder
	u32 remainder;
	m_seconds = divu_64x32_rem(m_seconds, factor, remainder);

	// combine the upper half of subseconds with the remainder and divide that
	u64 temp = s64(attohi) + mulu_32x32(remainder, subseconds::PER_SECOND_SQRT);
	u32 reshi = divu_64x32_rem(temp, factor, remainder);

	// combine the lower half of subseconds with the remainder and divide that
	temp = attolo + mulu_32x32(remainder, subseconds::PER_SECOND_SQRT);
	u32 reslo = divu_64x32_rem(temp, factor, remainder);

	// round based on the remainder
	m_subseconds = s64(reslo) + mulu_32x32(reshi, subseconds::PER_SECOND_SQRT);
	if (remainder >= factor / 2)
		if (++m_subseconds >= subseconds::PER_SECOND)
		{
			m_subseconds = 0;
			m_seconds++;
		}
	return *this;
}

template attotime &attotime::operator/=<s32>(s32);
template attotime &attotime::operator/=<u32>(u32);


//-------------------------------------------------
//  as_ticks - compute the number of ticks of the
//  given period
//-------------------------------------------------

u64 attotime::as_ticks(subseconds period) const
{
	if (is_never() || period.raw() == 0)
		return 0xffffffffffffffff;

	// if seconds are 0, it's easy
	u64 period_raw = period.raw();
	if (m_seconds == 0)
		return m_subseconds / period_raw;

	// compute how many whole ticks per second
	u64 whole_ticks_per_second = subseconds::PER_SECOND / period_raw;

	// compute the remaining subseconds we didn't account for
	u64 leftover_subs_per_second = subseconds::PER_SECOND - whole_ticks_per_second * period_raw;

	// start with the number of whole ticks per second
	u64 result = m_seconds * s64(whole_ticks_per_second);

	// compute the remainder: this should be a 64x64->128/64 divide
	s64 remainder = m_seconds * s64(leftover_subs_per_second);

	// add in the number of whole ticks this represents
	s64 extra = remainder / s64(period_raw);
	result += extra;
	remainder -= extra * period_raw;

	// add in the attos and compute the rest
	remainder += m_subseconds;
	result += remainder / period_raw;
	return result;
}


//-------------------------------------------------
//  as_string - return a temporary printable
//  string describing an attotime
//-------------------------------------------------

const char *attotime::as_string(int precision) const
{
	static char buffers[8][30];
	static int nextbuf;
	char *buffer = &buffers[nextbuf++ % 8][0];

	// special case: never
	if (*this == never)
		sprintf(buffer, "%-*s", precision, "(never)");

	// case 1: we want no precision; seconds only
	else if (precision == 0)
		sprintf(buffer, "%d", m_seconds);

	// case 2: we want 9 or fewer digits of precision
	else if (precision <= 9)
	{
		u32 upper = m_subseconds / subseconds::PER_SECOND_SQRT;
		int temp = precision;
		while (temp < 9)
		{
			upper /= 10;
			temp++;
		}
		sprintf(buffer, "%d.%0*d", m_seconds, precision, upper);
	}

	// case 3: more than 9 digits of precision
	else
	{
		u32 lower;
		u32 upper = divu_64x32_rem(m_subseconds, subseconds::PER_SECOND_SQRT, lower);
		int temp = precision;
		while (temp < 18)
		{
			lower /= 10;
			temp++;
		}
		sprintf(buffer, "%d.%09d%0*d", m_seconds, upper, precision - 9, lower);
	}
	return buffer;
}

//-------------------------------------------------
//  to_string - return a human-readable string
//  describing an attotime for use in logs
//-------------------------------------------------

std::string attotime::to_string() const
{
	attotime t = *this;
	const char *sign = "";
	if(t.seconds() < 0) {
		t = attotime::zero-t;
		sign = "-";
	}
	int nsec = t.frac().as_nsec_int();
	return util::string_format("%s%04d.%03d,%03d,%03d", sign, int(t.seconds()), nsec/1000000, (nsec/1000)%1000, nsec % 1000);
}
}
