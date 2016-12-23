// license:GPL-2.0+
// copyright-holders:Couriersud

#include <chrono>

#include "pchrono.h"

namespace plib {
namespace chrono {
#if defined(__x86_64__) &&  !defined(_clang__) && !defined(_MSC_VER) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6))

fast_ticks::type fast_ticks::per_second()
{
	static type persec = 0;
	if (persec == 0)
	{
		type x = 0;
		system_ticks::type t = system_ticks::start();
		system_ticks::type e;
		x = -start();
		do {
			e = system_ticks::stop();
		} while (e - t < system_ticks::per_second() / 100 );
		x += stop();
		persec = (type)(double)((double) x * (double) system_ticks::per_second() / double (e - t));
	}
	return persec;
}

#if PUSE_ACCURATE_STATS && PHAS_RDTSCP
exact_ticks::type exact_ticks::per_second()
{
	static type persec = 0;
	if (persec == 0)
	{
		type x = 0;
		system_ticks::type t = system_ticks::start();
		system_ticks::type e;
		x = -start();
		do {
			e = system_ticks::stop();
		} while (e - t < system_ticks::per_second() / 100 );
		x += stop();
		persec = (type)(double)((double) x * (double) system_ticks::per_second() / double (e - t));
	}
	return persec;
}
#endif

#endif

}
}
