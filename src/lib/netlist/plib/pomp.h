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

#if HAS_OPENMP
#include "omp.h"
#endif

namespace plib {
namespace omp {

template <class T>
void for_static(const int start, const int end, const T &what)
{
#if HAS_OPENMP && USE_OPENMP
	#pragma omp parallel
#endif
	{
#if HAS_OPENMP && USE_OPENMP
		#pragma omp for schedule(static)
#endif
		for (int i = start; i <  end; i++)
			what(i);
	}
}

inline void set_num_threads(const int threads)
{
#if HAS_OPENMP && USE_OPENMP
	omp_set_num_threads(threads);
#endif
}

inline int get_max_threads()
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

}
}

#endif /* PSTRING_H_ */
