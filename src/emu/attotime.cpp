// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    attotime.cpp

    Support functions for working with attotime data.

***************************************************************************/

#include "emucore.h"
#include "eminline.h"
#include "attotime.h"

const attotime attotime::zero(0);
const attotime attotime::never(0x7fffffffffffffffll, 0xfffffffd);

//-------------------------------------------------
//  generate_string - flexibly generate a string
//  of arbitrary precision
//-------------------------------------------------

char const *attotime::generate_string(char *buffer, int precision, bool dividers) const noexcept
{
	// special case: never
	if (is_never())
		sprintf(buffer, "%-*s", precision, "(never)");

	// start with seconds
	char *dest = buffer + sprintf(buffer, "%d", seconds());
	if (precision > 0)
	{
		*dest++ = '.';

		// handle the fraction
		static s64 const s_divisors[] =
		{
			(subseconds::PER_SECOND - 1) / 10ll + 1,
			(subseconds::PER_SECOND - 1) / 100ll + 1,
			(subseconds::PER_SECOND - 1) / 1000ll + 1,
			(subseconds::PER_SECOND - 1) / 10000ll + 1,
			(subseconds::PER_SECOND - 1) / 100000ll + 1,
			(subseconds::PER_SECOND - 1) / 1000000ll + 1,
			(subseconds::PER_SECOND - 1) / 10000000ll + 1,
			(subseconds::PER_SECOND - 1) / 100000000ll + 1,
			(subseconds::PER_SECOND - 1) / 1000000000ll + 1,
			(subseconds::PER_SECOND - 1) / 10000000000ll + 1,
			(subseconds::PER_SECOND - 1) / 100000000000ll + 1,
			(subseconds::PER_SECOND - 1) / 1000000000000ll + 1,
			(subseconds::PER_SECOND - 1) / 10000000000000ll + 1,
			(subseconds::PER_SECOND - 1) / 100000000000000ll + 1,
			(subseconds::PER_SECOND - 1) / 1000000000000000ll + 1,
			(subseconds::PER_SECOND - 1) / 10000000000000000ll + 1,
			(subseconds::PER_SECOND - 1) / 100000000000000000ll + 1,
			(subseconds::PER_SECOND - 1) / 1000000000000000000ll + 1
		};
		s64 rem = frac().raw();
		precision = std::min<int>(precision, std::size(s_divisors));
		for (int index = 0; index < precision; index++)
		{
			if (dividers && index != 0 && index % 3 == 0)
				*dest++ = '\'';
			s64 digit = rem / s_divisors[index];
			rem -= digit * s_divisors[index];
			*dest++ = '0' + digit;
		}
		*dest = 0;
	}
	return buffer;
}


//-------------------------------------------------
//  as_string - return a temporary printable
//  string describing an attotime
//-------------------------------------------------

const char *attotime::as_string(int precision, bool dividers) const noexcept
{
	static char s_buffers[8][MAX_STRING_LEN];
	static int s_nextbuf;
	return generate_string(&s_buffers[s_nextbuf++ % 8][0], precision, dividers);
}


//-------------------------------------------------
//  to_string - return a human-readable string
//  describing an attotime for use in logs
//-------------------------------------------------

std::string attotime::to_string(int precision, bool dividers) const
{
	char buffer[MAX_STRING_LEN];
	return std::string(generate_string(&buffer[0], precision, dividers));
}
