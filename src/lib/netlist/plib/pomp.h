// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pomp.h
 *
 * Wrap all OPENMP stuff here in a hopefully c++ compliant way.
 */

#ifndef POMP_H_
#define POMP_H_

#include "pconfig.h"

//#include <cstddef>

#if HAS_OPENMP
#include "omp.h"
#endif

namespace plib {
namespace omp {

template <typename I, class T>
void for_static(const I start, const I end, const T &what)
{
#if HAS_OPENMP && USE_OPENMP
	#pragma omp parallel
#endif
	{
#if HAS_OPENMP && USE_OPENMP
		#pragma omp for //schedule(static)
#endif
		for (I i = start; i <  end; i++)
			what(i);
	}
}

template <typename I, class T>
void for_static_np(const I start, const I end, const T &what)
{
	for (I i = start; i <  end; i++)
		what(i);
}


inline void set_num_threads(const std::size_t threads)
{
#if HAS_OPENMP && USE_OPENMP
	omp_set_num_threads(threads);
#else
	plib::unused_var(threads);
#endif
}

inline std::size_t get_max_threads()
{
#if HAS_OPENMP && USE_OPENMP
	return omp_get_max_threads();
#else
	return 1;
#endif
}


// ----------------------------------------------------------------------------------------
// pdynlib: dynamic loading of libraries  ...
// ----------------------------------------------------------------------------------------

} // namespace omp
} // namespace plib

#endif /* PSTRING_H_ */
