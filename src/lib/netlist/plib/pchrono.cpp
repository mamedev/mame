// license:GPL-2.0+
// copyright-holders:Couriersud

#if 0

#include "pchrono.h"

namespace plib {
namespace chrono {
#if defined(__x86_64__) &&  !defined(_clang__) && !defined(_MSC_VER) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6))

template <typename T>
auto per_sec() -> typename T :: type
{
	using ret_type = typename T :: type;
	static ret_type persec = 0;
	if (persec == 0)
	{
		ret_type x = 0;
		system_ticks::type t = system_ticks::start();
		system_ticks::type e;
		x = - T :: start();
		do {
			e = system_ticks::stop();
		} while (e - t < system_ticks::per_second() / 100 );
		x += T :: stop();
		persec = (ret_type)(double)((double) x * (double) system_ticks::per_second() / double (e - t));
	}
	return persec;
}


fast_ticks::type fast_ticks::per_second()
{
#if 1
	return per_sec<fast_ticks>();
#else
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
#endif
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

} // namespace chrono
} // namespace plib
#endif
